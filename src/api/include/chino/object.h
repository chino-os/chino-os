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
#include "error.h"
#include "result.h"
#include <cstdint>
#include <string_view>

namespace chino::object
{
struct object_type;

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)                                                              \
    inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) | ((int)b)); }           \
    inline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
    inline ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) & ((int)b)); }           \
    inline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
    inline ENUMTYPE operator~(ENUMTYPE a) { return ENUMTYPE(~((int)a)); }                                 \
    inline ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) ^ ((int)b)); }           \
    inline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); }
#endif

enum class access_mask
{
    none = 0,
    generic_all = 0b001,
    generic_read = 0b010,
    generic_write = 0b100
};
DEFINE_ENUM_FLAG_OPERATORS(access_mask);

struct access_state
{
    access_mask desired_access;
    access_mask granted_access;
};

typedef result<void, error_code> (*object_open_t)(void *object, access_state &access);
typedef result<void, error_code> (*object_close_t)(void *object);
typedef void (*object_delete_t)(void *object);
typedef result<void, error_code> (*object_parse_t)(void *object, std::string_view &complete_path, std::string_view &remaining_path, access_state &access);

struct object_header
{
    object_type *type;
};

struct object
{
    object(object &) = delete;
    object &operator=(object &) = delete;

    object_header &header() noexcept
    {
        return *reinterpret_cast<object_header *>(reinterpret_cast<uint8_t *>(this) - sizeof(object_header));
    }
};

struct object_type_initializer
{
    object_open_t open;
    object_close_t close;
    object_delete_t del;
    object_parse_t parse;
};

struct object_type : object
{
    object_type_initializer initializer;
};

result<object_type &, error_code> create_object_type(std::string_view name, const object_type_initializer &initializer);
}

namespace chino
{
typedef struct _handle
{
    uintptr_t value;

    static constexpr _handle invalid() { return {}; }
} handle_t;
}
