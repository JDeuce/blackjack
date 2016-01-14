#ifndef _LCD_LOWLEVEL_H
#define _LCD_LOWLEVEL_H

#include <avr/io.h>
#include "hx8347_regs.h"

#define LCD_PORT	PORTD
#define LCD_USART	USARTD1

// Displayed xplained hw users guide section 4.3
#define LCD_BACKLIGHT	PIN0_bm
#define LCD_RESET	PIN1_bm
#define LCD_TE		PIN2_bm
#define LCD_CS		PIN4_bm
#define LCD_SCL		PIN5_bm
#define LCD_RX		PIN6_bm
#define LCD_TX		PIN7_bm

// section 5.1.2
#define LCD_OP_SET_INDEX 	0x74
#define LCD_OP_WRITE_REG	0x76
#define LCD_OP_READ_REG		0x77


void lcd_reset();
void lcd_init_hw();
void lcd_activate();
void lcd_deactivate();
void lcd_enable_backlight();
void lcd_set_backlight(unsigned char level);
void lcd_wait_ready();
void lcd_wait_tx();
void lcd_tx(unsigned char byte);
void lcd_tx_async(unsigned char byte);
unsigned char lcd_rx();

void lcd_set_index(unsigned char index);
void lcd_write_reg(unsigned char index, unsigned char value);
#endif
