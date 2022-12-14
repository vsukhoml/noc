// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file string.h
/// @brief Memory and String processing functions. Subset of `string.h` from C
/// standard library.
///
/// The header <string.h> declares one type and several functions, and defines
/// one macro useful for manipulating arrays of character type and other
/// objects treated as arrays of character type).
/// - Various methods are used for determining the lengths of the arrays,
/// but in all cases a char * or void * argument points to the initial (lowest
/// addressed) character of the array. If an array is accessed beyond the end of
/// an object, the behavior is undefined.
///
/// - Where an argument declared as size_t `len` specifies the length of the
/// array for a function, `len` can have the value zero on a call to that
/// function. Unless explicitly stated otherwise in the description of a
/// particular function in this subclause, pointer arguments on such a call
/// shall still have valid values. On such a call, a function that locates a
/// character finds no occurrence, a function that compares two character
/// sequences returns zero, and a function that copies characters copies zero
/// characters.
///
/// - For all functions in this subclause, each character shall be interpreted
/// as if it had the type unsigned char (and therefore every possible object
/// representation is valid and has a different value).

#ifndef NOC_STRING_H
#define NOC_STRING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int errno_t;
typedef size_t rsize_t;

#ifndef RSIZE_MAX
#define RSIZE_MAX SIZE_MAX
#endif

/// @defgroup g1 Copy memory or string functions.
/// @{

/// @brief Copy memory.
///
/// The memcpy function copies len characters from the object pointed to by
/// `src` into the object pointed to by `dest`. If copying takes place between
/// objects that overlap, the behavior is undefined.
/// @param dest address of destination
/// @param src address of source object
/// @param len length in characters (bytes)
/// @return The memcpy function returns the value of dest
void *memcpy(void *restrict dest, const void *restrict src, size_t len)
    __attribute__((nonnull(1, 2)));

/// @brief Move memory.
///
/// The memmove function copies n characters from the object pointed to by `src`
/// into the object pointed to by `dest`. Copying takes place as if the n
/// characters from the object pointed to by `dest` are first copied into a
/// temporary array of `len` characters that does not overlap the objects
/// pointed to by `src` and `dest`, and then the `len` characters from the
/// temporary array are copied into the object pointed to by `src`.
/// @param dest address of destination
/// @param src address of source object
/// @param len length in characters (bytes)
/// @return The memmove function returns the value of dest
void *memmove(void *dest, const void *src, size_t len)
    __attribute__((nonnull(1, 2)));

/// @brief Copy string.
///
///  The strcpy function copies the string pointed to by `src` (including the
///  terminating null character) into the array pointed to by `dest`. If copying
///  takes place between objects that overlap, the behavior is undefined.
/// @param dest address of destination string
/// @param src address of source string
/// @return value of `dest`.
char *strcpy(char *restrict dest, const char *restrict src)
    __attribute__((nonnull(1, 2)));

char *strncpy(char *restrict dest, const char *restrict src, size_t len);

/// @brief Copy string with guaranteed zero termination.
///
/// @param dest destination string
/// @param src source string
/// @param len maximum capacity for destination
/// @return value of `dest`
char *strzcpy(char *restrict dest, const char *restrict src, size_t len);

/// @}

char *strcat(char *restrict s1, const char *restrict s2);
char *strncat(char *restrict s1, const char *restrict s2, size_t len);

/// @defgroup g2 Compare memory or string functions.
/// @{

/// @brief Compare memory.
///
/// The memcmp function compares the first n characters of the object pointed to
/// by `s1` to the first `len` characters of the object pointed to by `s2`.
/// @param s1 pointer to first memory object
/// @param s2 pointer to second memory object
/// @param len size of memory objects
/// @return The memcmp function returns an integer greater than, equal to, or
/// less than zero, accordingly as the object pointed to by s1 is greater than,
/// equal to, or less than the object pointed to by s2.
int memcmp(const void *s1, const void *s2, size_t len);

/// @brief Compare null-terminated strings
///
/// The strcmp function compares the string pointed to by `s1` to the string
/// pointed to by `s2`.
/// @param s1 pointer to first null-terminated string
/// @param s2 pointer to second null-terminated string
/// @returns an integer greater than, equal to, or less than zero, accordingly
/// as the string pointed to by `s1` is greater than, equal to, or less than the
/// string pointed by `s2`
int strcmp(const char *s1, const char *s2);

/// @brief Compare not more than n characters of null-terminated strings
///
/// The strncmp function compares the string pointed to by `s1` to the string
/// pointed to by `s2`.
/// @param s1 pointer to first null-terminated string
/// @param s2 pointer to second null-terminated string
/// @param len maximum number of characters to compare
/// @returns an integer greater than, equal to, or less than zero, accordingly
/// as the string pointed to by `s1` is greater than, equal to, or less than the
/// string pointed by `s2`
int strncmp(const char *s1, const char *s2, size_t len);

/// @brief Compare strings using locale.
///
/// The strcoll function compares the string pointed to by s1 to the string
/// pointed to by s2, both interpreted as appropriate to the LC_COLLATE category
/// of the current locale.
/// @param s1 pointer to first string.
/// @param s2 pointer to second string
/// @return The strcoll function returns an integer greater than, equal to, or
/// less than zero, accordingly as the string pointed to by s1 is greater than,
/// equal to, or less than the string pointed to by s2 when both are interpreted
/// as appropriate to the current locale.
int strcoll(const char *s1, const char *s2);

size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n);
/// @}

/// @defgroup g3 Search functions.
/// @{

/// @brief Find first occurence of character.
///
/// The memchr function locates the first occurrence of `c` (converted to an
/// unsigned char) in the initial `n` characters (each interpreted as unsigned
/// char) of the object pointed to by `buffer`. The implementation shall behave
/// as if it reads the characters sequentially and stops as soon as a matching
/// character is found.
/// @param buffer source buffer
/// @param c character to search
/// @param n length of buffer
/// @return pointer to the located character, or a null pointer if the character
/// does not occur in the object.
void *memchr(const void *buffer, int c, size_t n);

/// @brief Find first occurence of character in null-terminated string.
///
/// The strchr function locates the first occurrence of `c` (converted to a
/// char) in the string pointed to by `s`. The terminating null character is
/// considered to be part of the string.
/// @param s null-terminated string
/// @param c character to search
/// @return returns a pointer to the located character, or a null pointer if the
/// character does not occur in the string.
char *strchr(const char *s, int c);

///@brief Search for first character from the list.
/// The strcspn function computes the length of the maximum initial segment of
/// the string pointed to by `s` which consists entirely of characters not from
/// the string pointed to by `characters`.
///@param s pointer to the null-terminated byte string to be analyzed
///@param characters pointer to the null-terminated byte string that contains
/// the characters to search for
///@return length of the maximum initial segment
size_t strcspn(const char *s, const char *characters);

///@brief Search for first character not from the list.
/// The strspn function computes the length of the maximum initial segment of
/// the string pointed to by `s` which consists entirely of characters from
/// the string pointed to by `characters`.
///@param s pointer to the null-terminated byte string to be analyzed
///@param characters pointer to the null-terminated byte string that contains
/// the characters to search for.
///@return length of the maximum initial segment
size_t strspn(const char *s, const char *characters);

char *strpbrk(const char *s1, const char *s2);

char *strrchr(const char *s, int c);

char *strstr(const char *s1, const char *s2);

/// @}

/// @defgroup g4 Miscellaneous functions.
/// @{

/// @brief Set memory to value.
///
/// The memset function copies the value of `c` (converted to an unsigned char)
/// into each of the first `len` characters of the object pointed to by `dest`
/// @param dest destination buffer
/// @param c character to fill in
/// @param len length to fill
/// @return value of `dest`
void *memset(void *dest, int c, size_t len);

/// @brief Compute length of the string with limit.
///
/// The strlen function computes the length of the string pointed to by s.
/// @param s pointer to string
/// @param maxlen maximum number of characters to analyze
/// @return length of string
size_t strnlen(const char *s, size_t maxlen);
size_t strnlen_s(const char *s, size_t maxlen);

/// @brief Compute length of the null-terminated string.
///
/// The strlen function computes the length of the string pointed to by `s`.
/// @param s pointer to string
/// @return length of string
size_t strlen(const char *s);

/// @}

static inline int isspace(int c) { return c == ' ' || (c >= 0x9 && c <= 0xd); }

// Checks if a character is a decimal digit
static inline int isdigit(int c) { return c >= '0' && c <= '9'; }

// Checks if a character is a hexadecimal digit
static inline int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Checks if a character is alphabetic
static inline int isalpha(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Checks if a character is alphanumeric
static inline int isalnum(int c) { return isdigit(c) || isalpha(c); }

// Checks if a character is a control character
static inline int iscntrl(int c) { return (c >= 0 && c <= 31) || (c == 127); }

// Converts a character to lowercase
static inline int tolower(int c) {
    return c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c;
}

// Converts a character to uppercase
static inline int toupper(int c) {
    return c >= 'a' && c <= 'z' ? c + 'A' - 'a' : c;
}

#ifdef __cplusplus
}
#endif

#endif /* NOC_STRING_H */
