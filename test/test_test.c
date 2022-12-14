// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

// Testing framework itself
static bool $self(void) {
    TEST_EQ(0, 0);
    TEST_EQ(1, 1);
    TEST_NEQ(1, 0);
    TEST_GT(1, 0);
    TEST_LT(0, 1);
    TEST_INT_GT(1, 0);
    TEST_INT_GT(1, -1);
    TEST_INT_GT(0, -1);
    TEST_INT_LT(0, 1);
    TEST_INT_LT(-1, 1);
    TEST_INT_LT(-1, 0);
    TEST_FALSE(false);
    TEST_TRUE(true);
    TEST_TRUE(10);  // unconventional bool
    TEST_INT_EQ(-10, -10);
    TEST_INT_NEQ(-10, 10);
    TEST_PTR_EQ(NULL, NULL);
    TEST_PTR_NEQ(NULL, $self);
    TEST_PTR_NULL(NULL);
    TEST_PTR_NONNULL($self);
    TEST_STR_EQ("abc", "abc");
    TEST_STRN_EQ("abcde", "abc", 3);
    TEST_IN_RANGE(4, 3, 5);
    TEST_NEAR(5, 4, 2);
    TEST_INT_IN_RANGE(-5, -10, -4);
    TEST_INT_LE(-3, -2);
    TEST_INT_LE(-3, -3);
    TEST_INT_GE(-2, -3);
    TEST_INT_GE(-2, -2);
    TEST_MEMCMP("qwerty", "qwerty", 7);
    TEST_MEMCHK("aaaaaaa", 'a', 7);
    return is_test_succeed();
}

DECLARE_TEST($self);