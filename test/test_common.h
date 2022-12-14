// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Test result flags
struct test_flags {
    // Expect results to be unequal/false, instead of equal
    unsigned int invert : 1;

    // Compared values are signed
    unsigned int sign : 1;

    // Expect got to be true
    unsigned int is_true : 1;

    // Compare less than / greater than instead of equality
    unsigned int lt : 1;
    unsigned int gt : 1;

    // Range; expect=min, expect2=max (both inclusive)
    unsigned int range : 1;

    // Near value; expect=value, expect2=tolerance
    unsigned int near : 1;

    // Compared values are pointers
    unsigned int ptr : 1;

    // Compared values are null-terminated strings, expect2=length
    unsigned int str : 1;

    // Compared values are memory buffers, expect2=length
    unsigned int memcmp : 1;

    // Compared values are memory buffer and value, expect2=length
    unsigned int memchk : 1;

    // silence compiler warning
    unsigned int _pad : 21;
};

/// @brief Add a test result.
///
/// @param lineno        Line of test
/// @param cond          Condition tested, as string
/// @param got           Actual result
/// @param expect        Expected result
/// @param expect2       Expected result (second half)
/// @param flags         Test flags
void add_test_result(uint32_t lineno, const char* cond, uintptr_t got,
                     uintptr_t expect, uintptr_t expect2,
                     struct test_flags flags);

// Return true if there were no errors since last invocation of this function.
bool is_test_succeed(void);

// Explicit test failure
#define TEST_FAIL(cond_str)                          \
    add_test_result(__LINE__, cond_str, false, 0, 0, \
                    (struct test_flags){.is_true = 1})

// Check if two things are equal
#define TEST_EQ(got, expect)                                       \
    add_test_result(__LINE__, #got " == " #expect, got, expect, 0, \
                    (struct test_flags){})
/// Check if two things are unequal.  Note that you should NOT use this to check
/// if a function returns error; you should check for the specific error you
/// expect.
#define TEST_NEQ(got, expect)                                      \
    add_test_result(__LINE__, #got " != " #expect, got, expect, 0, \
                    (struct test_flags){.invert = 1})

#define TEST_INT_EQ(got, expect)                                   \
    add_test_result(__LINE__, #got " == " #expect, got, expect, 0, \
                    (struct test_flags){.sign = 1})

#define TEST_INT_NEQ(got, expect)                                  \
    add_test_result(__LINE__, #got " == " #expect, got, expect, 0, \
                    (struct test_flags){.sign = 1, .invert = 1})

// Check less than / greater than
#define TEST_LT(got, expect)                                      \
    add_test_result(__LINE__, #got " < " #expect, got, expect, 0, \
                    (struct test_flags){.lt = 1})
#define TEST_GT(got, expect)                                      \
    add_test_result(__LINE__, #got " > " #expect, got, expect, 0, \
                    (struct test_flags){.gt = 1})
#define TEST_INT_LT(got, expect)                                  \
    add_test_result(__LINE__, #got " < " #expect, got, expect, 0, \
                    (struct test_flags){.sign = 1, .lt = 1})
#define TEST_INT_GT(got, expect)                                  \
    add_test_result(__LINE__, #got " > " #expect, got, expect, 0, \
                    (struct test_flags){.sign = 1, .gt = 1})

#define TEST_LE(got, expect)                                       \
    add_test_result(__LINE__, #got " <= " #expect, got, expect, 0, \
                    (struct test_flags){.gt = 1, .invert = 1})
#define TEST_GE(got, expect)                                       \
    add_test_result(__LINE__, #got " >= " #expect, got, expect, 0, \
                    (struct test_flags){.lt = 1, .invert = 1})
#define TEST_INT_LE(got, expect)                                   \
    add_test_result(__LINE__, #got " <= " #expect, got, expect, 0, \
                    (struct test_flags){.gt = 1, .sign = 1, .invert = 1})
#define TEST_INT_GE(got, expect)                                   \
    add_test_result(__LINE__, #got " >= " #expect, got, expect, 0, \
                    (struct test_flags){.lt = 1, .sign = 1, .invert = 1})

// Check value in range (inclusive)
#define TEST_IN_RANGE(got, expect_min, expect_max)                           \
    add_test_result(__LINE__, #got " in (" #expect_min ", " #expect_max ")", \
                    got, expect_min, expect_max,                             \
                    (struct test_flags){.range = 1});
#define TEST_INT_IN_RANGE(got, expect_min, expect_max)                       \
    add_test_result(__LINE__, #got " in (" #expect_min ", " #expect_max ")", \
                    got, expect_min, expect_max,                             \
                    (struct test_flags){.range = 1, .sign = 1});

// Check value with tolerance
#define TEST_NEAR(got, expect, tolerance)                                  \
    add_test_result(__LINE__, #got " == " #expect " +/- " #tolerance, got, \
                    expect, tolerance, (struct test_flags){.near = 1});
#define TEST_INT_NEAR(got, expect, tolerance)                              \
    add_test_result(__LINE__, #got " == " #expect " +/- " #tolerance, got, \
                    expect, tolerance,                                     \
                    (struct test_flags){.near = 1, .sign = 1});

// Check if two pointers are equal
#define TEST_PTR_EQ(got, expect)                                     \
    add_test_result(__LINE__, #got " == " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), 0, (struct test_flags){.ptr = 1})

// Check if two pointers are unequal
#define TEST_PTR_NEQ(got, expect)                                    \
    add_test_result(__LINE__, #got " != " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), 0,                          \
                    (struct test_flags){.ptr = 1, .invert = 1})

// Check if a pointer is NULL
#define TEST_PTR_NULL(got)                                       \
    add_test_result(__LINE__, #got " == NULL", (uintptr_t)(got), \
                    (uintptr_t)NULL, 0, (struct test_flags){.ptr = 1})
#define TEST_PTR_NONNULL(got)                                    \
    add_test_result(__LINE__, #got " != NULL", (uintptr_t)(got), \
                    (uintptr_t)NULL, 0,                          \
                    (struct test_flags){.ptr = 1, .invert = 1})

// Check something is zero
#define TEST_ZERO(got) \
    add_test_result(__LINE__, #got, got, 0, 0, (struct test_flags){})

// Check something is true.
#define TEST_TRUE(got)                         \
    add_test_result(__LINE__, #got, got, 0, 0, \
                    (struct test_flags){.is_true = 1})

// Check something is false.
#define TEST_FALSE(got)                               \
    add_test_result(__LINE__, "not " #got, got, 0, 0, \
                    (struct test_flags){.is_true = 1, .invert = 1})

// Check if two strings are equal, like strcmp().
#define TEST_STR_EQ(got, expect)                                     \
    add_test_result(__LINE__, #got " == " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), -1, (struct test_flags){.str = 1})

// Check if two strings are equal in the first len chars, like strncmp().
#define TEST_STRN_EQ(got, expect, len)                               \
    add_test_result(__LINE__, #got " == " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), len, (struct test_flags){.str = 1})

// Check if two memory buffers are equal, like memcmp().
#define TEST_MEMCMP(got, expect, len)                                \
    add_test_result(__LINE__, #got " == " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), len,                        \
                    (struct test_flags){.memcmp = 1})

// Check if memory buffer is set to value, like memchk().
#define TEST_MEMCHK(got, expect, len)                                \
    add_test_result(__LINE__, #got " == " #expect, (uintptr_t)(got), \
                    (uintptr_t)(expect), len,                        \
                    (struct test_flags){.memchk = 1})

struct test_case {
    // Test case name. Case-insensitive.
    const char* name;
    // Implementation for the test case.
    bool (*handler)(void);
};

// This macro takes all possible args and discards the ones we don't use.
#define DECLARE_TEST(NAME)                                         \
    static const char __testcase_##NAME[] = #NAME;                 \
    const struct test_case __test_##NAME                           \
        __attribute__((section(".rodata.tests." #NAME), used)) = { \
            .name = __testcase_##NAME,                             \
            .handler = NAME,                                       \
    }

#define DECLARE_BENCH(NAME)                                        \
    static const char __testcase_##NAME[] = #NAME;                 \
    const struct test_case __bench_##NAME                          \
        __attribute__((section(".rodata.bench." #NAME), used)) = { \
            .name = __testcase_##NAME,                             \
            .handler = NAME,                                       \
    }
