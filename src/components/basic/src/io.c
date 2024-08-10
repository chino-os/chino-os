#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>

extern basic_putchar __putch;
extern basic_getchar __getch;

void basic_io_print(char *buffer) {
    for (size_t i = 0; i < strlen(buffer); ++i) {
        __putch(buffer[i]);
    }
}

char *basic_io_readline(char *prompt, char *buffer, size_t buffer_size) {
    size_t len = 0;
    char ch;
    basic_io_print(prompt);
    while ((ch = __getch()) != '\n' && len < buffer_size - 1) {
#if ARCH == ARCH_XMEGA
        ch = toupper(ch);
#endif
#ifdef BASIC_READLINE_ECHO
        __putch(ch);
#endif
        switch (ch) {
        case '\b':
            if (len > 0) {
                buffer[--len] = '\0';
#ifdef BASIC_READLINE_ECHO
                __putch(' ');
                __putch('\b');
#endif
            }
            break;
        default:
            buffer[len++] = ch;
        }
    }
#ifdef BASIC_READLINE_ECHO
    __putch('\n');
#endif
    buffer[len] = '\0';
    return buffer;
}
