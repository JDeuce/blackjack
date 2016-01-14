#ifndef _EEPROM_H
#define _EEPROM_H

unsigned char eeprom_read(unsigned char byte);
void eeprom_write(unsigned char addr, unsigned char value);

#endif
