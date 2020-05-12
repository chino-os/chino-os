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
#include <gsl/gsl-lite.hpp>
#include <string_view>

namespace chino::io
{
enum class std_handles
{
    in,
    out,
    err
};

enum class create_disposition
{
    create_always = 2,
    create_new = 1,
    open_always = 4,
    open_existing = 3,
    truncate_existing = 5
};

result<void, error_code> alloc_console() noexcept;
handle_t get_std_handle(std_handles type) noexcept;

result<handle_t, error_code> open(const insert_lookup_object_options &options, create_disposition create_disp = create_disposition::open_existing) noexcept;
result<size_t, error_code> read(handle_t file, gsl::span<gsl::byte> buffer) noexcept;
result<void, error_code> write(handle_t file, gsl::span<const gsl::byte> buffer) noexcept;
result<void, error_code> close(handle_t file) noexcept;
}
