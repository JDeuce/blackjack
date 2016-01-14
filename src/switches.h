#ifndef _SWITCH_H
#define _SWITCH_H

#include <avr/io.h>

#define SWITCHPORT PORTF

#define SWITCH0 PIN0_bm
#define SWITCH1	PIN1_bm
#define SWITCH2	PIN2_bm
#define SWITCH3	PIN3_bm
#define SWITCH4	PIN4_bm
#define SWITCH5	PIN5_bm
#define SWITCH6	PIN6_bm
#define SWITCH7	PIN7_bm
#define NUM_SWITCHES 8
#define SWITCH_ALL 0xFF

typedef void(*SWITCH_CALLBACK)(void);

#define SWITCH_MAX	5
#define SWITCH_MIN	0
#define SWITCH_ON	3
#define SWITCH_OFF	1
#define SWITCH_POLL_INTERVAL	25

void switches_init();
unsigned char switch_get(unsigned char mask);

void switch_callback_up(int which, SWITCH_CALLBACK cb);
void switch_callback_down(int which, SWITCH_CALLBACK cb);
void switch_task();

#endif
