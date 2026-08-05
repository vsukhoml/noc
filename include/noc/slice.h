// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file noc/slice.h
/// @brief Byte slices: pointer + length pairs mirroring Rust `&mut [u8]` /
/// `&[u8]`.
///
/// This is the native noc API surface: functions take explicit
/// (pointer, length) pairs instead of NUL-terminated strings or unbounded
/// pointers, and bounds violations are detected instead of undefined.
/// The layout matches what Rust's `core::slice::from_raw_parts{,_mut}`
/// expects, so a `#[repr(C)]` twin struct binds to it directly.
///
/// Invariant: `ptr` may be NULL only when `len == 0`. Constructors enforce
/// this; code hand-building slices must preserve it.

#ifndef NOC_SLICE_H
#define NOC_SLICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Mutable byte slice (`&mut [u8]`).
typedef struct noc_slice {
    uint8_t *ptr;
    size_t len;
} noc_slice_t;

/// Immutable byte slice (`&[u8]`).
typedef struct noc_cslice {
    const uint8_t *ptr;
    size_t len;
} noc_cslice_t;

/// Bounds-violation handler shared with the FORTIFY-style *_chk functions.
/// The weak default traps; a platform may override it.
void __chk_fail(void);

/// @brief Make a mutable slice. NULL `ptr` yields an empty slice.
static inline noc_slice_t noc_slice(void *ptr, size_t len) {
    noc_slice_t s;
    s.ptr = (uint8_t *)ptr;
    s.len = (ptr == NULL) ? 0U : len;
    return s;
}

/// @brief Make an immutable slice. NULL `ptr` yields an empty slice.
static inline noc_cslice_t noc_cslice(const void *ptr, size_t len) {
    noc_cslice_t s;
    s.ptr = (const uint8_t *)ptr;
    s.len = (ptr == NULL) ? 0U : len;
    return s;
}

/// @brief Reborrow a mutable slice as immutable (Rust: `&*s`).
static inline noc_cslice_t noc_slice_as_cslice(noc_slice_t s) {
    noc_cslice_t c;
    c.ptr = s.ptr;
    c.len = s.len;
    return c;
}

/// @brief Slice over a NUL-terminated string, excluding the terminator.
static inline noc_cslice_t noc_cslice_from_str(const char *str) {
    return noc_cslice(str, strlen(str));
}

static inline bool noc_slice_is_empty(noc_cslice_t s) { return s.len == 0U; }

/// Whole-array slice; ARR must be an actual array, not a pointer.
#define NOC_SLICE_ARR(ARR) noc_slice((ARR), sizeof(ARR))
#define NOC_CSLICE_ARR(ARR) noc_cslice((ARR), sizeof(ARR))
/// Slice over a string literal, excluding the terminating NUL.
#define NOC_CSLICE_STR(LIT) noc_cslice((LIT), sizeof(LIT) - 1U)

/// @brief Checked subslice [start, start+len) — Rust `get(start..start+len)`.
/// @return true and *out set on success; false and *out empty when the range
/// is out of bounds.
static inline bool noc_csubslice_checked(noc_cslice_t s, size_t start,
                                         size_t len,
                                         noc_cslice_t *restrict out) {
    if ((start > s.len) || (len > (s.len - start))) {
        out->ptr = NULL;
        out->len = 0U;
        return false;
    }
    out->ptr = s.ptr + start;
    out->len = len;
    return true;
}

static inline bool noc_subslice_checked(noc_slice_t s, size_t start, size_t len,
                                        noc_slice_t *restrict out) {
    if ((start > s.len) || (len > (s.len - start))) {
        out->ptr = NULL;
        out->len = 0U;
        return false;
    }
    out->ptr = s.ptr + start;
    out->len = len;
    return true;
}

/// @brief Trapping subslice — Rust `&s[start..start+len]`.
/// Calls __chk_fail() when the range is out of bounds.
static inline noc_cslice_t noc_csubslice(noc_cslice_t s, size_t start,
                                         size_t len) {
    noc_cslice_t out;
    if (__builtin_expect(!noc_csubslice_checked(s, start, len, &out), 0))
        __chk_fail();
    return out;
}

static inline noc_slice_t noc_subslice(noc_slice_t s, size_t start,
                                       size_t len) {
    noc_slice_t out;
    if (__builtin_expect(!noc_subslice_checked(s, start, len, &out), 0))
        __chk_fail();
    return out;
}

#ifdef __cplusplus
}
#endif

#endif /* NOC_SLICE_H */
