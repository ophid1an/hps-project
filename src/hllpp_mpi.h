#ifndef HLLPP_MPI_H
#define HLLPP_MPI_H

#include <stddef.h>
#include <stdint.h>

double hllpp_mpi(uint32_t *arr, size_t n, uint8_t end_of_buffer,
    uint8_t b);

#endif
