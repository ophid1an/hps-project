#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hll.h"
#include "sorting.h"

#define CREATE_ARRAYS 0

int main(void)
{
    static const uint8_t p = 20;
    static const uint8_t b = 14; // b -> [4..16]
    static const unsigned seed = 1;

    const uint32_t n = 1 << p;

    uint32_t* arr = malloc(n * sizeof(uint32_t));

    srand(seed);
    // Fill arr with random values from 0..
    uint32_t mask = 1 * n + (n - 1);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() & mask;
    }
    printf("Array of length %d filled with [0 .. %d]!\n", n, mask);

    // Distinct counts for p [4..30], seed = 1, mask = 1 * n + (n - 1)
    const size_t cnts_length = 27;
    size_t cnts[] = { 14, 25, 50, 104, 206, 394, 800, 1609, 3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900, 1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810, 211235547, 422476956, 844963071 };

    // sort(arr, n);
    // printf("Array sorted!\n");

    // Print distinct elements number of array
    // size_t cnt = count_sorted_array_distinct(arr, n);
    size_t cnt = cnts[p - 4];
    printf("Number of distinct elements: %zu\n", cnt);
    printf("Ratio %% : %.3f\n", cnt * 100.0 / n);

    // Find approximate distinct items with HyperLogLog
    printf("\nHyperLogLog\n\n");
    double estimate = hll(arr, n, b);

    printf("Estimate: %f\n", estimate);
    double error = (estimate - cnt) * 100 / cnt;
    printf("Error %% : %.3f\n", error);

    free(arr);

    return 0;

    // if (CREATE_ARRAYS) {
    //     printf("\n");
    //     size_t cnts_len = 27;
    //     // size_t cnts_len = 17;
    //     size_t cnts[cnts_len];
    //     uint8_t offset = 4;
    //     for (size_t i = 0; i < cnts_len; ++i)
    //         cnts[i] = 0;

    //     for (size_t j = offset; j < cnts_len + offset; ++j) {
    //         uint32_t len = 1 << j;
    //         uint32_t* arr = malloc(len * sizeof(uint32_t));
    //         srand(seed);

    //         // Fill arr with random values from 0..
    //         uint32_t mask = 1 * len + (len - 1);
    //         for (size_t i = 0; i < len; ++i) {
    //             arr[i] = rand() & mask;
    //         }

    //         sort(arr, len);

    //         cnts[j - offset] = count_sorted_array_distinct(arr, len);

    //         free(arr);
    //     }

    //     for (size_t i = 0; i < cnts_len; ++i)
    //         printf("%zu , ", cnts[i]);

    //     printf("\n");
    //     return 0;
    // }
}
