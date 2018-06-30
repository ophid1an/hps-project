#include <float.h>
#include <getopt.h>
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
    // double time_spent_one_thread;
};

static void write_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t current_n_tasks, FILE *fptr, uint8_t first_call);
static void print_results(const struct result *res);

int main(int argc, char *argv[])
{
    // MPI variables
    const int root = 0;
    int numtasks;
    int taskid;

    // Variables used by root task
    static const unsigned seed = 1;

    uint8_t p = 27; // Cardinality of elements 2^p , p -> [4..30]
    uint8_t b = 14; // Cardinality of registers 2^b , b -> [4..16]
    size_t n = 0; // Number of random integers to generate
    size_t cnt = 0; // Actual distinct count of integers

    // Mask used for controlling the range of
    // the values of the random integers
    uint32_t mask = 0;
    uint8_t *root_registers = NULL;

    size_t last_chunk_size = 0;

    FILE *fptr = NULL;
    uint8_t first_call_to_print = 0;

    double begin = 0.0;

    struct result res;
    res.estimate = -1.0;
    // res.time_spent_one_thread = -1.0;

    // Variables used by all
    size_t m = 0; // Number of registers
    uint32_t runs = 1; // Number of runs
    uint8_t *registers = NULL;
    uint32_t *buf = NULL;
    size_t buf_length = 0;
    size_t chunks = 0; // Number of chunks
    size_t chunk_size = 0;

    // MPI initialization
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

        n = 1UL << p;
        m = 1UL << b;

        // In root task allocate space for root registers
        root_registers = malloc(sizeof *root_registers * m);
        if (root_registers == NULL) {
            fprintf(stderr, "Fatal: failed to allocate memory.\n");
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Set buffer length equal to the minimum of
        // the ceiling(specified size / numtasks)
        // and the integer array size
        size_t buf_length_total = (1UL << 20) * u / sizeof(uint32_t);
        buf_length = buf_length_total / numtasks;
        if (buf_length_total % numtasks != 0) {
            ++buf_length;
        }
        buf_length = MIN(buf_length, n);
    }

    // Broadcast m and buf_length
    MPI_Bcast(&m, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);
    MPI_Bcast(&buf_length, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

    // In each task allocate space for registers
    registers = malloc(sizeof *registers * m);
    if (registers == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // In each task allocate space for buffer
    buf = malloc(sizeof *buf * buf_length);
    if (buf == NULL) {
        fprintf(stderr, "Fatal: failed to allocate memory.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

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

        mask = 1UL * n + (n - 1UL);

        // Distinct precalculated counts for p [0..30], seed = 1, mask = 1UL * n + (n - 1UL)
        static const size_t cnts[] = { 1, 2, 4, 8, 14, 25, 50, 104, 206, 394, 800, 1609,
            3194, 6434, 12852, 25733, 51567, 103075, 206331, 412503, 825900,
            1650602, 3300462, 6601586, 13202252, 26403875, 52807680, 105621810,
            211235547, 422476956, 844963071 };
        cnt = cnts[p];

        // Print the number of the distinct elements of array
        printf("Number of distinct elements: %zu\n", cnt);
        printf("Percentage: %.3f\n", cnt * 100.0 / n);

        // Find approximation of distinct items with HyperLogLog++ using Open MPI
        printf("\nHyperLogLog++ using Open MPI\n");
        first_call_to_print = 1;
        last_chunk_size = n % buf_length;
        chunks = n / buf_length;
        if (last_chunk_size != 0) {
            ++chunks;
        }
    }

    // Broadcast runs and chunks
    MPI_Bcast(&runs, 1, MPI_UINT32_T, root, MPI_COMM_WORLD);
    MPI_Bcast(&chunks, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

    for (uint8_t run = 1; run <= runs; ++run) {
        // Zero registers
        for (size_t i = 0; i < m; ++i) {
            registers[i] = 0;
            if (taskid == root) {
                root_registers[i] = 0;
            }
        }

        res.time_spent = 0.0;

        if (taskid == root) {
            printf("\nRun %u, using %d task(s).\n", run, numtasks);

            srand(seed);
            // uint8_t end_of_buffer = 0;
        }

        for (size_t i = 0; i < chunks; ++i) {
            if (taskid == root) {
                chunk_size = buf_length;
                if (i == chunks - 1) {
                    // end_of_buffer = 1;
                    if (last_chunk_size != 0) {
                        chunk_size = last_chunk_size;
                    }
                }

                // Fill buffer with random 32bit integers thoughtfully
                // (only when needed)
                if (run == 1 || chunks != 1) {
                    // printf("\n Filling buffer... \n");
                    for (size_t j = 0; j < chunk_size; ++j) {
                        buf[j] = rand() & mask;
                    }
                }

                begin = MPI_Wtime();
            }

            // Scatter the buffer of root to all tasks
            // Broadcast the buff to all tasks;
            MPI_Bcast(buf, buf_length, MPI_UINT32_T, root, MPI_COMM_WORLD);
            // MPI_Scatter(buf, chunk_size, MPI_UINT32_T, arr, chunk_size, MPI_UINT32_T, root, MPI_COMM_WORLD);
            // MPI_Barrier(MPI_COMM_WORLD);

            // Broadcast chunk size
            MPI_Bcast(&chunk_size, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

            // Calculate registers' values
            calc_registers(registers, b, buf, chunk_size, numtasks, taskid);

            // Reduce registers from all tasks to
            MPI_Reduce(registers, root_registers, m, MPI_UINT8_T, MPI_MAX, root, MPI_COMM_WORLD);

            if (taskid == root) {
                res.estimate = calc_estimate(root_registers, m);

                double end = MPI_Wtime();
                res.time_spent += end - begin;
                // if (current_n_tasks == 1) {
                //     res.time_spent_one_thread = res.time_spent;
                // }
                res.perc_error = ABS((cnt - res.estimate) * 100 / cnt);

                if (i == chunks - 1) {
                    print_results(&res);
                    write_results(&res, p, b, run, numtasks, fptr, first_call_to_print);
                    first_call_to_print = 0;
                }
            }
        }
    }

    free(registers);

    if (taskid == root) {
        fclose(fptr);
        free(root_registers);
        free(buf);
    }

    MPI_Finalize();
    return 0;
}

static void write_results(const struct result *res, uint8_t p, uint8_t b,
    uint8_t run, uint8_t current_n_tasks, FILE *fptr, uint8_t first_call)
{
    if (first_call == 1) {
        fprintf(fptr, "Array length,Registers,Run,Tasks,Time,Percent error\n");
    }
    fprintf(fptr, "%u,%u,%u,%u,%.*g,%f\n",
        p, b, run, current_n_tasks, DBL_DIG, res->time_spent, res->perc_error);
}

static void print_results(const struct result *res)
{
    printf("Estimate: %f\n", res->estimate);
    printf("Percent error: %.3f\n", res->perc_error);
    printf("Time in seconds: %.3f\n", res->time_spent);
    // double speedup = res->time_spent_one_thread / res->time_spent;
    // double efficiency = speedup / current_n_tasks;
    // printf("Speedup: %.3f\n", speedup);
    // printf("Efficiency: %.3f\n", efficiency);
}
