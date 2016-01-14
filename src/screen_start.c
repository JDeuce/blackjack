#include "screen.h"
#include "screen_start.h"
#include "screen_bet.h"

#include "game.h"
#include "lcd.h"
#include "font.h"
#include "clock.h"
#include "touch.h"
#include "rand.h"
#include "avr/io.h"


struct SCREEN_START_STATE {
	int do_draw;
	int current_pos;
	int clicked;
	unsigned long next_change;
};

static struct SCREEN_START_STATE g_state;

#define NUM_POS 	16
#define CHANGE_MS	3000
#define BACK_COLOR	FELT_GREEN
#define FORE_COLOR	WHITE
static unsigned short xPos[] = {
	0, 105, 210,
	0, 105, 210,
	0, 105, 210,
	0, 105, 210,
	0, 105, 210,
	0, 105, 210
};
static unsigned short yPos[] = {
	214, 260, 306,
	214, 306, 260,
	260, 214, 306,
	260, 306, 214,
	306, 260, 214,
	306, 214, 260
};

void start_activate()
{
	g_state.do_draw = 1;
	g_state.clicked = 0;
	g_state.next_change = clock_current_ms() + CHANGE_MS;

}


void start_deactivate()
{
}

void start_draw()
{
	if (g_state.do_draw)
	{
		g_state.do_draw = 0;
		lcd_blank_screen(BACK_COLOR);
		font_draw_string(xPos[g_state.current_pos], 
				 yPos[g_state.current_pos], 
				 FORE_COLOR, BACK_COLOR, "Tap To Start");
	}
}

void change_text_pos()
{
	g_state.do_draw = 1;

	g_state.current_pos =(g_state.current_pos + 1) % NUM_POS;
}

void start_update()
{
	struct TouchState touch;
	touch_get_state(&touch);
	if (g_state.clicked && touch.state == TOUCH_RELEASED )
	{
		// seed randomizer with value thats in the timer when we get a click
		srand(TCC0.CNT);
		set_screen(&screen_bet);
	} 
	else if (clock_current_ms() > g_state.next_change)
	{
		change_text_pos();
		g_state.next_change += CHANGE_MS;
	}

	if (touch.state == TOUCH_PRESSED)
		g_state.clicked = 1;

}

struct Screen screen_start = {
	start_activate,
	start_deactivate,
	start_draw,
	start_update,
	SCREEN_HIDE_HUD
};
