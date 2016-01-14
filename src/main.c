#include <avr/io.h>
#include <avr/interrupt.h>

#include "clock.h"
#include "leds.h"
#include "switches.h"
#include "lcd.h"
#include "touch.h"
#include "font.h"
#include "game.h"
#include "eeprom.h"

#define BACKLIGHT_LOC 0
static const unsigned char levels[] = {100, 80, 60, 40, 20};
static const int num_levels = 5;
static unsigned char backlight_level;

void enable_interrupts()
{
	// enable all 3 levels of interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	// globally enable interrupts
	sei();
}

void init_subsystems()
{
	clock_init();
	enable_interrupts();

	leds_init();
	switches_init();

	lcd_init();

	touch_init();
}

// restore backlight from EEPROM
void restore_backlight_level()
{
	backlight_level = eeprom_read(BACKLIGHT_LOC);

	// clamp value from eeprom into the range we want
	backlight_level %= num_levels;

	lcd_set_backlight(levels[backlight_level]);
}

// store backlight in eeprom
void store_backlight_level()
{
	eeprom_write(BACKLIGHT_LOC, backlight_level);
}

// adjust & store new backlight level into eeprom
void adjust_backlight_level()
{
	backlight_level = (backlight_level + 1) % num_levels;
	
	lcd_set_backlight(levels[backlight_level]);
	store_backlight_level();
}

int main(int argc, char **argv) 
{
	init_subsystems();

	restore_backlight_level();
	switch_callback_down(7, adjust_backlight_level);
	clock_register_task(switch_task);

	game_init();
	for (;;)
	{
		game_task();
		touch_task();
		switch_task();
	}

	/* NOT REACHED */
	return 0;
}
