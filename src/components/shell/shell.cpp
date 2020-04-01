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
#include <nr_micro_shell.h>

using namespace chino;

extern "C" int lua_main(int argc, char **argv);

namespace
{
uint32_t shell_main()
{
    io::alloc_console().unwrap();
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
}

extern "C"
{
    const static_cmd_st static_cmd[] = {
        { "lua", lua_cmd },
        { "\0", NULL }
    };
}

result<void, error_code> chino_start_shell()
{
    try_(threading::create_process(shell_main, {}));
    return ok();
}
