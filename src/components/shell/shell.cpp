// MIT License
//
// Copyright (c) 2020 SunnyCase
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <chino/io.h>
#include <chino/threading.h>
#include <chino/memory.h>
#include <nr_micro_shell.h>

using namespace chino;

extern "C" int lua_main(int argc, char *argv[]);
extern "C" int basic_main(int argc, char *argv[]);

namespace
{
uint32_t shell_main()
{
    io::alloc_console().unwrap();
    setbuf(stdout, nullptr);
    shell_init();

    while (1)
    {
        auto ch = getchar();
        shell(ch);
    }

    return 0;
}

void lua_cmd(char argc, char **argv)
{
    //lua_main(argc, argv);
}

void basic_cmd(char argc, char **argv)
{
    basic_main(argc, argv);
}

void cat_cmd(char argc, char **argv)
{
    if (argc == 2)
    {
        FILE *fp = fopen(argv[1], "r");
        if (!fp)
        {
            printf("Cannot open %s\n", argv[1]);
            return;
        }

        char buffer[64];
        while (fgets(buffer, 64, fp))
            printf(buffer);
        fclose(fp);
    }
    else
    {
        printf("Usage cat <filename>\n");
    }
}

void free_cmd(char argc, char **argv)
{
    auto info = memory::get_system_memory_info();
    printf("\ttotal\tused\tfree\n");
    printf("Mem:\t%d\t%d\t%d\n", (info.used_pages + info.free_pages) * info.page_size, info.used_pages * info.page_size, info.free_pages * info.page_size);
}
}

extern "C"
{
    const static_cmd_st static_cmd[] = {
        { "basic", basic_cmd },
        { "free", free_cmd },
        { "lua", lua_cmd },
        { "cat", cat_cmd },
        { "\0", NULL }
    };
}

result<void, error_code> chino_start_shell()
{
    try_(threading::create_process(shell_main, {}, threading::thread_priority::normal, 1024 * 3 * sizeof(uintptr_t)));
    return ok();
}
