#ifndef ROCKYROAD_H_
#define ROCKYROAD_H_

#include <stdbool.h>
#include <stdint.h>

/* Simbolos del tablero */
#define PLAYER_SYMBOL    '>'
#define OBSTACLE_SYMBOL  '*'
#define CRASH_SYMBOL     'X'

typedef struct {
    uint8_t x;
    uint8_t y;
} Object;

/* Inicializa / reinicia el estado del juego con una semilla para rand() */
void startGame(int seed);

/* Pausa y reanuda la lógica del juego */
void pauseGame(void);
void continueGame(void);

/*
 * Avanza un tick del juego.
 * Devuelve 1 si el jugador pierde, 0 en caso contrario.
 */
int tickGame(void);

/*
 * Mueve al jugador (dx, dy) y comprueba colisiones.
 * Devuelve 1 si el jugador pierde, 0 en caso contrario.
 */
int movePlayer(int x, int y);

/* Tiempo actual entre ticks en ms (disminuye conforme avanza el juego) */
int getGameTick(void);

/* Puntuación final / actual */
int getFinalScore(void);

/* Bandera que indica si se anotó un punto en el último tick */
bool getScoredOnTick(void);
void setScoredOnTick(bool state);

/* Dibuja el tablero en el LCD (llamar tras cada tick o movimiento) */
void drawGame(void);

/* Dibuja la pantalla de fin de juego con la puntuación final */
void drawGameOver(void);

#endif /* ROCKYROAD_H_ */
