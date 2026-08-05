// Copyright 2026 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file noc/parse.h
/// @brief Slice-based number parsing: the native core of the strto* family.

#ifndef NOC_PARSE_H
#define NOC_PARSE_H

#include <noc/slice.h>

#ifdef __cplusplus
extern "C" {
#endif

/// noc_parse_uint() consumed a '-' sign.
#define NOC_PARSE_NEGATIVE 0x1U
/// Magnitude exceeded UINT64_MAX; *value saturated to UINT64_MAX.
#define NOC_PARSE_OVERFLOW 0x2U

/// @brief Parse an integer magnitude from a byte slice.
///
/// Skips leading whitespace, accepts an optional +/- sign, and — for base 0
/// or 16 — a 0x/0X prefix (consumed only when a hex digit follows). Base 0
/// auto-detects octal/decimal/hex like strtol(). Valid bases: 0 and 2..36.
/// Never reads beyond `in` and never past the first byte that is not part of
/// the number.
/// @param in     input bytes
/// @param base   numeric base: 0 or 2..36; anything else parses nothing
/// @param value  out: parsed magnitude, saturated to UINT64_MAX on overflow
/// @param flags  out: NOC_PARSE_* bits
/// @return bytes consumed (whitespace + sign + prefix + digits);
///         0 when no valid number was found (*value = 0, *flags = 0)
size_t noc_parse_uint(noc_cslice_t in, unsigned int base,
                      uint64_t *restrict value, uint32_t *restrict flags);

#ifdef __cplusplus
}
#endif

#endif /* NOC_PARSE_H */
