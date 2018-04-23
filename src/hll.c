#include <limits.h>
#include <math.h>

#include "hll.h"
#include "hashing.h"

double hll(uint32_t* arr, size_t n, uint8_t b)
{
    uint16_t m = 1 << b;
    uint8_t registers[m];
    for (size_t i = 0; i < m; ++i) {
        registers[i] = 0;
    }

    double a = 0.0;

    if (m >= 128) {
        a = 0.7213 / (1 + 1.079 / m);
    } else if (m == 64) {
        a = 0.709;
    } else if (m == 32) {
        a = 0.697;
    } else if (m == 16) {
        a = 0.673;
    }

    for (size_t i = 0; i < n; ++i) {
        uint32_t hashed = hash_func1(arr[i]);
        uint16_t reg_id = hashed >> (32 - b);
        uint32_t w = hashed & (0xFFFFFFFF >> b);
        uint8_t lz = find_leftmost_one_position(w, b);
        registers[reg_id] = lz > registers[reg_id] ? lz : registers[reg_id];
    }

    uint16_t zero_registers_card = m;
    double sum_of_inverses = 0.0;
    for (size_t i = 0; i < m; i++) {
        if (registers[i] != 0)
            --zero_registers_card;
        sum_of_inverses += 1 / pow(2.0, registers[i]);
    }

    double raw_estimate = a * m * m / sum_of_inverses;

    // uint32_t lz_cnt = 0;
    // for (size_t i = 0; i < m; ++i) {
    //     lz_cnt += registers[i];
    // }

    // printf("Value of m: %u\n", m);
    // printf("Value of a: %f\n", a);
    // printf("Registers sum: %u\n", lz_cnt);
    // printf("Sum of inverses: %f\n", sum_of_inverses);
    // printf("Raw estimate: %f\n", raw_estimate);

    if (raw_estimate <= 2.5 * m) {
        // printf("E*<=2.5*mod*m , %u\n", zero_registers_card);
        if (zero_registers_card) {
            return m * log((double)m / zero_registers_card);
        } else {
            return raw_estimate;
        }
    } else if (raw_estimate <= UINT32_MAX / 30.0) {
        // printf("E*<=2^32*(1/30)\n");
        return raw_estimate;
    } else {
        // printf("E*>2^32*(1/30)\n");
        return -(UINT32_MAX * log(1 - raw_estimate / UINT32_MAX));
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