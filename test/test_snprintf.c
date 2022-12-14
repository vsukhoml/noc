// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

#define TEST_SNPRINTF(result, format, ...)                                    \
    TEST_EQ(snprintf(s, sizeof(s), format, __VA_ARGS__), sizeof(result) - 1); \
    TEST_STR_EQ(s, result);

#define TEST_SNPRINTF_TRUNC(result, len, format, ...)               \
    TEST_EQ(snprintf(s, sizeof(result), format, __VA_ARGS__), len); \
    TEST_STR_EQ(s, result);

static bool test_snprintf(void) {
    char s[256];

    TEST_SNPRINTF("2", "%d", 2);

    TEST_SNPRINTF("-2", "%d", -2);

    TEST_SNPRINTF("-100", "%d", -100);

    TEST_SNPRINTF("2", "%u", 2);

    TEST_SNPRINTF("3", "%zu", (size_t)3);

    TEST_SNPRINTF("+2", "%+d", 2);

    TEST_SNPRINTF("1abc2", "1%s2", "abc");

    TEST_SNPRINTF("-100 2", "%d %u", -100, 2);

    TEST_SNPRINTF("002", "%03u", 2);

    TEST_SNPRINTF("  2", "%3u", 2);

    TEST_SNPRINTF("A", "%c", 65);

    TEST_SNPRINTF("-127", "%hhd", (char)-127);

    TEST_SNPRINTF("-128", "%hhd", (char)-128);

    TEST_SNPRINTF("11", "%x", 17);

    TEST_SNPRINTF("00010001", "%08x", 65537);

    TEST_SNPRINTF("000177761", "%08o", 65521);

    TEST_SNPRINTF("0005", "%.4d", 5);

    TEST_SNPRINTF("eeeedd0112345678", "%16lx", (uint64_t)0xeeeedd0112345678);

    TEST_SNPRINTF_TRUNC("12345678", 10, "%d", 1234567890);

    TEST_SNPRINTF_TRUNC("123456789", 10, "%d", 1234567890);

    TEST_SNPRINTF("1234567890", "%d", 1234567890);

    // Unsupported format
    TEST_INT_EQ(snprintf(s, sizeof(s), "%z", 1234567890), -1);

    TEST_SNPRINTF("1234567890 499602d2 011145401322 A Hello", "%d %x %o %c %s",
                  1234567890, 1234567890, 1234567890, 'A', "Hello");

    TEST_SNPRINTF("Hel", "%.3s", "Hello");

    TEST_SNPRINTF("  Hel", "%5.3s", "Hello");

    return is_test_succeed();
}
DECLARE_TEST(test_snprintf);
