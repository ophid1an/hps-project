#include <float.h>
#include <getopt.h>
#include <omp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hllpp_omp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

struct result {
    double estimate;
    double perc_error;
    double time_spent;
    double time_spent_one_thread;
};

static void print_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t fixed_threads, uint8_t current_n_threads, FILE *fptr, uint8_t first_call);

int main(int argc, char *argv[])
{
    static const unsigned seed = 1;

    static uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    static uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]
    static uint32_t u = 256; // Size of buffer in MiBs
    static uint8_t runs = 1;
    // If fixed_threads is set to 0 apply algorithms using
    // 1 up to omp_get_num_procs() threads
    static uint8_t fixed_threads = 0;
    uint8_t max_threads = omp_get_num_procs();

    // Parse command line options
    static int c;
    while ((c = getopt(argc, argv, "p:b:u:r:t:")) != -1)
        switch (c) {
        case 'p':
            p = strtoul(optarg, NULL, 10);
            break;
        case 'b':
            b = strtoul(optarg, NULL, 10);
            break;
        case 'u':
            u = strtoul(optarg, NULL, 10);
            break;
        case 'r':
            runs = strtoul(optarg, NULL, 10);
            break;
        case 't':
            fixed_threads = strtoul(optarg, NULL, 10);
            break;
        default:
            return EXIT_FAILURE;
        }

    // Name of file to write results to
    static char filename[80];
    snprintf(filename, 80, "results_omp-p%u-b%u-t%u.csv", p, b, fixed_threads);

    FILE *fptr = fopen(filename, "w");
    if (fptr == NULL) {
        printf("Error opening file!\n");
        return EXIT_FAILURE;
    }

    // Number of random integers to generate
    const size_t n = 1UL << p;
    // Mask used for controlling the range of
    // the values of the random integers
    const uint32_t mask = 1UL * n + (n - 1UL);

    // Distinct precalculated counts for p [0..30], seed = 1, mask = 1UL * n + (n - 1UL)
    static const size_t cnts[] = { 1, 2, 4, 8, 14, 25, 50, 104, 206, 394, 800, 1609,
        3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900,
        1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810,
        211235547, 422476956, 844963071 };
    // Real distinct count
    const size_t cnt = cnts[p];

    // Print the number of the distinct elements of array
    printf("Number of distinct elements: %zu\n", cnt);
    printf("Percentage: %.3f\n", cnt * 100.0 / n);

    // Set buffer length equal to the minimum of
    // the specified size and the integer array size
    size_t buf_length = (1UL << 20) * u / sizeof(uint32_t);
    buf_length = MIN(buf_length, n);

    uint32_t *buf = malloc(sizeof *buf * buf_length);
    if (buf == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        return EXIT_FAILURE;
    }

    // Results struct
    struct result res;
    res.estimate = -1.0;
    res.time_spent_one_thread = -1.0;

    // Find approximation of distinct items with HyperLogLog++ using OpenMP
    printf("\nHyperLogLog++ using OpenMP\n");
    uint8_t first_call_to_print = 1;
    size_t last_chunk_size = n % buf_length;
    size_t chunks = n / buf_length;
    if (last_chunk_size != 0) {
        chunks++;
    }

    for (uint8_t run = 1; run <= runs; ++run) {
        uint8_t current_n_threads = 1;
        if (fixed_threads != 0) {
            current_n_threads = fixed_threads;
            max_threads = fixed_threads;
        }
        for (; current_n_threads <= max_threads; ++current_n_threads) {
            printf("\nRun %u, using %u thread(s).\n", run, current_n_threads);

            srand(seed);
            uint8_t end_of_buffer = 0;
            res.time_spent = 0.0;

            for (size_t i = 0; i < chunks; ++i) {
                size_t chunk_size = buf_length;
                if (i == chunks - 1) {
                    end_of_buffer = 1;
                    if (last_chunk_size != 0) {
                        chunk_size = last_chunk_size;
                    }
                }

                // Fill buffer with random 32bit integers thoughtfully
                // (only when needed)
                if (chunks != 1 || (run == 1 && (current_n_threads == 1 || fixed_threads != 0))) {
                    for (size_t j = 0; j < chunk_size; ++j) {
                        buf[j] = rand() & mask;
                    }
                }

                double begin = omp_get_wtime();

                res.estimate = hllpp_omp(buf, chunk_size, end_of_buffer, b, current_n_threads);

                double end = omp_get_wtime();
                res.time_spent += end - begin;
                if (current_n_threads == 1) {
                    res.time_spent_one_thread = res.time_spent;
                }
            }
            res.perc_error = ABS((cnt - res.estimate) * 100 / cnt);

            print_results(&res, p, b, run, fixed_threads, current_n_threads, fptr, first_call_to_print);
            first_call_to_print = 0;
        }
    }

    fclose(fptr);
    free(buf);

    return 0;
}

static void print_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t fixed_threads, uint8_t current_n_threads, FILE *fptr, uint8_t first_call)
{

    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    printf("Time in seconds: %.3f\n", res->time_spent);
    // If number of threads is fixed, don't calculate and print
    // speedup and efficiency since there is no baseline
    if (fixed_threads == 0) {
        double speedup = res->time_spent_one_thread / res->time_spent;
        double efficiency = speedup / current_n_threads;
        printf("Speedup: %.3f\n", speedup);
        printf("Efficiency: %.3f\n", efficiency);
    }
    if (first_call == 1) {
        fprintf(fptr, "Array length,Registers,Run,Threads,Time,Percent error\n");
    }
    fprintf(fptr, "%u,%u,%u,%u,%.*g,%f\n", p, b, run, current_n_threads, DBL_DIG, res->time_spent, res->perc_error);
}
