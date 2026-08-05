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

// Shift-subtract 64/64 division. Only lldiv() needs a true 64-bit divisor —
// everything else in noc divides 64-bit by 32-bit via umoddiv32() — so a
// tiny, slow loop beats pulling in compiler-rt's __divdi3.
static uint64_t udivmod64(uint64_t n, uint64_t d, uint64_t *rem) {
    uint64_t q = 0;
    uint64_t r = 0;
    // Constant shift amounts only: variable 64-bit shifts would lower to
    // compiler-rt __ashldi3/__lshrdi3 libcalls on 32-bit targets.
    for (int i = 0; i < 64; i++) {
        r = (r << 1) | (n >> 63);
        n <<= 1;
        q <<= 1;
        if (r >= d) {
            r -= d;
            q |= 1U;
        }
    }
    *rem = r;
    return q;
}

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
