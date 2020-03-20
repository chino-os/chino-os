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
#include <chino/kernel.h>
#include <chino/memory/heap_allocator.h>
#include <chino/memory/memory_manager.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>
#include <chino/threading/thread.h>
#include <chino/utility.h>
#include <numeric>

using namespace chino;
using namespace chino::ob;
using namespace chino::kernel;
using namespace chino::threading;
using namespace chino::memory;

kprocess &heap_allocator::owner() noexcept
{
    auto ptr = reinterpret_cast<uint8_t *>(this) - offsetof(kprocess, heap_allocator_);
    return *reinterpret_cast<kprocess *>(ptr);
}

result<void *, error_code> memory::heap_alloc(kprocess &process, size_t bytes) noexcept
{
    return process.heap_allocator_.allocate(bytes);
}

void memory::heap_free(kprocess &process, void *ptr) noexcept
{
    process.heap_allocator_.free(ptr);
}

result<void *, error_code> chino::heap_alloc(size_t bytes) noexcept
{
    return memory::heap_alloc(*current_process(), bytes);
}

void chino::heap_alloc(void *ptr) noexcept
{
    memory::heap_free(*current_process(), ptr);
}
