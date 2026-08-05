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

static bool test_strcpy(void) {
    char buf[16];
    memset(buf, 0x55, sizeof(buf));
    TEST_PTR_EQ(strcpy(buf, "hello"), buf);
    TEST_STR_EQ(buf, "hello");
    TEST_EQ(buf[6], 0x55);  // bytes past the terminator untouched
    TEST_PTR_EQ(strcpy(buf, ""), buf);
    TEST_EQ(buf[0], 0);
    return is_test_succeed();
}
DECLARE_TEST(test_strcpy);

static bool test_strncpy(void) {
    char buf[8];
    memset(buf, 0x55, sizeof(buf));
    TEST_PTR_EQ(strncpy(buf, "ab", sizeof(buf)), buf);
    TEST_STR_EQ(buf, "ab");
    TEST_MEMCHK(&buf[2], 0, 6);  // zero-padded to len

    memset(buf, 0x55, sizeof(buf));
    TEST_PTR_EQ(strncpy(buf, "abcdefgh", 4), buf);  // truncation: no NUL
    TEST_MEMCMP(buf, "abcd", 4);
    TEST_EQ(buf[4], 0x55);
    return is_test_succeed();
}
DECLARE_TEST(test_strncpy);

static bool test_strcat(void) {
    char buf[16] = "abc";
    TEST_PTR_EQ(strcat(buf, "def"), buf);
    TEST_STR_EQ(buf, "abcdef");
    TEST_PTR_EQ(strcat(buf, ""), buf);
    TEST_STR_EQ(buf, "abcdef");
    return is_test_succeed();
}
DECLARE_TEST(test_strcat);

static bool test_strncat(void) {
    char buf[16] = "ab";
    TEST_PTR_EQ(strncat(buf, "cdef", 2), buf);
    TEST_STR_EQ(buf, "abcd");  // truncated but always terminated
    TEST_PTR_EQ(strncat(buf, "ef", 8), buf);
    TEST_STR_EQ(buf, "abcdef");
    TEST_PTR_EQ(strncat(buf, "xyz", 0), buf);
    TEST_STR_EQ(buf, "abcdef");
    return is_test_succeed();
}
DECLARE_TEST(test_strncat);

static bool test_strchr_family(void) {
    const char *s = "abcabc";
    TEST_PTR_EQ(strchr(s, 'a'), s);
    TEST_PTR_EQ(strchr(s, 'b'), s + 1);
    TEST_PTR_EQ(strchr(s, 'x'), NULL);
    TEST_PTR_EQ(strchr(s, 0), s + 6);  // terminator is part of the string
    TEST_PTR_EQ(strrchr(s, 'b'), s + 4);
    TEST_PTR_EQ(strrchr(s, 'a'), s + 3);
    TEST_PTR_EQ(strrchr(s, 'x'), NULL);
    TEST_PTR_EQ(strrchr(s, 0), s + 6);
    TEST_PTR_EQ(strrchr(NULL, 'a'), NULL);
    return is_test_succeed();
}
DECLARE_TEST(test_strchr_family);

static bool test_strspn(void) {
    TEST_EQ(strspn("aabbcc", "ab"), 4);
    TEST_EQ(strspn("aabbcc", "abc"), 6);
    TEST_EQ(strspn("xyz", "ab"), 0);
    TEST_EQ(strspn("", "ab"), 0);
    TEST_EQ(strspn("abc", ""), 0);
    TEST_EQ(strspn(NULL, "ab"), 0);
    return is_test_succeed();
}
DECLARE_TEST(test_strspn);

static bool test_strnlen_s(void) {
    TEST_EQ(strnlen_s(NULL, 5), 0);
    TEST_EQ(strnlen_s("abc", 5), 3);
    TEST_EQ(strnlen_s("abcdef", 4), 4);
    TEST_EQ(strnlen_s("", 4), 0);
    return is_test_succeed();
}
DECLARE_TEST(test_strnlen_s);

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
