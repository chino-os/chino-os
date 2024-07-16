// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "ke_services.h"
#include <chino/os/kernel/io.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;

struct ke_services_mt : i_ke_services {
  public:
    ke_services_mt() noexcept : stdio_(io::open_file("/dev/console", create_disposition::open_existing).unwrap()) {}

    virtual ssize_t read(int __fd, void *__buf, size_t __nbyte) noexcept {
        return io::read_file(stdio_, {reinterpret_cast<std::byte *>(__buf), __nbyte}).unwrap();
    }

    virtual ssize_t write(int __fd, const void *__buf, size_t __nbyte) noexcept {
        return io::write_file(stdio_, {reinterpret_cast<const std::byte *>(__buf), __nbyte}).unwrap();
    }

  private:
    file stdio_;
};

result<void> kernel::initialize_ke_services() noexcept {
#ifdef CHINO_ARCH_EMULATOR
    VirtualAlloc((LPVOID)ke_services_address, sizeof(ke_services_mt), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    std::construct_at(reinterpret_cast<ke_services_mt *>(ke_services_address));
    VirtualProtect((LPVOID)ke_services_address, sizeof(ke_services_mt), PAGE_READONLY, nullptr);
#endif
    return ok();
}
