#include <avr/io.h>
void disable_events()
{
	PMIC.CTRL &= ~PMIC_LOLVLEN_bm;
}

void enable_events()
{
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
}
