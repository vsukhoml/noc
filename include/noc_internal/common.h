/**
 * @file noc_internal/common.h
 * @brief Internal definitions for noc
 */
#ifndef NOC_INTERNAL_H
#define NOC_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ == 201112L
//  Test condition at compile time, not run time.
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)
#else
// Test an important condition at compile time, not run time.
// Similar to _Static_assert, but works for C99
#define NOC_COND0_(cond, line) \
    extern int __build_assertion_##line[1 - 2 * !(cond)] __attribute__((unused))
#define NOC_COND1_(c, x) NOC_COND0_(c, x)
#define STATIC_ASSERT(cond) NOC_COND1_(cond, __LINE__)
#endif

#ifndef PLATFORM_MAX_ADDR
// Maximum valid address
#define PLATFORM_MAX_ADDR UINTPTR_MAX
#endif

#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        (_a < _b) ? _a : _b;    \
    })

#ifdef NDEBUG
#define LOG(s, ...) printf(s, __VA_ARGS__)
#else
#define LOG(s, ...)
#endif

#ifdef __clang__
static inline uint32_t rol32(const uint32_t x, int k) {
    return __builtin_rotateleft32(x, k);
}
#else
static inline uint32_t rol32(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}
#endif

// Division of 64 bit value by 32 bit
#if !USE_PLATFORM_64BIT_DIV
/// Divide 64 bit value `n` by `d`
/// @param n in/out value to divide
/// @param d divisor
/// @return remainder and update `n` to quotient
uint32_t umoddiv32(uint64_t *n, uint32_t d);
#else
static inline uint32_t umoddiv32(uint64_t *n, uint32_t d) {
    const uint64_t nn = *n;
    *n = nn / d;
    return nn % d;
}
#endif

/// @brief Count leading zeroes
/// @param x input argument
/// @return leading zeroes in `x`, 32 if x == 0
static inline uint32_t stdc_leading_zerosui(uint32_t x) {
    return (x) ? (uint32_t)__builtin_clz(x) : 32U;
}

#ifdef __cplusplus
}
#endif

#endif /* NOC_INTERNAL_H */
