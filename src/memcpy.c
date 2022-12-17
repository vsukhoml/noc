// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "noc_internal/common.h"

#if defined(ARCH_X86_64)
void *memcpy(void *restrict dest, const void *restrict src, size_t len) {
    __asm__("rep movsb\n" : : "D"(dest), "S"(src), "c"(len) :);
    return dest;
}
#else

void *memcpy(void *restrict dest, const void *restrict src, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    uintptr_t *dw;
    const uintptr_t *sw;
    const uintptr_t mask = sizeof(*dw) - 1;

    uint8_t *const tail = (uint8_t *)dest + len;
    uint8_t *head = tail;

    if (dest == src || len == 0) return dest;

    // Set 'body' to the last word boundary
    uintptr_t *const body = (uintptr_t *)((uintptr_t)tail & ~mask);
    // If equally aligned and long enough
    if (((uintptr_t)d & mask) == ((uintptr_t)s & mask) &&
        (uintptr_t)tail >= (((uintptr_t)d + mask) & ~mask))
        // Set 'head' to the first word boundary
        head = (uint8_t *)(((uintptr_t)d + mask) & ~mask);

    // Copy head
    while (d < head) *(d++) = *(s++);
    // Copy body
    dw = (uintptr_t *)(void *)d;
    sw = (uintptr_t *)(void *)s;
    while (dw < body) *(dw++) = *(sw++);
    // Copy tail
    d = (uint8_t *)dw;
    s = (const uint8_t *)sw;
    while (d < tail) *(d++) = *(s++);
    return dest;
}
#endif

// TODO: reconsider where check function should go
// https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/libc-ddefs.html
__attribute__((weak)) void __chk_fail(void) { __builtin_trap(); }

void *__memcpy_chk(void *dest, const void *src, size_t len, size_t destlen) {
    if (__builtin_expect(destlen < len, 0)) __chk_fail();
    return memcpy(dest, src, len);
}

void *memset(void *dest, int c, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    uintptr_t cccc = (uintptr_t)(c & 0xff);
    uintptr_t *dw;
    uint8_t *const tail = (uint8_t *)dest + len;
    uint8_t *head = tail;
    const uintptr_t mask = sizeof(*dw) - 1;

    // Set 'body' to the last word boundary
    uintptr_t *const body = (uintptr_t *)((uintptr_t)tail & ~mask);
    // Copy 8-bit in every byte of the word depending on size
    if (sizeof(cccc) == 8)
        cccc *= 0x0101010101010101UL;
    else
        cccc *= 0x01010101UL;
    if ((uintptr_t)tail >= (((uintptr_t)d + mask) & ~mask))
        // Set 'head' to the first word boundary
        head = (uint8_t *)(((uintptr_t)d + mask) & ~mask);

    // Copy head
    while (d < head) *(d++) = c;
    // Copy body
    dw = (uintptr_t *)(void *)d;
    while (dw < body) *(dw++) = cccc;
    // Copy tail
    d = (uint8_t *)dw;
    while (d < tail) *(d++) = c;
    return dest;
}

void *memset_explicit(void *dest, int c, size_t len)
    __attribute__((alias("memset")));

void *__memset_chk(void *dest, int c, size_t len, size_t destlen) {
    if (__builtin_expect(destlen < len, 0)) __chk_fail();
    return memset(dest, c, len);
}

void *memmove(void *dest, const void *src, size_t len) {
    if ((uintptr_t)dest <= (uintptr_t)src ||
        (uintptr_t)dest >= (uintptr_t)src + len) {
        // No overlap, so just use memcpy().
        return memcpy(dest, src, len);
    }
    // Need to start from the tail due to overlap
    uint8_t *d = (uint8_t *)dest + len;
    const uint8_t *s = (const uint8_t *)src + len;
    uintptr_t *dw;
    const uintptr_t *sw;
    const uintptr_t mask = sizeof(*dw) - 1;
    uint8_t *const tail = (uint8_t *)dest;
    uint8_t *head = tail;

    // Set 'body' to the last word boundary
    uintptr_t *const body = (uintptr_t *)(((uintptr_t)tail + mask) & ~mask);

    if (((uintptr_t)d & mask) == ((uintptr_t)s & mask) &&
        (uintptr_t)tail > ((uintptr_t)d & ~mask))
        // Set 'head' to the first word boundary
        head = (uint8_t *)((uintptr_t)d & ~mask);

    // Copy head
    while (d > head) *(--d) = *(--s);

    // Copy body
    dw = (uintptr_t *)(void *)d;
    sw = (uintptr_t *)(void *)s;
    while (dw > body) *(--dw) = *(--sw);

    // Copy tail
    s = (const uint8_t *)sw;
    d = (uint8_t *)dw;
    while (d > tail) *(--d) = *(--s);
    return dest;
}

void *__memmove_chk(void *dest, const void *src, size_t len, size_t destlen) {
    if (__builtin_expect(destlen < len, 0)) __chk_fail();
    return memmove(dest, src, len);
}