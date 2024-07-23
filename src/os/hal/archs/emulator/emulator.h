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

struct irq_overlapped : OVERLAPPED {
    arch_irq_number_t number;
};

class emulator {
  public:
    static void run(size_t memory_size);

    static emulator_cpu &cpu(size_t id);
    static emulator_cpu &current_cpu();

    static HANDLE iocp_port() noexcept { return iocp_port_; }

  private:
    static void create_cpu_threads();
    static DWORD WINAPI cpu_entry([[maybe_unused]] LPVOID pcpu_id);

  private:
    inline static size_t memory_size_;
    inline static std::array<HANDLE, chip_t::cpus_count> cpu_threads_;
    inline static HANDLE iocp_port_;
};
} // namespace chino::os::hal
