#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

void repl(void)
{
    puts(" _               _      ");
    puts("| |__   __ _ ___(_) ___ ");
    puts("| '_ \\ / _` / __| |/ __|");
    puts("| |_) | (_| \\__ \\ | (__ ");
    puts("|_.__/ \\__,_|___/_|\\___|");
    puts("(c) 2015-2016 Johan Van den Brande");
    puts("READY.");

    char input_buffer[128];
    while (1)
    {
        char *input = basic_io_readline("", input_buffer, sizeof(input_buffer));
        if (strcmp(input, "QUIT") == 0)
        {
            free(input);
            break;
        }

        basic_eval(input);

        if (evaluate_last_error())
        {
            printf("ERROR: %s\n", evaluate_last_error());
            clear_last_error();
        }
    }
}

int basic_main(int argc, char *argv[])
{
    basic_init(1024 * 8, 2048);
    repl();

    basic_destroy();

    return EXIT_SUCCESS;
}
