#ifndef _CLOCK_H
#define _CLOCK_H

#define MAX_TASKS	32
typedef void(*TASK_CALLBACK)(void);

void clock_init();
unsigned long clock_current_ms();
void clock_delay_ms(const unsigned long ms);

void clock_register_task(TASK_CALLBACK task);
#endif
