// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file math.h
/// @brief Mathematical functions. Subset of `math.h` from C standard library

#ifndef NOC_MATH_H
#define NOC_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Compute square root of argument
/// @param x argument
/// @return square root
float sqrtf(float x);

/// @brief Compute square root of argument
/// @param x argument
/// @return square root
double sqrt(double x);

#ifdef __cplusplus
}
#endif

#endif /* NOC_MATH_H */