// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

// Buffers used for testing.
uint32_t s_buf[768];
uint32_t s2_buf[768];
uint32_t d_buf[768];

// The 32 bit LFSR whose maximum length feedback polynomial is represented
// as X^32 + X^22 + X^2 + X^1 + 1 will produce 2^32-1 PN sequence.
// This LFSR can be initialized with 0,  but can't be initialized with
// 0xFFFFFFFF
static uint32_t lfsr32(uint32_t seed) {
    uint32_t mask = (-((int32_t)seed >= 0)) & 0xC0000401;
    return (seed << 1) ^ mask;
}

// Testing memory copy/move operations is a bit tricky and requires known
// patterns to be sure that data was copied as expected. Simple patterns
// don't work as logical errors may result in copying same data twice in
// different location or in wrong location.
// Use pseudo-random patterns with provided seed. Exact copy should have
// all elements equal. Wipes should result in most elements not equal.

// Fill buffer with pseudo-random pattern.
static void fill_rand(void *buf, uint32_t seed, size_t len) {
    uint8_t *ptr = (uint8_t *)buf;

    while (len) {
        size_t remaining_seed = sizeof(seed);
        uint32_t c_seed = seed;
        seed = lfsr32(seed);

        while (remaining_seed != 0 && len > 0) {
            *ptr = c_seed & 0xff;
            c_seed >>= 8;
            ptr++;
            len--;
            remaining_seed--;
        };
    }
}

// Count matches with specific pseudo-random pattern.
static size_t count_rand_equal(const void *buf, uint32_t seed, size_t len) {
    size_t count = 0;
    uint8_t *ptr = (uint8_t *)buf;

    while (len) {
        size_t remaining_seed = sizeof(seed);
        uint32_t c_seed = seed;
        seed = lfsr32(seed);

        while (remaining_seed != 0 && len > 0) {
            if (*ptr == (c_seed & 0xff)) count++;
            c_seed >>= 8;
            ptr++;
            len--;
            remaining_seed--;
        };
    }
    return count;
}

static bool memcpy_smoke_test(void) {
    fill_rand(s_buf, 0, sizeof(s_buf));
    fill_rand(d_buf, 10, sizeof(d_buf));
    TEST_EQ(count_rand_equal(s_buf, 0, sizeof(s_buf)), sizeof(s_buf));
    TEST_EQ(count_rand_equal(d_buf, 10, sizeof(d_buf)), sizeof(d_buf));
    TEST_NEQ(count_rand_equal(d_buf, 0, sizeof(d_buf)), sizeof(d_buf));

    // smoke test memcpy() works
    TEST_PTR_EQ(memcpy(d_buf, s_buf, sizeof(d_buf)), ((void *)d_buf));
    TEST_EQ(count_rand_equal(d_buf, 0, sizeof(d_buf)), sizeof(d_buf));

    fill_rand(s_buf, 0, sizeof(s_buf));
    fill_rand(d_buf, 10, sizeof(d_buf));
    TEST_PTR_EQ(memcpy(d_buf, s_buf, 7), ((void *)d_buf));
    TEST_EQ(count_rand_equal(d_buf, 0, 7), 7);

    return is_test_succeed();
}
DECLARE_TEST(memcpy_smoke_test);

// Test various memory sizes, aligned.
static bool memcpy_aligned_test(void) {
    for (size_t i = 1; i <= sizeof(d_buf); i++) {
        // first, clean up
        fill_rand(s_buf, 0x11111111, sizeof(s_buf));
        fill_rand(s_buf, 0x23456789, i);
        fill_rand(d_buf, 0x33333333, i);

        TEST_PTR_EQ(memcpy(d_buf, s_buf, i), d_buf);
        TEST_EQ(count_rand_equal(d_buf, 0x23456789, i), i);
        TEST_MEMCMP(d_buf, s_buf, i);

        d_buf[((i - 1) / sizeof(uint32_t))] ^= 0x01010101;
        TEST_NEQ(memcmp(d_buf, s_buf, i), 0);
    }
    return is_test_succeed();
}
DECLARE_TEST(memcpy_aligned_test);


// Test various memory sizes, unaligned
static bool memcpy_unaligned_test(void) {
    for (size_t sa = 0; sa < sizeof(uintptr_t); sa++)
        for (size_t da = 0; da < sizeof(uintptr_t); da++) {
            uint8_t *d = (uint8_t *)d_buf;
            uint8_t *s = (uint8_t *)s_buf;

            size_t copy_size = sizeof(d_buf) - sa - da;

            // make tests a bit faster for expected byte copies
            if (sa != da) {
                copy_size >>= 2;
            }

            d += da;
            s += sa;
            for (size_t i = 1; i <= copy_size; i += 13) {
                // initialize with specific pseudo-random pattern
                fill_rand(s, 0x11111111, copy_size);
                fill_rand(s, 0x12345678, i);
                fill_rand(d, 0x33333333, i);

                TEST_PTR_EQ(memcpy(d, s, i), d);
                // d should be exactly 's'
                TEST_EQ(count_rand_equal(d, 0x12345678, i), i);

                TEST_INT_EQ(memcmp(d, s, i), 0);

                d[i - 1] ^= 1;
                TEST_INT_NEQ(memcmp(d, s, i), 0);
            }
        }

    return is_test_succeed();
}
DECLARE_TEST(memcpy_unaligned_test);
