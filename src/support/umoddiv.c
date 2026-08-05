// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

// Software 64-bit division for targets without 64-bit hardware division.
// Compiled unconditionally under the _*_soft names so the hosted (x86-64)
// test suite can verify the algorithms against native division; on
// !USE_PLATFORM_64BIT_DIV targets umoddiv32()/udivmod64() alias them.
// Unused copies are dropped from linked images by --gc-sections.

#include <stddef.h>
#include <stdint.h>

#include "noc_internal/common.h"

// Divide (n_hi32:n_lo32) by d, with the top bit of d set and n_hi32 < d so
// the quotient fits 32 bits. Returns ((uint64_t)q << 32) | r: packing
// quotient and remainder into one value hands both back in a register pair
// on 32-bit targets instead of spilling out-parameters to the stack.
// Implements one step of Knuth's long division using 32/16 divisions, which
// need only 32-bit hardware division.
static uint64_t udiv_qrnnd(uint32_t n_hi32, uint32_t n_lo32, uint32_t d) {
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

    return (((uint64_t)((q_hi << 16) | q_lo)) << 32) | r_lo;
}

// Knuth's long division algorithm for unsigned division of 64-bit integer by
// 32-bit divisor. If target doesn't have hardware 64-bit by 32-bit division we
// have to either rely on compiler builtins (__udivdi3) or implement this code
// explicitly. To minimize dependencies provide our own division.
// This implementation is faster than generic 64/64 division.
uint32_t _umoddiv32_soft(uint64_t *n, uint32_t d) {
    uint32_t n_lo32, n_hi32, q_hi, bm;

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
            n_hi32 = fsl32(n_hi32, n_lo32, bm);
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
            const uint32_t n_top = n_hi32 >> (32 - bm);
            n_hi32 = fsl32(n_hi32, n_lo32, bm);
            n_lo32 <<= bm;
            const uint64_t qr = udiv_qrnnd(n_top, n_hi32, d);
            q_hi = (uint32_t)(qr >> 32);
            n_hi32 = (uint32_t)qr;
        }
    }
    const uint64_t qr = udiv_qrnnd(n_hi32, n_lo32, d);
    *n = (((uint64_t)q_hi) << 32) | (uint32_t)(qr >> 32);

    return ((uint32_t)qr) >> bm;
}

// 64/64 division built on the same primitive. Divisors fitting 32 bits take
// the umoddiv32 path; a larger divisor implies the quotient fits 32 bits, so
// one normalized udiv_qrnnd estimate suffices, corrected in the unshifted
// 64-bit domain -- at most two steps by Knuth TAOCP vol.2 4.3.1 Theorem B.
// Only 32-bit and constant-amount shifts: variable 64-bit shifts would lower
// to compiler-rt __ashldi3/__lshrdi3 libcalls on 32-bit targets.
uint64_t _udivmod64_soft(uint64_t n, uint64_t d, uint64_t *rem) {
    const uint32_t d_hi = (uint32_t)(d >> 32);

    if (d_hi == 0) {
        uint64_t q = n;
        *rem = _umoddiv32_soft(&q, (uint32_t)d);
        return q;
    }
    if (n < d) {
        *rem = n;
        return 0;
    }

    // Estimate the quotient from the top 64 bits of (n << s) divided by the
    // top 32 bits of (d << s), with s normalizing d's top bit into bit 63.
    const uint32_t n_hi = (uint32_t)(n >> 32);
    const uint32_t n_lo = (uint32_t)n;
    const uint32_t s = stdc_leading_zerosui(d_hi);  // 0..31
    uint32_t nt_hi, nt_lo, dn;
    if (s == 0) {
        dn = d_hi;
        nt_hi = n_hi;
        nt_lo = n_lo;
    } else {
        dn = fsl32(d_hi, (uint32_t)d, s);
        nt_hi = n_hi >> (32 - s);
        nt_lo = fsl32(n_hi, n_lo, s);
    }

    uint32_t qd;
    if (nt_hi >= dn) {
        // Estimate saturates (Knuth's min(q_est, b-1) case).
        qd = 0xFFFFFFFF;
    } else {
        qd = (uint32_t)(udiv_qrnnd(nt_hi, nt_lo, dn) >> 32);
    }

    // prod = qd * d as a 96-bit (prod_hi : prod_lo) value.
    const uint64_t p_lo = (uint64_t)qd * (uint32_t)d;
    const uint64_t p_mid = (uint64_t)qd * d_hi;
    uint64_t prod_lo = p_lo + (p_mid << 32);
    uint32_t prod_hi = (uint32_t)(p_mid >> 32) + ((prod_lo < p_lo) ? 1 : 0);

    // The estimate is never low; walk it down while qd * d > n.
    while ((prod_hi != 0) || (prod_lo > n)) {
        qd--;
        prod_hi -= (prod_lo < d) ? 1 : 0;
        prod_lo -= d;
    }
    *rem = n - prod_lo;
    return qd;
}

#if !USE_PLATFORM_64BIT_DIV
uint32_t umoddiv32(uint64_t *n, uint32_t d)
    __attribute__((alias("_umoddiv32_soft")));
uint64_t udivmod64(uint64_t n, uint64_t d, uint64_t *rem)
    __attribute__((alias("_udivmod64_soft")));
#endif
