#ifndef _TOUCH_H
#define _TOUCH_H

#include <avr/io.h>
#define TOUCH_PORT PORTA
#define TFT_XL	PIN4_bm
#define TFT_XR  PIN5_bm
#define TFT_YD  PIN6_bm
#define TFT_YU  PIN7_bm

#define TFT_XPOS	TFT_XL
#define TFT_XNEG	TFT_XR
#define TFT_YPOS	TFT_YD
#define TFT_YNEG	TFT_YU

#define TFT_YNEG_CTRL  TOUCH_PORT.PIN7CTRL
#define TOUCH_ADC 			ADCA
#define TOUCH_ADC_MUX_READ_YNEG 	ADC_CH_MUXPOS_PIN7_gc
#define TOUCH_ADC_MUX_READ_XNEG 	ADC_CH_MUXPOS_PIN5_gc
#define TOUCH_ADC_INT			ADCA_CH0_vect
#define TOUCH_PORT_INT0_vect		PORTA_INT0_vect

#define TOUCH_SAMPLE_INTERVAL 10 // sample ~100x a second

#define TOUCH_COUNT_MIN	0
#define TOUCH_COUNT_MAX	5
#define TOUCH_COUNT_ON  3
#define TOUCH_COUNT_OFF	1

// number of ADC measurements per sample
#define NUM_MEASURES 4
#define LOG_NUM_MEASURES 2 // base 2

// number of samples for rolling average
#define TOUCH_ROLLING_SAMPLES	10

enum FINGER_STATE {
	TOUCH_PRESSED,
	TOUCH_RELEASED
};

struct TouchState {
	enum FINGER_STATE state;
	unsigned short x;
	unsigned short y;
};

void touch_init();

void touch_get_state(struct TouchState *state);

void touch_task();
#endif
