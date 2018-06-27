#ifndef HLLPP_MPI_H
#define HLLPP_MPI_H

#include <stddef.h>
#include <stdint.h>

void hllpp_mpi(uint8_t b, uint8_t *registers,
    size_t m, uint32_t *arr, size_t n);

#endif
