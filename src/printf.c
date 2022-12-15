// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <noc_internal/common.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static const char ERROR_STR[] = "<ERROR>\n";

// Maximum characters in a single format field
#ifndef MAX_FORMAT
#define MAX_FORMAT 128
#endif

#ifndef PRINTF_BUFFER_SIZE
#define PRINTF_BUFFER_SIZE 256
#endif

// Get digit in base `d`, update n to n/d
static uint32_t get_digit(uint64_t *n, uint32_t d) {
    uint32_t r = 0;

    // Common powers of 2 = simple shifts
    switch (d) {
        case 2:
            r = *n & 1;
            *n >>= 1;
            break;
        case 8:
            r = *n & 0x7;
            *n >>= 3;
            break;
        case 16:
            r = *n & 0xf;
            *n >>= 4;
            break;
        default:
            // General case, using division
            r = umoddiv32(n, d);
            break;
    }
    return r;
}

static inline char char_digit(unsigned char c, char a) {
    return (char)(c > 9 ? (c + a - 10) : (c + '0'));
}

typedef bool (*write_char)(void *state, int c);

static int formatter(write_char write, void *state, const char *format,
                     va_list args) {
    char digits[68];  // Buffer for text representation.

    // Flags for format specifier
    struct fmt {
        unsigned int left : 1;      // Left-justify
        unsigned int pad_zero : 1;  // Pad with 0's not spaces
        unsigned int add_sign : 1;  // Add sign (+) for a positive number
        unsigned int bit64 : 1;     // Number is 64-bit
        unsigned int bit16 : 1;     // Number is 16-bit
        unsigned int bit8 : 1;      // Number is 8-bit
        unsigned int alt : 1;       // Alternative representation
        unsigned int space : 1;     // Prepend space
        unsigned int _pad : 24;     // to avoid warnings on padding
    } flags;

    uint32_t precision, pad_width = 0;
    int count = 0;  // Counter for output characters
    char c;

    if (!format) return EOF;

    while ((c = *format++)) {
        if (c != '%') {
            if (!write(state, c)) goto stop;
            count++;
            continue;
        }

        c = *format++;  // Read format specifier
        if (c == '%') {
            if (!write(state, c)) goto stop;
            count++;
            continue;
        } else if (c == 0) {  // Incomplete format flag
            format = ERROR_STR;
            continue;
        }

        // Zero flags, now that we're in a format
        flags = (struct fmt){};
        char sign = 0;

        // Format modifiers can come in any order
        do {
            if (c == '-') {
                // Left-justified ("%-5s")
                flags.left = 1;
            } else if (c == '+') {
                // Handle positive sign (%+d)
                flags.add_sign = 1;
            } else if (c == '0') {
                // Handle padding with 0's
                flags.pad_zero = 1;
            } else if (c == '#') {
                // Alternate format (TBD)
                flags.alt = 1;
            } else if (c == ' ') {
                // Put ' ' instead of '+'
                flags.space = 1;
            } else
                break;
            c = *format++;
        } while (true);

        // Process padding width
        pad_width = 0;
        if (c == '*') {
            int p = va_arg(args, int);
            pad_width = (p < 0) ? 0 : (uint32_t)p;
            c = *format++;
        } else {
            while (isdigit(c)) {
                pad_width = (10 * pad_width) + c - '0';
                c = *format++;
            }
        }

        // Extract precision
        precision = 0;
        if (c == '.') {
            c = *format++;
            if (c == '*') {
                int p = va_arg(args, int);
                precision = (p < 0) ? 0 : (uint32_t)p;
                c = *format++;
            } else {
                while (isdigit(c)) {
                    precision = (10 * precision) + c - '0';
                    c = *format++;
                }
            }
        }

        if (pad_width > MAX_FORMAT || precision > MAX_FORMAT) {
            format = ERROR_STR;
            continue;
        }

        char *value_str = NULL;  // text representation of argument

        if (c == 's') {
            value_str = va_arg(args, char *);
            if (value_str == NULL) value_str = (char *)"[null]";
        } else if (c == 'H') {
            // Extension: hex dump output (e.g. %32H will print 32 bytes)
            value_str = va_arg(args, char *);

            if (!value_str || !precision) {
                // Hex dump requires precision
                format = ERROR_STR;
                continue;
            }

            while (precision) {
                if (!write(state, char_digit(*value_str >> 4, 'a'))) goto stop;
                count++;
                if (!write(state, char_digit(*value_str & 0xf, 'a'))) goto stop;
                count++;
                precision--;
                value_str++;
            }
            continue;
        } else {
            uint32_t base = 10;
            uint64_t v;

            // Handle length & type
            if (c == 'h') {
                c = *format++;
                if (c == 'h') {
                    flags.bit8 = 1;  // %hh specifier
                    c = *format++;
                } else
                    flags.bit16 = 1;  // %h specifier
            } else if (c == 'l') {
                flags.bit64 = 1;
                c = *format++;
                if (c == 'l') {
                    // TODO: add support for long long?
                    c = *format++;
                }
            } else if (c == 'z') {
                if (sizeof(size_t) == sizeof(uint64_t)) flags.bit64 = 1;
                c = *format++;
            } else if (c == 'p') {
                if (sizeof(void *) == sizeof(uint64_t)) flags.bit64 = 1;
                flags.pad_zero = 1;
            } else if (c == 'j') {
                if (sizeof(intmax_t) == sizeof(uint64_t)) flags.bit64 = 1;
                c = *format++;
            } else if (c == 't') {
                if (sizeof(ptrdiff_t) == sizeof(uint64_t)) flags.bit64 = 1;
                c = *format++;
            }

            if (c == 'c') {  // '%c', read char
                c = va_arg(args, int);
                if (!write(state, c)) goto stop;
                count++;
                continue;
            }

            if (flags.bit64) {
                v = va_arg(args, uint64_t);
            } else {
                v = va_arg(args, uint32_t);
                if (flags.bit16) v = v & 0xffff;
                if (flags.bit8) v = v & 0xff;
            }

            switch (c) {
                case 'd':
                    // sign extension for smaller types
                    if (flags.bit16) {
                        int16_t vv = (int16_t)v;
                        if (vv < 0) {
                            sign = '-';
                            v = -vv;
                        } else
                            v = (uint16_t)vv;
                    } else if (flags.bit8) {
                        int8_t vv = (int8_t)v;
                        if (vv < 0) {
                            sign = '-';
                            v = -vv;
                        } else
                            v = (uint8_t)vv;
                    } else if (flags.bit64) {
                        if ((int64_t)v < 0) {
                            sign = '-';
                            if (v != (1ULL << 63)) v = -v;
                        } else if (flags.add_sign) {
                            sign = '+';
                        }
                    } else {
                        if ((int)v < 0) {
                            sign = '-';
                            if (v != (1ULL << 31)) v = -(int)v;
                        } else if (flags.add_sign) {
                            sign = '+';
                        }
                    }
                    if (!sign && flags.space) sign = ' ';
                    break;
                case 'u':
                    break;
                case 'p':
                    // Pointers printed with 0x prefix
                    base = 16;
                    if (!write(state, '0')) goto stop;
                    count++;

                    if (!write(state, 'x')) goto stop;
                    count++;
                    break;
                case 'X':
                case 'x':
                    base = 16;
                    break;
                case 'o':  // Octal numbers starts with 0
                    base = 8;
                    if (!write(state, '0')) goto stop;
                    count++;
                    break;
                case 'b':
                    base = 2;
                    break;
                default:
                    format = ERROR_STR;
            }

            // Leave space for the terminating null.
            if (precision > sizeof(digits) - 1) format = ERROR_STR;

            // Unrecognized / unsupported format
            if (format == ERROR_STR) continue;

            // Convert integer to string starting backwards
            value_str = digits + sizeof(digits) - 1;
            *value_str = 0;

            char hex = (c == 'X' || c == 'p' || flags.alt) ? 'A' : 'a';

            // Print requested number of digits independent of value
            for (size_t i = 0; i < precision; i++)
                *(--value_str) = char_digit(get_digit(&v, base), hex);

            if (!precision && !v) *(--value_str) = '0';

            while (v) *(--value_str) = char_digit(get_digit(&v, base), hex);

            if (sign) *(--value_str) = sign;

            precision = 0;  // consumed for number outputs
        }

        // value_str points to either %s string or text representation, get it
        // length
        size_t value_len = strlen(value_str);

        // No padding strings to wider than the precision
        if (precision && pad_width > precision) pad_width = precision;

        // If precision is zero, print everything
        if (!precision)
            precision = (value_len > pad_width) ? value_len : pad_width;

        // Padding right if requested
        if (!(flags.left)) {
            const char pad_char = flags.pad_zero ? '0' : ' ';
            while (value_len < pad_width) {
                if (!write(state, pad_char)) goto stop;
                count++;
                value_len++;
            }
        }
        while (*value_str && precision) {
            if (!write(state, *value_str++)) goto stop;
            count++;
            precision--;
        }
        // Padding left if requested
        if (flags.left)
            while (value_len < pad_width) {
                if (!write(state, ' ')) goto stop;
                count++;
                value_len++;
            }
    }
stop:

    return count;
}

struct printf_state {
    size_t len;
    char buf[PRINTF_BUFFER_SIZE];
};

static bool write_printf(void *state, int c) {
    struct printf_state *ctx = (struct printf_state *)state;

    if (ctx->len < sizeof(ctx->buf)) {
        ctx->buf[ctx->len++] = (char)c;
    } else {
        // If buffer is full, print it
        if (putnstr(ctx->buf, sizeof(ctx->buf)) < 0) return false;
        ctx->len = 1;
        ctx->buf[0] = (char)c;
    }
    return true;
}

int printf(const char *format, ...) {
    va_list args;
    struct printf_state ctx;
    int res;
    ctx.len = 0;

    va_start(args, format);
    res = formatter(write_printf, &ctx, format, args);
    va_end(args);
    if (res > 0) {
        if (ctx.len) res = (int)putnstr(ctx.buf, ctx.len);
    } else
        putnstr(ERROR_STR, sizeof(ERROR_STR));

    return res;
}

static const char NEWLINE[] = "\n";

int puts(const char *str) {
    if (!str)
        return EOF;
    size_t len = strlen(str);
    int res = (int)putnstr(str, len);
    if (res >= 0) res = (int)putnstr(NEWLINE, 1);
    return res;
}

// Alias for gcc / FORTIFY_SOURCES>0
int __printf_chk(const char *format, ...)
    __attribute__((weak, alias("printf")));

struct snprintf_state {
    char *str_end;
    char *str;
};

// Write to null-terminated string
static bool write_str(void *state, int c) {
    struct snprintf_state *ctx = (struct snprintf_state *)state;
    return (ctx->str < ctx->str_end) ? ((void)(*(ctx->str++) = (char)c), true)
                                     : false;
}

int snprintf(char *restrict str, size_t n, const char *restrict format, ...) {
    struct snprintf_state ctx = {.str_end = str + n, .str = str};
    va_list args;
    va_start(args, format);
    int res = formatter(write_str, &ctx, format, args);
    va_end(args);
    if (ctx.str >= ctx.str_end)
        *(ctx.str_end - 1) = 0;
    else
        *ctx.str = 0;
    return res;
}

int vsnprintf(char *restrict str, size_t n, const char *restrict format,
              va_list args) {
    struct snprintf_state ctx = {.str_end = str + n, .str = str};
    int res = formatter(write_str, &ctx, format, args);
    if (ctx.str >= ctx.str_end)
        *(ctx.str_end - 1) = 0;
    else
        *ctx.str = 0;
    return res;
}
