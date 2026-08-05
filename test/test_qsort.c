// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static int cmp_int(const void *a, const void *b) {
    const int x = *(const int *)a;
    const int y = *(const int *)b;
    return (x > y) - (x < y);
}

static bool is_sorted(const int *v, size_t n) {
    for (size_t i = 1; i < n; i++)
        if (v[i - 1] > v[i]) return false;
    return true;
}

static bool test_qsort_int(void) {
    int v[] = {5, -3, 7, 7, 0, 42, -100, 3, 3, 1};
    qsort(v, 10, sizeof(v[0]), cmp_int);
    TEST_TRUE(is_sorted(v, 10));
    TEST_INT_EQ(v[0], -100);
    TEST_INT_EQ(v[9], 42);

    int rev[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    qsort(rev, 8, sizeof(int), cmp_int);
    TEST_TRUE(is_sorted(rev, 8));

    int sorted[5] = {1, 2, 3, 4, 5};
    qsort(sorted, 5, sizeof(int), cmp_int);
    TEST_TRUE(is_sorted(sorted, 5));

    int same[6] = {9, 9, 9, 9, 9, 9};
    qsort(same, 6, sizeof(int), cmp_int);
    TEST_TRUE(is_sorted(same, 6));

    int one[] = {7};
    qsort(one, 1, sizeof(int), cmp_int);
    TEST_INT_EQ(one[0], 7);

    // Degenerate calls must be no-ops, not crashes
    qsort(one, 0, sizeof(int), cmp_int);
    qsort(NULL, 0, sizeof(int), cmp_int);
    TEST_INT_EQ(one[0], 7);
    return is_test_succeed();
}
DECLARE_TEST(test_qsort_int);

struct rec {
    int key;
    int a;
    int b;
};

static int cmp_rec(const void *a, const void *b) {
    return cmp_int(&((const struct rec *)a)->key,
                   &((const struct rec *)b)->key);
}

static bool test_qsort_struct(void) {
    struct rec v[4] = {
        {.key = 3, .a = 30, .b = 300},
        {.key = 1, .a = 10, .b = 100},
        {.key = 4, .a = 40, .b = 400},
        {.key = 2, .a = 20, .b = 200},
    };
    qsort(v, 4, sizeof(v[0]), cmp_rec);
    for (int i = 0; i < 4; i++) {
        // Whole records must move together
        TEST_INT_EQ(v[i].key, i + 1);
        TEST_INT_EQ(v[i].a, (i + 1) * 10);
        TEST_INT_EQ(v[i].b, (i + 1) * 100);
    }
    return is_test_succeed();
}
DECLARE_TEST(test_qsort_struct);

static bool test_bsearch(void) {
    static const int v[] = {-5, 0, 3, 7, 9, 12, 100};
    const size_t n = sizeof(v) / sizeof(v[0]);

    for (size_t i = 0; i < n; i++) {
        const int key = v[i];
        TEST_PTR_EQ(bsearch(&key, v, n, sizeof(int), cmp_int), &v[i]);
    }

    static const int missing[] = {-6, 1, 8, 101};
    for (size_t i = 0; i < 4; i++)
        TEST_PTR_NULL(bsearch(&missing[i], v, n, sizeof(int), cmp_int));

    const int k = 5;
    TEST_PTR_NULL(bsearch(&k, v, 0, sizeof(int), cmp_int));
    TEST_PTR_NULL(bsearch(&k, NULL, 0, sizeof(int), cmp_int));
    return is_test_succeed();
}
DECLARE_TEST(test_bsearch);
