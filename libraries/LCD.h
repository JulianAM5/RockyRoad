#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

/* ============================================================
 *  Comandos estándar HD44780
 * ============================================================ */
#define LCD_CMD_CLEAR_DISPLAY        0x01
#define LCD_CMD_RETURN_HOME          0x02
#define LCD_CMD_ENTRY_MODE           0x06
#define LCD_CMD_DISPLAY_OFF          0x08
#define LCD_CMD_DISPLAY_ON           0x0C
#define LCD_CMD_DISPLAY_NO_CURSOR    0x0C   /* display ON, cursor OFF, blink OFF */
#define LCD_CMD_DISPLAY_CURSOR       0x0E   /* display ON, cursor ON,  blink OFF */
#define LCD_CMD_DISPLAY_BLINK        0x0F   /* display ON, cursor ON,  blink ON  */
#define LCD_CMD_SHIFT_CURSOR_LEFT    0x10
#define LCD_CMD_SHIFT_CURSOR_RIGHT   0x14
#define LCD_CMD_SHIFT_DISPLAY_LEFT   0x18
#define LCD_CMD_SHIFT_DISPLAY_RIGHT  0x1C

/* Function Set: 4-bit bus, 2 rows, 5x7 font */
#define LCD_CMD_4BIT_1ROW_5X7        0x20
#define LCD_CMD_4BIT_2ROW_5X7        0x28
#define LCD_CMD_8BIT_1ROW_5X7        0x30
#define LCD_CMD_8BIT_2ROW_5X7        0x38

/* Set DDRAM address (cursor position) — OR with the address */
#define LCD_CMD_SET_DDRAM_ADDR       0x80

/* Inicializa el LCD en modo 4 bits (debe llamarse antes que cualquier otra) */
void lcd_init(void);

/* Envía un comando al controlador */
void lcd_send_command(uint8_t command);

/* Escribe un solo carácter en la posición actual del cursor */
void lcd_write_character(uint8_t character);

/* Escribe una cadena terminada en '\0' */
void lcd_write_string(const char *word);

/* Limpia la pantalla y lleva el cursor a (0,0) */
void lcd_clear(void);

/* Posiciona el cursor. line: 0 o 1. pos: 0..15 */
void lcd_goto_xy(uint8_t line, uint8_t pos);

#endif /* LCD_H_ */
