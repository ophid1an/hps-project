#ifndef HLL_H
#define HLL_H

#include <stdint.h>
#include <stddef.h>

double hll(uint32_t* arr, size_t n, uint8_t b);
static uint8_t find_leftmost_one_position(uint32_t a, uint8_t offset);

#endif