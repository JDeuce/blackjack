#ifndef _LCD_H
#define _LCD_H
#define BLUE 	0xF800
#define GREEN 	0x07E0
#define RED 	0x001F
#define WHITE  	0xFFFF
#define BLACK  	0x0000
#define CYAN 	(GREEN | BLUE)
#define MAGENTA (RED | BLUE)
#define YELLOW  (RED | GREEN)

#define COLOR(r, g, b)  \
		((unsigned short)( \
		 ((b & 0x1F) << 11) | \
		 ((g & 0x3F) << 5) | \
		 (r & 0x1F) \
		))
#define FELT_GREEN	COLOR(1, 31, 3)
#define LIGHT_BLUE	COLOR(0, 31, 18)

#define LCD_COLUMNS 	240
#define LCD_ROWS	320

void lcd_init();
void lcd_blank_screen(unsigned short color);
void lcd_set_backlight(unsigned char power);
void lcd_wait_tear();


void lcd_draw_rect(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned short color);



void lcd_draw_bordered_rect(unsigned short x, unsigned short y,
		   unsigned short width, unsigned short height,
		   unsigned short color, unsigned short border_width,
		   unsigned short border_color);

void lcd_draw_rect_normalized(short x, short y, short width, short height, unsigned short color);
void lcd_set_target(unsigned short x, unsigned short y, unsigned short width, unsigned short height);
#endif
