#include "UART.h"
#include <stdint.h>

void UART_init(uint16_t baudRate, uint8_t dataSize, uint8_t parityMode, uint8_t stopBits) {
    UCSR0A &= ~(1<<U2X0);//Double USART Speed Disabled
    UCSR0A &= ~(1<<MPCM0);//Multiprocessor Mode Disabled
    

    UCSR0B &= ~(1<<UDRIE0);// Data Register Empty IntEn
    UCSR0B &= ~(1<<TXCIE0);// Tx Complete IntEn
    UCSR0B &= ~(1<<RXCIE0);// Rx Complete IntEn
    UCSR0C &= ~((1<<UMSEL01) | (1<<UMSEL00));//Mode:00=Asynchronous
    
    UCSR0C |= ((parityMode & 0x03) << UPM00); // Parity Mode: 00=None
    UCSR0C |= ((stopBits & 0x01) << USBS0); // Stop bit: 0=1-bit
    UCSR0C |= ((dataSize & 0x03) << UCSZ00); // Data Size: 11=8-bit
    UCSR0B &= ~(1 << UCSZ02);
    UCSR0C &= ~(1<<UCPOL0); // Clock Polarity: 0 for Async.
    UBRR0 = baudRate;

    UCSR0B |= (1<<TXEN0);// Tx Enable
    UCSR0B |= (1<<RXEN0);// Rx Enable
}

unsigned char UART_read() {
    if (UCSR0A&(1<<RXC0)) {
        return UDR0;
    }
    return 0;
}

void UART_write(unsigned char data) {
    while(!(UCSR0A&(1<<UDRE0)));
    UDR0 = data;
}

void UART_string(char* string) {
    while (*string != 0) {
        UART_write(*string);
        string++;
    }
}
