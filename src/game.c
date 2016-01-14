#include "events.h"
#include "game.h"
#include "eeprom.h"
#include "screen.h"
#include "screen_start.h"

#include "clock.h"
#include "leds.h"
#include "lcd.h"
#include "switches.h"
#include "rand.h"
#include "hud.h"

struct GAME_STATE game_state;
static struct Screen *g_screen = 0;

static unsigned char g_nvm_slot = 0;
static unsigned char g_nvm_offset = 0;

// load game state from NVM
void nvm_load_game(unsigned char start, struct GAME_STATE *game_state)
{
	unsigned char i;
	unsigned char byte;

	for (i = 0; i < SLOT_SIZE; i++)
	{
		byte = eeprom_read(start + i);
		((unsigned char *)game_state)[i] = byte;
	}

}

// write game state to NVM
void nvm_save_game(unsigned char start, struct GAME_STATE *game_state)
{

	unsigned char i;
	unsigned char byte;
	for (i = 0; i < SLOT_SIZE; i++) {
		byte = ((unsigned char *)game_state)[i];
		eeprom_write(start + i, byte);
	}
}


// returns the start address of a given slot & offset 
unsigned char nvm_slot_addr(int slot, int offset)
{
	return STATE_SAVE_START + slot * SLOT_SIZE + offset;
}

// initialize NVM, called when can't make sense of anything in NVM
void nvm_initialize()
{
	g_nvm_slot = 0;
	g_nvm_offset = 0;	


	int i;
	//flash leds for 2 seconds as debug output
	for (i = 0; i < 4; i++) {
		led_toggle(0xAA);
		clock_delay_ms(500);
	}
	
	game_reset();
	nvm_save_game(nvm_slot_addr(g_nvm_slot, g_nvm_offset), &game_state);
	eeprom_write(EEPROM_SLOT_OFFSET, g_nvm_offset);
}

// restore wear level state and game state
// the game writes state out to eeprom sequentially
// basically i can tell if a slot is full by looking for the a combination of 4 bytes (magic) included with the state when saved
// when the micro comes up, i search eeprom sequentially for the last valid slot and restore from that.
// however, this isn't enough because once all the slots are full, the last slot will always be valid.
// to fix this, i store another byte in EEPROM which offsets the start address of the scan that i increment when looping back to the first slot
void nvm_restore_state()
{
	int i;
	int last = -1;
	int invalid = 0;
	struct GAME_STATE state;

	// load the slot offset
	g_nvm_offset  = eeprom_read(EEPROM_SLOT_OFFSET);
	if (g_nvm_offset >= SLOT_SIZE) {
		// E2 is in invalid state because this offset is invalid
		nvm_initialize();
	} else {
		// offset looks good, lets see if we can find a slot

		// iterate eeprom to find the last written slot
		// note we might see a corrupt slot at the beginning if the micro powers down after writing the state with a new offset
		// but before writing the new offset
		// this is okay though, just find the state successfully written state & load that
		for (i = 0; i < STATE_SLOTS; i++) {
			nvm_load_game(nvm_slot_addr(i, g_nvm_offset), &state);
			if (state.magic == STATE_MAGIC)
				last = i;
			else
				invalid++;
		}

		// didn't find any valid slots, eeprom is corrupt
		if (last == -1) {
			nvm_initialize();
		} else {
			g_nvm_slot = last;
		}
	}

}


// change the active screen
void set_screen(struct Screen *screen)
{
	struct Screen *last_screen = g_screen;

	if (g_screen)
		g_screen->deactivate();

	g_screen = screen;

	if (g_screen) {
		// redraw the hud if it wasn't shown before
		if ((last_screen == 0 || last_screen->show_hud == 0) &&
		    g_screen->show_hud) {
			hud_redraw();
		}
 
		g_screen->activate();
	}

}

// reset game state to initial values
// can be called at any time
// pushes the state to eeprom
void game_reset()
{
	game_state.magic = STATE_MAGIC;
	game_state.balance = INITIAL_BALANCE;
	game_state.current_bet = MIN_BET;
}

void wear_level_test()
{
	for (;;) { 
		font_draw_string(0, 300, WHITE, BLACK, "Wear Level Test");
		font_draw_string(30, 300, WHITE, BLACK, "NVM Slot:");
		font_draw_number(30, 100, WHITE, BLACK, g_nvm_slot);
		font_draw_string(60, 300, WHITE, BLACK, "NVM Offset:");
		font_draw_number(60, 100, WHITE, BLACK, g_nvm_offset);

		font_draw_string(120, 300, WHITE, BLACK, "Balance");
		font_draw_number(120, 100, WHITE, BLACK, game_state.balance);
		font_draw_string(150, 300, WHITE, BLACK, "Bet");
		font_draw_number(150, 100, WHITE, BLACK, game_state.current_bet);
		unsigned char slot, offset;

		// basically just randomize the game state
		game_state.balance = rand();
		game_state.current_bet = rand();

		// then write it out 
		game_save_state();

		// save off the slot&offset
		slot = g_nvm_slot;
		offset = g_nvm_offset;

		// do a restore
		nvm_restore_state();

		// then verify the slot & offset match
		font_draw_string(90, 300, WHITE, BLACK, "TEST:");
		if (g_nvm_slot == slot && g_nvm_offset == offset)
			font_draw_string(90, 100, GREEN, BLACK, "Good");
		else {
			font_draw_string(90, 100, RED, BLACK, " Bad");
			for (;;);
		}
		clock_delay_ms(10);
	}
}

//  initializes the game.
// reads in state from eeprom,
// and initializes the start screen
void game_init()
{
	nvm_restore_state();

	nvm_load_game(nvm_slot_addr(g_nvm_slot, g_nvm_offset), &game_state);

	clock_register_task(hud_task);

	g_screen = &screen_start;
	g_screen->activate();
}

// this is the game loop
void game_task()
{
	if (g_screen)
	{
		if (g_screen->show_hud) {
			hud_update();
			hud_draw();
		}
		
		g_screen->draw();
		g_screen->update();
	}

}

// blank the non-hud portion of the screen
void game_blank_table(unsigned short color)
{
	lcd_draw_rect(HUD_HEIGHT, 0, LCD_COLUMNS - HUD_HEIGHT, LCD_ROWS, color);
}

// write game_state out into eeprom using wear leveling
void game_save_state() {

	unsigned char slot = g_nvm_slot + 1;
	unsigned char offset = g_nvm_offset;
	
	if (slot == STATE_SLOTS) {
		// start again at beginning of eeprom, with an offset
		offset = (offset + 1) % SLOT_SIZE;
		slot = 0;
	} 

	nvm_save_game(nvm_slot_addr(slot, offset), &game_state);

	// don't update the offset until we've successfully written the state.
	if (offset != g_nvm_offset) {
		eeprom_write(EEPROM_SLOT_OFFSET, offset);
		g_nvm_offset = offset;
	}
	g_nvm_slot = slot;
}
