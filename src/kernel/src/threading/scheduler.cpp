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
#include <array>
#include <chino/board/board.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::threading;
using namespace chino::chip;
using namespace chino::arch;

namespace
{
template <uint32_t id>
constexpr scheduler make_scheduler()
{
    constexpr scheduler s(id);
    return s;
}

template <uint32_t... ids>
constexpr auto make_schedulers(std::integer_sequence<uint32_t, ids...>)
    -> std::array<scheduler, chip_t::processors_count>
{
    return { make_scheduler<ids>()... };
}

std::array<scheduler, chip_t::processors_count> schedulers_
    = make_schedulers(std::make_integer_sequence<uint32_t, chip_t::processors_count>());
}

scheduler &scheduler::current() noexcept
{
    return schedulers_[arch_t::current_processor()];
}

void scheduler::suspend() noexcept
{
    suspend_count_.fetch_add(1, std::memory_order_acq_rel);
}

void scheduler::resume() noexcept
{
    suspend_count_.fetch_sub(1, std::memory_order_acq_rel);
}
