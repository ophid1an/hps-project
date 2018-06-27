#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "hllpp_omp.h"
#include "xxhash.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

static uint8_t find_leftmost_one_position(uint64_t a, uint8_t offset);

void hllpp_mpi(uint8_t b, uint8_t *registers,
    size_t m, uint32_t *arr, size_t n)
{
    static const size_t arr_elem_len = sizeof *arr;
    size_t hash_mask = sizeof(uint64_t) * CHAR_BIT - b;

    for (size_t i = 0; i < n; ++i) {
        uint64_t hashed = XXH64(arr + i, arr_elem_len, 0ULL);
        uint16_t reg_id = hashed >> hash_mask;
        uint64_t w = hashed & (UINT64_MAX >> b);
        uint8_t lm_one_pos = find_leftmost_one_position(w, b);
        registers[reg_id] = MAX(lm_one_pos, registers[reg_id]);
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
