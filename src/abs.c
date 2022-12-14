// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <stdlib.h>

#include "noc_internal/common.h"

int abs(int j) { return (j < 0) ? -j : j; }

long int labs(long int j) { return (j < 0) ? -j : j; };

long long int llabs(long long int j) { return (j < 0) ? -j : j; };
