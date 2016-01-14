#ifndef _LEDS_H
#define _LEDS_H

#include <avr/io.h>
#define LEDPORT PORTE

#define LED0	PIN0_bm
#define LED1	PIN1_bm
#define LED2	PIN2_bm
#define LED3	PIN3_bm
#define LED4	PIN4_bm
#define LED5	PIN5_bm
#define LED6	PIN6_bm
#define LED7	PIN7_bm
#define LED8	PIN8_bm

#define LED_ALL 0xFF


void leds_init();
void led_set(unsigned char mask);
void led_clear(unsigned char mask);
void led_toggle(unsigned char mask);
#endif
