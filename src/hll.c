#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "hashing.h"
#include "hll.h"
// #include "xxhash.h"

static uint8_t find_leftmost_one_position(uint32_t a, uint8_t offset);

double hll(uint32_t *arr, size_t n, uint8_t b)
{
    uint32_t m = 1UL << b;
    uint8_t *registers = malloc(sizeof *registers * m);
    for (size_t i = 0; i < m; ++i) {
        registers[i] = 0;
    }

    double a = 0.0;

    if (m >= 128) {
        a = 0.7213 / (1.0 + 1.079 / m);
    } else if (m == 64) {
        a = 0.709;
    } else if (m == 32) {
        a = 0.697;
    } else if (m == 16) {
        a = 0.673;
    }

    // size_t s_arr = sizeof *arr;
    size_t hash_mask = sizeof(uint32_t) * CHAR_BIT - b;

    for (size_t i = 0; i < n; ++i) {
        uint32_t hashed = hash_func1(arr[i]);
        // uint32_t hashed = XXH32(arr + i, s_arr, 0UL);
        uint16_t reg_id = hashed >> hash_mask;
        uint32_t w = hashed & (UINT32_MAX >> b);
        uint8_t lm_one_pos = find_leftmost_one_position(w, b);
        registers[reg_id] = lm_one_pos > registers[reg_id] ? lm_one_pos : registers[reg_id];
    }

    uint32_t zero_registers_card = m;
    double sum_of_inverses = 0.0;
    for (size_t i = 0; i < m; i++) {
        if (registers[i] != 0)
            --zero_registers_card;
        sum_of_inverses += 1 / pow(2.0, registers[i]);
    }

    free(registers);

    double raw_estimate = a * m * m / sum_of_inverses;

    if (raw_estimate <= 2.5 * m) {
        if (zero_registers_card) {
            return m * log((double)m / zero_registers_card);
        } else {
            return raw_estimate;
        }
    } else if (raw_estimate <= UINT32_MAX / 30.0) {
        return raw_estimate;
    } else {
        return -(UINT32_MAX * log(1.0 - raw_estimate / UINT32_MAX));
    }
}

static uint8_t find_leftmost_one_position(uint32_t a, uint8_t offset)
{
    uint8_t cnt = (sizeof a) * CHAR_BIT - offset + 1;
    while (a != 0) {
        a >>= 1;
        --cnt;
    }
    return cnt;
}
