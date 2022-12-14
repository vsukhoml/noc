// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <linux/time.h>
#include <linux/time_types.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

inline intptr_t __syscall0(uintptr_t n) {
    intptr_t ret;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall1(uintptr_t n, uintptr_t a1) {
    intptr_t ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1)
                     : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall2(uintptr_t n, uintptr_t a1, uintptr_t a2) {
    intptr_t ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1), "S"(a2)
                     : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall3(uintptr_t n, uintptr_t a1, uintptr_t a2,
                           uintptr_t a3) {
    intptr_t ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall4(uintptr_t n, uintptr_t a1, uintptr_t a2,
                           uintptr_t a3, uintptr_t a4) {
    intptr_t ret;
    register uintptr_t r10 __asm__("r10") = a4;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                     : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall5(uintptr_t n, uintptr_t a1, uintptr_t a2,
                           uintptr_t a3, uintptr_t a4, uintptr_t a5) {
    intptr_t ret;
    register uintptr_t r10 __asm__("r10") = a4;
    register uintptr_t r8 __asm__("r8") = a5;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                     : "rcx", "r11", "memory");
    return ret;
}

inline intptr_t __syscall6(uintptr_t n, uintptr_t a1, uintptr_t a2,
                           uintptr_t a3, uintptr_t a4, uintptr_t a5,
                           uintptr_t a6) {
    intptr_t ret;
    register uintptr_t r10 __asm__("r10") = a4;
    register uintptr_t r8 __asm__("r8") = a5;
    register uintptr_t r9 __asm__("r9") = a6;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8),
                       "r"(r9)
                     : "rcx", "r11", "memory");
    return ret;
}

intptr_t putnstr(const char *str, size_t len) {
    return __syscall3(SYS_write, 1, (uintptr_t)str, len);
}

void exit(int ret) { __syscall1(SYS_exit, ret); }

// _brk_start, _brk_end comes from linker script
extern char _brk_start[];
extern char _brk_end[];

// The sbrk() function adds incr function bytes to the break value and changes
// the allocated space accordingly. The incr function can be negative, in which
// case the amount of allocated space is decreased.
// On success, sbrk() returns the previous program break.  (If the break was
// increased, then this value is a pointer to the start of the newly allocated
// memory).  On error, (void *) -1 is returned, and errno is set to ENOMEM.
static intptr_t brk_off = (uintptr_t)_brk_start;
void *sbrk(intptr_t incr) {
    intptr_t new_brk = brk_off + incr;

    if ((new_brk <= (intptr_t)_brk_end) && (new_brk >= (intptr_t)_brk_start)) {
        brk_off = new_brk;
#ifdef VERBOSE_SBRK
        printf("sbrk(%zd) new brk at %p\n", incr, (void *)brk_off);
#endif
        return (void *)brk_off;
    }
#ifdef VERBOSE_SBRK
    printf("sbrk(%zd) failed, brk at %p\n", incr, (void *)brk_off);
#endif
    return (void *)-1;
}

uint64_t get_clock(void) {
    struct __kernel_timespec t;
    __syscall2(SYS_clock_gettime, CLOCK_MONOTONIC, (uintptr_t)&t);
    return (uint64_t)t.tv_nsec + (uint64_t)t.tv_sec * 1000000000;
}

//  LMA and VMA for .data segment
extern char _data_source[];
extern char _data_start[];
extern char _data_end[];

extern char _bss_start[];
extern char _bss_end[];

int main(int argc, char **argv, char **envp);

static void __attribute__((used)) __start(int argc, char **argv, char **envp) {
    // Due to unexpected (undocumented?) behavior of ld/lld/loader,
    // this copy not needed. Seems dependent on PHDRS for .data, needs
    // more investigation. But this is beneficial for this example.
    // memcpy(_data_start, _data_source, (uintptr_t)(_data_end - _data_start));

    // Initialize .bss to zero.
    memset(_bss_start, 0, (uintptr_t)(_bss_end - _bss_start));

    // For benchmarks make sure we only run on same core
    static const cpu_set_t set = {.__bits = {2}};
    __syscall3(SYS_sched_setaffinity, 0, sizeof(set), (uintptr_t)&set);

    exit(main(argc, argv, envp));
}

// %rsp initialized with argc and argv on stack, so load it in %rdi/%rsi as
// input arguments to __start() which passes it to main()
// Add explicit call to exit() since GCC removes call to exit()
void __attribute__((naked)) _start(void) {
    __asm__(
        "pop %%rdi\n"
        "mov %%rsp, %%rsi\n"
        "leaq 8(%%rsi, %%rdi, 8), %%rdx\n"
        "and $-16, %%rsp\n"
        "push %%rdi\n"
        "push %%rsi\n"
        "xor %%rbp, %%rbp\n"
        "call __start\n"
        "int3\n"
        :
        :
        : "memory");
}