// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stddef.h>
#include <stdint.h>

#include "noc_internal/common.h"

#if !USE_PLATFORM_64BIT_DIV

// Divide 64-bit (n_hi32<<32 | n_lo32) by d with top bit set.
// Returns q and r (remainder). Implements hard coded Knuth long division
// using set of 32/16 divisions which can be implemented using 32-bit division
// on platforms without support for 64/32 division.
static void udiv_adjusted(uint32_t *q, uint32_t *r, uint32_t n_hi32,
                          uint32_t n_lo32, uint32_t d);

// Knuth's long division algorithm for unsigned division of 64-bit integer by
// 32-bit divisor. If target doesn't have hardware 64-bit by 32-bit division we
// have to either rely on compiler builtins (__udivdi3) or implement this code
// explicitly. To minimize dependencies provide our own division.
// This implementation is faster than generic 64/64 division.
uint32_t umoddiv32(uint64_t *n, uint32_t d) {
    uint32_t n_lo32, n_hi32, q_lo, q_hi, bm;

    n_hi32 = (uint32_t)(*n >> 32);
    n_lo32 = (uint32_t)*n;

    // Use 32-bit hardware division if available. Also fault on div by zero.
    if (d == 0 || n_hi32 == 0) {
        *n = n_lo32 / d;
        return n_lo32 % d;
    }

    bm = stdc_leading_zerosui(d);

    if (d > n_hi32) {  // 0q = nn / 0D
        // Normalize denominator to have top bit set.
        if (bm != 0) {
            d <<= bm;
            n_hi32 = (n_hi32 << bm) | (n_lo32 >> (32 - bm));
            n_lo32 <<= bm;
        }
        q_hi = 0;
    } else {
        // qq = NN / 0d
        if (bm == 0) {
            n_hi32 -= d;
            q_hi = 1;
        } else {
            // Normalize denominator to have top bit set.
            d <<= bm;
            // Some extra bits due to shifting 64-bit value left
            uint32_t n_top = n_hi32 >> (32 - bm);
            n_hi32 = (n_hi32 << bm) | (n_lo32 >> (32 - bm));
            n_lo32 <<= bm;
            udiv_adjusted(&q_hi, &n_hi32, n_top, n_hi32, d);
        }
    }
    udiv_adjusted(&q_lo, &n_lo32, n_hi32, n_lo32, d);
    *n = (((uint64_t)q_hi) << 32) | q_lo;

    return n_lo32 >> bm;
}

static void udiv_adjusted(uint32_t *q, uint32_t *r, uint32_t n_hi32,
                          uint32_t n_lo32, uint32_t d) {
    uint32_t d_hi, d_lo, q_hi, q_lo, r_hi, r_lo, m;

    // Treat 32-bit d as 16-bit digits d_hi | d_lo
    d_hi = d >> 16;
    d_lo = d & 0xffff;

    q_hi = n_hi32 / d_hi;
    r_hi = n_hi32 - q_hi * d_hi;
    m = q_hi * d_lo;
    r_hi = (r_hi << 16) | (n_lo32 >> 16);

    // Fine tune estimate.
    if (r_hi < m) {
        q_hi--;
        r_hi += d;
        if (r_hi >= d)
            if (r_hi < m) {
                q_hi--;
                r_hi += d;
            }
    }
    r_hi -= m;

    // TODO: may be arch-specific code to use `%` if efficient?
    // r_lo = r_hi % d_hi;
    q_lo = r_hi / d_hi;
    r_lo = r_hi - (q_lo * d_hi);
    m = q_lo * d_lo;
    r_lo = (r_lo << 16) | (n_lo32 & 0xffff);

    // Fine tune estimate
    if (r_lo < m) {
        q_lo--;
        r_lo += d;
        if (r_lo >= d)
            if (r_lo < m) {
                q_lo--;
                r_lo += d;
            }
    }
    r_lo -= m;

    *q = (q_hi << 16) | q_lo;
    *r = r_lo;
}

#endif