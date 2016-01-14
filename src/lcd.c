#include "clock.h"
#include "lcd.h"
#include "lcd_lowlevel.h"

// whoops i misread the datasheet as used millisecond instead of microsecond delays
// oh well, it only uses them during initialization and it works/is fast enough

static unsigned char current_index = 0; // no register has address 0
					// so this is a safe default

void lcd_set_index(unsigned char index)
{
	if (current_index != index)  
	{
		lcd_activate();
		lcd_tx_async(LCD_OP_SET_INDEX);
		lcd_tx(index);
		lcd_deactivate();
		current_index = index;
	}
}

void lcd_write_reg(unsigned char index, unsigned char value) 
{
	lcd_set_index(index);
	lcd_activate();
	lcd_tx_async(LCD_OP_WRITE_REG);
	lcd_tx(value);
	lcd_deactivate();
}


// section 5.12 of hymax datasheet
void lcd_exit_standby()
{
	// enable the oscillator
	lcd_write_reg(OSC_CTL1,	OSC_CTL1_OSC_EN);
	clock_delay_ms(10);
	// remove the standby bit 
	lcd_write_reg(PWR_CTL1, 0);
}

// section 5.13 power supply setting flow
void lcd_power_on()
{
	lcd_write_reg(PWR_CTL1,		PWR_CTL1_DK);
	lcd_write_reg(DISP_CTL1,	0);
	lcd_write_reg(VCOM_CTL1,	0);
	clock_delay_ms(10);

	lcd_write_reg(PWR_CTL6,		PWR_CTL6_BT);
	lcd_write_reg(PWR_CTL5,		PWR_CTL5_VRH);
	lcd_write_reg(VCOM_CTL2,	VCOM_CTL2_VCM);
	lcd_write_reg(VCOM_CTL3,	VCOM_CTL3_VDV);
	lcd_write_reg(PWR_CTL3,		PWR_CTL3_VC1);
	lcd_write_reg(PWR_CTL4,		PWR_CTL4_VC3);
	
	lcd_write_reg(PWR_CTL2,		PWR_CTL2_AP);
	lcd_write_reg(PWR_CTL1,		PWR_CTL1_PON);
	clock_delay_ms(40);

	lcd_write_reg(VCOM_CTL1,	VCOM_CTL1_VCOMG);
	clock_delay_ms(60);
}

// section 5.12 of hymax data sheet
void lcd_display_on()
{
	lcd_write_reg(DISP_CTL8,	DISP_CTL8_SAP);
	lcd_write_reg(DISP_CTL1,	DISP_CTL1_D_0);
	clock_delay_ms(10);
	lcd_write_reg(DISP_CTL1,	DISP_CTL1_GON | DISP_CTL1_D_0);
	lcd_write_reg(DISP_CTL1,	DISP_CTL1_GON | DISP_CTL1_D);
	clock_delay_ms(10);
	lcd_write_reg(DISP_CTL1,	DISP_CTL1_GON | DISP_CTL1_D | DISP_CTL1_DTE);

}


// setup default register state of the device
void lcd_set_modes()
{
	// turn on inverted memory so colors we can write colors like 0xF800 for full red instead of non-inverted 0x07FF
	lcd_write_reg(DISP_MODE_CTL, DISP_MODE_CTL_INVON);

	// use 65K-color mode
	lcd_write_reg(DATA_CTL,	0);

	// black looks weird unless I set this
	lcd_write_reg(CYCLE_CTL5, CYCLE_CTL5_SON);

	lcd_write_reg(INTERNAL_28, 0b1000);
}

// set the targeted area for a gram update
void lcd_set_target(unsigned short x, unsigned short y, unsigned short width, unsigned short height)
{
	lcd_write_reg(COL_START_HIGH,	x >> 8);
	lcd_write_reg(COL_START_LOW,	x & 0xFF);
	lcd_write_reg(COL_END_HIGH,	(x + width - 1) >> 8);
	lcd_write_reg(COL_END_LOW,	(x + width - 1) & 0xFF);

	lcd_write_reg(ROW_START_HIGH,	y >> 8);
	lcd_write_reg(ROW_START_LOW,	y & 0xFF);
	lcd_write_reg(ROW_END_HIGH,	(y + height - 1) >> 8);
	lcd_write_reg(ROW_END_LOW,	(y + height - 1) & 0xFF);
}

// external initialize function
void lcd_init()
{
	lcd_init_hw();

	lcd_exit_standby();
	lcd_power_on();
	lcd_display_on();

	lcd_set_modes();

	lcd_blank_screen(0x00);

	lcd_enable_backlight();
}


// draw the screen a particular color
void lcd_blank_screen(unsigned short color)
{
	lcd_draw_rect(0, 0, LCD_COLUMNS, LCD_ROWS, color);
}


// "normalized" as in, if off screen coordinates are passed in, that part of the
// rectangle will be discarded and only the on screen part of the rectangle will be drawn
void lcd_draw_rect_normalized(short x, short y,
			      short width, short height,
			      unsigned short color)
{

	// totally off the screen
	if (x >= LCD_COLUMNS)
		return;

	if (y >= LCD_ROWS)
		return;


	// x off the screen, but some of rect might still be on screen
	if (x < 0) {
		width += x;
		x = 0;

		if (width < 0) // no rect left
			return;
	}

	// y off the screen, but some of rect might still be on screen
	if (y < 0) {
		height += y;
		y = 0;
		if (height < 0)
			return;
	}

	// drawing full height is off screen
	if (y + height > LCD_ROWS) {
		height = LCD_ROWS - y;
	}


	// drawing full width is off screen
	if (x + width > LCD_COLUMNS) {
		width = LCD_COLUMNS - x;
	}

	lcd_draw_rect(x, y, width, height, color);
}

void lcd_wait_tear()
{

	// see the tear effect signal oscillating but
	// waiting for either of these doesn't seem to make a difference 
	// graphically. maybe its very subtle 
	// while ((LCD_PORT.IN & LCD_TE) == 0); 
	// while ((LCD_PORT.IN & LCD_TE) == 1); 
}

void lcd_draw_rect(unsigned short x, unsigned short y,
		   unsigned short width, unsigned short height,
		   unsigned short color)
{
	unsigned char high = color >> 8;
	unsigned char low = color & 0xFF;

	lcd_set_target(x, y, width, height);
	lcd_set_index(SRAM_WRITE_CTL);

	lcd_activate();
	lcd_tx(LCD_OP_WRITE_REG);

	unsigned int i, j;
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			lcd_tx_async(high);
			lcd_tx_async(low);
		}
	}

	lcd_deactivate();
}

void lcd_draw_bordered_rect(unsigned short x, unsigned short y,
		   unsigned short width, unsigned short height,
		   unsigned short color, unsigned short border_width,
		   unsigned short border_color)
{
	lcd_draw_rect(x, y, width, height, border_color);
	lcd_draw_rect(x + border_width, y + border_width,
		      width - border_width*2,
		      height - border_width*2,
		      color);
}

