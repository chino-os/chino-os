#include <ctype.h>
#include <math.h>
#include <microrl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

static microrl_t rl;
static bool quit = false;

static int repl_line(const char *line)
{
    if (strcmp(line, "QUIT") == 0)
    {
        quit = true;
        return 1;
    }
    else
    {
        basic_eval(line);

        if (evaluate_last_error())
        {
            printf("ERROR: %s\n", evaluate_last_error());
            clear_last_error();
        }

        return 0;
    }
}

static void repl(void)
{
    microrl_init(&rl);
    microrl_set_newline_callback(&rl, repl_line);
    rl.prompt_str = "\033[32m>\033[0m ";
    rl.prompt_len = 2;
    quit = false;

    puts(" _               _      ");
    puts("| |__   __ _ ___(_) ___ ");
    puts("| '_ \\ / _` / __| |/ __|");
    puts("| |_) | (_| \\__ \\ | (__ ");
    puts("|_.__/ \\__,_|___/_|\\___|");
    puts("(c) 2015-2016 Johan Van den Brande");
    puts("READY.");

    microrl_prompt(&rl);
    while (!quit)
    {
        microrl_insert_char(&rl, toupper(getchar()));
    }
}

void run(char *file_name)
{
    FILE *file = fopen(file_name, "r");

    if (file == NULL)
    {
        fprintf(stderr, "Can't open %s\n", file_name);
        return;
    }

    char line[tokenizer_string_length];
    while (fgets(line, sizeof(line), file))
    {
        if (line[strlen(line) - 1] != '\n')
        {
            printf("ERROR: NO EOL\n");
            return;
        }
        basic_eval(line);
    }
    fclose(file);

    basic_run();
}

int basic_main(int argc, char *argv[])
{
    basic_init(1024 * 8, 2048);

    if (argc > 1)
        run(argv[1]);
    else
        repl();

    basic_destroy();

    return EXIT_SUCCESS;
}
