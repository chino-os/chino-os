// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "process.h"
#include "../sched/scheduler.h"
#include <Windows.h>
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

void process::attach_thread(thread &thread) noexcept { threads_.push_back(&thread); }

result<void> ps::create_process(std::string_view filepath) noexcept {
    char host_filename[MAX_PATH];
    {
        try_var(file, io::open_file(filepath, create_disposition::open_existing));
        GetFinalPathNameByHandleA(file.data<HANDLE>(), host_filename, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    }
    auto lib = LoadLibraryA(host_filename);
    auto dos_header = reinterpret_cast<const IMAGE_DOS_HEADER *>(lib);
    auto nt_header = reinterpret_cast<const IMAGE_NT_HEADERS *>((std::byte *)lib + dos_header->e_lfanew);
    auto entry = reinterpret_cast<void (*)()>((std::byte *)lib + nt_header->OptionalHeader.AddressOfEntryPoint);
    entry();
    return lib ? ok() : err(error_code::not_found);
}
