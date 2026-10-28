#include "Ranking.h"

void shift_scores(int from_address) {
    for (int i = MAX_ENTRIES - 1; i > from_address; i--) {
        EEPROM_update(i, EEPROM_read(i - 1));
    }
}

void add_ranking(uint8_t score) {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (EEPROM_read(i) >= score) continue;

        shift_scores(i);
        EEPROM_update(i, score);
        return;
    }
}

void clear_rankings(void) {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        EEPROM_update(i, 0);
    }
}

uint8_t get_ranking(uint8_t id) {
    if (id >= MAX_ENTRIES) return 0;

    return EEPROM_read(id);
}
