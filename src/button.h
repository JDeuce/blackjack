#ifndef _BUTTON_H
#define _BUTTON_H

#define BUTTON_MAX_TITLE 16
#define MAX_BUTTONS 	 32

// hysterisis values
#define BUTTON_COUNT_MIN  0
#define BUTTON_COUNT_MAX  7
#define BUTTON_COUNT_ON   4
#define BUTTON_COUNT_OFF  2

// use these to adjust the buttons logical size proportionally with its hysterisis count.
// require a more focused hit to declare an 'on' state, but be more lax to continue declaring on
// this reduces oscillation
#define MAX_EDGE_IGNORE 5
#define MIN_EDGE_IGNORE -5


// time to wait before starting repeat mode
#define BUTTON_REPEAT_WAIT	500

enum ButtonState {
	IDLE,
	HOVERED,
	REPEATING
};

typedef void(*BUTTON_CALLBACK)(void);
struct Button {
	enum ButtonState state;
	int count_on;
	char title[16];
	int x;
	int y;
	int width;
	int height;
	int border;
	unsigned short bordercolor;
	unsigned short forecolor;
	unsigned short idlecolor;
	unsigned short hovercolor;
	unsigned long dirty;

	unsigned char pressed;
	BUTTON_CALLBACK callback;
	unsigned char first_draw;
	unsigned long repeat_trigger;
	unsigned short repeats;
};


void buttons_init();
void buttons_stop();
void button_set_title(struct Button *button, char *title);
struct Button *init_button(char *title, 
			int x, int y, 
			int width, int height,
			int border, unsigned short bordercolor,
			unsigned short forecolor, 
			unsigned short idlecolor,
			unsigned short hovercolor,
			BUTTON_CALLBACK callback);
void buttons_draw();
void buttons_update();
#endif
