// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

// Verify the portable software division against native x86-64 division.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static void check32(uint64_t n, uint32_t d) {
    uint64_t q = n;
    const uint32_t r = _umoddiv32_soft(&q, d);
    TEST_EQ(q, n / d);
    TEST_EQ(r, n % d);
}

static bool test_umoddiv32_soft(void) {
    static const uint64_t ns[] = {
        0,
        1,
        9,
        10,
        0xFFFFFFFFULL,
        0x100000000ULL,
        0x100000001ULL,
        1000000007ULL,
        0x123456789ABCDEF0ULL,
        0x8000000000000000ULL,
        0xFFFFFFFF00000001ULL,
        0xFFFFFFFFFFFFFFFFULL,
    };
    static const uint32_t ds[] = {
        1,      2,          3,          10,         16,         0x7FFF,
        0x8000, 0x10001,    0xFFFF,     0x7FFFFFFF, 0x80000000, 0x80000001,
        0xFFFE, 0xFFFFFFFE, 0xFFFFFFFF, 1000000007,
    };
    for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++)
        for (size_t j = 0; j < sizeof(ds) / sizeof(ds[0]); j++)
            check32(ns[i], ds[j]);
    return is_test_succeed();
}
DECLARE_TEST(test_umoddiv32_soft);

static void check64(uint64_t n, uint64_t d) {
    uint64_t r = 0;
    const uint64_t q = _udivmod64_soft(n, d, &r);
    TEST_EQ(q, n / d);
    TEST_EQ(r, n % d);
}

static bool test_udivmod64_soft(void) {
    static const uint64_t ns[] = {
        0,
        1,
        10,
        0xFFFFFFFFULL,
        0x100000000ULL,
        0x100000001ULL,
        0x123456789ABCDEF0ULL,
        0x7FFFFFFF80000000ULL,
        0x8000000000000000ULL,
        0x8000000000000001ULL,
        0xFFFFFFFE00000000ULL,
        0xFFFFFFFF00000001ULL,
        0xFFFFFFFFFFFFFFFEULL,
        0xFFFFFFFFFFFFFFFFULL,
    };
    static const uint64_t ds[] = {
        1,
        3,
        10,
        0xFFFFFFFFULL,
        // 64-bit divisors: the udiv_qrnnd estimate + correction path,
        // including top-bit-set (s == 0) and saturated-estimate cases.
        0x100000000ULL,
        0x100000001ULL,
        0x4000000080000000ULL,
        0x7FFFFFFFFFFFFFFFULL,
        0x8000000000000000ULL,
        0x8000000000000001ULL,
        0xFFFFFFFF00000000ULL,
        0xFFFFFFFF00000001ULL,
        0xFFFFFFFFFFFFFFFFULL,
    };
    for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++)
        for (size_t j = 0; j < sizeof(ds) / sizeof(ds[0]); j++)
            check64(ns[i], ds[j]);
    return is_test_succeed();
}
DECLARE_TEST(test_udivmod64_soft);

static bool bench_umoddiv32_soft(void) {
    volatile uint32_t sink = 0;
    for (uint32_t i = 1; i < 64; i++) {
        uint64_t n = 0xFFFFFFFFFFFFFFFFULL - ((uint64_t)i << 32);
        sink += _umoddiv32_soft(&n, 0x9E3779B9U + i);
        sink += (uint32_t)n;
    }
    (void)sink;
    return true;
}
DECLARE_BENCH(bench_umoddiv32_soft);

static bool bench_udivmod64_soft(void) {
    volatile uint64_t sink = 0;
    for (uint32_t i = 1; i < 64; i++) {
        uint64_t r = 0;
        sink += _udivmod64_soft(0xFFFFFFFFFFFFFFFFULL - i,
                                0x100000000ULL + ((uint64_t)i << 8), &r);
        sink += r;
    }
    (void)sink;
    return true;
}
DECLARE_BENCH(bench_udivmod64_soft);
