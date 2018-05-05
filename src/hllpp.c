#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "hllpp.h"
#include "xxhash.h"

static uint8_t find_leftmost_one_position(uint64_t a, uint8_t offset);

double hllpp(uint32_t *arr, size_t n, uint8_t b)
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

    size_t arr_elem_len = sizeof *arr;
    size_t hash_mask = sizeof(uint64_t) * CHAR_BIT - b;

    for (size_t i = 0; i < n; ++i) {
        uint64_t hashed = XXH64(arr + i, arr_elem_len, 0ULL);
        uint16_t reg_id = hashed >> hash_mask;
        uint64_t w = hashed & (UINT64_MAX >> b);
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

    if (raw_estimate <= 5 * m) {
        if (zero_registers_card) {
            return m * log((double)m / zero_registers_card);
        } else {
            return raw_estimate;
        }
    } else {
        return raw_estimate;
    }
}

static uint8_t find_leftmost_one_position(uint64_t a, uint8_t offset)
{
    uint8_t cnt = (sizeof a) * CHAR_BIT - offset + 1;
    while (a != 0) {
        a >>= 1;
        --cnt;
    }
    return cnt;
}
