#include "switches.h"
#include "clock.h"

enum SWITCH_STATE { 
	SWITCH_UP,
	SWITCH_DOWN
};


struct SWITCH_DATA {
	enum SWITCH_STATE state;
	unsigned char count;

	SWITCH_CALLBACK switch_down_cb;
	SWITCH_CALLBACK	switch_up_cb;
};

static struct SWITCH_DATA switch_data[NUM_SWITCHES];
static unsigned long next_poll = 0;

void switches_init()
{
	PORTCFG.MPCMASK = 0xFF;
	SWITCHPORT.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_INVEN_bm;
	SWITCHPORT.DIR = 0x00;

	int i;
	for (i = 0; i < NUM_SWITCHES; i++) {
		switch_data[i].state = SWITCH_UP;
		switch_data[i].count = 0;
		switch_data[i].switch_down_cb = 0;
		switch_data[i].switch_up_cb = 0;
	}
}

unsigned char switch_get(unsigned char mask) 
{
	return SWITCHPORT.IN & mask;
}

void switch_down(int which)
{
	if (switch_data[which].switch_down_cb)
		switch_data[which].switch_down_cb();
}

void switch_up(int which)
{
	if (switch_data[which].switch_up_cb)
		switch_data[which].switch_up_cb();
}

void switch_callback_up(int which, SWITCH_CALLBACK cb) {
	switch_data[which].switch_up_cb = cb;
}

void switch_callback_down(int which, SWITCH_CALLBACK cb) {
	switch_data[which].switch_down_cb = cb;
}

void switch_task()
{
	unsigned long clock = clock_current_ms();
	int i;
	if (clock > next_poll) { 
		next_poll = clock + SWITCH_POLL_INTERVAL;

		for (i = 0; i < NUM_SWITCHES; i++) {
			if (SWITCHPORT.IN & (1 << i)) {
				if (switch_data[i].count < SWITCH_MAX)
					switch_data[i].count++;

				if (switch_data[i].state == SWITCH_UP && 
				    switch_data[i].count >= SWITCH_ON) {
					switch_data[i].state = SWITCH_DOWN;
					switch_down(i);
				}
				
			} else {
				if (switch_data[i].count > SWITCH_MIN)
					switch_data[i].count--;

				if (switch_data[i].state == SWITCH_DOWN && 
				    switch_data[i].count <= SWITCH_OFF) {
					switch_data[i].state = SWITCH_UP;
					switch_up(i);
				}
			}
		}
	}
}

