// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <noc/parse.h>
#include <noc/slice.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "noc_internal/common.h"
#include "test_common.h"

static bool test_slice_ctor(void) {
    uint8_t buf[16];

    noc_slice_t s = noc_slice(buf, sizeof(buf));
    TEST_PTR_EQ(s.ptr, buf);
    TEST_EQ(s.len, 16);
    TEST_FALSE(noc_slice_is_empty(noc_slice_as_cslice(s)));

    // NULL pointer forces the empty slice regardless of len
    noc_slice_t z = noc_slice(NULL, 100);
    TEST_PTR_NULL(z.ptr);
    TEST_EQ(z.len, 0);
    TEST_TRUE(noc_slice_is_empty(noc_slice_as_cslice(z)));

    noc_cslice_t lit = NOC_CSLICE_STR("hello");
    TEST_EQ(lit.len, 5);
    TEST_EQ(lit.ptr[0], 'h');

    noc_cslice_t cs = noc_cslice_from_str("hello");
    TEST_EQ(cs.len, 5);
    TEST_TRUE(noc_slice_is_empty(noc_cslice_from_str("")));

    uint8_t arr[7];
    TEST_EQ(NOC_SLICE_ARR(arr).len, 7);
    TEST_EQ(NOC_CSLICE_ARR(arr).len, 7);
    return is_test_succeed();
}
DECLARE_TEST(test_slice_ctor);

static bool test_subslice(void) {
    uint8_t buf[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    noc_slice_t s = NOC_SLICE_ARR(buf);
    noc_slice_t sub;

    TEST_TRUE(noc_subslice_checked(s, 2, 5, &sub));
    TEST_PTR_EQ(sub.ptr, &buf[2]);
    TEST_EQ(sub.len, 5);

    // Empty subslice one-past-the-end is valid (Rust: &s[10..10])
    TEST_TRUE(noc_subslice_checked(s, 10, 0, &sub));
    TEST_EQ(sub.len, 0);

    TEST_FALSE(noc_subslice_checked(s, 11, 0, &sub));
    TEST_FALSE(noc_subslice_checked(s, 5, 6, &sub));
    TEST_FALSE(noc_subslice_checked(s, 0, 11, &sub));
    TEST_PTR_NULL(sub.ptr);
    TEST_EQ(sub.len, 0);

    // start + len wrapping around must be rejected
    TEST_FALSE(noc_subslice_checked(s, 8, SIZE_MAX, &sub));

    noc_cslice_t c = noc_slice_as_cslice(s);
    noc_cslice_t csub;
    TEST_TRUE(noc_csubslice_checked(c, 4, 3, &csub));
    TEST_PTR_EQ(csub.ptr, &buf[4]);
    TEST_EQ(csub.ptr[0], 4);

    // Trapping variant on the happy path
    csub = noc_csubslice(c, 4, 3);
    TEST_PTR_EQ(csub.ptr, &buf[4]);
    TEST_EQ(csub.len, 3);
    noc_slice_t msub = noc_subslice(s, 0, 10);
    TEST_PTR_EQ(msub.ptr, buf);
    return is_test_succeed();
}
DECLARE_TEST(test_subslice);

static bool test_parse_uint(void) {
    uint64_t v = 0;
    uint32_t f = 0;

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("123"), 10, &v, &f), 3);
    TEST_EQ(v, 123);
    TEST_EQ(f, 0);

    // Length-bounded: must not read past the slice
    TEST_EQ(noc_parse_uint(noc_cslice("12345", 3), 10, &v, &f), 3);
    TEST_EQ(v, 123);

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR(" -7f"), 16, &v, &f), 4);
    TEST_EQ(v, 0x7f);
    TEST_TRUE(f & NOC_PARSE_NEGATIVE);

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("0x40"), 0, &v, &f), 4);
    TEST_EQ(v, 0x40);

    // Slice ends inside the 0x prefix: parses the leading "0"
    TEST_EQ(noc_parse_uint(noc_cslice("0x40", 2), 0, &v, &f), 1);
    TEST_EQ(v, 0);

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("18446744073709551615"), 10, &v, &f),
            20);
    TEST_EQ(v, UINT64_MAX);
    TEST_EQ(f, 0);

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("18446744073709551616"), 10, &v, &f),
            20);
    TEST_EQ(v, UINT64_MAX);
    TEST_TRUE(f & NOC_PARSE_OVERFLOW);

    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("xyz"), 10, &v, &f), 0);
    TEST_EQ(v, 0);
    TEST_EQ(noc_parse_uint(NOC_CSLICE_STR("-"), 10, &v, &f), 0);
    TEST_EQ(noc_parse_uint(noc_cslice(NULL, 0), 10, &v, &f), 0);
    return is_test_succeed();
}
DECLARE_TEST(test_parse_uint);
