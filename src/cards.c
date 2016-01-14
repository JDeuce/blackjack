#include "cards.h"
#include "rand.h"
#include "lcd.h"
#include "font.h"
#include "suits.h"

// return suit from card
suit_t card_suit(card_t card)
{
	return card / 13;
}

// return rank of card
rank_t card_rank(card_t card)
{
	return card % 13;
}


// return ascii code for card
// note % is hardcoded in font specially for the '10' character
char rank_ascii(rank_t rank)
{
	static char ranks[] = "23456789%JQKA";
	return ranks[rank];
}

// make a card given a suit and rank
card_t make_card(suit_t suit, rank_t value)
{
	return suit * 13 + value;
}

// initialize an empty deck
void init_deck(deck_t *deck)
{
	deck->n_cards = 0;
	deck->last_drawn = -1;
}

// initialize a full deck of 52 cards
void fill_deck(deck_t *deck)
{
	card_t card;
	for (card = 0; card < 52; card++)
		deck->cards[card] = card;
	deck->n_cards = 52;
	deck->last_drawn = -1;

}

// copy a deck
void copy_deck(deck_t *dest, deck_t *src)
{
	card_t card;
	for (card = 0; card < src->n_cards; card++)
		dest->cards[card] = src->cards[card];
	dest->n_cards = src->n_cards;
}

// assume for now we only shuffle full decks
void shuffle_deck(deck_t *deck)
{
	deck_t shuffled;
	shuffled.n_cards = 52;

	int shuffled_cards = 0;
	int card_index;

	while (shuffled_cards < 52)
	{
		// choose random card
		card_index = rand_range(0, 52-shuffled_cards);

		// insert that card into shuffled array
		shuffled.cards[shuffled_cards] = deck->cards[card_index];

		// move the last card into the position of the card we just picked
		// in this way, the first 0-(52-shuffled_cards) are still unchosen
		deck->cards[card_index] = deck->cards[52-shuffled_cards-1];

		shuffled_cards++;
	}

	copy_deck(deck, &shuffled); 
}


#define CARD_HEIGHT	50
#define CARD_WIDTH	40
#define CARD_BORDER	2

void draw_card(card_t card, int x, int y)
{
	unsigned short forecolor;
	unsigned short backcolor = WHITE;
	unsigned short border_color = BLACK;

	suit_t suit = card_suit(card);
	rank_t rank = card_rank(card);
	char rank_char = rank_ascii(rank);

	if (suit == DIAMONDS || suit == HEARTS)
		forecolor = RED;
	else
		forecolor = BLACK;

	
	lcd_draw_bordered_rect(x, y, 
			       CARD_HEIGHT, CARD_WIDTH, 
			       backcolor, 
			       CARD_BORDER, border_color);
	struct GenericGlyph glyph;

	suit_glyph(&glyph, suit);
	font_draw_letter(x + 2, y + 22, forecolor, backcolor, rank_char); 
	glyph_draw(x + 32, y + 21, forecolor, backcolor, 1, &glyph);

	font_draw_letter_flipped(x + 26, y + 3, forecolor, backcolor, rank_char); 
	glyph_draw_flipped(x + 2, y + 3, forecolor, backcolor, 1, &glyph);
}

#define CARD_DISPLACE 20
void draw_deck(deck_t *deck, int x, int y)
{
	int i;
	for (i = deck->last_drawn + 1; i < deck->n_cards; i++)
	{
		draw_card(deck->cards[i], x, y - CARD_DISPLACE*i);
		deck->last_drawn = i;
	}

}

// deal a card from one deck to another
// for simplicity & performance this method deals from the bottom of the deck rather than the top
// this is okay for blackjack but may not be for other card games?
int deck_deal(deck_t *deck_src, deck_t *deck_dst)
{
	int success = 0;
	if (deck_src && deck_dst) {
		if (deck_src->n_cards > 0)
		{
			deck_dst->cards[deck_dst->n_cards] = deck_src->cards[deck_src->n_cards - 1];
			deck_src->n_cards--;
			deck_dst->n_cards++;

			success = 1;
		}
	}

	return success;
}

// return the numeric each card is worth
// assume aces are worth 1 here
unsigned char rank_value(rank_t rank) {
	static unsigned char values[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 1 };
	return values[rank];
}

// compute the blackjack value of a deck of cards
// output is a total value, and a flag indicating if the total is soft/hard 
void deck_value(deck_t *deck, hand_value_t *out)
{
	int i;
	int total = 0;
	int aces = 0;
	int soft = 0;

	for (i = 0; i < deck->n_cards; i++)
	{
		card_t card = deck->cards[i];
		rank_t rank = card_rank(card);
		int value = rank_value(rank);

		if (rank == CARD_ACE)
			aces++;

		total += value;
	}

	// use as many aces as '11' as we can
	while (total <= 11 && aces > 0)
	{
		total += 10;
		aces--;
		soft = 1;
	}

	out->soft = soft;
	out->total = total;
}
