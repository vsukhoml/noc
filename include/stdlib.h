// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file stdlib.h
/// @brief Subset of `stdlib.h` from C standard library
/// @-mainpage Not a C Standard Library
/// @section stdlib.h

#ifndef NOC_STDLIB_H
#define NOC_STDLIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @defgroup a0 Memory allocation functions.
/// @{

/// @brief Free allocated block.
///
/// The free function causes the space pointed to by ptr to be deallocated, that
/// is, made available for further allocation. If ptr is a null pointer, no
/// action occurs. Otherwise, if the argument does not match a pointer earlier
/// returned by a memory management function, or if the space has been
/// deallocated by a call to free or realloc, the behavior is undefined.
/// @param ptr pointer to block previously allocated with malloc().
void free(void *ptr);

/// @brief Allocate memory from global heap.
///
/// The malloc function allocates space for an object whose size is specified by
/// size and whose value is indeterminate. The pointer returned if the
/// allocation succeeds is suitably aligned so that it may be assigned to a
/// pointer to any type of object with a fundamental alignment requirement and
/// then used to access such an object or an array of such objects in the space
/// allocated (until the space is explicitly deallocated). The lifetime of an
/// allocated object extends from the allocation until the deallocation. Each
/// such allocation shall yield a pointer to an object disjoint from any other
/// object. The pointer returned points to the start (lowest byte address) of
/// the allocated space. If the space cannot be allocated, a null pointer is
/// returned. If the size of the space requested is zero, the behavior is
/// implementation-defined: either a null pointer is returned, or the behavior
/// is as if the size were some nonzero value, except that the returned pointer
/// shall not be used to access an object.
/// @param size size of block to allocate in bytes
/// @returns void * pointer to allocated blocked or NULL if failed
void *malloc(size_t size) __attribute__((malloc));

/// @brief Allocated memory from global heap and clean it.
///
/// Allocates `n` * `size` bytes of memory and initialize it to zero.
///
/// @param nmemb number of elements
/// @param size size of each element
/// @return void * pointer to allocated blocked or NULL if failed
void *calloc(size_t nmemb, size_t size);

/// @brief Reallocate memory.
///
/// The realloc function deallocates the old object pointed to by ptr and
/// returns a pointer to a new object that has the size specified by size. The
/// contents of the new object shall be the same as that of the old object prior
/// to deallocation, up to the lesser of the new and old sizes. Any bytes in the
/// new object beyond the size of the old object have indeterminate values. If
/// ptr is a null pointer, the realloc function behaves like the malloc function
/// for the specified size. Otherwise, if ptr does not match a pointer earlier
/// returned by a memory management function, or if the space has been
/// deallocated by a call to the free or realloc function, the behavior is
/// undefined. If memory for the new object cannot be allocated, the old object
/// is not deallocated and its value is unchanged.
/// The realloc function returns a pointer to the new object (which may have the
/// same value as a pointer to the old object), or a null pointer if the new
/// object could not be allocated.
/// @param ptr pointer to previously allocated
/// @param size new memory size
/// @return pointer to reallocated object or NULL if failed.
void *realloc(void *ptr, size_t size);
/// @}

/// @defgroup a3 Numeric conversion functions
/// @{

int atoi(const char *nptr);
long int atol(const char *nptr);
long long int atoll(const char *nptr);
long double strtold(const char *restrict nptr, char **restrict endptr);
long int strtol(const char *restrict nptr, char **restrict endptr, int base);
long long int strtoll(const char *restrict nptr, char **restrict endptr,
                      int base);
unsigned long int strtoul(const char *restrict nptr, char **restrict endptr,
                          int base);
unsigned long long int strtoull(const char *restrict nptr,
                                char **restrict endptr, int base);
/// @}

/// @defgroup a4  Pseudo-random sequence generation functions
/// @{

#define RAND_MAX INT32_MAX

/// @brief Generate pseudo-random numbers.
///
/// The rand function computes a sequence of pseudo-random integers in the range
/// 0 to RAND_MAX.
/// @return pseudo-random integer
int rand(void);

/// @brief Set a seed for a new sequence of pseudo-random numbers.
///
/// The srand function uses the argument as a seed for a new sequence of
/// pseudo-random numbers to be returned by subsequent calls to rand. If srand
/// is then called with the same seed value, the sequence of pseudo-random
/// numbers shall be repeated. If rand is  called before any calls to srand have
/// been made, the same sequence shall be generated as when srand is first
/// called with a seed value of 1.
///
/// @param seed seed to set
void srand(unsigned int seed);
/// @}

/// @defgroup a5 Integer arithmetic functions
/// @{

int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

// div_t div(int numer, int denom);
// ldiv_t ldiv(long int numer, long int denom);
// lldiv_t lldiv(long long int numer, long long int denom);

/// @}

/// @defgroup a6 Multibyte/wide character conversion functions
/// @{

int mblen(const char *s, size_t n);

int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n);

/// @}

#ifdef __cplusplus
}
#endif

#endif /* NOC_STDLIB_H */
