// Copyright 2024 Vadim Sukhomlinov
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file assert.h
/// @brief Runtime and static assertions. Subset of `assert.h` from C standard
/// library.

#undef assert

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else

#ifndef NOC_ASSERT_H_DECLS
#define NOC_ASSERT_H_DECLS
#ifdef __cplusplus
extern "C" {
#endif

/// @brief Called when assertion fails.
/// @param expr The stringified expression that failed.
/// @param file The filename where the assertion occurred.
/// @param line The line number where the assertion occurred.
/// @param func The function name where the assertion occurred (or NULL).
void __assert_fail(const char *expr, const char *file, int line,
                   const char *func) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif
#endif  // NOC_ASSERT_H_DECLS

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define assert(expr) \
    ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
#else
#define assert(expr) \
    ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, 0))
#endif

#endif  // NDEBUG

#ifndef NOC_ASSERT_H
#define NOC_ASSERT_H

#if defined(__cplusplus) && __cplusplus >= 201103L
#ifndef static_assert
#define static_assert static_assert
#endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#ifndef static_assert
#define static_assert _Static_assert
#endif
#endif

#endif /* NOC_ASSERT_H */