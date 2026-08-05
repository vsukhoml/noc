// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_div(void) {
    // C11: truncation toward zero in all sign combinations
    div_t d = div(7, 2);
    TEST_INT_EQ(d.quot, 3);
    TEST_INT_EQ(d.rem, 1);
    d = div(-7, 2);
    TEST_INT_EQ(d.quot, -3);
    TEST_INT_EQ(d.rem, -1);
    d = div(7, -2);
    TEST_INT_EQ(d.quot, -3);
    TEST_INT_EQ(d.rem, 1);
    d = div(-7, -2);
    TEST_INT_EQ(d.quot, 3);
    TEST_INT_EQ(d.rem, -1);
    return is_test_succeed();
}
DECLARE_TEST(test_div);

static bool test_ldiv(void) {
    ldiv_t ld = ldiv(1000000007L, 10L);
    TEST_INT_EQ(ld.quot, 100000000L);
    TEST_INT_EQ(ld.rem, 7L);
    ld = ldiv(-1000000007L, 10L);
    TEST_INT_EQ(ld.quot, -100000000L);
    TEST_INT_EQ(ld.rem, -7L);
    return is_test_succeed();
}
DECLARE_TEST(test_ldiv);

static bool test_lldiv(void) {
    lldiv_t lld = lldiv((1LL << 40) + 3, 10);
    TEST_INT_EQ(lld.quot, 109951162777LL);
    TEST_INT_EQ(lld.rem, 9LL);

    lld = lldiv(-((1LL << 40) + 3), 10);
    TEST_INT_EQ(lld.quot, -109951162777LL);
    TEST_INT_EQ(lld.rem, -9LL);

    lld = lldiv(1LL << 62, 1LL << 31);
    TEST_INT_EQ(lld.quot, 1LL << 31);
    TEST_INT_EQ(lld.rem, 0);

    lld = lldiv(5, 7);
    TEST_INT_EQ(lld.quot, 0);
    TEST_INT_EQ(lld.rem, 5);
    return is_test_succeed();
}
DECLARE_TEST(test_lldiv);
