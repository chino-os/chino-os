#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>

void basic_io_print(char *buffer)
{
    for (size_t i = 0; i < strlen(buffer); ++i)
    {
        putchar(buffer[i]);
    }
}

char *
basic_io_readline(char *prompt, char *buffer, size_t buffer_size)
{
    size_t len = 0;
    char ch;
    basic_io_print(prompt);
    while ((ch = getchar()) != '\n' && len < buffer_size - 1)
    {
        ch = toupper(ch);
        putchar(ch);
        switch (ch)
        {
        case '\b':
            if (len > 0)
            {
                buffer[--len] = '\0';
                putchar(' ');
                putchar('\b');
            }
            break;
        default:
            buffer[len++] = ch;
        }
    }
    putchar('\n');
    buffer[len] = '\0';
    return buffer;
}
