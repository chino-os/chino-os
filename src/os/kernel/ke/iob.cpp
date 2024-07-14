// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <sys/cdefs.h>

/*
 * Dummy stdio hooks. This allows programs to link without requiring
 * any system-dependent functions. This is only used if the program
 * doesn't provide its own version of stdin, stdout, stderr
 */

static int dummy_putc(char c, FILE *file) {
    (void)c;
    (void)file;
    return (unsigned char)c;
}

static int dummy_getc(FILE *file) {
    (void)file;
    return EOF;
}

static int dummy_flush(FILE *file) {
    (void)file;
    return 0;
}

static FILE __stdio = FDEV_SETUP_STREAM(dummy_putc, dummy_getc, dummy_flush, _FDEV_SETUP_RW);

#ifdef __strong_reference
#define STDIO_ALIAS(x) __strong_reference(stdin, x);
#else
#define STDIO_ALIAS(x) FILE *const x = &__stdio;
#endif

extern "C" {
FILE *const stdin = &__stdio;
STDIO_ALIAS(stdout);
STDIO_ALIAS(stderr);
}
