// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../object.h"

namespace chino::os::kernel::ob {
inline constexpr char directory_separator = '/';

result<void> insert_object(object &object, std::string_view full_path) noexcept;
result<object_ptr<object>> lookup_object_partial(std::string_view &remaining_path) noexcept;

template <class T> result<object_ptr<T>> lookup_object_partial(std::string_view &remaining_path) noexcept {
    try_var(object, lookup_object_partial(remaining_path));
    return object.as<T>();
}
} // namespace chino::os::kernel::ob
