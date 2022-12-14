// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_strlen(void) {
    TEST_EQ(strlen(NULL), 0);
    TEST_EQ(strlen(""), 0);
    TEST_EQ(strlen("1"), 1);
    TEST_EQ(strlen("12"), 2);
    TEST_EQ(strlen("abcdefghijklmnoprqstuvwxyz"), 26);
    return is_test_succeed();
}
DECLARE_TEST(test_strlen);

static bool bench_strlen(void) {
    char s[256];
    for (size_t i = 0; i < 256; i++) {
        s[i] = 0;
        strlen(s);
        s[i] = 32;
    }
    return true;
}
DECLARE_BENCH(bench_strlen);

static bool test_strcmp(void) {
    TEST_EQ(strcmp(NULL, NULL), 0);
    TEST_EQ(strcmp(NULL, ""), 0);
    TEST_EQ(strcmp("", NULL), 0);
    TEST_EQ(strcmp("", ""), 0);
    TEST_EQ(strcmp("azx", "azx"), 0);
    TEST_INT_GT(strcmp("a", NULL), 0);
    TEST_INT_GT(strcmp("a", ""), 0);
    TEST_INT_GT(strcmp("a", "1"), 0);
    TEST_INT_GT(strcmp("a", "12"), 0);
    TEST_INT_LT(strcmp("", "a"), 0);
    TEST_INT_LT(strcmp("a", "b"), 0);
    TEST_INT_LT(strcmp("12", "a"), 0);

    return is_test_succeed();
}
DECLARE_TEST(test_strcmp);

static bool test_strncmp(void) {
    TEST_EQ(strncmp(NULL, NULL, 0), 0);
    TEST_EQ(strncmp(NULL, NULL, 1), 0);
    TEST_EQ(strncmp(NULL, "", 2), 0);
    TEST_EQ(strncmp("", NULL, 2), 0);
    TEST_EQ(strncmp("", "", 2), 0);
    TEST_EQ(strncmp("azx", "azx", 3), 0);
    TEST_EQ(strncmp("azx", "azx", 4), 0);
    TEST_EQ(strncmp("azx", "azx", 5), 0);
    TEST_INT_GT(strncmp("a", NULL, 2), 0);
    TEST_INT_GT(strncmp("a", "", 2), 0);
    TEST_INT_GT(strncmp("a", "1", 2), 0);
    TEST_INT_GT(strncmp("a", "12", 2), 0);
    TEST_INT_LT(strncmp("", "a", 2), 0);
    TEST_INT_LT(strncmp("a", "b", 2), 0);
    TEST_INT_LT(strncmp("12", "a", 2), 0);
    TEST_EQ(strncmp("azx345", "azx346", 5), 0);
    TEST_INT_LT(strncmp("azx345", "azx346", 6), 0);
    return is_test_succeed();
}
DECLARE_TEST(test_strncmp);

static bool test_memcmp(void) {
    TEST_EQ(memcmp(NULL, NULL, 0), 0);
    TEST_EQ(memcmp(NULL, NULL, 1), 0);
    TEST_EQ(memcmp(NULL, "", 2), 0);
    TEST_EQ(memcmp("", NULL, 2), 0);
    TEST_EQ(memcmp("", "", 2), 0);
    TEST_EQ(memcmp("azx", "azx", 3), 0);
    TEST_EQ(memcmp("azx", "azx", 4), 0);
    TEST_EQ(memcmp("azx", "azx", 5), 0);
    TEST_INT_GT(memcmp("a", NULL, 2), 0);
    TEST_INT_GT(memcmp("a", "", 2), 0);
    TEST_INT_GT(memcmp("a", "1", 2), 0);
    TEST_INT_GT(memcmp("a", "12", 2), 0);
    TEST_INT_LT(memcmp("", "a", 2), 0);
    TEST_INT_LT(memcmp("a", "b", 2), 0);
    TEST_INT_LT(memcmp("12", "ab", 2), 0);
    TEST_EQ(memcmp("azx345", "azx346", 5), 0);
    TEST_INT_LT(memcmp("azx345", "azx346", 6), 0);
    return is_test_succeed();
}
DECLARE_TEST(test_memcmp);
