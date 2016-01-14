#ifndef _SCREEN_H
#define _SCREEN_H

#define SCREEN_SHOW_HUD 	1
#define SCREEN_HIDE_HUD		0
struct Screen {
	void (*activate)(void);
	void (*deactivate)(void);

	void (*draw)(void);
	void (*update)(void);

	int show_hud;
};

#endif
