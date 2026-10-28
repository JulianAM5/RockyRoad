#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

void UART_init(uint16_t, uint8_t, uint8_t, uint8_t);
unsigned char UART_read();
void UART_write(unsigned char);
void UART_string(char*);

#endif
