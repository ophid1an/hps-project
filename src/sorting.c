#include <stdlib.h>
#include "sorting.h"

size_t sort_count_distinct(uint32_t *a, size_t n) {
    sort(a, n);
    return count_sorted_array_distinct(a, n);
}

/* Count distinct items in a sorted array of n uint32_t, pointed to by a. */
size_t count_sorted_array_distinct(uint32_t* a, size_t n)
{
    if (n) {
        size_t cnt = 1;
        for (size_t i = 1; i < n; ++i) {
            if (a[i] != a[i - 1])
                ++cnt;
        }
        return cnt;
    }
    return 0;
}

/* Sort an array of n uint32_t, pointed to by a. */
void sort(uint32_t* a, size_t n)
{
    qsort(a, n, sizeof *a, &compare);
}

/* Comparison function. Receives two generic (void) pointers to the items under comparison. */
int compare(const void* p, const void* q)
{
    int x = *(const uint32_t*)p;
    int y = *(const uint32_t*)q;

    /* Avoid return x - y, which can cause undefined behaviour
       because of signed integer overflow. */
    if (x < y)
        return -1; // Return -1 if you want ascending, 1 if you want descending order.
    else if (x > y)
        return 1; // Return 1 if you want ascending, -1 if you want descending order.

    return 0;
}