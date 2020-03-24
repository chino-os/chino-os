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
#include <chino/memory/memory_manager.h>
#include <chino/object.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::ob;
using namespace chino::memory;
using namespace chino::threading;
using namespace chino::kernel;

static result<handle_t, error_code> create_unnamed_handle(object &object, access_mask desired_access) noexcept;

result<object *, error_code> ob::create_object(const object_type &type, size_t body_size) noexcept
{
    try_var(base, memory::heap_alloc(kernel_process(), sizeof(object_header) + body_size));
    auto header = reinterpret_cast<object_header *>(base);
    new (header) object_header(object_attributes::none, type);
    return reinterpret_cast<object *>(reinterpret_cast<uintptr_t>(header) + sizeof(object_header));
}

result<handle_t, error_code> ob::insert_object(object &object, const insert_lookup_object_options &options) noexcept
{
    // 1. Unnamed object
    if (options.name.empty())
    {
        return create_unnamed_handle(object, options.desired_access);
    }
    else
    {
    }

    return err(error_code::not_implemented);
}

static result<handle_t, error_code> create_unnamed_handle(object &object, access_mask desired_access) noexcept
{
    if (desired_access == access_mask::none)
        return err(error_code::invalid_argument);

    try_var(entry, current_process().handle_table_.alloc());
    entry.first->ob = &object.header();
    entry.first->granted_access = desired_access;
    assert(entry.second < std::numeric_limits<decltype(handle_t::value)>::max());
    return ok(handle_t { uint16_t(entry.second) });
}
