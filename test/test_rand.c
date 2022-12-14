// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_rand(void) {
    int r[256];
    size_t i, j;

    for (i = 0; i < 256; i++) r[i] = rand();
    for (i = 0; i < 256; i++)
        for (j = i + 1; j < 256; j++)
            if (r[i] == r[j]) TEST_NEQ(r[i], r[j]);

    return is_test_succeed();
}
DECLARE_TEST(test_rand);

static bool bench_rand(void) {
    for (size_t i = 0; i < 1000; i++) {
        rand();
    };

    return true;
}
DECLARE_BENCH(bench_rand);
