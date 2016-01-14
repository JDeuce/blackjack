#include "screen.h"
#include "screen_bet.h"

#include "game.h"
#include "lcd.h"
#include "font.h"
#include "button.h"
#include "screen_start.h"


void balance_reset()
{
	set_screen(&screen_start);
}

void broke_activate()
{
	lcd_blank_screen(RED);
	buttons_init();
	

	font_draw_string(20, 300, WHITE, RED, "GAME OVER");
	font_draw_string(50, 300, WHITE, RED, "You lost all your $$$");
	init_button("Try Again",
			120, 40,
			240, 50,
			2,
			WHITE, WHITE,
			BLACK, BLUE,
			balance_reset);

}


void broke_deactivate()
{
	buttons_stop();
}

void broke_draw()
{
	buttons_draw();
}

void broke_update()
{
	buttons_update();
}

struct Screen screen_broke = {
	broke_activate,
	buttons_stop,
	buttons_draw,
	buttons_update,
	SCREEN_HIDE_HUD
};
