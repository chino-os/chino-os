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
#pragma once
#include "chinodef.h"
#include <atomic>
#include <chino/error.h>
#include <chino/result.h>
#include <string_view>

namespace chino::io
{
struct machine_desc
{
    const void *fdt;
    std::string_view model;
    std::string_view bootargs;
};

enum class std_handles
{
    in,
    out,
    err
};

machine_desc get_machine_desc() noexcept;

result<void, error_code> alloc_console() noexcept;
handle_t get_std_handle(std_handles type) noexcept;

result<handle_t, error_code> open(std::string_view path, access_mask access) noexcept;
result<void, error_code> write(handle_t file, gsl::span<const gsl::byte> buffer) noexcept;
}
