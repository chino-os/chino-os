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
#include <atomic>
#include <chino/error.h>
#include <chino/list.h>
#include <chino/result.h>
#include <cstdint>
#include <string_view>

namespace chino::ob
{
struct object_type;

struct access_state
{
    access_mask desired_access;
    access_mask granted_access;
};

enum class object_attributes : uint16_t
{
    none = 0,
    permanent = 0b001
};
DEFINE_ENUM_FLAG_OPERATORS(object_attributes);

typedef result<void, error_code> (*object_open_t)(void *object, access_state &access);
typedef result<void, error_code> (*object_close_t)(void *object);
typedef void (*object_delete_t)(void *object);
typedef result<void, error_code> (*object_parse_t)(void *object, std::string_view &complete_path, std::string_view &remaining_path, access_state &access);

struct object_header
{
    list_node<object_header, void, 0> directory_entry;

    std::atomic<uint16_t> refs;
    object_attributes attributes;
    char name[MAX_OBJECT_NAME + 1];

    const object_type *type;

    object_header() = default;

    constexpr object_header(object_attributes attributes, const object_type &type) noexcept
        : attributes(attributes), type(&type), name {}, refs(1)
    {
    }
};

struct object
{
    object() = default;
    object(object &) = delete;
    object(object &&) = default;
    object &operator=(object &) = delete;
    object &operator=(object &&) = default;

    object_header &header() noexcept
    {
        return *reinterpret_cast<object_header *>(reinterpret_cast<uint8_t *>(this) - sizeof(object_header));
    }

    size_t inc_ref() noexcept
    {
        if ((header().attributes & object_attributes::permanent) != object_attributes::permanent)
            return header().refs.fetch_add(1);
        return 1;
    }

    size_t dec_ref() noexcept
    {
        if ((header().attributes & object_attributes::permanent) != object_attributes::permanent)
        {
            auto ref = header().refs.fetch_sub(1);
            if (ref == 1)
                release();
            return ref;
        }

        return 2;
    }

private:
    void release() noexcept;
};

template <class TObject>
struct static_object
{
    object_header header;
    TObject body;

    constexpr static_object(const object_type &type) noexcept
        : header(object_attributes::permanent, type), body {} {}

    constexpr static_object(const object_type &type, TObject &&body) noexcept
        : header(object_attributes::permanent, type), body(std::move(body)) {}

    constexpr TObject &get() const noexcept { return const_cast<TObject &>(body); }
};

struct object_operations
{
    object_open_t open;
    object_close_t close;
    object_delete_t del;
    object_parse_t parse;
};

struct object_type
{
    object_operations operations;
};

namespace wellknown_types
{
    extern const object_type directory;
    extern const object_type process;
    extern const object_type thread;
}

struct insert_lookup_object_options
{
    handle_t root = handle_t::invalid();
    std::string_view name;
    access_mask desired_access;
};

result<object *, error_code> create_object(const object_type &type, size_t body_size) noexcept;
result<handle_t, error_code> insert_object(object &object, const insert_lookup_object_options &options) noexcept;
}
