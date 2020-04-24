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
#include <cstdint>
#include <string_view>
#include <chino/error.h>
#include <chino/result.h>

namespace chino::io::pinctrl
{
struct pin_bank
{
};

struct pin_group
{
    const pin_bank &bank;
    gsl::span<const uint16_t> pins;
};

struct pin_setting
{
    const pin_group& group;
    uint32_t func;
};

struct pin_state_desc
{
    std::string_view name;
    gsl::span<const pin_setting> settings;
};

struct device_pin_state_desc
{
    gsl::span<const pin_state_desc> states;
};

namespace wellknwon_states
{
    using namespace std::string_view_literals;

    inline constexpr std::string_view default_ = "default";
    inline constexpr std::string_view sleep = "sleep";
}
}
