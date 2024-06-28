/* Copyright 2019-2021 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <cstddef>
#include <cstdint>

namespace chino {
inline constexpr size_t KiB = 1024;
inline constexpr size_t MiB = 1024 * KiB;
inline constexpr size_t GiB = 1024 * MiB;

template <typename T, typename U> constexpr T ceil_div(T divisor, U dividend) noexcept {
    T quotient = static_cast<T>(divisor / dividend);
    return (quotient * dividend < divisor) ? (quotient + 1) : quotient;
}

template <typename T, typename U> constexpr T algin_up(T val, U base) noexcept {
    if constexpr (std::is_integral_v<T> && std::is_integral_v<U>) {
        return ((val + base - 1) & (~(base - 1)));
    }
    return ceil_div(val, base) * base;
}
} // namespace chino
