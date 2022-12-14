// Based on https://prng.di.unimi.it/xoroshiro64starstar.c
/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>
#include <stdlib.h>

#include "noc_internal/common.h"

/* This is xoroshiro64** 1.0, our 32-bit all-purpose, rock-solid,
   small-state generator. It is extremely fast and it passes all tests we
   are aware of, but its state space is not large enough for any parallel
   application.

   For generating just single-precision (i.e., 32-bit) floating-point
   numbers, xoroshiro64* is even faster.

   The state must be seeded so that it is not everywhere zero. */

static uint32_t rand_state[2];

static uint32_t next(void) {
    uint32_t s0 = rand_state[0];
    uint32_t s1 = rand_state[1];

    // Make sure we not stuck at 0 after init
    if ((s0 | s1) == 0) s0 = 1;

    const uint32_t result = rol32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
    rand_state[0] = rol32(s0, 26) ^ s1 ^ (s1 << 9);  // a, b
    rand_state[1] = rol32(s1, 13);                   // c
    return result;
}

int rand(void) {
    // treat 32-bit random as fixed point value in range [0..(n/2^32)]
    // which is [0..0.99999999976716935635], so we can avoid division.
    // TODO: since RAND_MAX is INT_MAX - may be just use logical AND?
    return (int)(((uint64_t)RAND_MAX * (uint64_t)next()) >> 32);
}

void srand(unsigned int seed) {
    rand_state[0] = seed;
    rand_state[1] = seed;
}
