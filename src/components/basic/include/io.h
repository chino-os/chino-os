#ifndef __IO_H__
#define __IO_H__

#include <stdlib.h>

typedef int (*basic_putchar)(int ch);
typedef int (*basic_getchar)(void);
void basic_io_print(char *buffer);
char *basic_io_readline(char *prompt, char *buffer, size_t buffer_size);

#endif // __IO_H__
