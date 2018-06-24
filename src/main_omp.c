#include <float.h>
#include <omp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hllpp_omp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct result {
    double estimate;
    double perc_error;
    double time_spent;
    double time_spent_one_thread;
};

static void calc(struct result *res, uint32_t *arr, size_t n, uint8_t b,
    uint32_t cnt, uint8_t n_threads);
static void print_results(const struct result *res, uint8_t p, uint8_t b, unsigned seed,
    uint8_t run, uint8_t n_threads, FILE *fptr, uint8_t first_call);

int main(int argc, char *argv[])
{
    static const unsigned seed = 1;
    static const char *filename = "results.csv";

    static uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    static uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]
    static uint8_t runs = 1;
    static uint8_t n_threads = 0;

    if (argc >= 2) {
        p = strtoul(argv[1], NULL, 10);
    }

    if (argc >= 3) {
        b = strtoul(argv[2], NULL, 10);
    }

    if (argc >= 4) {
        runs = strtoul(argv[3], NULL, 10);
    }

    if (argc >= 5) {
        filename = argv[4];
    }

    if (argc >= 6) {
        n_threads = strtoul(argv[5], NULL, 10);
    }

    uint8_t max_threads = omp_get_num_procs();

    FILE *fptr = fopen(filename, "w");
    if (fptr == NULL) {
        printf("Error opening file!\n");
        return (EXIT_FAILURE);
    }

    const size_t n = 1UL << p;

    uint32_t *arr = malloc(sizeof *arr * n);
    if (arr == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        return EXIT_FAILURE;
    }

    // Fill array with random integers
    const uint32_t mask = 1UL * n + (n - 1UL);
    srand(seed);

    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() & mask;
    }
    printf("Array of length %zu filled with integers from [0 .. %u].\n", n, mask);

    // Distinct precalculated counts for p [0..30], seed = 1, mask = 1UL * n + (n - 1UL)
    size_t cnts[] = { 1, 2, 4, 8, 14, 25, 50, 104, 206, 394, 800, 1609,
        3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900,
        1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810,
        211235547, 422476956, 844963071 };

    size_t cnt = cnts[p];

    // Print distinct elements number of array
    printf("Number of distinct elements: %zu\n", cnt);
    printf("Percentage: %.3f\n", cnt * 100.0 / n);

    struct result res;
    res.time_spent_one_thread = -1.0;

    // Find approximation of distinct items with HyperLogLog++ using OpenMP
    printf("\nHyperLogLog++ using OpenMP\n");
    uint8_t first_call_to_print = 1;
    for (uint8_t r = 1; r <= runs; ++r) {
        uint8_t current_thread = 1;
        if (n_threads != 0) {
            current_thread = n_threads;
            max_threads = n_threads;
        }
        for (; current_thread <= max_threads; ++current_thread) {
            printf("\nRun %u, using %u thread(s).\n", r, current_thread);

            calc(&res, arr, n, b, cnt, current_thread);
            print_results(&res, p, b, seed, r, current_thread, fptr, first_call_to_print);
            first_call_to_print = 0;
        }
    }

    fclose(fptr);
    free(arr);

    return 0;
}

static void calc(struct result *res, uint32_t *arr, size_t n, uint8_t b,
    uint32_t cnt, uint8_t n_threads)
{
    double begin = omp_get_wtime();

    res->estimate = hllpp_omp(arr, n, b, n_threads);

    double end = omp_get_wtime();

    res->time_spent = end - begin;
    if (n_threads == 1)
        res->time_spent_one_thread = res->time_spent;
    res->perc_error = ABS((cnt - res->estimate) * 100 / cnt);
}

static void print_results(const struct result *res, uint8_t p, uint8_t b, unsigned seed,
    uint8_t run, uint8_t n_threads, FILE *fptr, uint8_t first_call)
{
    double speedup = res->time_spent_one_thread / res->time_spent;
    double efficiency = speedup / n_threads;
    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    printf("Time in seconds: %.3f\n", res->time_spent);
    printf("Speedup: %.3f\n", speedup);
    printf("Efficiency: %.3f\n", efficiency);
    if (first_call == 1) {
        fprintf(fptr, "Array length,Registers,Seed,Run,Threads,Time,Percent error\n");
    }
    fprintf(fptr, "%u,%u,%u,%u,%u,%.*g,%f\n", p, b, seed, run, n_threads, DBL_DIG, res->time_spent, res->perc_error);
}
