#ifndef _HUD_H
#define _HUD_H

#include "font.h"
#define HUD_HEIGHT (CHAR_HEIGHT + 3)

#define MS_PER_HIGHLIGHT 100
#define HUD_BACK 	BLUE
#define HUD_FORE 	WHITE
#define HUD_OUTLINE 	BLACK


void hud_highlight_bet();
void hud_highlight_balance();

void hud_draw();
void hud_update();
void hud_redraw();

void hud_animate_adjustment(unsigned char factor);

void hud_task();

void hud_unfreeze_bet();
int hud_highlighting();
#endif
