#include "glyph.h"

#include "lcd.h"
#include "lcd_lowlevel.h"

// draw a (horribly pixelated) glyph at an arbitrary size using nearest neighbor interpolation
void glyph_draw(unsigned short x, unsigned short y, 
		unsigned short forecolor, unsigned short backcolor, 
		unsigned char size, struct GenericGlyph *letter)
{
	if (letter == 0)
		return;

	lcd_set_target(x, y, letter->height * size, letter->width * size);
	lcd_set_index(SRAM_WRITE_CTL);

	lcd_activate();
	lcd_tx(LCD_OP_WRITE_REG);
	unsigned short col, row;

	for (col = 0; col < letter->width * size; col++) {
		for (row = 0; row < letter->height * size; row++) {
			unsigned short bit = letter->lines[row/size] & (1 << (col/size));
			if (bit) {
				lcd_tx_async(forecolor >> 8);
				lcd_tx_async(forecolor);
			} else {
				lcd_tx_async(backcolor >> 8);
				lcd_tx_async(backcolor);
			}
		}
	}

	lcd_deactivate();
}

// flip glyph diagonally
void glyph_draw_flipped(unsigned short x, unsigned short y, 
		unsigned short forecolor, unsigned short backcolor, 
		unsigned char size, struct GenericGlyph *letter)
{
	if (letter == 0)
		return;

	lcd_set_target(x, y, letter->height * size, letter->width * size);
	lcd_set_index(SRAM_WRITE_CTL);

	lcd_activate();
	lcd_tx(LCD_OP_WRITE_REG);
	short col, row;

	for (col = letter->width * size - 1; col >= 0; col--) {
		row = letter->height * size - 1;
		for (row = letter->height - 1; row >= 0; row--) {
			unsigned short bit = letter->lines[row/size] & (1 << (col/size));
			if (bit) {
				lcd_tx_async(forecolor >> 8);
				lcd_tx_async(forecolor);
			} else {
				lcd_tx_async(backcolor >> 8);
				lcd_tx_async(backcolor);
			}
		}
	}

	lcd_deactivate();
}
