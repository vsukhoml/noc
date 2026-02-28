// Copyright 2024 Vadim Sukhomlinov
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((weak)) void __assert_fail(const char *expr, const char *file,
                                         int line, const char *func) {
    if (func) {
        printf("Assertion failed: %s, file %s, line %d, function %s\n", expr,
               file, line, func);
    } else {
        printf("Assertion failed: %s, file %s, line %d\n", expr, file, line);
    }
    abort();
}