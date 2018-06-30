#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hllpp_omp.h"
#include "xxhash.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

static uint8_t find_leftmost_one_position(uint64_t a, uint8_t offset);

void calc_registers(uint8_t *registers, uint8_t b,
    uint32_t *arr, size_t n, int numtasks, int taskid)
{
    static const size_t arr_elem_len = sizeof *arr;
    size_t hash_mask = sizeof(uint64_t) * CHAR_BIT - b;

    size_t chunk_size = n / numtasks;
    size_t start_offset = chunk_size * taskid;
    size_t end_offset = start_offset + chunk_size;
    if (taskid == numtasks - 1) {
        end_offset += n % numtasks;
    }

    // printf("\nNumtasks: %d, taskid: %d\n", numtasks, taskid);
    // printf("Chunk size: %zu, Start: %zu, End: %zu\n", chunk_size, start_offset, end_offset);

    for (size_t i = start_offset; i < end_offset; ++i) {
        uint64_t hashed = XXH64(arr + i, arr_elem_len, 0ULL);
        uint16_t reg_id = hashed >> hash_mask;
        uint64_t w = hashed & (UINT64_MAX >> b);
        uint8_t lm_one_pos = find_leftmost_one_position(w, b);
        registers[reg_id] = MAX(lm_one_pos, registers[reg_id]);
    }
}

double calc_estimate(uint8_t *registers, size_t m)
{
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

    uint32_t zero_registers_card = m;
    double sum_of_inverses = 0.0;
    for (size_t i = 0; i < m; i++) {
        if (registers[i] != 0)
            --zero_registers_card;
        sum_of_inverses += 1 / pow(2.0, registers[i]);
    }

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
