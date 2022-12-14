# noc - Bare metal subset of standard C library

*noc* project


This project is inspired by projects like [newlib](https://sourceware.org/newlib/),
[picolib](https://github.com/picolibc/picolibc), [AVR-libc](https://www.nongnu.org/avr-libc/)

However, design goals are different:

* Focused on embedded software for platforms with constraint resources
* Specifically avoid any OS integration within library, but rather require very limited set of functions
* Permissive license compatible with everything (so rewritten from scratch)
* Minimized third party dependencies
* C11 as development language
* Limited support of compilers, primarily clang and gcc
* UTF-8 only support for multibyte locale (TBD)
* Limited floating point support (TBD)

## Building

On x86_64 library includes platform adaptation for Linux which is used for testing. To run tests ```make test```

Following variables are used:
* `ARCH` defines architecture class (x86_64, riscv32, aarch64, etc)
* `TARGET` defines target triple to build (e.g. riscv32-unknown-unknown-elf)
* `CC` defines compiler to use (e.g. /usr/bin/clang)
* `CFLAGS` define compilation flags for `gcc` and `clang`
* `CFLAGS_CLANG` defines compilation flags specific for `clang`
* `CFLAGS_CLANG_LD` defines compilation flags specific for `clang` for linking
* `CFLAGS_GCC` defines compilation flags specific for `gcc`
* `CFLAGS_GCC_LD` defines compilation flags specific for `gcc` for linking
* `AR` name of `ar` to use (e.g. /usr/bin/llvm-ar-15)
* `OBJDUMP` name of `objdump` to use (e.g. /usr/bin/llvm-objdump-15)
* `OBJDUMP_FLAGS` specific flags to `OBJDUMP`
* `PLATFORM_SOURCES` list of platform specific sources to build for tests
* `LD_SCRIPT` specifies platform specific linker script for tests

Currently only limited targets are configured as example: ```make ARCH=riscv32```, ```make ARCH=aarch64```
You can provide additional rules in `Makefile.toolchain` or call `make` with abovementioned variables configured for your target.

## Integration Requirements

### Platform shall provide following functions:

* sbrk() - allocate memory
* putnstr() - print buffer to console. can be implemented on top of putchar()

### Compiler should provide:
 * stddef.h
 * stdalign.h
 * stdint.h
 * stdbool.h
 * stdarg.h

## Supported architectures

* x86_64 (Native and Linux hosted, for testing)
* RISC-V (both 32- and 64- bit)
* ARM (32- and 64- bit)

## Supported compilers

* GNU C 10+
* Clang 11+
