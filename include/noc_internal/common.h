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

/// Funnel shift left: high 32 bits of the 64-bit value (hi:lo) << s.
/// Requires 0 < s < 32 (s == 0 shifts lo by 32: undefined). One instruction
/// where the ISA has it (x86 shld, RISC-V Zbb rol-based sequences) but
/// compilers don't reliably pattern-match it yet, so keep the idiom here.
static inline uint32_t fsl32(uint32_t hi, uint32_t lo, uint32_t s) {
    return (hi << s) | (lo >> (32U - s));
}

/// Funnel shift right: low 32 bits of the 64-bit value (hi:lo) >> s.
/// Requires 0 < s < 32.
static inline uint32_t fsr32(uint32_t hi, uint32_t lo, uint32_t s) {
    return (lo >> s) | (hi << (32U - s));
}

// Division of 64 bit value by 32 bit
#if !USE_PLATFORM_64BIT_DIV
/// Divide 64 bit value `n` by `d`
/// @param n in/out value to divide
/// @param d divisor
/// @return remainder and update `n` to quotient
uint32_t umoddiv32(uint64_t *n, uint32_t d);

/// Divide 64 bit value `n` by 64 bit `d`
/// @param n value to divide
/// @param d divisor
/// @param rem out: remainder
/// @return quotient
uint64_t udivmod64(uint64_t n, uint64_t d, uint64_t *rem);
#else
static inline uint32_t umoddiv32(uint64_t *n, uint32_t d) {
    const uint64_t nn = *n;
    *n = nn / d;
    return nn % d;
}

static inline uint64_t udivmod64(uint64_t n, uint64_t d, uint64_t *rem) {
    *rem = n % d;
    return n / d;
}
#endif

// Portable software division, compiled on every target so the hosted test
// suite can verify it against hardware division; umoddiv32()/udivmod64()
// alias these on !USE_PLATFORM_64BIT_DIV targets. Unused copies are dropped
// from linked images by --gc-sections.
uint32_t _umoddiv32_soft(uint64_t *n, uint32_t d);
uint64_t _udivmod64_soft(uint64_t n, uint64_t d, uint64_t *rem);

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
