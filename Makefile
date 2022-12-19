SHELL:=/bin/bash
MAKEFLAGS+=rR
CLANG_FORMAT?=/usr/bin/clang-format
CLANG_TIDY?=/usr/bin/clang-tidy

ARCH?=x86_64

ODIR_ROOT?=./build

NOC_NAME?=c

NOC_LIBNAME?=lib$(NOC_NAME).a
NOC_LD_NAME?=-l$(NOC_NAME)

$(info Running from CURDIR=$(CURDIR))

# Verbose output control. Use V=1 to print details.
ifeq ($(V),0)
Q := @
else ifeq ($(V),)
# V=
Q := @
else
# V=*
Q :=
endif

# Change string to uppercase + change '-' to '_'
uppercase = $(shell echo $(1) | tr '[:lower:]-' '[:upper:]_')

-include Makefile.toolchain

ifeq ("$(ARCH)","x86_64")
TARGET?=x86_64-pc-linux-gnu
endif

ifeq ("$(TARGET)", "x86_64-pc-linux-gnu")
# Defines to consider
# MALLOC_ALIGN
ARCH=x86_64
# CC, AR are predefined, so need special handling
ifeq ($(origin CC),default)
CC:=/usr/bin/gcc
#CC:=/usr/bin/clang-15
endif

ifeq ($(origin AR),default)
AR:=/usr/bin/ar
endif

OBJDUMP?=/usr/bin/objdump
OBJDUMP_FLAGS?=-dCSrt

CFLAGS_CLANG?=-Oz -march=native
CFLAGS_GCC?=-Os -march=native

CFLAGS+=-g
CFLAGS+=-fno-pic
CFLAGS+=-fno-omit-frame-pointer
CFLAGS+=-static

CFLAGS_CLANG+=--target=$(TARGET)
CFLAGS_CLANG+=-fvectorize -freroll-loops
CFLAGS_GCC+=-fgcse-lm -fgcse-sm -fgcse-las -fgcse-after-reload -fweb -fstdarg-opt

CFLAGS+=-DUSE_PLATFORM_64BIT_DIV=1

PLATFORM_SOURCES?=./platform/$(TARGET).c
LD_SCRIPT?=./platform/$(TARGET).ld
CFLAGS_CLANG_LD+=-fuse-ld=lld
CFLAGS_LD+=-fno-pic -fno-plt -fno-pie
CFLAGS_LD+=-static -nostdlib -L$(ODIR) -l$(NOC_NAME)
CFLAGS_LD+=-Wl,--gc-sections
CFLAGS_LD+=-Wl,--orphan-handling=warn
endif


ifeq ("$(ARCH)","riscv32")
TARGET?=riscv32-unknown-unknown-elf
endif
ifeq ("$(TARGET)","riscv32-unknown-unknown-elf")
ARCH:=riscv32
ifeq ($(origin CC),default)
CC:=/usr/bin/clang-15
endif
ifeq ($(origin AR),default)
AR:=/usr/bin/llvm-ar-15
endif
OBJDUMP:=/usr/bin/llvm-objdump-15
OBJDUMP_FLAGS:=-dCSr

CFLAGS+=-g
CFLAGS+=-fno-pic
CFLAGS+=-fno-omit-frame-pointer
CFLAGS+=-static

CFLAGS_CLANG:=--target=$(TARGET) -Oz -march=rv32imc
CFLAGS_CLANG+=-fvectorize -freroll-loops
CFLAGS+=-DUSE_PLATFORM_64BIT_DIV=0
endif


ifeq ("$(ARCH)","aarch64")
TARGET?=aarch64-unknown-unknown-elf
endif
ifeq ("$(TARGET)","aarch64-unknown-unknown-elf")
ARCH:=aarch64
ifeq ($(origin CC),default)
CC:=/usr/bin/clang-15
endif
ifeq ($(origin AR),default)
AR:=/usr/bin/llvm-ar-15
endif
OBJDUMP?=/usr/bin/llvm-objdump-15
OBJDUMP_FLAGS:=-dCSr

CFLAGS+=-g
CFLAGS+=-fno-pic
CFLAGS+=-fno-omit-frame-pointer
CFLAGS+=-static
CFLAGS_CLANG?=--target=$(TARGET) -Oz -march=armv8-r
CFLAGS+=-DUSE_PLATFORM_64BIT_DIV=1
endif


# CFLAGS to be added at last, so can redefine default
CFLAGS_LAST?=

# Platform specific sources
PLATFORM_SOURCES?=

# Additional flags to compile platform sources
PLATFORM_FLAGS?=
CFLAGS_GCC_LD?=

# Linker script to use when building tests
LD_SCRIPT?=

# Clang compiler comes with stdint.h, stddef.h headers
CFLAGS+=-I./include

# Turn all warnings for MISRA C
CFLAGS+=-Wall -Wextra -Werror -Wundef
# We target C11 for MISRA C compatibility though
CFLAGS+=-std=c11
CFLAGS+=-ffunction-sections -fdata-sections
CFLAGS+=-fmerge-all-constants
CFLAGS+=-ffreestanding
CFLAGS+=-fno-builtin-abs
CFLAGS+=-fno-builtin-labs
CFLAGS+=-fno-builtin-memcpy
CFLAGS+=-fno-builtin-memset
CFLAGS+=-fno-builtin-memcmp
CFLAGS+=-fno-builtin-malloc
CFLAGS+=-fno-builtin-printf
CFLAGS+=-fno-builtin-realloc
CFLAGS+=-fno-builtin-putchar
CFLAGS+=-fno-builtin-scanf
CFLAGS+=-fno-builtin-snprintf
CFLAGS+=-fno-builtin-sprintf
CFLAGS+=-fno-builtin-strcat
CFLAGS+=-fno-builtin-strchr
CFLAGS+=-fno-builtin-strcmp
CFLAGS+=-fno-builtin-strlen
CFLAGS+=-fno-builtin-strcpy
CFLAGS+=-fno-builtin-strncat
CFLAGS+=-fno-builtin-strncmp
CFLAGS+=-fno-builtin-strncpy
CFLAGS+=-fno-builtin-strrchr
CFLAGS+=-fno-builtin-strspn
CFLAGS+=-fno-builtin-strstr
CFLAGS+=-fno-builtin-vprintf
CFLAGS+=-fno-builtin-vsprintf

# And additional compiler-specific warnings
CFLAGS_CLANG+=-Wmost
CFLAGS_CLANG+=-Wc11-extensions
CFLAGS_CLANG+=-Wimplicit-fallthrough
CFLAGS_CLANG+=-Wunknown-warning-option
CFLAGS_CLANG+=-Wzero-as-null-pointer-constant
CFLAGS_CLANG+=-Warray-bounds-pointer-arithmetic
CFLAGS_CLANG+=-Wenum-conversion
CFLAGS_CLANG+=-Wbitfield-enum-conversion
CFLAGS_CLANG+=-Wassign-enum -Wno-error=assign-enum
CFLAGS_CLANG+=-Wanon-enum-enum-conversion
CFLAGS_CLANG+=-Wcomma -Wcomment -Wembedded-directive
CFLAGS_CLANG+=-Wflexible-array-extensions
CFLAGS_CLANG+=-Wnested-anon-types
CFLAGS_CLANG+=-Wstring-conversion
CFLAGS_CLANG+=-Wtautological-overlap-compare
CFLAGS_CLANG+=-Wthread-safety-analysis
CFLAGS_CLANG+=-Wunneeded-internal-declaration
CFLAGS_CLANG+=-Wunreachable-code-loop-increment
CFLAGS_CLANG+=-Wfor-loop-analysis
CFLAGS_CLANG+=-Wno-flexible-array-extensions
CFLAGS_CLANG+=-Wasm-operand-widths
CFLAGS_CLANG+=-Wbitwise-instead-of-logical
CFLAGS_CLANG+=-Wbitwise-op-parentheses
CFLAGS_CLANG+=-Wcast-align -Wno-error=cast-align
CFLAGS_CLANG+=-Wno-error=sign-compare
CFLAGS_CLANG+=-Wpadded -Wno-error=padded
CFLAGS_CLANG+=-Wno-error=unused-parameter
CFLAGS_CLANG+=-Wdocumentation -Wno-error=documentation
CFLAGS_CLANG+=-Wunused-command-line-argument -Wno-error=unused-command-line-argument

CFLAGS_GCC+=-Wimplicit-fallthrough=3
CFLAGS_GCC+=-Wuninitialized
CFLAGS_GCC+=-Wstrict-overflow=2 -Wno-error=strict-overflow
CFLAGS_GCC+=-Walloca-larger-than=512
CFLAGS_GCC+=-Warith-conversion
CFLAGS_GCC+=-Wduplicated-branches
CFLAGS_GCC+=-Wduplicated-cond
CFLAGS_GCC+=-Wpointer-arith
CFLAGS_GCC+=-Wbad-function-cast
CFLAGS_GCC+=-Wcast-align
CFLAGS_GCC+=-Wvector-operation-performance
CFLAGS_GCC+=-Wpadded  -Wno-error=padded
CFLAGS_GCC+=-Wredundant-decls
CFLAGS_GCC+=-Wno-free-nonheap-object
CFLAGS_GCC+=-Wsign-compare
CFLAGS_GCC+=-fstack-check=no -fno-stack-protector

# Determine wherever we run gcc or clang
COMPILER:=$(shell $(CC) -v 2>&1 | grep -q "clang version" && echo clang || echo gcc)

ifeq ($(COMPILER),clang)
CFLAGS+=$(CFLAGS_CLANG)
CFLAGS_LD+=$(CFLAGS_CLANG_LD)
else
CFLAGS+=$(CFLAGS_GCC)
CFLAGS_LD+=$(CFLAGS_GCC_LD)
endif

ODIR:=$(abspath $(ODIR_ROOT)/$(TARGET))

# Add target specific defines so we can customize implementations
UC_ARCH =$(call uppercase,$(ARCH))
UC_TARGET =$(call uppercase,$(TARGET))
CFLAGS+=-DARCH_$(UC_ARCH)=1 -DTARGET_$(UC_TARGET)=1

C_HDRS:=$(shell find ./include -name *.h)

SOURCES:=$(patsubst ./%.c,%.c,$(shell find ./src -type f -name '*.c' | sort))

OBJECTS:=$(patsubst %.c,$(ODIR)/%.o,$(SOURCES))

SOURCE_DEPS:=$(C_HDRS) $(MAKEFILE_LIST) $(CC)

TEST_SOURCES:=$(patsubst ./%.c,%.c,$(shell find ./test -type f -name '*.c' | sort))
TEST_OBJECTS:=$(patsubst %.c,$(ODIR)/%.o,$(TEST_SOURCES))

PLATFORM_OBJECTS:=$(abspath $(patsubst %.c,$(ODIR)/%.o,$(PLATFORM_SOURCES)))
$(PLATFORM_OBJECTS): CFLAGS+=$(PLATFORM_FLAGS)

# Allow bad formats for testing
$(ODIR)/test/test_snprintf.o: CFLAGS+=-Wno-error=format -Wno-error=format-extra-args

# Return current date or use provided
TIMESTAMP?="$(shell date +%s)"

# Return git hash for current directory
githash:="$(shell git describe --always --dirty --exclude '*')"

$(info Building TARGET=$(TARGET), ARCH=$(ARCH))
$(info CC=$(CC), AR=$(AR), OBJDUMP=$(OBJDUMP))
$(info SOURCES=$(SOURCES))
$(info OBJECTS=$(OBJECTS))
$(info TEST_SOURCES=$(TEST_SOURCES))
$(info PLATFORM_SOURCES = $(PLATFORM_SOURCES))
$(info git hash $(githash), timestamp $(TIMESTAMP))
$(info ------)

CFLAGS+=-DGITHASH=$(githash) -DTIMESTAMP=$(TIMESTAMP)

CFLAGS+=$(CFLAGS_LAST)

.PHONY: clean test lib

all: lib

lib: $(ODIR)/$(NOC_LIBNAME)

$(ODIR)/$(NOC_LIBNAME): $(OBJECTS) $(PLATFORM_OBJECTS) | $(ODIR)
	@echo "  AR      $(notdir $@)"
	$(Q)$(AR) rcs $@.tmp $^ && mv $@.tmp $@

test: $(ODIR)/test/test
	$(Q)$<

$(ODIR)/test/test: lib $(TEST_OBJECTS) $(LD_SCRIPT)
	$(Q)$(CC) $(CFLAGS) $(CFLAGS_LD) -Wl,-Map=$@.map  -Wl,-T $(LD_SCRIPT) \
	      $(TEST_OBJECTS) -o $@ $(NOC_LD_NAME)
	$(Q)$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $@.lst
	$(Q)strip --strip-all $@
	$(Q)ls -al $@

clean:
	rm -rf ./build

tidy: $(SOURCES)
	$(Q)$(CLANG_TIDY) --config-file=.clang-tidy $^ -- -I./include

format:
	@echo Formatting...
	$(Q)find . -name '*.[ch]' -exec $(CLANG_FORMAT) -i {} \;

doc:
	$(Q)mkdir -p ./build/doc
	$(Q)doxygen ./noc.dox

# Generic rule to compile C sources
$(ODIR)/%.o:%.c $(SOURCE_DEPS)
	@echo "  CC      $(notdir $<)"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $(CFLAGS) -MMD -MF $@.d $<
	$(Q)$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $@.lst
