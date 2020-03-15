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
#include "../../arch/win32/target.h"
#include <Windows.h>
#include <chino/kernel.h>
#include <chino/memory.h>

using namespace chino;
using namespace chino::kernel;

alignas(PAGE_SIZE) static uint8_t memory[1024 * 1024 * 4];

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
    physical_memory_desc mem_desc = {
        .runs_count = 1,
        .pages_count = std::size(memory) / PAGE_SIZE,
        .runs = {
            { memory, std::size(memory) / PAGE_SIZE } }
    };

    memory_manager_init(mem_desc)
        .expect("Cannot init memory manager");
    auto ret = kernel_main();
    return ret.is_ok() ? 0 : -1;
}
