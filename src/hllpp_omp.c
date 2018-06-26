#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "hllpp_omp.h"
#include "xxhash.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

static uint8_t find_leftmost_one_position(uint64_t a, uint8_t offset);

double hllpp_omp(uint32_t *arr, size_t n, uint8_t end_of_buffer,
    uint8_t b, uint8_t n_threads)
{
    uint32_t m = 1UL << b;
    static uint8_t *registers = NULL;
    static uint8_t is_first_chunk = 0;

    if (registers == NULL) {
        registers = malloc(sizeof *registers * m);
        is_first_chunk = 1;
    }

    if (registers == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    if (is_first_chunk == 1) {
        for (size_t i = 0; i < m; ++i) {
            registers[i] = 0;
        }
    }

    static double a;

    if (is_first_chunk == 1) {
        if (m >= 128) {
            a = 0.7213 / (1.0 + 1.079 / m);
        } else if (m == 64) {
            a = 0.709;
        } else if (m == 32) {
            a = 0.697;
        } else if (m == 16) {
            a = 0.673;
        }
    }

    static const size_t arr_elem_len = sizeof *arr;
    size_t hash_mask = sizeof(uint64_t) * CHAR_BIT - b;

    omp_set_num_threads(n_threads);
#pragma omp parallel
    {
        int t_id = omp_get_thread_num();
        size_t batch_size = n / n_threads;
        uint8_t *thread_registers = malloc(sizeof *thread_registers * m);
        if (thread_registers == NULL) {
            fprintf(stderr, "Fatal: failed to allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < m; ++i) {
            thread_registers[i] = 0;
        }

        if (t_id == n_threads - 1) {
            batch_size += n % n_threads;
        }

        for (size_t i = n / n_threads * t_id, j = i + batch_size; i < j; ++i) {
            uint64_t hashed = XXH64(arr + i, arr_elem_len, 0ULL);
            uint16_t reg_id = hashed >> hash_mask;
            uint64_t w = hashed & (UINT64_MAX >> b);
            uint8_t lm_one_pos = find_leftmost_one_position(w, b);
            thread_registers[reg_id] = MAX(lm_one_pos, thread_registers[reg_id]);
        }

#pragma omp critical
        {
            for (size_t i = 0; i < m; ++i) {
                registers[i] = MAX(thread_registers[i], registers[i]);
            }
        }

        free(thread_registers);
    }

    if (end_of_buffer == 1) {
        uint32_t zero_registers_card = m;
        double sum_of_inverses = 0.0;
        for (size_t i = 0; i < m; i++) {
            if (registers[i] != 0)
                --zero_registers_card;
            sum_of_inverses += 1 / pow(2.0, registers[i]);
        }

        free(registers);
        registers = NULL;

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
    } else {
        is_first_chunk = 0;
        return -1.0;
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
