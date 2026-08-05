// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "noc_internal/common.h"

div_t div(int numer, int denom) {
    const div_t r = {.quot = numer / denom, .rem = numer % denom};
    return r;
}

ldiv_t ldiv(long int numer, long int denom) {
    const ldiv_t r = {.quot = numer / denom, .rem = numer % denom};
    return r;
}

#if USE_PLATFORM_64BIT_DIV

lldiv_t lldiv(long long int numer, long long int denom) {
    const lldiv_t r = {.quot = numer / denom, .rem = numer % denom};
    return r;
}

#else

lldiv_t lldiv(long long int numer, long long int denom) {
    const bool nneg = numer < 0;
    const bool dneg = denom < 0;
    const uint64_t un = nneg ? (0ULL - (uint64_t)numer) : (uint64_t)numer;
    const uint64_t ud = dneg ? (0ULL - (uint64_t)denom) : (uint64_t)denom;
    uint64_t ur = 0;
    const uint64_t uq = udivmod64(un, ud, &ur);
    lldiv_t r;
    // C11 requires truncation toward zero: negate via unsigned wrap.
    r.quot = (nneg != dneg) ? (long long int)(0ULL - uq) : (long long int)uq;
    r.rem = nneg ? (long long int)(0ULL - ur) : (long long int)ur;
    return r;
}

#endif
