// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "noc_internal/common.h"

int atoi(const char *nptr) {
    bool minus = false;
    int result = 0;
    char c = 0;

    while ((c = *nptr++) && isspace(c))
        ;
    if (c == '-') {
        c = *nptr++;
        minus = true;
    }
    while (isdigit(c)) {
        result = result * 10 + (c - '0');
        c = *nptr++;
    }
    return minus ? -result : result;
}
