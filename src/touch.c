#include <avr/io.h>
#include <avr/interrupt.h>
#include "clock.h"
#include "lcd.h"
#include "map.h"
#include "touch.h"

/* touch driver polls every TOUCH_SAMPLE_INTERVAL ms.
 * if the touch driver detects a touch during the poll, it 
 * uses the ADC to take NUM_MEASURES measurements in both the x and y directions.
 * each measurement triggers an interrupt, which automatically starts the next conversion.
 * the driver calculates an average x and y from those conversions.
 * this converted result is sufficient when using a stylus.
 * however, fingers are fat and the touch location is extremely jittery with them.
 * For this reason, the driver keeps TOUCH_ROLLING_SAMPLES seperate conversion averages 
 * which it uses to compute an average touch location that can be returned to the application 
 * level.
 *
 * An improvement to the design would be to use the pipelined ADC to get NUM_MEASURES faster,
 * but this seems good enough for now. */ 

#define INT_LOW 1
#define INT_OFF 0


enum GRADIENT_MODE {
	GRADIENT_NONE,
	GRADIENT_X,
	GRADIENT_Y,
};

enum MEASURE_STATE {
	MEASURE_STOPPED,
	MEASURE_IN_PROGRESS,
	MEASURE_COMPLETED
};

struct MEASUREMENT {
	unsigned long sum_x;
	unsigned long sum_y;
	unsigned char measures_x;
	unsigned char measures_y;
};

struct ROLLING_AVG {
	unsigned long sum_x;
	unsigned long sum_y;
	
	unsigned short x_samples[TOUCH_ROLLING_SAMPLES];
	unsigned short y_samples[TOUCH_ROLLING_SAMPLES];

	unsigned int samples;
	unsigned int index;

	unsigned short avg_x;
	unsigned short avg_y;
};

struct DRIVER_STATE {
	enum FINGER_STATE state;
	enum GRADIENT_MODE gradient_mode;
	enum MEASURE_STATE measuring;

	struct MEASUREMENT measures;
	struct ROLLING_AVG samples;
	unsigned long next_sample;
	unsigned char count;

	unsigned short x;
	unsigned short y;
};


static volatile struct DRIVER_STATE g_state;


// reset the rolling avg samples (i.e. when there's no touch)
void rolling_reset_samples()
{
	int i;

	for (i = 0; i < TOUCH_ROLLING_SAMPLES; i++) {
		g_state.samples.x_samples[i] = 0;
		g_state.samples.y_samples[i] = 0;
	}

	g_state.samples.index = 0;
	g_state.samples.samples = 0;
	g_state.samples.sum_x = 0;
	g_state.samples.sum_y = 0;
}

// add an x,y point to the rolling average
void rolling_add_sample(unsigned short x, unsigned short y)
{	
	g_state.samples.sum_x -= g_state.samples.x_samples[g_state.samples.index];
	g_state.samples.x_samples[g_state.samples.index] = x;
	g_state.samples.sum_x += x;

	g_state.samples.sum_y -= g_state.samples.y_samples[g_state.samples.index];
	g_state.samples.y_samples[g_state.samples.index] = y;
	g_state.samples.sum_y += y;
		
	if (g_state.samples.samples < TOUCH_ROLLING_SAMPLES)
		g_state.samples.samples++;

	g_state.samples.index = (g_state.samples.index + 1) % TOUCH_ROLLING_SAMPLES;

	g_state.samples.avg_x = g_state.samples.sum_x / g_state.samples.samples;
	g_state.samples.avg_y = g_state.samples.sum_y / g_state.samples.samples;
}

// return touch state to the caller
void touch_get_state(struct TouchState *state)
{
	state->state = g_state.state;
	state->x = g_state.samples.avg_x;
	state->y = g_state.samples.avg_y;
}

// enable the standby pullup resistor
void touch_enable_standby_pullup()
{
	TFT_YNEG_CTRL = (TFT_YNEG_CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc;  
}

// disable standby pullup resistor
void touch_disable_standby_pullup()
{
	TFT_YNEG_CTRL = (TFT_YNEG_CTRL & ~PORT_OPC_gm) | PORT_OPC_TOTEM_gc;  
}

/* put the touch into standby mode.
 * idea is to pull y- pin up to a voltage and set x+ to gnd, when the 2 layers touch
 * y- goes low.
 * Can use this in an edge change interrupt, but decided to just poll the value
 * at interval so I could debounce easier. */
void touch_enter_standby()
{
	touch_enable_standby_pullup();
	TOUCH_PORT.DIRCLR = TFT_YPOS | TFT_YNEG | TFT_XNEG;
	TOUCH_PORT.OUTCLR = TFT_XPOS;
	TOUCH_PORT.DIRSET = TFT_XPOS;

	g_state.gradient_mode = GRADIENT_NONE;

}

// initialize the touch driver
void touch_init()
{
	// setup the ADC
	TOUCH_ADC.CTRLA |= ADC_ENABLE_bm;
	TOUCH_ADC.CTRLB = ADC_RESOLUTION_12BIT_gc;
	TOUCH_ADC.REFCTRL = (1 << 4); // should be ADC_REFSEL_INTVCC_gc but it doesn't seem to exist?
 	TOUCH_ADC.PRESCALER = ADC_PRESCALER_DIV16_gc;
	TOUCH_ADC.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;

	// enable interrupt when ADC finishes a measure
	TOUCH_ADC.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | INT_LOW; 

	g_state.state = TOUCH_RELEASED;
	g_state.gradient_mode = GRADIENT_NONE;

	g_state.measures.sum_x = 0;
	g_state.measures.sum_y = 0;
	g_state.measures.measures_x = 0;
	g_state.measures.measures_y = 0;
	g_state.measuring = MEASURE_STOPPED;
	g_state.next_sample = 0;
	g_state.count = 0;

	rolling_reset_samples();

	touch_enter_standby();
}

// setup a voltage gradient on the x layer
void touch_gradient_x()
{
	if (g_state.gradient_mode != GRADIENT_X) {
		TOUCH_PORT.DIRCLR = TFT_YPOS | TFT_YNEG;

		TOUCH_PORT.OUTCLR = TFT_XPOS; // xpos = gnd
		TOUCH_PORT.OUTSET = TFT_XNEG; // xneg = vcc
		TOUCH_PORT.DIRSET = TFT_XPOS | TFT_XNEG;

		g_state.gradient_mode = GRADIENT_X;
	}
}

// setup a voltage gradient on the y layer
void touch_gradient_y()
{
	if (g_state.gradient_mode != GRADIENT_Y) {
		TOUCH_PORT.DIRCLR = TFT_XPOS | TFT_XNEG;

		TOUCH_PORT.OUTCLR = TFT_YPOS; // ypos = gnd
		TOUCH_PORT.OUTSET = TFT_YNEG; // yneg = vcc
		TOUCH_PORT.DIRSET = TFT_YPOS | TFT_YNEG;

		g_state.gradient_mode = GRADIENT_Y;
	}
}

// start an ADC measurement
void touch_start_measurement()
{
	g_state.measures.sum_x = 0;
	g_state.measures.sum_y = 0;
	g_state.measures.measures_x = 0;
	g_state.measures.measures_y = 0;
	g_state.measuring = MEASURE_IN_PROGRESS;

	touch_disable_standby_pullup();
	touch_gradient_y();

	// start ADC 
	TOUCH_ADC.CH0.MUXCTRL = TOUCH_ADC_MUX_READ_XNEG;
	TOUCH_ADC.CH0.CTRL |= ADC_CH_START_bm;
}


// return true if screen is currently pressed;
int touch_pressed()
{
	return (TOUCH_PORT.IN & TFT_YNEG) == 0;
}


// complete an ADC measurement
void touch_complete_measure()
{
	unsigned short adcX, adcY;
	unsigned short screenX, screenY;


	// average the measurements
	adcX = (g_state.measures.sum_x >> LOG_NUM_MEASURES);
	adcY = (g_state.measures.sum_y >> LOG_NUM_MEASURES);

	// input range (tediously) tuned by outputting x,y value on LEDs
	screenX = (unsigned short)map(adcX, 0x0A27, 0x03FF, 0, LCD_COLUMNS - 1);
	screenY = (unsigned short)map(adcY, 0x03FF, 0x0DFF, 0, LCD_ROWS - 1);

	rolling_add_sample(screenX, screenY);
}

#include "leds.h"

void touch_task()
{
	unsigned long clock = clock_current_ms();
	if (clock > g_state.next_sample) {
		g_state.next_sample = clock + TOUCH_SAMPLE_INTERVAL;

		if (touch_pressed() || g_state.measuring) {
			if (g_state.count < TOUCH_COUNT_MAX)
				g_state.count++;

			if (g_state.count >= TOUCH_COUNT_ON) {
				if (g_state.measuring == MEASURE_STOPPED) {
					touch_start_measurement();
				} else if (g_state.measuring == MEASURE_COMPLETED) {
					touch_complete_measure();
					g_state.measuring = 0;
					g_state.state = TOUCH_PRESSED;
				}
			}
		} else {
			if (g_state.count > TOUCH_COUNT_MIN)
				g_state.count--;

			if (g_state.count <= TOUCH_COUNT_OFF &&
				g_state.state == TOUCH_PRESSED) {
				g_state.state = TOUCH_RELEASED;
				rolling_reset_samples();
			}
		}

	}
}



// interrupt when adc measurement is complete
ISR(TOUCH_ADC_INT)
{
	if (g_state.measures.measures_y < NUM_MEASURES) 
	{
		g_state.measures.sum_y += TOUCH_ADC.CH0RES;
		g_state.measures.measures_y++;

		if (g_state.measures.measures_y == NUM_MEASURES) {
			// transition from reading y to reading x
			touch_gradient_x();
			TOUCH_ADC.CH0.MUXCTRL = TOUCH_ADC_MUX_READ_YNEG;
		}
	} else if (g_state.measures.measures_x < NUM_MEASURES)
	{
		g_state.measures.sum_x += TOUCH_ADC.CH0RES;
		g_state.measures.measures_x++;
	}

	if (g_state.measures.measures_x < NUM_MEASURES)
	{
		//start next measurement
		TOUCH_ADC.CH0.CTRL |= ADC_CH_START_bm;
	} else {
		touch_enter_standby();
		g_state.measuring = MEASURE_COMPLETED;
	}
	
}




