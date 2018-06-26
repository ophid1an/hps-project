#ifndef HLLPP_OMP_H
#define HLLPP_OMP_H

#include <stddef.h>
#include <stdint.h>

double hllpp_omp(uint32_t *arr, size_t n, uint8_t end_of_buffer,
    uint8_t b, uint8_t n_threads);

#endif
