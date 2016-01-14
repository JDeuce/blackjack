#include "lcd.h"
#include "font.h"
#include "clock.h"
#include "game.h"
#include "hud.h"

#define NUM_HIGHLIGHTS 9

static unsigned long bet_next_highlight = 0;
static unsigned char bet_highlights = 0;
static unsigned short bet_fore = HUD_FORE;
static unsigned short bet_back = HUD_BACK;

static unsigned long balance_next_highlight = 0;
static unsigned char balance_highlights = 0;
static unsigned short balance_fore = HUD_FORE;
static unsigned short balance_back = HUD_BACK;

static unsigned short last_bet_fore = 0;
static unsigned short last_bet_back = 0;
static unsigned short last_balance_fore = 0;
static unsigned short last_balance_back = 0;
static unsigned short last_balance = 0;
static unsigned short last_bet = 0;
static unsigned short freeze_balance = 0;
static unsigned short freeze_bet = 0;

// use these gradients to draw attention to the bet area
// on the screen so the player can see when its been updated
// Red to White gradient on the foreground
static unsigned short highlight_fore[] = {
	COLOR(31, 0, 0),
	COLOR(31, 8, 4),
	COLOR(31, 16, 8),
	COLOR(31, 24, 12),
	COLOR(31, 32, 16),
	COLOR(31, 40, 20),
	COLOR(31, 48, 24),
	COLOR(31, 56, 28),
	COLOR(31, 63, 31),
};

// White to blue gradient on the background
static unsigned short highlight_back[] = {
	COLOR(31, 63, 31),
	COLOR(28, 56, 31),
	COLOR(24, 48, 31),
	COLOR(20, 40, 31),
	COLOR(16, 32, 31),
	COLOR(12, 24, 31),
	COLOR(0, 0, 31),
	COLOR(0, 0, 31),
	COLOR(0, 0, 31)
};

void hud_redraw() 
{
	lcd_draw_rect(0, 0, HUD_HEIGHT, 320, HUD_BACK);
	lcd_draw_rect(HUD_HEIGHT - 2, 0, 2, 320, HUD_OUTLINE);
	lcd_draw_rect(0, 175, HUD_HEIGHT, 2, HUD_OUTLINE);

	font_draw_string(0, 305, HUD_FORE, HUD_BACK, "Bet:$");
	font_draw_number(0, 235, HUD_FORE, HUD_BACK, game_state.current_bet);

	font_draw_string(0, 154, HUD_FORE, HUD_BACK, "Chips:$");
	font_draw_number(0, 56,  HUD_FORE, HUD_BACK, game_state.balance);

	freeze_bet = 0;
	freeze_balance = 0;
}


void hud_update()
{
	unsigned long clock = clock_current_ms();

	unsigned char hl_state;

	// update bet highlight state
	if (bet_highlights && clock > bet_next_highlight) {
		hl_state = NUM_HIGHLIGHTS - bet_highlights;

		bet_fore = highlight_fore[hl_state];
		bet_back = highlight_back[hl_state];

		bet_highlights--;
		bet_next_highlight = clock + MS_PER_HIGHLIGHT;
	} else if (!bet_highlights) {
		bet_fore = HUD_FORE;
		bet_back = HUD_BACK;
	}


	// update balance highlight state
	if (balance_highlights && clock > balance_next_highlight) {
		hl_state = NUM_HIGHLIGHTS - balance_highlights;

		balance_fore = highlight_fore[hl_state];
		balance_back = highlight_back[hl_state];

		balance_highlights--;
		balance_next_highlight = clock + MS_PER_HIGHLIGHT;
	} else if (!balance_highlights) {
		balance_fore = HUD_FORE;
		balance_back = HUD_BACK;
	}


}


void hud_draw_bet(unsigned short bet)
{
	font_draw_number(0, 235, bet_fore, bet_back, bet);

	last_bet_fore = bet_fore;
	last_bet_back = bet_back;
	last_bet = bet;
}


void hud_draw_balance(unsigned short balance)
{
	font_draw_number(0, 56, balance_fore, balance_back, balance);

	last_balance_fore = balance_fore;
	last_balance_back = balance_back;
	last_balance = balance;
}

void hud_draw()
{

	// declare state changes
	if (bet_fore != last_bet_fore ||
	    bet_back != last_bet_back || 
	    game_state.current_bet != last_bet) {
		if (!freeze_bet)
			hud_draw_bet(game_state.current_bet);	
	}


	if (balance_fore != last_balance_fore ||
	    balance_back != last_balance_back || 
	    game_state.balance != last_balance) {
		if (!freeze_balance)
			hud_draw_balance(game_state.balance);	
	}

}

// freeze the bet, preventing updates from displaying
// used after the animation when dropping bet to 0
// to prevent bet immediately re-displaying
void hud_freeze_bet()
{
	freeze_bet = 1;
}

void hud_unfreeze_bet()
{
	freeze_bet = 0;
}


// returns true if hud is currently highlighting something
int hud_highlighting() 
{
	return bet_highlights + balance_highlights;
}

void hud_highlight_bet()
{
	bet_highlights = NUM_HIGHLIGHTS;
}

void hud_highlight_balance()
{
	balance_highlights = NUM_HIGHLIGHTS;
}

// increments the balance by factor * bet
// blocks while showing an animation of the chips increasing
void hud_animate_adjustment(unsigned char factor)
{
	unsigned short disp_bet = game_state.current_bet;
	unsigned short disp_bal = game_state.balance;

	while (disp_bet > 0)
	{

		disp_bal += factor;
		disp_bet--; 

		hud_draw_bet(disp_bet);
		hud_draw_balance(disp_bal);

	}
	
	// highlight the balance if the player has one
	if (factor > 0)
		hud_highlight_balance();

	hud_freeze_bet();
}


void hud_task()
{
	hud_update();
	hud_draw();
}

