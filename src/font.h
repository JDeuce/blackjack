#ifndef _FONT_H
#define _FONT_H
#define CHAR_WIDTH 14
#define CHAR_HEIGHT 22
	struct FontGlyph 
	{
		unsigned short lines[CHAR_HEIGHT];
	};


	void font_draw_letter(unsigned short x, unsigned short y, 
			      unsigned short forecolor, unsigned short backcolor, 
			      char ch);

	void font_draw_letter_sized(unsigned short x, unsigned short y, 
			      unsigned short forecolor, unsigned short backcolor, 
			      char ch, int size);

	void font_draw_letter_flipped(unsigned short x, unsigned short y, 
			      unsigned short forecolor, unsigned short backcolor, 
			      char ch);

	void font_draw_string(unsigned short x, unsigned short y,
			      unsigned short forecolor, unsigned short backcolor,
			      char *string);

	void font_draw_string_flipped(unsigned short x, unsigned short y,
			      unsigned short forecolor, unsigned short backcolor,
			      char *string);

	void font_draw_number(unsigned short x, unsigned short y,
			      unsigned short forecolor, unsigned short backcolor,
			      unsigned int number);

	void font_draw_number_flipped(unsigned short x, unsigned short y,
			      unsigned short forecolor, unsigned short backcolor,
			      unsigned int number);

	int font_measure_string(char *string);
#endif
