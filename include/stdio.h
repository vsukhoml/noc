// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file stdio.h
/// @brief Input/output functions. Subset of `stdio.h` from C standard library

#ifndef NOC_STDIO_H
#define NOC_STDIO_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Formatted print on standard output.
///
/// The printf function writes output to standard output, under control of the
/// string pointed to by format that specifies how subsequent arguments are
/// converted for output. If there are insufficient arguments for the format,
/// the behavior is undefined. If the format is exhausted while arguments
/// remain, the excess arguments are evaluated (as always) but are otherwise
/// ignored. The printf function returns when the end of the format string is
/// encountered.
/// - The format shall be a multibyte character sequence, beginning and ending
/// in its initial shift state. The format is composed of zero or more
/// directives: ordinary multibyte characters (not %), which are copied
/// unchanged to the output stream; and conversion specifications, each of which
/// results in fetching zero or more subsequent arguments, converting them, if
/// applicable, according to the corresponding conversion specifier, and then
/// writing the result to the output stream.
/// - Each conversion specification is introduced by the character %. After the
/// %, the following appear in sequence:
///   * Zero or more flags (in any order) that modify the meaning of the
///     conversion specification.
///   * An optional minimum field width. If the converted value has fewer
///     characters than the field width, it is padded with spaces (by default)
///     on the left (or right, if the left adjustment flag, described later, has
///     been given) to the field width. The field width takes the form of an
///     asterisk * (described later) or a nonnegative decimal integer)
///   * An optional precision that gives the minimum number of digits to appear
///     for the d, i, o, u, x, and X conversions, the number of digits to appear
///     after the decimal-point character for a, A, e, E, f, and F conversions,
///     the maximum number of significant digits for the g and G conversions, or
///     the maximum number of bytes to be written for s conversions. The
///     precision takes the form of a period (.) followed either by an
///     asterisk * (described later) or by an optional decimal integer; if only
///     the period is specified, the precision is taken as zero. If a precision
///     appears with any other conversion specifier, the behavior is undefined.
///   * An optional length modifier that specifies the size of the argument.
///   * A conversion specifier character that specifies the type of conversion
///     to be applied.
/// Supported formats: %d (int), %u(unsigned int), %s(char *), %c (unsigned
/// char)
///
/// @param format format string
/// @param ... arguments to print
/// @return number of characters written
int printf(const char *format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

/// @brief Formatted output to string with length control.
/// @param s destination string buffer
/// @param n size of destination buffer
/// @param format format string
/// @param ... input arguments
/// @return the number of characters that would have been written had n been
/// sufficiently large, not counting the terminating null character, or a neg
/// ative value if an encoding error occurred. Thus, the null-terminated output
/// has been completely written if and only if the returned value is nonnegative
/// and less than n.
int snprintf(char *restrict s, size_t n, const char *restrict format, ...)
    __attribute__((__format__(__printf__, 3, 4)));

/// @brief Formatted output to string with length control.
///
/// The vsnprintf function is equivalent to snprintf, with the variable argument
/// list replaced by arg, which shall have been initialized by the va_start
/// macro (and possibly subsequent va_arg calls). The vsnprintf function does
/// not invoke the va_end macro. If copying takes place between objects that
/// overlap, the behavior is undefined.
/// @param s destination string buffer
/// @param n size of destination buffer
/// @param format format string
/// @param arg input arguments
/// @return number of characters written. The vsnprintf function returns the
/// number of characters that would have been written had n been sufficiently
/// large, not counting the terminating null character, or a neg ative value if
/// an encoding error occurred. Thus, the null-terminated output has been
/// completely written if and only if the returned value is nonnegative and less
/// than n.
int vsnprintf(char *restrict s, size_t n, const char *restrict format,
              va_list arg);

#ifdef __cplusplus
}
#endif

#endif /* NOC_STDIO_H */