#ifndef _GAME_H
#define _GAME_H

#define INITIAL_BALANCE 100
#define MIN_BET	5
#define MAX_BET 50

#define EEPROM_SLOT_OFFSET	18

#define STATE_MAGIC		0xDEADBEEFUL
#define SLOT_SIZE		(sizeof(struct GAME_STATE))
#define STATE_SAVE_START		20
#define STATE_SLOTS			25

struct Screen;

struct GAME_STATE {
	unsigned short balance;
	unsigned short current_bet;
	unsigned long magic;
};

extern struct GAME_STATE game_state;

void set_screen(struct Screen *screen);
void game_reset();
void game_init();
void game_task();

void game_save_state();
void game_restore_state();
void game_blank_table(unsigned short color);
#endif
