#include <avr/interrupt.h>
#include "lcd_lowlevel.h"
#include "clock.h"
#include "leds.h"
#include "map.h"
#include "events.h"

// don't want to interrupt importantly timed lcd code
// so will use critical section between activate and deactivates
// by disabling non-timer interrupts on activate and restoring interrupt state in deactivate

void lcd_enable_backlight()
{
	// 32Mhz clock, 1x prescaler, frequency is 1ms 
	TCD0.PER = 32000;
	TCD0.CNT = 0;
	TCD0.CCA = 32000;
	TCD0.CTRLB = TC_WGMODE_SS_gc | TC0_CCAEN_bm;
	TCD0.CTRLA = TC_CLKSEL_DIV1_gc;
}

void lcd_init_hw()
{
	LCD_PORT.OUT = LCD_RESET;
	LCD_PORT.DIR = LCD_BACKLIGHT | LCD_CS | LCD_SCL | LCD_TX | LCD_RESET;

	LCD_USART.CTRLC = USART_CMODE_MSPI_gc; // set USART to master spi mode
	LCD_USART.BAUDCTRLA = 0;               // set bitrate to fastest possible
	LCD_USART.CTRLB = USART_TXEN_bm;       // enable transmit

	lcd_reset();

}

// set the lcd controlller to free running mode
void lcd_reset()
{
	// send reset signal for 5ms
	LCD_PORT.OUTCLR = LCD_RESET;
	clock_delay_ms(5);

	// remove reset signal
	LCD_PORT.OUTSET = LCD_RESET;
	clock_delay_ms(50);
}

// set the backlight pwm
void lcd_set_backlight(unsigned char value)
{
	TCD0.CCA = map(value, 0, 100, 0, 32000);
}


// chip select the lcd controller
// enter lcd critical section
void lcd_activate() 
{ 
	disable_events();
	LCD_PORT.OUTCLR = LCD_CS; 
}

// remove chip select from lcd controller
// exit lcd critical section
void lcd_deactivate()
{
	LCD_PORT.OUTSET = LCD_CS;
	enable_events();
}

// wait until shift registers can accept new data in lcd usart
void lcd_wait_ready()
{
	while ((LCD_USART.STATUS & USART_DREIF_bm) == 0);
}

// wait until a transfer is complete
void lcd_wait_tx()
{
	while ((LCD_USART.STATUS & USART_TXCIF_bm) == 0);
	
	// clear transmit complete flag
	LCD_USART.STATUS = USART_TXCIF_bm;
}

// transfer a byte without waiting for it to finish
// call is not fully asynchronous, in that if a byte is still being transfered
// when this function is called the function will block
void lcd_tx_async(unsigned char byte)
{
	lcd_wait_ready();
	LCD_USART.DATA = byte;
}

// transfer a byte and wait for it to finish
void lcd_tx(unsigned char byte) 
{
	lcd_tx_async(byte);
	lcd_wait_tx();
}

