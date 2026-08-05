// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

// Integer parsing: noc_parse_uint() is the native slice-based core; the
// C-standard strto*/ato* functions are thin clamping wrappers around it.

#include <errno.h>
#include <limits.h>
#include <noc/parse.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "noc_internal/common.h"

// Value of a digit in bases up to 36, or ~0U when not a digit.
static unsigned int digit_value(uint8_t c) {
    if (isdigit((int)c)) return (unsigned int)c - (unsigned int)'0';
    c |= 0x20U;  // lowercase
    if ((c >= (uint8_t)'a') && (c <= (uint8_t)'z'))
        return ((unsigned int)c - (unsigned int)'a') + 10U;
    return ~0U;
}

size_t noc_parse_uint(noc_cslice_t in, unsigned int base,
                      uint64_t *restrict value, uint32_t *restrict flags) {
    const uint8_t *s = in.ptr;
    const size_t n = in.len;
    size_t i = 0;
    uint32_t f = 0;

    *value = 0;
    *flags = 0;
    if ((base == 1U) || (base > 36U)) return 0;

    while ((i < n) && isspace((int)s[i])) i++;

    if ((i < n) && ((s[i] == (uint8_t)'-') || (s[i] == (uint8_t)'+'))) {
        if (s[i] == (uint8_t)'-') f |= NOC_PARSE_NEGATIVE;
        i++;
    }

    // 0x/0X prefix: consumed only when a hex digit follows, so "0x" alone
    // still parses as "0" with one byte of digits consumed.
    if (((base == 0U) || (base == 16U)) && ((n - i) >= 3U) &&
        (s[i] == (uint8_t)'0') && ((s[i + 1U] | 0x20U) == (uint8_t)'x') &&
        isxdigit((int)s[i + 2U])) {
        i += 2U;
        base = 16U;
    }
    if (base == 0U) base = ((i < n) && (s[i] == (uint8_t)'0')) ? 8U : 10U;

    // Saturation threshold: acc may grow past limit only when the next digit
    // is small enough. One 64/32 division per call (umoddiv32 on soft-div
    // targets), none per digit.
    uint64_t limit = UINT64_MAX;
    const uint32_t limit_digit = umoddiv32(&limit, base);

    uint64_t acc = 0;
    bool any = false;
    bool overflow = false;

    while (i < n) {
        const unsigned int d = digit_value(s[i]);
        if (d >= base) break;
        if ((acc > limit) || ((acc == limit) && (d > limit_digit))) {
            overflow = true;  // keep consuming digits per strtol() contract
        } else {
            acc = (acc * base) + d;
        }
        any = true;
        i++;
    }

    if (!any) return 0;
    if (overflow) {
        acc = UINT64_MAX;
        f |= NOC_PARSE_OVERFLOW;
    }
    *value = acc;
    *flags = f;
    return i;
}

// View a NUL-terminated string as a slice reaching to the end of the address
// space: the parser stops at the first byte that cannot be part of a number,
// and NUL never can, so it never reads past the terminator.
static noc_cslice_t cslice_from_cstr(const char *s) {
    return noc_cslice(s, (size_t)(PLATFORM_MAX_ADDR - (uintptr_t)s));
}

static long long int strtox_signed(const char *restrict nptr,
                                   char **restrict endptr, int base,
                                   uint64_t max_pos) {
    uint64_t v = 0;
    uint32_t f = 0;
    const size_t consumed =
        noc_parse_uint(cslice_from_cstr(nptr), (unsigned int)base, &v, &f);
    if (endptr != NULL) *endptr = (char *)nptr + consumed;

    const bool neg = (f & NOC_PARSE_NEGATIVE) != 0U;
    const uint64_t lim = neg ? (max_pos + 1U) : max_pos;
    if (((f & NOC_PARSE_OVERFLOW) != 0U) || (v > lim)) {
        errno = ERANGE;
        v = lim;
    }
    // For the negative extreme (v == max_pos + 1) the unsigned negation
    // converts to exactly LLONG_MIN-style two's complement minimum.
    return neg ? (long long int)(0ULL - v) : (long long int)v;
}

static unsigned long long int strtox_unsigned(const char *restrict nptr,
                                              char **restrict endptr, int base,
                                              uint64_t max_val) {
    uint64_t v = 0;
    uint32_t f = 0;
    const size_t consumed =
        noc_parse_uint(cslice_from_cstr(nptr), (unsigned int)base, &v, &f);
    if (endptr != NULL) *endptr = (char *)nptr + consumed;

    if (((f & NOC_PARSE_OVERFLOW) != 0U) || (v > max_val)) {
        errno = ERANGE;
        return max_val;
    }
    // Minus sign negates in the return type (wraps modulo type width).
    return ((f & NOC_PARSE_NEGATIVE) != 0U) ? (0ULL - v) : v;
}

long int strtol(const char *restrict nptr, char **restrict endptr, int base) {
    const long long int r = strtox_signed(nptr, endptr, base, LONG_MAX);
    return (long int)r;
}

long long int strtoll(const char *restrict nptr, char **restrict endptr,
                      int base) {
    return strtox_signed(nptr, endptr, base, LLONG_MAX);
}

unsigned long int strtoul(const char *restrict nptr, char **restrict endptr,
                          int base) {
    const unsigned long long int r =
        strtox_unsigned(nptr, endptr, base, ULONG_MAX);
    return (unsigned long int)r;
}

unsigned long long int strtoull(const char *restrict nptr,
                                char **restrict endptr, int base) {
    return strtox_unsigned(nptr, endptr, base, ULLONG_MAX);
}

int atoi(const char *nptr) {
    const long int r = strtol(nptr, NULL, 10);
    return (int)r;
}

long int atol(const char *nptr) { return strtol(nptr, NULL, 10); }

long long int atoll(const char *nptr) { return strtoll(nptr, NULL, 10); }
