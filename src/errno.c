// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <errno.h>

#include "noc_internal/common.h"

__thread volatile errno_t errno;

// For compatibility with other libc variants
volatile errno_t* __errno(void) { return &errno; }
