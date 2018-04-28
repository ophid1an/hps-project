#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hll.h"
#include "hllpp.h"
#include "sorting.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct result {
    double estimate;
    double perc_error;
    double time_spent;
};

static size_t fill_and_count_distinct(uint32_t* arr, uint8_t p, size_t n,
    uint32_t mask, unsigned seed);
static void calc(struct result* res, double (*ptf)(uint32_t*, size_t, uint8_t),
    uint32_t* arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt);
static void print_results(const struct result* res);

int main(int argc, char* argv[])
{
    static uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    static uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]
    static unsigned seed = 1;

    if (argc >= 2) {
        p = strtoul(argv[1], NULL, 10);
    }

    if (argc >= 3) {
        b = strtoul(argv[2], NULL, 10);
    }

    if (argc >= 4) {
        seed = strtoul(argv[3], NULL, 10);
    }

    static const uint8_t measure_time = 1;

    const size_t n = 1UL << p;

    uint32_t* arr = malloc(sizeof *arr * n);

    uint32_t mask = 1UL * n + (n - 1UL);

    size_t cnt = fill_and_count_distinct(arr, p, n, mask, seed);

    struct result res;

    // Find approximation of distinct items with HyperLogLog
    printf("\nHyperLogLog\n\n");

    calc(&res, hll, arr, n, b, measure_time, cnt);
    print_results(&res);

    // Find approximation of distinct items with HyperLogLog++
    printf("\nHyperLogLog++\n\n");

    calc(&res, hllpp, arr, n, b, measure_time, cnt);
    print_results(&res);

    free(arr);

    return 0;
}

static size_t fill_and_count_distinct(uint32_t* arr, uint8_t p, size_t n,
    uint32_t mask, unsigned seed)
{
    srand(seed);

    // Fill arr with random values from 0..
    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() & mask;
    }
    printf("Array of length %zu filled with [0 .. %u]!\n", n, mask);

    size_t cnt = 0;

    // Distinct precalculated counts for p [0..30], seed = 1, mask = 1UL * n + (n - 1UL)
    if (seed == 1 && mask == 1UL * n + (n - 1UL) && p <= 30 && p >= 0) {
        printf("Using precalculated values...\n");
        size_t cnts[] = { 1, 2, 4, 8, 14, 25, 50, 104, 206, 394, 800, 1609,
            3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900,
            1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810,
            211235547, 422476956, 844963071 };
        cnt = cnts[p];
    } else {
        printf("Counting distinct elements...\n");
        cnt = sort_count_distinct(arr, n);
    }

    // Print distinct elements number of array
    printf("Number of distinct elements: %zu\n", cnt);
    printf("Ratio %% : %.3f\n", cnt * 100.0 / n);

    return cnt;
}

static void calc(struct result* res, double (*ptf)(uint32_t*, size_t, uint8_t),
    uint32_t* arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt)
{
    res->time_spent = -1.0;

    if (measure_time != 0) {
        clock_t begin = clock();

        res->estimate = (*ptf)(arr, n, b);

        clock_t end = clock();

        res->time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        res->perc_error = ABS((cnt - res->estimate) * 100 / cnt);
    } else {
        res->estimate = (*ptf)(arr, n, b);
        res->perc_error = ABS((cnt - res->estimate) * 100 / cnt);
    }
}

static void print_results(const struct result* res)
{
    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    if (res->time_spent >= 0)
        printf("Time spent: %.3f\n", res->time_spent);
}
