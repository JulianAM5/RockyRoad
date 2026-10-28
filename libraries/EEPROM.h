#ifndef EEPROM_H
#define EEPROM_H

#include <avr/io.h>
#include <stdint.h>

unsigned char EEPROM_read(uint16_t);
void EEPROM_write(uint16_t, uint8_t);
void EEPROM_update(uint16_t, uint8_t);

#endif
