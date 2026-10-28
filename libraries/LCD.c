#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"

/* ============================================================
 *  Mapeo de pines
 * ============================================================ */
#define LCD_RS  PB5
#define LCD_EN  PC1
#define LCD_D4  PC2
#define LCD_D5  PC3
#define LCD_D6  PC4
#define LCD_D7  PC5

#define DATA_BUS PORTC
#define CTL_BUS  PORTC
#define DATA_DDR DDRC
#define CTL_DDR  DDRC

/* Máscara de los 4 bits de datos en el puerto (PC2..PC5) */
#define DATA_MASK 0b00111100

/* ============================================================
 *  Escritura de un nibble (4 bits) en el bus de datos
 *  rs = 0 -> comando, rs = 1 -> dato
 *  Se preservan RS/EN porque comparten puerto con los datos.
 * ============================================================ */
static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
	DATA_BUS = (DATA_BUS & ~DATA_MASK) | ((nibble << 2) & DATA_MASK);

	if (rs)
	PORTB |=  (1 << LCD_RS);
	else
	PORTB &= ~(1 << LCD_RS);

	CTL_BUS |=  (1 << LCD_EN);
	_delay_us(50);
	CTL_BUS &= ~(1 << LCD_EN);
	_delay_us(50);
}

/* ============================================================
 *  Inicialización estándar HD44780 en modo 4 bits
 * ============================================================ */
void lcd_init(void)
{
	/* Configurar pines como salida */
	DATA_DDR |= (1 << LCD_D7) | (1 << LCD_D6) | (1 << LCD_D5) | (1 << LCD_D4);
	CTL_DDR  |= (1 << LCD_EN);      // EN sigue en PORTC (PC1)
	DDRB     |= (1 << LCD_RS);      // RS en PORTB (PB5)

    _delay_ms(50); /* Espera de encendido */

    /* Secuencia mágica de despertar:
     * 0x3 tres veces (modo 8 bits conocido), luego 0x2 (pasa a 4 bits).
     * Funciona porque D0..D3 están a GND -> el LCD ve 0x30 / 0x20. */
    lcd_write_nibble(0x03, 0);
    _delay_ms(5);
    lcd_write_nibble(0x03, 0);
    _delay_us(150);
    lcd_write_nibble(0x03, 0);
    _delay_us(150);
    lcd_write_nibble(0x02, 0);  /* 0x20 en modo 8-bit = switch a 4-bit */
    _delay_us(150);

    /* Ya en modo 4-bit: Function Set (4-bit, 2 líneas, 5x7) */
    lcd_send_command(LCD_CMD_4BIT_2ROW_5X7);
    _delay_ms(1);

    /* Display ON, cursor visible, parpadeo */
    lcd_send_command(LCD_CMD_DISPLAY_NO_CURSOR);
    _delay_ms(1);

    /* Limpiar pantalla */
    lcd_send_command(LCD_CMD_CLEAR_DISPLAY);
    _delay_ms(2);

    /* Cursor al inicio */
    lcd_send_command(0x80);
    _delay_ms(1);
}

/* ============================================================
 *  Enviar un comando (2 nibbles: alto, luego bajo)
 * ============================================================ */
void lcd_send_command(uint8_t command)
{
    lcd_write_nibble(command >> 4, 0);
    lcd_write_nibble(command & 0x0F, 0);
	
	// Solamente Clear y HOME ocupan 2 ms
	if (command == 0x01 || command == 0x02) _delay_ms(2);
}

/* ============================================================
 *  Escribir un carácter
 * ============================================================ */
void lcd_write_character(uint8_t character)
{
    lcd_write_nibble(character >> 4, 1);
    lcd_write_nibble(character & 0x0F, 1);
}

/* ============================================================
 *  Escribir una cadena terminada en '\0'
 * ============================================================ */
void lcd_write_string(const char *word)
{
    while (*word)
        lcd_write_character((uint8_t)*word++);
}

/* ============================================================
 *  Limpiar pantalla
 * ============================================================ */
void lcd_clear(void)
{
    lcd_send_command(LCD_CMD_CLEAR_DISPLAY);
    _delay_ms(2);
}

/* ============================================================
 *  Posicionar cursor. line: 0 o 1. pos: 0..15
 * ============================================================ */
void lcd_goto_xy(uint8_t line, uint8_t pos)
{
    lcd_send_command((0x80 | (line << 6)) + pos);
    _delay_us(50);
}
