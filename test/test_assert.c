// Copyright 2024 Vadim Sukhomlinov
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <assert.h>
#include <stdbool.h>

#include "test_common.h"

static bool test_assert(void) {
    // We only test the success path because a failed assert is marked
    // 'noreturn' and would call abort(), terminating the test runner.
    // Testing the failure path would require setjmp/longjmp, which
    // is currently not implemented in this library.

    assert(1 == 1);
    assert(2 + 2 == 4);

    static_assert(sizeof(char) == 1, "char size is not 1");

    return true;
}

DECLARE_TEST(test_assert);