#include <avr/io.h>
#include <avr/interrupt.h>

#include "leds.h"


// set CMDEX in NVM.CTRLA
void eeprom_set_cmdex()
{
	// save & disable interrupts
	unsigned char sreg = SREG;
	cli();

	// disable change protection on CTRLA bits for 4 clock cycles
	CCP = CCP_IOREG_gc;
	NVM.CTRLA = NVM_CMDEX_bm;

	// restore interrupt state
	SREG = sreg;

}

// read a byte from e2
uint8_t eeprom_read(uint8_t byte)
{
	// section 30.11.5.7 in datasheet
	NVM.CMD = NVM_CMD_READ_EEPROM_gc;
	NVM.ADDR0 = byte;
	
	eeprom_set_cmdex();
	
	return NVM.DATA0;
}

// write a byte to e2
void eeprom_write(uint8_t addr, uint8_t value)
{
	// load
	// section 30.11.5.1 in datasheet
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;
	NVM.ADDR0 = addr;
	NVM.DATA0 = value;

	// then erase&write
	// 30.11.5.5
	NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
	
	// execute
	eeprom_set_cmdex();

	// block until NVM command is complete
	while(NVM.STATUS & NVM_NVMBUSY_bm);
}

void eeprom_test()
{

	eeprom_write(200, 0xAA);
	NVM.DATA0 = 0xFF;
	led_set(eeprom_read(200));

	for (;;);
}
