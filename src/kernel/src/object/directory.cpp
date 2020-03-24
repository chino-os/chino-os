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
#include <algorithm>
#include <chino/directory.h>
#include <cstring>

using namespace chino;
using namespace chino::ob;

const object_type wellknown_types::directory { .operations = {} };

namespace
{
static_object<directory> root_directory_(wellknown_types::directory);
}

result<void, error_code> directory::insert_nolock(object_header &header) noexcept
{
    auto node = entries_.first_nolock();
    while (node)
    {
        auto cmp = std::strncmp(node->owner()->name, header.name, MAX_OBJECT_NAME);
        if (cmp == 0)
            return err(error_code::key_already_exists);
        else if (cmp > 0)
            break;
        node = node->next;
    }

    entries_.insert_before_nolock(node, &header.directory_entry);
    return ok();
}

result<object_header *, error_code> directory::lookup_nolock(std::string_view name) noexcept
{
    auto max_len = std::min(name.length(), MAX_OBJECT_NAME);
    auto node = entries_.first_nolock();
    while (node)
    {
        auto cmp = std::strncmp(node->owner()->name, name.data(), max_len);
        if (cmp == 0)
            return ok(node->owner());
        else if (cmp > 0)
            break;
        node = node->next;
    }

    return err(error_code::not_found);
}

directory &ob::root_directory() noexcept
{
    return root_directory_.get();
}

static result<object_header *, error_code> insert_or_lookup(std::string_view &complete_name, std::string_view &remaining_name, directory &root, access_state &access, object_header *insert_ob) noexcept
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

    directory *parent = &root;
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
            std::unique_lock lock(parent->syncroot());
            if (insert_ob)
            {
                if (component_name.length() > MAX_OBJECT_NAME)
                    return err(error_code::invalid_path);

                std::strncpy(insert_ob->name, component_name.data(), component_name.length());
                try_(parent->insert_nolock(*insert_ob));
                return ok(insert_ob);
            }
            else
            {
                return parent->lookup_nolock(component_name);
            }
        }
        // 4.2. Find the component obj
        else
        {
            std::unique_lock lock(parent->syncroot());
            try_var(component, parent->lookup_nolock(component_name));

            // 4.2.1. Lookup in component dir
            return err(error_code::not_implemented);
        }
    }

    return err(error_code::not_implemented);
}

result<directory *, error_code> ob::create_directory(const insert_lookup_object_options &options) noexcept
{
    if (options.name.empty())
        return err(error_code::invalid_path);

    auto complete_name = options.name;
    auto remaining_name = options.name;
    access_state access { options.desired_access };
    directory *root;

    if (options.root == handle_t::invalid())
    {
        // Should starts with '/'
        if (remaining_name[0] == DIRECTORY_SEPARATOR)
        {
            root = &root_directory();
            remaining_name = remaining_name.substr(1);
        }
        else
        {
            return err(error_code::invalid_path);
        }
    }
    else
    {
        return err(error_code::invalid_path);
    }

    try_var(dir, create_object(wellknown_types::directory, sizeof(directory)));
    new (dir) directory();

    insert_or_lookup(complete_name, remaining_name, *root, access, &dir->header());
    return ok(reinterpret_cast<directory *>(dir));
}
