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

/// The rawmemchr() function is similar to memchr(): it assumes (i.e., the
/// programmer knows for certain) that an instance of c lies somewhere in the
/// memory area starting at the location pointed to by s, and so performs an
/// optimized search for c (i.e., no use of a count argument to limit the range
/// of the search). If an instance of c is not found, the results are
/// unpredictable.
void *rawmemchr(const void *s, int c) {
    const char *scan = (const char *)s;
    while (*scan != c) scan++;
    return (void *)scan;
}

const void *memchr(const void *buffer, int c, size_t n) {
    const char *current = (char *)buffer;
    // Adjust n to avoid address wrapping
    n = MIN(n, PLATFORM_MAX_ADDR - (uintptr_t)buffer);
    const char *end = current + n;
    while (current < end) {
        if (*current == c) return current;
        current++;
    }
    return NULL;
}

// Find first occurence of character in null-terminated string.
const char *strchr(const char *s, int c) {
    if (!s) return s;
    while (*s) {
        if (*s == c) return s;
        s++;
    }
    return NULL;
}

size_t strnlen(const char *str, size_t maxlen) {
    const char *p = memchr(str, 0, maxlen);
    return (p) ? (size_t)(p - str) : maxlen;
}

const void *memrchr(const void *buffer, int c, size_t n) {
    const char *start = (char *)buffer;
    // Adjust n to avoid address wrapping
    n = MIN(n, PLATFORM_MAX_ADDR - (uintptr_t)buffer);
    const char *current = start + n;
    while (current > start) {
        current--;
        if (*current == c) return current;
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

// Search for first character from the list
char *strpbrk(const char *s, const char *accept) {
    if (!s) return NULL;
    if (!accept) return (char *)s;
    while (*s) {
        const char *a = accept;
        while (*a) {
            if (*a == *s) return (char *)s;
            a++;
        }
        s++;
    }
    return NULL;
}

// Search for first character from the list.
size_t strcspn(const char *s, const char *characters) {
    if (!s) return 0;
    if (!characters) return strlen(s);
    const char *start = s;
    while (*s) {
        const char *a = characters;
        while (*a) {
            if (*a == *s) return (size_t)(s - start);
            a++;
        }
        s++;
    }
    return (size_t)(s - start);
}

/// Locates the first occurrence of the null-terminated string s2 in the
/// null-terminated string s1. The strstr generic function locates the first
/// occurrence in the string pointed to by s1 of the sequence of characters
/// (excluding the terminating null character) in the string pointed to by s2.
/// @param s1 pointer to the null-terminated byte string to be analyzed
/// @param s2 pointer to the null-terminated byte string to search for
/// @return The strstr generic function returns a pointer to the located string,
/// or a null pointer if the string is not found. If s2 points to a string with
/// zero length, the function returns s1.
char *strstr(const char *s1, const char *s2) {
    if (!s1) return NULL;
    size_t len = strlen(s2);
    if (!len) return (char *)s1;
    while (*s1) {
        if (!memcmp(s1, s2, len)) return (char *)s1;
        s1++;
    }
    return NULL;
}
