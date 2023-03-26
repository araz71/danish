#ifndef DELAY_H
#define DELAY_H

#include <time.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t get_timestamp();
uint8_t delay_ms(uint64_t ts, uint32_t delay);

/// Atach this function to thread
void* system_tick_simulator(void* args);

#endif // DELAY_H
