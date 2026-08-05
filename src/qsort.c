// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

// qsort() as heapsort: in-place, no recursion (bounded stack on small
// targets), guaranteed O(n log n) with no adversarial worst case.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static void byte_swap(uint8_t *a, uint8_t *b, size_t size) {
    while (size--) {
        const uint8_t t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *)) {
    uint8_t *b = (uint8_t *)base;

    if ((b == NULL) || (nmemb < 2U) || (size == 0U) || (compar == NULL)) return;

    size_t start = nmemb / 2U;  // heapify phase while start > 0
    size_t end = nmemb;         // extraction phase shrinks the heap
    while (end > 1U) {
        if (start > 0U) {
            start--;
        } else {
            end--;
            byte_swap(b, b + (end * size), size);
        }
        // Sift element at `start` down the heap of length `end`.
        size_t root = start;
        for (;;) {
            size_t child = (2U * root) + 1U;
            if (child >= end) break;
            if (((child + 1U) < end) &&
                (compar(b + (child * size), b + ((child + 1U) * size)) < 0))
                child++;
            if (compar(b + (root * size), b + (child * size)) >= 0) break;
            byte_swap(b + (root * size), b + (child * size), size);
            root = child;
        }
    }
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *)) {
    const uint8_t *lo = (const uint8_t *)base;

    if ((lo == NULL) || (size == 0U) || (compar == NULL)) return NULL;

    while (nmemb > 0U) {
        const size_t mid = nmemb / 2U;
        const uint8_t *p = lo + (mid * size);
        const int c = compar(key, p);
        if (c == 0) return (void *)(uintptr_t)p;
        if (c > 0) {
            lo = p + size;
            nmemb -= mid + 1U;
        } else {
            nmemb = mid;
        }
    }
    return NULL;
}
