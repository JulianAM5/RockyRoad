/*
 * Practica4.c
 *
 * Created: 06/10/2026 02:26:40 p.m.
 * Author: Julian
 */
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdbool.h>
#include "libraries/LCD.h"
#include "libraries/RockyRoad.h"
#include "libraries/UART.h"
#include "libraries/Ranking.h"

#define BUZZER_PIN   PB4
#define BUZZER_ON_MS 20

/* ============================================================
 *  Configuración UART
 *  UART_init recibe el VALOR de UBRR0, no el baudrate.
 *  UBRR0 = F_CPU/(16*BAUD) - 1
 *  Ej: 9600 baud @ 16 MHz -> 103
 * ============================================================ */
#define BAUD         9600
#define UBRR_VALUE   ((F_CPU / (16UL * BAUD)) - 1)

/* dataSize: 0b11 = 8 bits, parity: 0b00 = None, stopBits: 0 = 1 bit */
#define UART_DATA_8BIT   0x03
#define UART_PARITY_NONE 0x00
#define UART_STOP_1BIT   0x00

/* ============================================================
 *  Comandos recibidos por UART
 * ============================================================ */
#define START_BUTTON    'S'
#define PAUSE_BUTTON    'P'
#define MOVE_UP         'K'
#define MOVE_DOWN       'J'
#define SHOW_RANKINGS   'R'
#define CLEAR_RANKINGS  'C'

static volatile uint8_t  buzzerMsLeft = 0;
static volatile uint32_t msCounter    = 0;

/* ============================================================
 *  ADC
 * ============================================================ */
void adc_init(void) {
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
}

uint16_t adc_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF8) | (ch & 0x07);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

uint16_t get_random_seed(void) {
    uint16_t seed = 0;
    for (uint8_t i = 0; i < 16; i++) {
        seed = (seed << 1) | (adc_read(0) & 0x01);
    }
    return seed;
}

/* ============================================================
 *  Buzzer
 * ============================================================ */
void buzzerInit(void) {
    DDRB |= (1 << BUZZER_PIN);
    PORTB &= ~(1 << BUZZER_PIN);
}

void activateBuzzer(void) {
    PORTB |= (1 << BUZZER_PIN);
    buzzerMsLeft = BUZZER_ON_MS;
}

/* ============================================================
 *  Timer 0 -> 1 ms
 * ============================================================ */
ISR(TIMER0_COMPA_vect) {
    msCounter++;

    if (buzzerMsLeft > 0) {
        buzzerMsLeft--;
        if (buzzerMsLeft == 0) {
            PORTB &= ~(1 << BUZZER_PIN);
        }
    }
}

void timerInit(void) {
    TCCR0A = (1 << WGM01);              // CTC
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
    OCR0A  = 249;                       // 1 kHz
    TIMSK0 = (1 << OCIE0A);
    sei();
}

uint32_t millis(void) {
    uint32_t ms;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ms = msCounter;
    }
    return ms;
}

/* ============================================================
 *  Rankings
 * ============================================================*/
void showRanking(uint8_t id) {
    lcd_goto_xy(0, 0);
    lcd_clear();

    uint8_t score = get_ranking(id);
    char string_ranking[32];  // adjust size as needed

    snprintf(string_ranking, sizeof(string_ranking),
             "%u. Puntaje = %u", id + 1, score);

    lcd_write_string(string_ranking);
}

void hideRankings() {
    lcd_goto_xy(0, 0);
    lcd_clear();
    lcd_write_string("Rocky Road: ");
}

/* ============================================================
 *  main
 * ============================================================ */
int main(void)
{
    adc_init();
    timerInit();
    buzzerInit();

    UART_init(UBRR_VALUE, UART_DATA_8BIT, UART_PARITY_NONE, UART_STOP_1BIT);

    lcd_init();
    lcd_goto_xy(0, 0);
    lcd_write_string("Rocky Road:");

    bool gameRunning = false;
    bool gamePaused  = false;
    bool rankingsVisible = false;
    uint8_t current_ranking = 0;
    uint32_t lastTick = 0;

    char pressed_key = 0;
    char prev_key    = 0;

    while (1) {
        pressed_key = UART_read();

        /* Solo actúa en el flanco */
        if (pressed_key != 0 && pressed_key != prev_key) {
            switch (pressed_key) {
            case START_BUTTON:
                startGame(get_random_seed());
                gameRunning = true;
                gamePaused  = false;
                rankingsVisible = false;
                current_ranking = 0;
                lastTick    = millis();
                drawGame();
                break;

            case PAUSE_BUTTON:
                if (gameRunning) {
                    if (gamePaused) {
                        continueGame();
                        lastTick = millis();
                    } else {
                        pauseGame();
                    }
                    gamePaused = !gamePaused;
                }
                break;

            case MOVE_UP:
                if (!gameRunning && rankingsVisible) {
                    current_ranking--;

                    if (current_ranking >= MAX_ENTRIES) current_ranking = MAX_ENTRIES - 1;
                    showRanking(current_ranking);
                    break;
                }

                if (!gameRunning || gamePaused) break;
                if (movePlayer(0, -1) == 1) {
                    gameRunning = false;
                    drawGame();
                    _delay_ms(100);
                    drawGameOver();
                } else {
                    drawGame();
                }
                break;

            case MOVE_DOWN:
                if (!gameRunning && rankingsVisible) {
                    current_ranking++;
                    
                    if (current_ranking >= MAX_ENTRIES) current_ranking = 0;

                    showRanking(current_ranking);
                    break;
                }

                if (!gameRunning || gamePaused) break;
                if (movePlayer(0, 1) == 1) {
                    gameRunning = false;
                    drawGame();
                    _delay_ms(100);
                    drawGameOver();
                } else {
                    drawGame();
                }
                break;
            
            case SHOW_RANKINGS:
                if (gameRunning) break;
                
                rankingsVisible = !rankingsVisible;

                if (rankingsVisible) {
                    showRanking(current_ranking);
                } else {
                    hideRankings();
                }

                break;

            case CLEAR_RANKINGS:
                clear_rankings();
                break;

            default:
                break;
            }
        }
        prev_key = pressed_key;

        /* --- Tick del juego --- */
        if (gameRunning && !gamePaused) {
            uint32_t now = millis();
            if (now - lastTick >= (uint32_t) getGameTick()) {
                lastTick = now;

                if (tickGame() == 1) {
                    gameRunning = false;
                    drawGame();
                    _delay_ms(100);
                    add_ranking(getFinalScore());
                    drawGameOver(); 

                    char msg[16];
                    snprintf(msg, sizeof(msg), "FINAL: %d\r\n", getFinalScore());
                    UART_string(msg);
                } else {
                    drawGame();

                    if (getScoredOnTick()) {
                        activateBuzzer();

                        char msg[16];
                        snprintf(msg, sizeof(msg), "PUNTOS: %d\r\n", getFinalScore());
                        UART_string(msg);

                        setScoredOnTick(false);
                    }
                }
            }
        }
    }
}
