#include <float.h>
#include <getopt.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hllpp_mpi.h"
#include "mpi.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

struct result {
    double estimate;
    double perc_error;
    double time_spent;
    double time_spent_one_thread;
};

static double calc_estimate(uint8_t *registers, size_t m);
static void print_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t current_n_tasks, FILE *fptr, uint8_t first_call);

int main(int argc, char *argv[])
{
    // Variables used by root task
    static const unsigned seed = 1;

    uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]
    size_t n = 0; // Number of random integers to generate
    size_t cnt = 0; // Actual distinct count of integers
    uint32_t runs = 1; // Number of runs
    uint32_t mask = 0;
    uint32_t *buf = NULL;
    size_t buf_length = 0;

    size_t chunks = 0;
    size_t last_chunk_size = 0;

    FILE *fptr = NULL;
    uint8_t first_call_to_print = 0;

    double end = 0.0;
    double begin = 0.0;

    struct result res;
    res.estimate = -1.0;
    res.time_spent_one_thread = -1.0;

    // Variables used by all
    size_t m = 0; // Number of registers
    uint8_t *registers = NULL;
    size_t arr_length = 0;
    uint32_t *arr = NULL;

    // MPI variables
    const int root = 0;

    int numtasks;
    int taskid;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    if (taskid == root) {
        static uint32_t u = 256; // Size of buffer in MiBs

        // Parse command line options
        static int c;
        while ((c = getopt(argc, argv, "p:b:u:r:")) != -1)
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
            default:
                MPI_Finalize();
                return EXIT_FAILURE;
            }

        size_t n = 1UL << p;

        // Set buffer length equal to the minimum of
        // the ** half ** specified size and the integer array size
        buf_length = (1UL << 19) * u / sizeof(uint32_t);
        buf_length = MIN(buf_length, n);

        buf = malloc(sizeof *buf * buf_length);
        if (buf == NULL) {
            fprintf(stderr, "Fatal: failed to allocate memory.\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Calculate maximum length of array for each task
        // (buf_length / numtasks)
        arr_length = buf_length / numtasks;
        if (buf_length % numtasks != 0) {
            arr_length++;
        }
    }

    // Broadcast variables needed by every task
    MPI_Bcast(&m, 1, MPI_UINT8_T, root, MPI_COMM_WORLD);
    MPI_Bcast(&arr_length, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

    // In each task allocate space for registers
    registers = malloc(sizeof *registers * m);
    if (registers == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Zero registers
    for (int i = 0; i < m; ++i) {
        registers[i] = 0;
    }

    // In each task allocate space for array
    arr = malloc(sizeof *arr * arr_length);
    if (arr == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (taskid == root) {
        // Name of file to write results to
        static char filename[80];
        snprintf(filename, 80, "results_mpi-p%u-b%u.csv", p, b);

        fptr = fopen(filename, "w");
        if (fptr == NULL) {
            printf("Error opening file!\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Mask used for controlling the range of
        // the values of the random integers
        uint32_t mask = 1UL * n + (n - 1UL);

        // Distinct precalculated counts for p [0..30], seed = 1, mask = 1UL * n + (n - 1UL)
        static const size_t cnts[] = { 1, 2, 4, 8, 14, 25, 50, 104, 206, 394, 800, 1609,
            3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900,
            1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810,
            211235547, 422476956, 844963071 };
        size_t cnt = cnts[p];

        // Print the number of the distinct elements of array
        printf("Number of distinct elements: %zu\n", cnt);
        printf("Percentage: %.3f\n", cnt * 100.0 / n);

        // Find approximation of distinct items with HyperLogLog++ using Open MPI
        printf("\nHyperLogLog++ using Open MPI\n");
        first_call_to_print = 1;
        last_chunk_size = n % buf_length;
        chunks = n / buf_length;
        if (last_chunk_size != 0) {
            chunks++;
        }
    }

    for (uint8_t run = 1; run <= runs; ++run) {
        for (uint8_t current_n_tasks = 1; current_n_tasks <= numtasks; ++current_n_tasks) {
            if (taskid == root) {
                printf("\nRun %u, using %u task(s).\n", run, current_n_tasks);

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
                    if (chunks != 1 || (run == 1 && current_n_tasks == 1)) {
                        for (size_t j = 0; j < chunk_size; ++j) {
                            buf[j] = rand() & mask;
                        }
                    }

                    begin = MPI_Wtime();
                }
            }

            // Scatter the buffer of root to all tasks
            MPI_Scatter(buf, arr_length, MPI_UINT32_T, arr, arr_length, MPI_UINT32_T, root, MPI_COMM_WORLD);

            // Calculate registers' values
            hllpp_mpi(b, registers, m, arr, arr_length);

            // Reduce registers from all tasks to
            MPI_Reduce(registers, registers, m, MPI_UINT8_T, MPI_MAX, root, MPI_COMM_WORLD);

            if (taskid == root) {
                double estimate = calc_estimate(registers, m);

                double end = MPI_Wtime();
                res.time_spent += end - begin;
                if (current_n_tasks == 1) {
                    res.time_spent_one_thread = res.time_spent;
                }
                res.perc_error = ABS((cnt - res.estimate) * 100 / cnt);

                print_results(&res, p, b, run, current_n_tasks, fptr, first_call_to_print);
                first_call_to_print = 0;
            }
        }

        fclose(fptr);
        free(registers);
        free(buf);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

static double calc_estimate(uint8_t *registers, size_t m)
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

static void print_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t current_n_tasks, FILE *fptr, uint8_t first_call)
{

    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    printf("Time in seconds: %.3f\n", res->time_spent);
    double speedup = res->time_spent_one_thread / res->time_spent;
    double efficiency = speedup / current_n_tasks;
    printf("Speedup: %.3f\n", speedup);
    printf("Efficiency: %.3f\n", efficiency);
    if (first_call == 1) {
        fprintf(fptr, "Array length,Registers,Run,Tasks,Time,Percent error\n");
    }
    fprintf(fptr, "%u,%u,%u,%u,%.*g,%f\n", p, b, run, current_n_tasks, DBL_DIG, res->time_spent, res->perc_error);
}
