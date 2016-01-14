#include "button.h"
#include "clock.h"
#include "lcd.h"
#include "font.h"
#include "touch.h"
#include "map.h"

// keep track of all the buttons we have on screen
static struct Button g_buttons[MAX_BUTTONS];
static unsigned char g_next_button = 0;
static unsigned long g_next_update = 0;


// return true if the given point is inside of the button
int point_in_button(struct Button *button, unsigned short x, unsigned short y)
{
	int in_button = 0;

	// so to improve behaviour around edges
	// i want to ignore starting a touch for a few pixels around the
	// border of the button.
	// but if we've already declared a touch, then we should
	// continue declaring it until the pointer is all the way out.
	// use map to scale the pixels to check with the on counter.
	// note: this works ideally when using a stylus
	short edge_ignore = (short)map(button->count_on, 
					 BUTTON_COUNT_MIN, BUTTON_COUNT_MAX,
					 MAX_EDGE_IGNORE, MIN_EDGE_IGNORE);


	if (x > button->x + edge_ignore &&
	    x < button->x + button->height - edge_ignore)
		if (y > button->y + edge_ignore &&
		    y < button->y + button->width - edge_ignore)
			in_button = 1;
	return in_button;
}


void buttons_reset()
{
	g_next_button = 0;
	g_next_update = 0;
}

void button_set_title(struct Button *button, char *title)
{
	unsigned char i;	
	for (i = 0; i < BUTTON_MAX_TITLE; i++)
	{
		if (*title != '\0') {
			button->title[i] = *title;
			title++;
		} else {
			button->title[i] = '\0';
		}
	}
}

struct Button *init_button(char *title, 
			int x, int y, 
			int width, int height,
			int border, unsigned short bordercolor,
			unsigned short forecolor, unsigned short idlecolor,
			unsigned short hovercolor,
			BUTTON_CALLBACK callback)
{
	struct Button *button = &g_buttons[g_next_button];
	g_next_button++;

	button->state = IDLE;
	button_set_title(button, title);
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->border = border;
	button->bordercolor = bordercolor;
	button->forecolor = forecolor;
	button->idlecolor = idlecolor;
	button->hovercolor = hovercolor;
	button->dirty = 1;
	button->count_on = 0;
	button->callback = callback;
	button->pressed = 0;
	button->first_draw = 1;
	button->repeats = 0;
	button->repeat_trigger = 0;
	return button;
}

void button_draw(struct Button *button)
{
	/*
	 * _______________
	 * | +         + |
	 * | +text here+ |
	 * | +         + | 
	 * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	 * figure out the width from + to | then
	 * add that to the known location of |
	 * then add the width of the string to get the top left part of the string
	 * 
	 * subtract a character width since we pass the top right location
	 * of the first character to the draw_string function
	 *
	 * Note: Screen is like quadrant 3, with the origin in the top right.
	 *
	 */
	unsigned short title_width;
	unsigned short edge_width;

	unsigned short backcolor = WHITE;

	switch (button->state)
	{
	case IDLE:
		backcolor = button->idlecolor;
		break;
	case REPEATING:
	case HOVERED:
		backcolor = button->hovercolor;
		break;
	}

	if (button->first_draw) {
		// only need to draw the border on the first draw
		// since the outside doesn't change
		lcd_draw_bordered_rect(button->x, button->y,
				       button->height, button->width,
				       backcolor, button->border,
				       button->bordercolor);
		button->first_draw = 0;
	} 
	else
	{
		lcd_draw_rect(button->x + button->border,
			      button->y + button->border,
			      button->height - 2 * button->border,
			      button->width - 2 * button->border,
			      backcolor);
	}

	title_width = font_measure_string(button->title);
	edge_width = (button->width - title_width) / 2;

	font_draw_string(button->x + (button->height / 2 - button->border - 8), 
			button->y + edge_width + title_width - CHAR_WIDTH,
			button->forecolor, backcolor,
			button->title);
			
}

void buttons_draw()
{
	int i;
	for (i = 0; i < g_next_button; i++)
	{
		if (g_buttons[i].dirty) {
			g_buttons[i].dirty = 0;
			button_draw(&g_buttons[i]);
		}
	}

}

void buttons_update()
{
	unsigned long clock = clock_current_ms();
	struct TouchState touch;
	int i;

	// use these to keep 1 button active at a time based on the buttons priority (defined by the order it was added)
	int triggered = 0;    // use this flag to only trigger 1 button callback 
	int incremented = 0;  // use this flag to only increment one button per update

	if (clock > g_next_update) {
		g_next_update = clock + 10;

		touch_get_state(&touch);

		for (i = 0; i < g_next_button; i++)
		{
			// update count on value
			if (touch.state == TOUCH_PRESSED && 
			    point_in_button(&g_buttons[i], touch.x, touch.y) &&
			    !incremented) {
				incremented = 1;
				if (g_buttons[i].count_on < BUTTON_COUNT_MAX) 
					g_buttons[i].count_on++;
			} else if (g_buttons[i].count_on > BUTTON_COUNT_MIN) {
				g_buttons[i].count_on--;
			}

			// check for state change
			if (g_buttons[i].count_on >= BUTTON_COUNT_ON) {
				if (g_buttons[i].state == IDLE) {
					g_buttons[i].state = HOVERED;
					g_buttons[i].dirty = 1;
					g_buttons[i].repeat_trigger = clock + BUTTON_REPEAT_WAIT;
				} else if (g_buttons[i].repeats && 
				    clock_current_ms() > g_buttons[i].repeat_trigger){ 
					if (g_buttons[i].callback != 0)
						g_buttons[i].callback();
					g_buttons[i].repeat_trigger = clock + g_buttons[i].repeats;
					g_buttons[i].state = REPEATING;
				} 
			} else if (g_buttons[i].count_on <= BUTTON_COUNT_OFF &&
				   g_buttons[i].state != IDLE) {

				// if touch state transitions to released while
				// this button is on, assume button is triggered 
				// also make sure only 1 button triggers per update
				if (touch.state == TOUCH_RELEASED &&
				    g_buttons[i].state != REPEATING && 
				    !triggered) {
					triggered = 1;
					if (g_buttons[i].callback != 0) 
						g_buttons[i].callback();
				}

				g_buttons[i].state = IDLE;
				g_buttons[i].dirty = 1;
			}
		}
	}
}


void buttons_init()
{
	buttons_reset();
}

void buttons_stop()
{
	buttons_reset();
}
