// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "free_page_list.h"

using namespace chino;
using namespace chino::os::kernel::mm;

void free_page_list::initialize(std::span<const boot_memory_desc> descs) noexcept {
    size_t avail_pages = 0;
    free_page_node *prev = nullptr;
    for (auto &desc : descs) {
        if (desc.kind == boot_memory_kind::free) {
            auto node = reinterpret_cast<free_page_node *>(desc.virtual_address);
            auto pages = desc.size_bytes / hal::arch_t::min_page_size;
            for (size_t i = 0; i < pages; i++) {
                if (prev)
                    prev->next = node;
                else
                    head_ = node;
                prev = node;
                node = reinterpret_cast<free_page_node *>(node->end());
            }

            avail_pages += pages;
        }
    }

    if (prev)
        prev->next = nullptr;
    avail_pages_ = avail_pages;
}

result<std::byte *> free_page_list::allocate() noexcept { return err(error_code::out_of_memory); }

void free_page_list::free([[maybe_unused]] void *base) noexcept {}
