#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hll.h"

int main(int argc, char* argv[])
{
    static uint8_t p = 27;
    static uint8_t b = 14; // b -> [4..16]

    if (argc >= 2) {
        p = (uint8_t)strtoul(argv[1], NULL, 10);
    }

    if (argc >= 3) {
        b = (uint8_t)strtoul(argv[2], NULL, 10);
    }

    static const unsigned seed = 1;

    const size_t n = 1 << p;

    uint32_t* arr;
    arr = malloc(n * sizeof(arr));

    srand(seed);
    // Fill arr with random values from 0..
    uint32_t mask = 1 * n + (n - 1);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() & mask;
    }
    printf("Array of length %zu filled with [0 .. %u]!\n", n, mask);

    // Distinct counts for p [4..30], seed = 1, mask = 1 * n + (n - 1)
    uint32_t cnts[] = { 14, 25, 50, 104, 206, 394, 800, 1609, 3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900, 1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810, 211235547, 422476956, 844963071 };

    // Print distinct elements number of array
    uint32_t cnt = cnts[p - 4];
    printf("Number of distinct elements: %u\n", cnt);
    printf("Ratio %% : %.3f\n", cnt * 100.0 / n);

    // Find approximation of distinct items with HyperLogLog
    printf("\nHyperLogLog\n\n");
    double estimate = hll(arr, n, b);

    printf("Estimate: %f\n", estimate);
    double error = (estimate - cnt) * 100 / cnt;
    printf("Error %% : %.3f\n", error);

    free(arr);

    return 0;
}
