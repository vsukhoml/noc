// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

extern char _brk_start[];
extern char _brk_end[];
extern char _brk_size[];

// internal function for tests
extern size_t mem_free(void);

static bool test_malloc_simple(void) {
    void *ptrs[10];
    size_t free_mem = mem_free();
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 10; i > 0; i--) free(ptrs[i - 1]);
    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_malloc_simple);

static bool test_malloc_simple2(void) {
    void *ptrs[10];
    size_t free_mem = mem_free();
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 0; i < 10; i++) free(ptrs[i]);
    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_malloc_simple2);

static bool test_malloc_simple3(void) {
    void *ptrs[10];
    size_t free_mem = mem_free();
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 0; i < 10; i += 2) free(ptrs[i]);
    for (size_t i = 1; i < 10; i += 2) free(ptrs[i]);
    TEST_GE(mem_free(), free_mem);
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 1; i < 10; i += 2) free(ptrs[i]);
    for (size_t i = 0; i < 10; i += 2) free(ptrs[i]);
    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_malloc_simple3);

static bool test_malloc_simple4(void) {
    void *ptrs[10];
    size_t free_mem = mem_free();
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 0; i < 10; i += 2) free(ptrs[i]);
    for (size_t i = 0; i < 10; i += 2) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 1; i < 10; i += 2) free(ptrs[i]);
    for (size_t i = 1; i < 10; i += 2) TEST_PTR_NONNULL(ptrs[i] = malloc(10));

    for (size_t i = 0; i < 10; i += 3) free(ptrs[i]);
    for (size_t i = 1; i < 10; i += 3) free(ptrs[i]);
    for (size_t i = 2; i < 10; i += 3) free(ptrs[i]);

    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_malloc_simple4);

static bool test_malloc_simple5(void) {
    void *ptrs[10];
    size_t free_mem = mem_free();
    for (size_t i = 0; i < 10; i++) TEST_PTR_NONNULL(ptrs[i] = malloc(10));
    for (size_t i = 0; i < 10; i++) memset(ptrs[i], i, 10);
    for (size_t i = 2; i < 8; i += 1) free(ptrs[i]);
    for (size_t i = 2; i < 4; i += 1) TEST_PTR_NONNULL(ptrs[i] = malloc(20));
    for (size_t i = 8; i < 10; i++) free(ptrs[i]);
    for (size_t i = 0; i < 2; i++) free(ptrs[i]);
    for (size_t i = 2; i < 4; i++) free(ptrs[i]);

    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_malloc_simple5);

static bool test_realloc1(void) {
    size_t free_mem = mem_free();
    void *ptr1;

    TEST_PTR_NONNULL(ptr1 = malloc(10));
    memset(ptr1, 2, 10);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 30));
    TEST_MEMCHK(ptr1, 2, 10);
    memset(ptr1, 3, 30);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 10));
    TEST_MEMCHK(ptr1, 3, 10);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 40));
    free(ptr1);

    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_realloc1);

static bool test_realloc2(void) {
    size_t free_mem = mem_free();
    void *ptr1, *ptr0;

    TEST_PTR_NONNULL(ptr0 = malloc(10));
    TEST_PTR_NONNULL(ptr1 = malloc(10));
    free(ptr0);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 40));
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 30));
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 10));
    free(ptr1);

    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_realloc2);

static bool test_realloc3(void) {
    size_t free_mem = mem_free();
    void *ptr1, *ptr0, *ptr2;

    TEST_PTR_NONNULL(ptr0 = malloc(10));
    TEST_PTR_NONNULL(ptr1 = malloc(40));
    TEST_PTR_NONNULL(ptr2 = malloc(40));
    free(ptr0);
    memset(ptr1, 5, 40);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 10));
    TEST_MEMCHK(ptr1, 5, 10);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 30));
    memset(ptr1, 7, 30);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 40));
    TEST_MEMCHK(ptr1, 7, 30);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 60));
    memset(ptr1, 9, 60);
    TEST_PTR_NONNULL(ptr1 = realloc(ptr1, 90));
    TEST_MEMCHK(ptr1, 9, 60);
    memset(ptr1, 11, 90);
    free(ptr1);
    free(ptr2);
    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_realloc3);

static bool test_calloc(void) {
    size_t free_mem = mem_free();
    void *ptr1, *ptr0;

    TEST_PTR_NONNULL(ptr0 = calloc(10, 10));
    TEST_PTR_NULL(ptr1 = calloc(0xfffffffff, 0xfffffffff));
    TEST_MEMCHK(ptr0, 0, 100);
    free(ptr0);
    TEST_GE(mem_free(), free_mem);
    return is_test_succeed();
}
DECLARE_TEST(test_calloc);