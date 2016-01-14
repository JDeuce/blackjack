#ifndef _GLYPH_H
#define _GLYPH_H

struct GenericGlyph {
	unsigned short *lines;
	unsigned short height;
	unsigned short width;
};

void glyph_draw(unsigned short x, unsigned short y, 
		unsigned short forecolor, unsigned short backcolor, 
		unsigned char size, struct GenericGlyph *letter);

void glyph_draw_flipped(unsigned short x, unsigned short y, 
		unsigned short forecolor, unsigned short backcolor, 
		unsigned char size, struct GenericGlyph *letter);
#endif
