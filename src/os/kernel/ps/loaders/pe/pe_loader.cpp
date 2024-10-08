// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "pe_loader.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

#ifdef CHINO_EMULATOR
#define TRY_WIN32_IF_NOT(x)                                                                                            \
    if (!(x)) {                                                                                                        \
        return err(error_code::fail);                                                                                  \
    }
#else
// Protection flags for memory pages (Executable, Readable, Writeable)
static int ProtectionFlags[2][2][2] = {
    {
        // not executable
        {PAGE_NOACCESS, PAGE_WRITECOPY},
        {PAGE_READONLY, PAGE_READWRITE},
    },
    {
        // executable
        {PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY},
        {PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE},
    },
};
#endif

result<void> pe_loader::load(std::string_view filepath) {
#ifdef CHINO_EMULATOR
    char host_filename[MAX_PATH];
    {
        kernel::io::file f;
        try_(io::open_file(f, access_mask::generic_read, filepath, create_disposition::open_existing));
        TRY_WIN32_IF_NOT(GetFinalPathNameByHandleA(f.data<HANDLE>(), host_filename, MAX_PATH,
                                                   FILE_NAME_NORMALIZED | VOLUME_NAME_DOS));
    }

    auto lib = LoadLibraryA(host_filename);
    TRY_WIN32_IF_NOT(lib);
    image_ = reinterpret_cast<std::byte *>(lib);
    return ok();
#else
    auto dos_header = reinterpret_cast<const IMAGE_DOS_HEADER *>(pe.data());
    auto nt_header = reinterpret_cast<const IMAGE_NT_HEADERS *>(pe.data() + dos_header->e_lfanew);
    image_ = (std::byte *)VirtualAlloc(nullptr, nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_READWRITE);

    // 1. copy header
    memcpy(image_, dos_header, nt_header->OptionalHeader.SizeOfHeaders);
    auto new_nt_header = reinterpret_cast<IMAGE_NT_HEADERS *>(image_ + dos_header->e_lfanew);
    new_nt_header->OptionalHeader.ImageBase = (ULONGLONG)image_;

    // 2. copy sections
    IMAGE_SECTION_HEADER *sections_base = reinterpret_cast<IMAGE_SECTION_HEADER *>(
        (size_t)new_nt_header + sizeof(DWORD) + (size_t)(sizeof(IMAGE_FILE_HEADER)) +
        (size_t)new_nt_header->FileHeader.SizeOfOptionalHeader);
    auto optional_section_size = new_nt_header->OptionalHeader.SectionAlignment;

    for (int i = 0; i < new_nt_header->FileHeader.NumberOfSections; i++) {
        auto &section = sections_base[i];
        size_t section_size;
        if (section.SizeOfRawData == 0) {
            // section doesn't contain data in the dll itself, but may define
            // uninitialized data
            section_size = optional_section_size;
            auto dest = image_ + section.VirtualAddress;
            memset(dest, 0, section_size);
            section.Misc.PhysicalAddress = (DWORD)((uintptr_t)dest & 0xffffffff);
        } else {
            section_size = section.SizeOfRawData;
            auto dest = image_ + section.VirtualAddress;
            memcpy(dest, pe.data() + section.PointerToRawData, section.SizeOfRawData);
            section.Misc.PhysicalAddress = (DWORD)((uintptr_t)dest & 0xffffffff);
        }

        // determine protection flags based on characteristics
        auto executable = (section.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        auto readable = (section.Characteristics & IMAGE_SCN_MEM_READ) != 0;
        auto writeable = (section.Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
        auto protect = ProtectionFlags[executable][readable][writeable];
        if (section.Characteristics & IMAGE_SCN_MEM_NOT_CACHED) {
            protect |= PAGE_NOCACHE;
        }

        DWORD oldProtect;
        VirtualProtect(image_ + section.VirtualAddress, section_size, protect, &oldProtect);
    }
#endif
}

void *pe_loader::entry() const noexcept {
    auto dos_header = reinterpret_cast<const IMAGE_DOS_HEADER *>(image_);
    auto nt_header = reinterpret_cast<const IMAGE_NT_HEADERS *>(image_ + dos_header->e_lfanew);
    return image_ + nt_header->OptionalHeader.AddressOfEntryPoint;
}
