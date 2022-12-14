// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "noc_internal/common.h"

#if defined(ARCH_X86_64)
size_t strlen(const char *s) {
    size_t ret;
    __asm__(
        "xor %%rax, %%rax\n"
        "testq %%rdi, %%rdi\n"
        "jz 1f\n"
        "dec %%rax\n"
        ".align 16\n"
        "2: \n"
        "leaq 1(%%rax), %%rax\n"
        "cmpb $0, (%%rdi, %%rax)\n"
        "jne 2b\n"
        "1:\n"
        : "=a"(ret)
        : "D"(s)
        :);
    return ret;
}
#else
size_t strlen(const char *s) {
    if (s == NULL) return 0;
    const char *scan = s;
    while (*scan) scan++;
    return (size_t)(scan - s);
}
#endif

size_t strnlen(const char *s, size_t maxlen) {
    if (s == NULL) return 0;
    // adjust maxlen to avoid address wrapping
    maxlen = MIN(maxlen, PLATFORM_MAX_ADDR - (uintptr_t)s);
    const char *end = s + maxlen;
    const char *scan = s;
    while (scan < end && *scan) scan++;
    return (size_t)(scan - s);
}

char *strzcpy(char *dest, const char *src, size_t len) {
    char *d = dest;
    if (!len) return dest;
    while (len > 1 && *src) {
        *(d++) = *(src++);
        len--;
    }
    *d = 0;
    return dest;
}

void *memchr(const void *buffer, int c, size_t n) {
    char *current = (char *)buffer;
    // Adjust n to avoid address wrapping
    n = MIN(n, PLATFORM_MAX_ADDR - (uintptr_t)buffer);
    char *end = current + n;
    while (current < end) {
        if (*current == c) return current;
        current++;
    }
    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t len) {
    const uint8_t *sa = s1;
    const uint8_t *sb = s2;

    if (!len) return 0;
    if (!sa) return (sb) ? -(int)*sb : 0;
    if (!sb) return (int)*sa;
    len = MIN(len, PLATFORM_MAX_ADDR - (uintptr_t)sa);

    const uint8_t *sa_end = sa + len;
    uint8_t c1, c2;
    do {
        c1 = *sa++;
        c2 = *sb++;
    } while ((c1 == c2) && sa < sa_end);
    return (int)c1 - (int)c2;
}

int strcmp(const char *s1, const char *s2) {
    if (!s1) return (s2) ? -(int)(uint8_t)*s2 : 0;
    if (!s2) return (int)(uint8_t)*s1;

    uint8_t c1, c2;
    do {
        c1 = *s1++;
        c2 = *s2++;
    } while ((c1 == c2) && c1);
    return (int)c1 - (int)c2;
}

int strncmp(const char *s1, const char *s2, size_t len) {
    if (!len) return 0;
    if (!s1) return (s2) ? -(int)(uint8_t)*s2 : 0;
    if (!s2) return (int)(uint8_t)*s1;

    len = MIN(len, PLATFORM_MAX_ADDR - (uintptr_t)s1);
    const char *s1_end = s1 + len;

    uint8_t c1, c2;
    do {
        c1 = *s1++;
        c2 = *s2++;
    } while ((c1 == c2) && c1 && s1 < s1_end);

    return (int)c1 - (int)c2;
}
