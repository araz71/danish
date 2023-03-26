#include "delay.h"

uint64_t system_tick = 0;

uint64_t get_timestamp() {
    return system_tick;
}

uint8_t delay_ms(uint64_t ts, uint32_t delay) {
    if ((system_tick - ts) >= delay)
        return 1;

    return 0;
}

void* system_tick_simulator(void* args) {
    clock_t current_tick = clock();

    while (1) {
        if ((clock() - current_tick) >= 1000) {
            current_tick = clock();
            system_tick++;
        }
    }
}
