#include <omp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hllpp_omp.h"
#include "sorting.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct result {
    double estimate;
    double perc_error;
    double time_spent;
    double time_spent_one_thread;
};

static size_t fill_and_count_distinct(uint32_t *arr, uint8_t p, size_t n,
    uint32_t mask, unsigned seed);
static void calc(struct result *res, double (*ptf)(uint32_t *, size_t, uint8_t, uint8_t),
    uint32_t *arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt, uint8_t n_threads);
static void print_results(const struct result *res, uint8_t n_threads);

int main(int argc, char *argv[])
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

    const uint8_t n_threads = omp_get_num_procs();
    const size_t n = 1UL << p;

    uint32_t *arr = malloc(sizeof *arr * n);
    if (arr == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        return EXIT_FAILURE;
    }

    uint32_t mask = 1UL * n + (n - 1UL);

    size_t cnt = fill_and_count_distinct(arr, p, n, mask, seed);

    struct result res;
    res.time_spent_one_thread = -1.0;

    // Find approximation of distinct items with HyperLogLog++ using OpenMP
    printf("\nHyperLogLog++\n");
    for (uint8_t i = 1; i <= n_threads; i++) {
        printf("\nUsing %u thread(s).\n", i);

        calc(&res, hllpp_omp, arr, n, b, measure_time, cnt, i);
        print_results(&res, i);
    }

    free(arr);

    return 0;
}

static size_t fill_and_count_distinct(uint32_t *arr, uint8_t p, size_t n,
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
    if (seed == 1 && mask == 1UL * n + (n - 1UL) && p <= 30) {
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
    printf("Percentage: %.3f\n", cnt * 100.0 / n);

    return cnt;
}

static void calc(struct result *res, double (*ptf)(uint32_t *, size_t, uint8_t, uint8_t),
    uint32_t *arr, size_t n, uint8_t b, uint8_t measure_time, uint32_t cnt, uint8_t n_threads)
{
    res->time_spent = -1.0;

    if (measure_time != 0) {
        double begin = omp_get_wtime();

        res->estimate = (*ptf)(arr, n, b, n_threads);

        double end = omp_get_wtime();

        res->time_spent = end - begin;
        if (n_threads == 1)
            res->time_spent_one_thread = res->time_spent;
        res->perc_error = ABS((cnt - res->estimate) * 100 / cnt);
    } else {
        res->estimate = (*ptf)(arr, n, b, n_threads);
        res->perc_error = ABS((cnt - res->estimate) * 100 / cnt);
    }
}

static void print_results(const struct result *res, uint8_t n_threads)
{
    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    if (res->time_spent >= 0) {
        printf("Time in seconds: %.3f\n", res->time_spent);
        double speedup = res->time_spent_one_thread / res->time_spent;
        printf("Speedup: %.3f\n", speedup);
        printf("Efficieny: %.3f\n", speedup / n_threads);
    }
}
