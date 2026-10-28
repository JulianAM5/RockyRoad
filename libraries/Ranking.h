#ifndef RANKING_H
#define RANKING_H

#include <stdint.h>
#include "EEPROM.h"

#define MAX_ENTRIES 5

void add_ranking(uint8_t);
void clear_rankings(void);
uint8_t get_ranking(uint8_t);

#endif
