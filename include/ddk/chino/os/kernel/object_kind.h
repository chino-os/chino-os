// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/compiler.h>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace chino::os::kernel {
struct object_kind {
    uint32_t id;
    std::string_view name;
};

constexpr inline bool operator==(const object_kind &lhs, const object_kind &rhs) noexcept { return lhs.id == rhs.id; }

#define DEFINE_OBJECT_KIND(id, name, value) inline constexpr object_kind object_kind_##id{value, #name};

#include "object_kind.def"

#undef DEFINE_OBJECT_KIND
} // namespace chino::os::kernel

namespace std {
template <> struct hash<chino::os::kernel::object_kind> {
    [[nodiscard]] size_t operator()(const chino::os::kernel::object_kind &opcode) const noexcept {
        return std::hash<uint32_t>()(opcode.id);
    }
};
} // namespace std
