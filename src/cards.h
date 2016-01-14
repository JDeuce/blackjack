#ifndef _CARDS_H
#define _CARDS_H

typedef unsigned char card_t;

struct DECK {
	card_t cards[52];
	unsigned char n_cards;
	char last_drawn;
};
typedef struct DECK deck_t;

enum SUIT {
	HEARTS,
	DIAMONDS,
	CLUBS,
	SPADES
};
typedef enum SUIT suit_t;

enum card_rank {
	CARD_2,
	CARD_3,
	CARD_4,
	CARD_5,
	CARD_6,
	CARD_7,
	CARD_8,
	CARD_9,
	CARD_10,
	CARD_JACK,
	CARD_QUEEN,
	CARD_KING,
	CARD_ACE
};
typedef enum card_rank rank_t;

struct HAND_VALUE {
	int soft;
	int total;
};
typedef struct HAND_VALUE hand_value_t;


unsigned char suit_ascii(suit_t suit);

suit_t card_suit(card_t card);
rank_t card_rank(card_t card);
card_t make_card(suit_t suit, rank_t value);

void init_deck(deck_t *deck);
void fill_deck(deck_t *deck);
void copy_deck(deck_t *dest, deck_t *src);
void shuffle_deck(deck_t *deck);

void draw_card(card_t card, int x, int y);
void draw_deck(deck_t *deck, int x, int y);
int deck_deal(deck_t *deck_src, deck_t *deck_dst);

void deck_value(deck_t *deck, hand_value_t *value);
unsigned char rank_value(rank_t rank);
#endif
