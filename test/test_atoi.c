// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_atoi(void) {
    TEST_INT_EQ(atoi("0"), 0);
    TEST_INT_EQ(atoi("100"), 100);
    TEST_INT_EQ(atoi("-100"), -100);
    TEST_INT_EQ(atoi("2147483647"), 2147483647);
    TEST_INT_EQ(atoi("-2147483648"), -2147483648);
    return is_test_succeed();
}
DECLARE_TEST(test_atoi);
