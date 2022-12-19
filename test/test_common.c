// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.
//
// Minimalistic test framework
#include "test_common.h"

#include <stdio.h>
#include <unistd.h>

extern void exit(int ret);

static struct {
    uint32_t failed_count;
    uint32_t last_fail;
    uint32_t verbose;
} test_status;

// Test strings are equal or different
static bool test_string(uint32_t lineno, const char* cond, const char* got,
                        const char* expect, size_t len,
                        struct test_flags flags) {
    size_t i;

    // Stop checking if we hit the length, or end of expect, or a mismatch.
    for (i = 0; i < len && expect[i] && (got[i] == expect[i]); i++)
        ;

    // We failed if we stopped before len unless both strings terminated
    if ((i < len && (expect[i] || got[i])) != flags.invert) {
        const char* eq = flags.invert ? "!=" : "==";
        printf(
            "FAIL(%u): strcmp %s, at offset %zu, '%c' %s '%c' (\'%s\' %s "
            "\'%s\') \n",
            lineno, cond, i, got[i], eq, expect[i], got, eq, expect);
        return false;
    }

    return true;
}

// Test memory buffers are equal or different
static bool test_memcmp(uint32_t lineno, const char* cond, const uint8_t* got,
                        const uint8_t* expect, size_t len,
                        struct test_flags flags) {
    size_t i;

    // Stop checking if we hit the length, or a mismatch
    for (i = 0; i < len && got[i] == expect[i]; i++)
        ;

    // We failed if we stopped before len
    if ((i < len) != flags.invert) {
        printf("FAIL(%u): memcmp %s, at offset %zu, %02x %s %02x\n", lineno,
               cond, i, got[i], flags.invert ? "!=" : "==", expect[i]);
        return false;
    }

    return true;
}

// Test memory buffer set to a value
static bool test_memchk(uint32_t lineno, const char* cond, const uint8_t* got,
                        const uint8_t expect, size_t len,
                        struct test_flags flags) {
    size_t i;

    // Stop checking if we hit the length, or a mismatch
    for (i = 0; i < len && got[i] == expect; i++)
        ;

    // We failed if we stopped before len.
    if ((i < len) != flags.invert) {
        printf("FAIL(%u): memchk %s, at offset %zu, %02x %s %02x\n", lineno,
               cond, i, got[i], flags.invert ? "!=" : "==", expect);
        return false;
    }

    return true;
}

// Test boolean value
static bool test_bool(uint32_t lineno, const char* cond, uintptr_t got,
                      struct test_flags flags) {
    if (got != true && got != false)
        printf("WARNING(%u): %s unconventional bool = %u\n", lineno, cond,
               (unsigned int)got);

    if (got == flags.invert) {
        printf("FAIL(%u): %s\n", lineno, cond);
        return false;
    }

    return true;
}

// Test integer values
static bool test_int(uint32_t lineno, const char* cond, uintptr_t got,
                     uintptr_t expect, struct test_flags flags) {
    const char* op_str;
    bool check;

    // Truncate non-pointers to 32-bit(?)
    // if (!flags.ptr) {
    //     got = (uint32_t)got;
    //     expect = (uint32_t)expect;
    // }
    // Print the actual values for both got and expect rather than just
    // the condition, because the condition may use #defined constants or
    // math where the expected value is not obvious.
    if (flags.lt) {
        op_str = flags.invert ? ">=" : "<";
        if (flags.sign)
            check = (intptr_t)got < (intptr_t)expect;
        else
            check = got < expect;
    } else if (flags.gt) {
        op_str = flags.invert ? "<=" : ">";
        if (flags.sign)
            check = (intptr_t)got > (intptr_t)expect;
        else
            check = got > expect;
    } else {
        op_str = flags.invert ? "!=" : "==";
        check = got == expect;
    }
    if (flags.invert) check = !check;

    if (check) return true;

    // Print failures
    if (flags.ptr) {
        printf("FAIL(%u): %s, %p %s %p\n", lineno, cond, (void*)got, op_str,
               (void*)expect);
    } else if (flags.sign) {
        printf("FAIL(%u): %s, %d %s %d\n", lineno, cond, (int)got, op_str,
               (int)expect);
    } else {
        printf("FAIL(%u): %s, %u %s %u\n", lineno, cond, (unsigned int)got,
               op_str, (unsigned int)expect);
    }
    return check;
}

void add_test_result(uint32_t lineno, const char* cond, uintptr_t got,
                     uintptr_t expect, uintptr_t expect2,
                     struct test_flags flags) {
    bool pass = true;

    // Convert value + tolerance to min/max.
    if (flags.near) {
        uint32_t value = expect;
        uint32_t tolerance = expect2;
        expect = value - tolerance;
        expect2 = value + tolerance;
        flags.range = 1;
    }

    if (flags.str) {
        pass = test_string(lineno, cond, (const char*)got, (const char*)expect,
                           expect2, flags);
    } else if (flags.memcmp) {
        pass = test_memcmp(lineno, cond, (const uint8_t*)got,
                           (const uint8_t*)expect, expect2, flags);
    } else if (flags.memchk) {
        pass = test_memchk(lineno, cond, (const uint8_t*)got, expect, expect2,
                           flags);
    } else if (flags.is_true) {
        pass = test_bool(lineno, cond, got, flags);
    } else if (flags.range) {
        flags.lt = 1;
        flags.invert = !flags.invert;
        pass = test_int(lineno, cond, got, expect, flags);
        flags.lt = 0;
        flags.gt = 1;
        pass &= test_int(lineno, cond, got, expect2, flags);
    } else {
        pass = test_int(lineno, cond, got, expect, flags);
    }
    if (pass && test_status.verbose) printf("pass(%u): %s\n", lineno, cond);

    test_status.failed_count += pass ? 0 : 1;
}

static void init_test(void) { memset(&test_status, 0, sizeof(test_status)); }

bool is_test_succeed(void) {
    uint32_t fails = test_status.last_fail;
    test_status.last_fail = test_status.failed_count;
    return fails == test_status.failed_count;
}

// Use DECLARE_TEST to fill in data in between
extern struct test_case __tests_start[];
extern struct test_case __tests_end[];

// Use DECLARE_BENCH to fill in data in between
extern struct test_case __bench_start[];
extern struct test_case __bench_end[];

#if ARCH_X86_64
static inline uint64_t get_cycles(void) {
    uint64_t ret;
    __asm__ __volatile__(
        "xorq %%rax, %%rax\n"
        "rdtsc\n"
        "shlq $32, %%rdx\n"
        "orq %%rdx, %%rax\n"
        : "=A"(ret)
        :
        : "rdx");
    return ret;
}
#else
static inline uint64_t get_cycles(void) {
#ifdef __clang
    return __builtin_readcyclecounter();
#else
    return 0;
#endif  // __clang
}
#endif  // ARCH_X86_64

int main(int argc, char* argv[], char* envp[]) {
    (void)envp;
    printf("Starting %s, total program args=%d\n", argv[0], argc);

    init_test();
    // Parse command-line options.
    // TODO: implement as getopt() or so.
    for (int i = 1; i < argc; i++)
        if (!strncmp(argv[i], "-v", 2)) test_status.verbose = true;

    struct test_case* test = __tests_start;
    uint32_t test_count = 0;
    while (test < __tests_end) {
        printf("\n~~~~~~~~~~~~~\nTest \"%s\"\n", test->name);
        test_count++;
        uint64_t time = get_clock();
        bool result = test->handler();
        time = get_clock() - time;
        printf("completed in %lu ns\n", time);
        if (!result) break;
        test++;
    }
    if (test_status.failed_count)
        printf("%u TESTS FAILURES IN %u TESTS\n", test_status.failed_count,
               test_count);

    else
        printf("ALL %u TESTS PASSED\n", test_count);

    test = __bench_start;
    while (test < __bench_end) {
        uint64_t time = get_clock();
        uint64_t cycles = get_cycles();
        test->handler();
        time = get_clock() - time;
        cycles = get_cycles() - cycles;
        printf("%s completed in %lu ns, %lu cycles\n", test->name, time,
               cycles);
        test++;
    }

    if (test_status.failed_count) return 1;
    return 0;
}
