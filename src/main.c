#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hll.h"
#include "hllpp.h"

#define ABS(x) ((x)<0 ? -(x) : (x))

static struct result calc(double (*ptf)(uint32_t *, size_t, uint8_t), uint32_t *arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt);
static void print_results(struct result res);

struct result {
    double estimate;
    double abs_error;
    double time_spent;
};

int main(int argc, char *argv[])
{
    static uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    static uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]

    if (argc >= 2) {
        p = strtoul(argv[1], NULL, 10);
    }

    if (argc >= 3) {
        b = strtoul(argv[2], NULL, 10);
    }

    static const unsigned seed = 1;

    static const uint8_t measure_time = 1;

    const size_t n = 1UL << p;

    uint32_t *arr = malloc(sizeof *arr * n);

    srand(seed);

    // Fill arr with random values from 0..
    uint32_t mask = 1UL * n + (n - 1UL);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() & mask;
    }
    printf("Array of length %zu filled with [0 .. %u]!\n", n, mask);

    // Distinct counts for p [4..30], seed = 1, mask = 1UL * n + (n - 1UL)
    uint32_t cnts[] = { 14, 25, 50, 104, 206, 394, 800, 1609, 3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900, 1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810, 211235547, 422476956, 844963071 };

    // Print distinct elements number of array
    uint32_t cnt = cnts[p - 4U];
    printf("Number of distinct elements: %u\n", cnt);
    printf("Ratio %% : %.3f\n", cnt * 100.0 / n);

    // Find approximation of distinct items with HyperLogLog
    printf("\nHyperLogLog\n\n");

    print_results(calc(hll, arr, n, b, measure_time, cnt));

    // Find approximation of distinct items with HyperLogLog++
    printf("\nHyperLogLog++\n\n");

    print_results(calc(hllpp, arr, n, b, measure_time, cnt));

    free(arr);

    return 0;
}

static struct result calc(double (*ptf)(uint32_t *, size_t, uint8_t), uint32_t *arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt) {
    struct result res;
    res.time_spent = -1.0;
   
    if (measure_time != 0) {
        clock_t begin = clock();

        res.estimate = (*ptf)(arr, n, b);

        clock_t end = clock();

        res.time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        res.abs_error = ABS((res.estimate - cnt) * 100 / cnt);
    } else {
        res.estimate = (*ptf)(arr, n, b);
        res.abs_error = ABS((res.estimate - cnt) * 100 / cnt);
    }
    return res;
}

static void print_results(struct result res) {
    printf("Estimate: %f\n", res.estimate);
    printf("Absolute error %% : %.3f\n", res.abs_error);
    if (res.time_spent >= 0)
        printf("Time spent: %.3f\n", res.time_spent);
}
