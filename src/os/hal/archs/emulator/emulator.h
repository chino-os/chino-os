// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/hal/archs/emulator/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ps.h>

#define NTDDI_VERSION NTDDI_WIN10_RS5
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace chino::os::hal {
class emulator_cpu;

struct irq_overlapped {
    OVERLAPPED overlapped;
    arch_irq_number_t number;
    error_code error;
    DWORD bytes_transferred;
};

class emulator {
  public:
    static void run(size_t memory_size);

    static emulator_cpu &cpu(size_t id);
    static emulator_cpu &current_cpu();

  private:
    static void create_cpu_threads();
    static DWORD WINAPI cpu_entry([[maybe_unused]] LPVOID pcpu_id);

  private:
    inline static size_t memory_size_;
    inline static std::array<HANDLE, chip_t::cpus_count> cpu_threads_;
};

error_code win32_to_error_code(DWORD win32) noexcept;
error_code wsa_to_error_code(DWORD wsa) noexcept;

#define TRY_WIN32_IGNORE(x)                                                                                            \
    {                                                                                                                  \
        chino::os::kernel::ps::current_irq_lock irq_lock;                                                              \
        x;                                                                                                             \
    }

#define TRY_WIN32_IF_NOT(x)                                                                                            \
    {                                                                                                                  \
        chino::os::kernel::ps::current_irq_lock irq_lock;                                                              \
        if (!(x)) {                                                                                                    \
            return err(chino::os::hal::win32_to_error_code(GetLastError()));                                           \
        }                                                                                                              \
    }

#define TRY_WIN32_IO_IF_NOT(x)                                                                                         \
    {                                                                                                                  \
        chino::os::kernel::ps::current_irq_lock irq_lock;                                                              \
        if (!(x)) {                                                                                                    \
            auto error = GetLastError();                                                                               \
            if (error != ERROR_IO_PENDING)                                                                             \
                return err(chino::os::hal::win32_to_error_code(error));                                                \
        }                                                                                                              \
    }

#define TRY_WSA_IF_NOT(x)                                                                                              \
    {                                                                                                                  \
        chino::os::kernel::ps::current_irq_lock irq_lock;                                                              \
        if (!(x)) {                                                                                                    \
            return err(chino::os::hal::wsa_to_error_code(WSAGetLastError()));                                          \
        }                                                                                                              \
    }
} // namespace chino::os::hal
