#include "leds.h"

void leds_init()
{
	PORTCFG.MPCMASK = 0xFF;
	LEDPORT.PIN0CTRL = PORT_INVEN_bm;
	LEDPORT.DIR = 0xFF;
	LEDPORT.OUT = 0x00;
}

void led_set(unsigned char mask)
{
	LEDPORT.OUTSET = mask;
}

void led_clear(unsigned char mask)
{
	LEDPORT.OUTCLR = mask;
}

void led_toggle(unsigned char mask)
{
	LEDPORT.OUTTGL = mask;
}
