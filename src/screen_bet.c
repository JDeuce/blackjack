#include "screen_bet.h"
#include "screen_hand.h"
#include "screen.h"

#include "game.h"
#include "lcd.h"
#include "avr/io.h"

#include "font.h"
#include "clock.h"
#include "touch.h"
#include "rand.h"
#include "button.h"
#include "cards.h"
#include "hud.h"



#define BACK_COLOR	FELT_GREEN
#define TEXT_COLOR	WHITE

#define BUTTON_OUTLINE	BLACK
#define BUTTON_BACK	LIGHT_BLUE
#define BUTTON_HOVER	YELLOW
#define BUTTON_TEXT	BLACK

#define DEAL_BACK	GREEN


#define BET_REPEAT_INTERVAL	100


struct SCREEN_BET_STATE {
	struct Button *cmd_button_min;
	struct Button *cmd_button_down;
	struct Button *cmd_button_up;
	struct Button *cmd_button_max;
	struct Button *cmd_button_deal;
	unsigned long next_highlight;
	int deal;
};

static struct SCREEN_BET_STATE g_state;

void bet_min()
{
	game_state.current_bet = MIN_BET;
	hud_highlight_bet();
}

void bet_max()
{
	if (game_state.balance >= MAX_BET)
		game_state.current_bet = MAX_BET;
	else
		game_state.current_bet = game_state.balance / MIN_BET * MIN_BET;

	hud_highlight_bet();
}

void bet_up()
{
	if (game_state.current_bet < MAX_BET && 
	    game_state.current_bet + MIN_BET <= game_state.balance)
	{
		game_state.current_bet += MIN_BET;
	}

	hud_highlight_bet();
}

void bet_down()
{
	if (game_state.current_bet > MIN_BET)
	{
		game_state.current_bet -= MIN_BET;
	}

	hud_highlight_bet();
}

void bet_deal()
{
	g_state.deal = 1;
}

// make sure the bet is sane
void bet_normalize()
{
	// betting more than the balance
	if (game_state.current_bet > game_state.balance)
	{
		bet_max();
	} else if (game_state.current_bet < MIN_BET) {
		bet_min();
	}
}

void bet_activate()
{
	bet_normalize();
	hud_unfreeze_bet();
	game_blank_table(BACK_COLOR);

	buttons_init();
	g_state.cmd_button_min = init_button("Min", 
					    190, 240,
					    80, 50,
					    2,
					    BUTTON_OUTLINE, BUTTON_TEXT,
					    BUTTON_BACK,   BUTTON_HOVER,
					    bet_min);

	g_state.cmd_button_down = init_button("Down", 
					    190, 160,
					    80, 50,
					    2,
					    BUTTON_OUTLINE, BUTTON_TEXT,
					    BUTTON_BACK,   BUTTON_HOVER,
					    bet_down);
	g_state.cmd_button_down->repeats = BET_REPEAT_INTERVAL;

	g_state.cmd_button_up = init_button("Up", 
					    190, 80,
					    80, 50,
					    2,
					    BUTTON_OUTLINE, BUTTON_TEXT,
					    BUTTON_BACK,   BUTTON_HOVER,
					    bet_up);

	g_state.cmd_button_up->repeats = BET_REPEAT_INTERVAL;
	g_state.cmd_button_max = init_button("Max", 
					    190, 0,
					    80, 50,
					    2,
					    BUTTON_OUTLINE, BUTTON_TEXT,
					    BUTTON_BACK,   BUTTON_HOVER,
					    bet_max);

	g_state.cmd_button_deal = init_button("Deal",
						120, 85,
						150, 50,
						2,
						BUTTON_OUTLINE, BUTTON_TEXT,
					        DEAL_BACK,   BUTTON_HOVER,
						bet_deal);
	g_state.deal = 0;

	font_draw_string(25, 209,
			 BLACK, BACK_COLOR,
			 "Black");
	font_draw_string(25, 209 - 5 * CHAR_WIDTH,
			 RED, BACK_COLOR,
			 "Jack");

	card_t ace_spades = make_card(SPADES, CARD_ACE);
	card_t jack_spades = make_card(SPADES, CARD_JACK);
	draw_card(ace_spades, 30, 270);
	draw_card(jack_spades, 50, 250);


	card_t ace_hearts = make_card(HEARTS, CARD_ACE);
	card_t jack_hearts = make_card(HEARTS, CARD_JACK);
	draw_card(ace_hearts, 30, 10);
	draw_card(jack_hearts, 50, 30);


	font_draw_string(45, 225, TEXT_COLOR, BACK_COLOR, "Place A Bet");

	font_draw_string(65, 220, TEXT_COLOR, BACK_COLOR, "Min: $");
	font_draw_number(65, 220 - 6 * CHAR_WIDTH, TEXT_COLOR, BACK_COLOR, MIN_BET); 

	font_draw_string(90, 220, TEXT_COLOR, BACK_COLOR, "Max: $");
	font_draw_number(90, 220 - 6 * CHAR_WIDTH, TEXT_COLOR, BACK_COLOR, MAX_BET); 


}


void bet_deactivate()
{
	buttons_stop();
	game_state.balance -= game_state.current_bet;
	hud_highlight_balance();

}

void bet_draw()
{

	buttons_draw();
	
}


void bet_update()
{
	if (g_state.deal) {
		set_screen(&screen_hand);
	} else {
		buttons_update();
	}
}

struct Screen screen_bet = {
	bet_activate,
	bet_deactivate,
	bet_draw,
	bet_update,
	SCREEN_SHOW_HUD
};
