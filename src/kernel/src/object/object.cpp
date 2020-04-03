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
#include <chino/ddk/directory.h>
#include <chino/ddk/kernel.h>
#include <chino/ddk/object.h>
#include <chino/memory/memory_manager.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::ob;
using namespace chino::memory;
using namespace chino::threading;
using namespace chino::kernel;

static result<handle_t, error_code> create_handle(object &object, access_mask desired_access) noexcept;
static result<handle_t, error_code> create_named_handle(object &object, const insert_lookup_object_options &options) noexcept;

void object::release() noexcept
{
    memory::heap_free(kernel_process(), &this->header());
}

result<object *, error_code> ob::create_object(const object_type &type, size_t body_size) noexcept
{
    try_var(base, kernel::kheap_alloc(sizeof(object_header) + body_size));
    auto header = reinterpret_cast<object_header *>(base);
    new (header) object_header(object_attributes::none, type);
    return reinterpret_cast<object *>(reinterpret_cast<uintptr_t>(header) + sizeof(object_header));
}

result<handle_t, error_code> ob::insert_object(object &object, const insert_lookup_object_options &options) noexcept
{
    // 1. Unnamed object
    if (options.name.empty())
    {
        auto ret = create_handle(object, options.desired_access);
        if (ret.is_err())
            object.dec_ref();
        return ret;
    }
    // 2. Named object
    else
    {
        auto ret = create_named_handle(object, options);
        if (ret.is_err())
            object.dec_ref();
        return ret;
    }
}

result<object *, error_code> ob::reference_object(handle_t handle) noexcept
{
    try_var(entry, current_process().handle_table_.at(handle.value));
    return &entry->ob->body();
}

static result<handle_t, error_code> create_handle(object &object, access_mask desired_access) noexcept
{
    if (desired_access == access_mask::none)
        return err(error_code::invalid_argument);

    try_var(entry, current_process().handle_table_.alloc());
    entry.first->ob = &object.header();
    entry.first->granted_access = desired_access;
    assert(entry.second < std::numeric_limits<decltype(handle_t::value)>::max());
    return ok(handle_t { uint16_t(entry.second) });
}

static result<object_header *, error_code> insert_or_lookup(std::string_view &complete_name, std::string_view &remaining_name, object &root, access_state &access, object_header *insert_ob) noexcept
{
    // 1. Empty name
    if (remaining_name.empty())
    {
        if (!insert_ob)
            return ok(&root.header());
        return err(error_code::invalid_path);
    }

    // 2. Should not start with '/'
    if (remaining_name[0] == DIRECTORY_SEPARATOR)
        return err(error_code::invalid_path);

    object *parent = &root;
    while (true)
    {
        // 3. Find component name
        auto comp_split = remaining_name.find_first_of(DIRECTORY_SEPARATOR);
        auto component_name = comp_split == remaining_name.npos ? remaining_name : remaining_name.substr(0, comp_split);
        remaining_name = comp_split == remaining_name.npos ? std::string_view {} : remaining_name.substr(comp_split + 1);

        if (component_name.empty())
        {
            if (remaining_name.empty() && !insert_ob)
                return ok(&root.header());
            return err(error_code::invalid_path);
        }

        // 4.1. No remaing, lookup or insert
        if (remaining_name.empty())
        {
            if (parent->header().type == &wellknown_types::directory)
            {
                auto &parent_dir = static_cast<directory &>(*parent);
                std::unique_lock lock(parent_dir.syncroot());
                if (insert_ob)
                {
                    if (component_name.length() > MAX_OBJECT_NAME)
                        return err(error_code::invalid_path);

                    std::strncpy(insert_ob->name, component_name.data(), component_name.length());
                    try_(parent_dir.insert_nolock(*insert_ob));
                    return ok(insert_ob);
                }
                else
                {
                    return parent_dir.lookup_nolock(component_name);
                }
            }
            else
            {
                return err(error_code::not_supported);
            }
        }
        // 4.2. Find the component obj
        else
        {
            if (parent->header().type == &wellknown_types::directory)
            {
                auto &parent_dir = static_cast<directory &>(*parent);
                std::unique_lock lock(parent_dir.syncroot());
                try_var(component, parent_dir.lookup_nolock(component_name));

                // 4.2.1. Lookup in component dir
                if (component->type == &wellknown_types::directory)
                {
                    parent = &static_cast<directory &>(component->body());
                    continue;
                }
                else
                {
                    return ok(component);
                }
            }

            return err(error_code::not_found);
        }
    }

    return err(error_code::not_implemented);
}

static result<object_header *, error_code> insert_or_lookup(const insert_lookup_object_options &options, std::string_view *remaining_name, object_header *insert)
{
    if (options.name.empty())
        return err(error_code::invalid_path);

    auto complete_name = options.name;
    std::string_view remaining_name_st;
    auto &remaining_name_ref = remaining_name ? *remaining_name : remaining_name_st;
    remaining_name_ref = options.name;
    access_state access { options.desired_access };
    object *root;

    if (options.root == handle_t::invalid())
    {
        // Should starts with '/'
        if (remaining_name_ref[0] == DIRECTORY_SEPARATOR)
        {
            root = &root_directory();
            remaining_name_ref = remaining_name_ref.substr(1);
        }
        else
        {
            return err(error_code::invalid_path);
        }
    }
    else
    {
        // Should not starts with '/'
        if (!remaining_name_ref.empty() && remaining_name_ref[0] == DIRECTORY_SEPARATOR)
        {
            return err(error_code::invalid_path);
        }
        else
        {
            try_set(root, reference_object(options.root));
        }
    }

    try_var(ob, insert_or_lookup(complete_name, remaining_name_ref, *root, access, insert));
    if (remaining_name_ref.empty() || remaining_name)
        return ok(ob);
    return err(error_code::not_found);
}

static result<handle_t, error_code> create_named_handle(object &object, const insert_lookup_object_options &options) noexcept
{
    access_state access { options.desired_access };
    try_(insert_or_lookup(options, nullptr, &object.header()));
    return create_handle(object, access.desired_access);
}

result<object *, error_code> ob::reference_object_partial(const insert_lookup_object_options &options, std::string_view &remaining_name) noexcept
{
    try_var(header, insert_or_lookup(options, &remaining_name, nullptr));
    return &header->body();
}

result<object *, error_code> ob::reference_object(const insert_lookup_object_options &options) noexcept
{
    try_var(header, insert_or_lookup(options, nullptr, nullptr));
    return &header->body();
}
