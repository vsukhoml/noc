// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_strtol_basic(void) {
    char *end = NULL;
    const char *s;

    TEST_INT_EQ(strtol("0", NULL, 10), 0);
    TEST_INT_EQ(strtol("123", NULL, 10), 123);
    TEST_INT_EQ(strtol("-123", NULL, 10), -123);
    TEST_INT_EQ(strtol("+55", NULL, 10), 55);
    TEST_INT_EQ(strtol(" \t\n42", NULL, 10), 42);

    s = "12ab";
    TEST_INT_EQ(strtol(s, &end, 10), 12);
    TEST_PTR_EQ(end, s + 2);

    // No conversion: endptr == nptr
    s = "abc";
    TEST_INT_EQ(strtol(s, &end, 10), 0);
    TEST_PTR_EQ(end, s);
    s = "   ";
    TEST_INT_EQ(strtol(s, &end, 10), 0);
    TEST_PTR_EQ(end, s);
    s = "-";
    TEST_INT_EQ(strtol(s, &end, 10), 0);
    TEST_PTR_EQ(end, s);
    return is_test_succeed();
}
DECLARE_TEST(test_strtol_basic);

static bool test_strtol_base(void) {
    char *end = NULL;
    const char *s;

    TEST_INT_EQ(strtol("0x1A", NULL, 16), 0x1a);
    TEST_INT_EQ(strtol("1A", NULL, 16), 0x1a);
    TEST_INT_EQ(strtol("0X1a", NULL, 0), 0x1a);
    TEST_INT_EQ(strtol("017", NULL, 0), 017);
    TEST_INT_EQ(strtol("017", NULL, 10), 17);
    TEST_INT_EQ(strtol("101", NULL, 2), 5);
    TEST_INT_EQ(strtol("zz", NULL, 36), (35 * 36) + 35);
    TEST_INT_EQ(strtol("-0x10", NULL, 0), -16);

    // Base 0 with leading 0: octal, stops at '9'
    s = "09";
    TEST_INT_EQ(strtol(s, &end, 0), 0);
    TEST_PTR_EQ(end, s + 1);

    // "0x" without a hex digit parses as "0", endptr at 'x'
    s = "0x";
    TEST_INT_EQ(strtol(s, &end, 16), 0);
    TEST_PTR_EQ(end, s + 1);
    s = "0xg";
    TEST_INT_EQ(strtol(s, &end, 0), 0);
    TEST_PTR_EQ(end, s + 1);

    s = "0";
    TEST_INT_EQ(strtol(s, &end, 0), 0);
    TEST_PTR_EQ(end, s + 1);

    // Invalid bases: no conversion
    s = "123";
    TEST_INT_EQ(strtol(s, &end, 1), 0);
    TEST_PTR_EQ(end, s);
    TEST_INT_EQ(strtol(s, &end, 37), 0);
    TEST_PTR_EQ(end, s);
    TEST_INT_EQ(strtol(s, &end, -5), 0);
    TEST_PTR_EQ(end, s);
    return is_test_succeed();
}
DECLARE_TEST(test_strtol_base);

static bool test_strtol_range(void) {
    errno = 0;
    TEST_INT_EQ(strtol("9223372036854775807", NULL, 10), LONG_MAX);
    TEST_EQ(errno, 0);
    TEST_INT_EQ(strtol("-9223372036854775808", NULL, 10), LONG_MIN);
    TEST_EQ(errno, 0);

    TEST_INT_EQ(strtol("9223372036854775808", NULL, 10), LONG_MAX);
    TEST_EQ(errno, ERANGE);
    errno = 0;
    TEST_INT_EQ(strtol("-9223372036854775809", NULL, 10), LONG_MIN);
    TEST_EQ(errno, ERANGE);
    errno = 0;
    TEST_INT_EQ(strtol("99999999999999999999999999", NULL, 10), LONG_MAX);
    TEST_EQ(errno, ERANGE);
    errno = 0;
    return is_test_succeed();
}
DECLARE_TEST(test_strtol_range);

static bool test_strtoul(void) {
    char *end = NULL;
    const char *s;

    errno = 0;
    TEST_EQ(strtoul("123", NULL, 10), 123);
    TEST_EQ(strtoul("0xffffffffffffffff", NULL, 0), ULONG_MAX);
    TEST_EQ(errno, 0);

    // Minus sign negates in the return type
    TEST_EQ(strtoul("-1", NULL, 10), ULONG_MAX);
    TEST_EQ(errno, 0);

    TEST_EQ(strtoul("18446744073709551615", NULL, 10), ULONG_MAX);
    TEST_EQ(errno, 0);
    TEST_EQ(strtoul("18446744073709551616", NULL, 10), ULONG_MAX);
    TEST_EQ(errno, ERANGE);
    errno = 0;

    s = "77x";
    TEST_EQ(strtoul(s, &end, 8), 077);
    TEST_PTR_EQ(end, s + 2);
    return is_test_succeed();
}
DECLARE_TEST(test_strtoul);

static bool test_strtoll_ull(void) {
    errno = 0;
    TEST_INT_EQ(strtoll("-9223372036854775808", NULL, 10), LLONG_MIN);
    TEST_INT_EQ(strtoll("9223372036854775807", NULL, 10), LLONG_MAX);
    TEST_EQ(errno, 0);
    TEST_INT_EQ(strtoll("-9223372036854775809", NULL, 10), LLONG_MIN);
    TEST_EQ(errno, ERANGE);
    errno = 0;

    TEST_EQ(strtoull("18446744073709551615", NULL, 10), ULLONG_MAX);
    TEST_EQ(errno, 0);
    TEST_EQ(strtoull("28446744073709551615", NULL, 10), ULLONG_MAX);
    TEST_EQ(errno, ERANGE);
    errno = 0;
    return is_test_succeed();
}
DECLARE_TEST(test_strtoll_ull);

static bool test_atol_atoll(void) {
    TEST_INT_EQ(atol("-42"), -42);
    TEST_INT_EQ(atol("2147483647"), 2147483647L);
    TEST_INT_EQ(atoll("1099511627776"), 1099511627776LL);
    TEST_INT_EQ(atoll("-1099511627776"), -1099511627776LL);
    return is_test_succeed();
}
DECLARE_TEST(test_atol_atoll);
