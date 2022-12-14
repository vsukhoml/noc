// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file unistd.h
/// @brief Functionality to be provided by OS/platform.

#ifndef NOC_UNISTD_H
#define NOC_UNISTD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SBRK_FAILURE ((void *)-1)

/// @brief adds incr function bytes to the break value and changes
/// the allocated space for process accordingly.
///
/// The incr function can be negative, in which case the amount of allocated
/// space is decreased.  When used with zero as input can be used to discover
/// current break address.
///
/// @param incr number of bytes to be added (if positive) or subtracted from
/// current break address.
/// @return void*, On success, sbrk() returns the previous program break.  (If
/// the break was increased, then this value is a pointer to the start of the
/// newly allocated memory). On error, (void *) -1 is returned.
void *sbrk(intptr_t incr);

/// @brief Print string to standard output.
///
/// @param str pointer to string to print
/// @param len length of string
/// @return positive number if successful, negative if failed
intptr_t putnstr(const char *str, size_t len);

/// @brief Return clock data
/// @return nanoseconds of wall clock
uint64_t get_clock(void);

#ifdef __cplusplus
}
#endif

#endif /* NOC_UNISTD_H */
