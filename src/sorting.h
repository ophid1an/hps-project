#ifndef SORTING_H
#define SORTING_H

#include <stdint.h>
#include <stddef.h>

void sort(uint32_t* a, size_t n);
int compare(const void* p, const void* q);
size_t count_sorted_array_distinct(uint32_t* a, size_t n);

#endif