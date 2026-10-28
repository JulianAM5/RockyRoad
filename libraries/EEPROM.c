#include "EEPROM.h"

unsigned char EEPROM_read(uint16_t uiAddress) {
    while (EECR & (1<<EEPE));

    EEAR = uiAddress;
    EECR |= (1<<EERE);

    return EEDR;
}

void EEPROM_write(uint16_t uiAddress, uint8_t ucData) {
    while (EECR & (1<<EEPE));

    EEAR = uiAddress;
    EEDR = ucData;

    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
}

void EEPROM_update(uint16_t uiAddress, uint8_t ucData) {
    // Si la direccion tiene el mismo valor a escribir, regresar
    if (EEPROM_read(uiAddress) == ucData) return;

    // Actualizar valor
    EEPROM_write(uiAddress, ucData);
}
