/*
 * RockyRoad.c
 *
 * Created: 21/10/2026 03:28:12 p.m.
 * Author: Julian
 */

#include "RockyRoad.h"
#include "LCD.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4
#define MIN_TICK_TIME 100 // Velocidad máxima del juego (ms por tick)

typedef struct {
    Object *data;
    size_t count;
    size_t capacity;
} ObstacleList;

int tickTime = 1000;
int score = 0;
uint8_t xSize;
uint8_t ySize;
uint8_t pause = false;

bool scoredOnTick;

Object player;

ObstacleList obstacles = {0};
ObstacleList obstaclesToRemove = {0};

uint8_t ticksForNextObstacle = 0;

// Prototipos (las funciones se usan antes de ser definidas)
static bool listResize(ObstacleList *list, size_t newCapacity);
bool listAdd(ObstacleList *list, Object obstacle);
bool listRemove(ObstacleList *list, Object obstacle);
void listFree(ObstacleList *list);
void generateNewObstacle(void);

void startGame(int seed) {
    tickTime = 1000;
    ticksForNextObstacle = 0;
    score = 0;
    xSize = 16;
    ySize = 2;
    player.x = 0;
    player.y = 0;
    scoredOnTick = false;
    pause = false;

    // Libera la memoria y deja las listas vacías
    listFree(&obstacles);
    listFree(&obstaclesToRemove);

    srand(seed);
}

void pauseGame() {
    pause = 1;
}

void continueGame() {
    pause = 0;
}

// Devuelve 1 si el jugador pierde, 0 en caso contrario
int tickGame() {
    // 1. Crear nuevo obstáculo
    if (ticksForNextObstacle == 0) {
        generateNewObstacle();
    } else {
        ticksForNextObstacle--;
    }

    // 2. Mover obstáculos y marcar los que salen de la pantalla
    for (size_t i = 0; i < obstacles.count; i++) {
        // Puntero para modificar el obstáculo real y no una copia
        Object *obstacle = &obstacles.data[i];

        // Se comprueba x == 0 ANTES de decrementar para evitar
        // el desbordamiento de uint8_t (0 - 1 = 255)
        if (obstacle->x == 0) {
            listAdd(&obstaclesToRemove, *obstacle);
            score++;
            setScoredOnTick(true);

            // Aumenta la velocidad sin bajar del mínimo
            if (tickTime > MIN_TICK_TIME) {
                tickTime -= 20;
            }
        } else {
            obstacle->x--;
        }
    }

    // 3. Colisión (después de mover, para detectar el choque en el mismo tick)
    for (size_t i = 0; i < obstacles.count; i++) {
        if (obstacles.data[i].x == player.x && obstacles.data[i].y == player.y) {
            return 1; // Jugador pierde
        }
    }

    // 4. Remover obstáculos que salieron de la pantalla
    for (size_t i = 0; i < obstaclesToRemove.count; i++) {
        listRemove(&obstacles, obstaclesToRemove.data[i]);
    }
    // Vacía la lista conservando la memoria reservada para reutilizarla
    obstaclesToRemove.count = 0;

    return 0;
}

void generateNewObstacle(void) {
    int y = rand() % 2;

    Object newObstacle = {xSize, y};
    ticksForNextObstacle = (rand() % 4) + 2; // Entre 2 y 6

    listAdd(&obstacles, newObstacle);
}

int movePlayer(int x, int y) {
    player.y += y;
    player.x += x;

    if (player.y == ySize) player.y = ySize - 1;
    else if (player.y < 0 || player.y > ySize) player.y = 0;

    if (player.x == xSize) player.x = xSize - 1;
    else if (player.x < 0 || player.x > xSize) player.x = 0;

    for (size_t i = 0; i < obstacles.count; i++) {
        if (obstacles.data[i].x == player.x && obstacles.data[i].y == player.y) {
            return 1; // Jugador pierde
        }
    }

    return 0;
}

int getGameTick() {
    return tickTime;
}

int getFinalScore() {
    return score;
}

bool getScoredOnTick() {
    return scoredOnTick;
}

void setScoredOnTick(bool state) {
    scoredOnTick = state;
}

// Dibuja todo el tablero. Llamar después de cada tick o movimiento del jugador.
void drawGame(void) {
    // Buffer con una fila por línea del LCD (+1 para el '\0')
    char screen[2][xSize + 1];

    // 1. Rellena todo con espacios
    for (uint8_t row = 0; row < 2; row++) {
        for (uint8_t col = 0; col < xSize; col++) {
            screen[row][col] = ' ';
        }
        screen[row][xSize] = '\0';
    }

    // 2. Coloca los obstáculos (los que están fuera de la pantalla no se dibujan)
    for (size_t i = 0; i < obstacles.count; i++) {
        Object o = obstacles.data[i];
        if (o.x < xSize && o.x < xSize && o.y < 2) {
            screen[o.y][o.x] = OBSTACLE_SYMBOL;
        }
    }

    // 3. Coloca al jugador encima (si coincide con un obstáculo, muestra el choque)
    if (player.x < xSize && player.y < 2) {
        char *cell = &screen[player.y][player.x];
        *cell = (*cell == OBSTACLE_SYMBOL) ? CRASH_SYMBOL : PLAYER_SYMBOL;
    }

    // 4. Envía cada fila al LCD. No se borra la pantalla para evitar parpadeo
    for (uint8_t row = 0; row < 2; row++) {
        lcd_goto_xy(row, 0);
        lcd_write_string(screen[row]);
    }
}

// Pantalla de fin de juego
void drawGameOver(void) {
    char buffer[8];
    itoa(getFinalScore(), buffer, 10); // Más ligero que sprintf en un AVR
    
    lcd_clear();
    lcd_goto_xy(0, 0);
    lcd_write_string("GAME OVER");
    lcd_goto_xy(1, 0);
    lcd_write_string("Puntos: ");
    lcd_write_string(buffer);
    lcd_write_string(" "); // Borra restos de texto anterior
}

// Cambia la capacidad reservada de la lista. Devuelve false si falla.
static bool listResize(ObstacleList *list, size_t newCapacity) {
    // Se usa un puntero temporal: si realloc falla, list->data sigue siendo válido
    Object *tmp = realloc(list->data, newCapacity * sizeof(Object));
    if (tmp == NULL) return false;
    list->data = tmp;
    list->capacity = newCapacity;
    return true;
}

// Agrega un elemento al final, duplicando la capacidad si está llena
bool listAdd(ObstacleList *list, Object obstacle) {
    if (list->count == list->capacity) {
        size_t newCap = list->capacity ? list->capacity * 2 : INITIAL_CAPACITY;
        if (!listResize(list, newCap)) return false;
    }
    list->data[list->count++] = obstacle;
    return true;
}

// Elimina el primer elemento con las mismas coordenadas. No conserva el orden.
bool listRemove(ObstacleList *list, Object obstacle) {
    for (size_t i = 0; i < list->count; i++) {
        if (list->data[i].x == obstacle.x && list->data[i].y == obstacle.y) {
            // Reemplaza el elemento por el último (O(1))
            list->data[i] = list->data[--list->count];

            // Reduce la capacidad a la mitad si se usa menos de 1/4
            if (list->capacity > INITIAL_CAPACITY &&
                list->count <= list->capacity / 4) {
                listResize(list, list->capacity / 2); // Si falla, no pasa nada
            }
            return true;
        }
    }
    return false; // No se encontró
}

// Libera la memoria y reinicia la lista
void listFree(ObstacleList *list) {
    free(list->data);
    list->data = NULL;
    list->count = list->capacity = 0;
}
