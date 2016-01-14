#include "screen.h"
#include "screen_bet.h"
#include "screen_broke.h"

#include "game.h"
#include "cards.h"
#include "lcd.h"
#include "button.h"
#include "clock.h"
#include "font.h"
#include "hud.h"

#define TABLE_BACK 	FELT_GREEN	
#define TABLE_TEXT 	WHITE
#define TABLE_OUTLINE	BLACK
#define BUST_TEXT 	RED
#define BUST_BACK 	BLACK
#define BUTTON_BACK	LIGHT_BLUE	
#define BUTTON_HOVER	YELLOW	
#define BUTTON_TEXT	BLACK

enum HAND_STATE
{
	WAIT_PLAYER,
	REMOVE_BUTTONS,
	DO_DEALER,
	SCORE
};

struct SCREEN_HAND_STATE {
	enum HAND_STATE state;

	deck_t player_hand;
	hand_value_t player_value;
	deck_t dealer_hand;
	hand_value_t dealer_value;
	deck_t shoe;
	struct Button *cmd_button_hit;
	struct Button *cmd_button_stand;
	struct Button *cmd_button_double;
	int finished;
	int doubled;
	int player_dirty;
	int dealer_dirty;

};

static struct SCREEN_HAND_STATE g_state;

void player_hit()
{
	if (g_state.player_value.total < 21 || g_state.player_value.soft)
	{
		deck_deal(&g_state.shoe, &g_state.player_hand); 
		deck_value(&g_state.player_hand, &g_state.player_value);
		g_state.player_dirty = 1;
	}
}

void dealer_hit()
{
	if (g_state.dealer_value.total < 21 || g_state.dealer_value.soft)
	{
		deck_deal(&g_state.shoe, &g_state.dealer_hand); 
		deck_value(&g_state.dealer_hand, &g_state.dealer_value);
		g_state.dealer_dirty = 1;
	}
}

void player_stand()
{
	g_state.state = REMOVE_BUTTONS;
}

void player_double()
{
	if (g_state.player_value.total < 21 || g_state.player_value.soft) {
		if (game_state.balance >= game_state.current_bet) {
			game_state.balance -= game_state.current_bet;
			game_state.current_bet <<= 1;
			player_hit();
			player_stand();
			g_state.doubled = 1;
			hud_highlight_bet();
		} else {
			// change the double button to say "NO $" if the user has no money to double
			g_state.cmd_button_double->idlecolor = RED;
			button_set_title(g_state.cmd_button_double, "NO $");
			g_state.cmd_button_double->dirty = 1;
		}
		
		hud_highlight_balance();
	}
}

void hand_finish()
{
	g_state.finished = 1;
}

int blackjack(deck_t *deck, hand_value_t *value)
{
	return deck->n_cards == 2 && value->total == 21;
}

// increments the balance by factor * bet
// shows an animation of the chips increasing
void adjust_balance(unsigned char factor)
{

	hud_animate_adjustment(factor);
	game_state.balance += game_state.current_bet * factor;
}

void score_msg(char *msg1, char *msg2)
{
	font_draw_string(190, 304, TABLE_TEXT, TABLE_BACK, msg1);
	font_draw_string(215, 304, TABLE_TEXT, TABLE_BACK, msg2);
}

void hand_score()
{
	if (blackjack(&g_state.player_hand, &g_state.player_value))
	{ // player blackjack 
		if (blackjack(&g_state.dealer_hand, &g_state.dealer_value))
		{ // dealer blackjack
			// push
			score_msg("Push.", "Take back your chips.");
			adjust_balance(1);
		} else
		{
			score_msg("You got blackjack.", "Get paid 3 to 1.");
			adjust_balance(3);
		}
	} else if (g_state.player_value.total > 21) {
		score_msg("You bust.", "You lost your bet.");
		adjust_balance(0);
	}
	else if (g_state.dealer_value.total > 21) 
	{
		score_msg("Dealer busts.", "You win.");
		adjust_balance(2);
	}
       	else if (blackjack(&g_state.dealer_hand, &g_state.dealer_value))
	{
		// player loses
		score_msg("Dealer blackjack.", "You lost your bet.");
		adjust_balance(0);
	} 
	else if (g_state.player_value.total > g_state.dealer_value.total)
	{
		// player wins
		score_msg("You are the winner.", "Get paid 2 to 1.");
		adjust_balance(2);
	}
	else if (g_state.player_value.total == g_state.dealer_value.total)
	{
		// player pushes
		score_msg("Push.", "Take back your chips.");
		adjust_balance(1);
	} else {
		// player loses
		score_msg("Dealer wins.", "You lost your bet.");
		adjust_balance(0);
	}
}



void hand_activate()
{
	game_blank_table(TABLE_BACK);

	buttons_init();
	fill_deck(&g_state.shoe);
	init_deck(&g_state.player_hand);
	init_deck(&g_state.dealer_hand);
	shuffle_deck(&g_state.shoe);

	deck_value(&g_state.player_hand, &g_state.player_value);
	deck_value(&g_state.dealer_hand, &g_state.dealer_value);

	g_state.finished = 0;
	g_state.doubled = 0;
	g_state.player_dirty = 0;
	g_state.dealer_dirty = 0;

	g_state.cmd_button_hit = init_button("Hit",
						190, 220,
						100, 50,
						2,
						TABLE_OUTLINE, BUTTON_TEXT,
						BUTTON_BACK,   BUTTON_HOVER,
						player_hit);

	g_state.cmd_button_stand = init_button("Stand",
						190, 120,
						100, 50,
						2,
						TABLE_OUTLINE, BUTTON_TEXT,
						BUTTON_BACK,   BUTTON_HOVER,
						player_stand);

	g_state.cmd_button_double = init_button("Double",
						190, 0,
						120, 50,
						2,
						TABLE_OUTLINE, BUTTON_TEXT,
						BUTTON_BACK,   BUTTON_HOVER,
						player_double);


	font_draw_string(25, 300, TABLE_TEXT, TABLE_BACK, "Dealer:");
	font_draw_string(105, 300, TABLE_TEXT, TABLE_BACK, "Player:");

	player_hit();
	draw_deck(&g_state.player_hand, 130, 270);
	clock_delay_ms(500);

	dealer_hit();
	draw_deck(&g_state.dealer_hand, 50, 270);
	clock_delay_ms(500);

	player_hit();
	draw_deck(&g_state.player_hand, 130, 270);
	clock_delay_ms(500);

	font_draw_number(105, 180, TABLE_TEXT, TABLE_BACK, g_state.player_value.total);
	font_draw_number(25, 180, TABLE_TEXT, TABLE_BACK, g_state.dealer_value.total);

	if (g_state.player_value.total == 21) {
		// BLACKJACK off the draw
		g_state.state = DO_DEALER;
	} else {
		g_state.state = WAIT_PLAYER;
	}


}


void hand_deactivate()
{
	buttons_stop();

	if (g_state.doubled)
	{// reverse teh double before making next bet
		game_state.current_bet >>= 1;
	}
}

void hand_draw()
{

	switch(g_state.state)
	{
	case WAIT_PLAYER:
		if (g_state.player_dirty)
		{
			g_state.player_dirty = 0;

			// draw player total
			font_draw_number(105, 180, 
					 TABLE_TEXT, TABLE_BACK, 
					 g_state.player_value.total);

			// draw player hand
			draw_deck(&g_state.player_hand, 130, 270);
		}

		buttons_draw();
		break;
	case REMOVE_BUTTONS:

		// make sure the players latest cards are visible 
		font_draw_number(105, 180, TABLE_TEXT, TABLE_BACK, g_state.player_value.total);
		draw_deck(&g_state.player_hand, 130, 270);

		// did player bust?
		if (g_state.player_value.total > 21)
			font_draw_string(155, 280, BUST_TEXT, BUST_BACK, "BUST");

		// blank where the buttons used to be
		lcd_draw_rect(190, 0, 50, 320, TABLE_BACK);

		break;
	case DO_DEALER:
		if (g_state.dealer_dirty)
		{
			g_state.dealer_dirty = 0;

			// draw the dealers total
			font_draw_number(25, 180, TABLE_TEXT, TABLE_BACK, g_state.dealer_value.total);
			// draw the dealers hand
			draw_deck(&g_state.dealer_hand, 50, 270);
		}
		break;

	case SCORE:
		// show something if the dealer busted
		if (g_state.dealer_value.total > 21)
			font_draw_string(75, 280, BUST_TEXT, BUST_BACK, "BUST");
		break;
	}


}

void hand_update()
{

	switch(g_state.state)
	{
		case WAIT_PLAYER:
			buttons_update();

			// advance if player busts or has 21 
			if (g_state.player_value.total >= 21)
				player_stand();
			break;
		case REMOVE_BUTTONS:
			buttons_stop();
			g_state.state = DO_DEALER; 
			break;
		case DO_DEALER:
			// dealer hits on < 17, or soft 17
			if (g_state.dealer_value.total < 17 ||
				(g_state.dealer_value.total == 17 &&
				 g_state.dealer_value.soft))
			{
				dealer_hit();
				clock_delay_ms(500);
			}
			else
			{
				g_state.state = SCORE;
			}
			break;

		case SCORE:
			clock_delay_ms(2000);
			hand_score();
			clock_delay_ms(2000);
			hand_finish();
			break;
	}

	if (g_state.finished)
	{
		if (game_state.balance < MIN_BET) {
			game_reset();
			game_save_state();
			set_screen(&screen_broke);
		} else {
			game_save_state();
			set_screen(&screen_bet);
		}
	}
}


struct Screen screen_hand = {
	hand_activate,
	hand_deactivate,
	hand_draw,
	hand_update,
	SCREEN_SHOW_HUD
};
