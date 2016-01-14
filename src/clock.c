#include <avr/io.h>
#include <avr/interrupt.h>
#include "clock.h"

static volatile unsigned long elapsed_ms = 0;
static TASK_CALLBACK tasks[MAX_TASKS];
static int number_tasks = 0;

ISR(TCC0_OVF_vect)
{
	elapsed_ms++;
}

void timer0_init()
{
	TCC0.PER = 32000; // want 32Mhz clock to overflow in 1ms
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
	TCC0.CNT = 0;
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
}

void select_clock_source()
{
	// enable 32Mhz oscillator
	OSC_CTRL |= OSC_RC32MEN_bm;

	// wait for the 32Mhz osc to stabilize
	while ((OSC_STATUS & OSC_RC32MRDY_bm) == 0);


	// disable interrupts
	unsigned char sreg = SREG;
	cli();
	// SCLKSEL bits in CLK_CTRL are protected by Configuration Change Protection mechanism
	// need to set bits in CCP register flag before writing to them
	// need to set CLK_CTRL bits within 4 cpu cycles of setting this flag
	// disable/re-enable interrupts to help guaranteee this
	CCP = CCP_IOREG_gc;

	// set system clock to 32Mhz oscillator
	CLK_CTRL = CLK_SCLKSEL_RC32M_gc;

	// restore interrupt setting 
	SREG = sreg;

	// disable 2Mhz oscillator
	OSC_CTRL &= ~OSC_RC2MEN_bm;

}


void clock_init()
{
	select_clock_source();
	timer0_init();

	number_tasks = 0;
}

unsigned long clock_current_ms()
{
	return elapsed_ms;
}


// register a task for the clock to call while busy waiting
// in clock_delay_ms();
void clock_register_task(TASK_CALLBACK task) 
{
	if (task)
		tasks[number_tasks++] = task;
}


// just block for the number of ms
// if some tasks have been registered, they are cycled through
// as we wait for the clock
void clock_delay_ms(const unsigned long ms)
{
	unsigned long target = elapsed_ms + ms;
	int task = 0;
	while(elapsed_ms < target) {
		if (number_tasks > 0) {
			tasks[task]();
			task = (task + 1) % number_tasks; 
		}
	}
}
