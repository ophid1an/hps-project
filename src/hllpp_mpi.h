#ifndef HLLPP_MPI_H
#define HLLPP_MPI_H

#include <stddef.h>
#include <stdint.h>

void calc_registers(uint8_t b, uint8_t *registers,
    uint32_t *arr, size_t n);

double calc_estimate(uint8_t *registers, size_t m);

#endif
