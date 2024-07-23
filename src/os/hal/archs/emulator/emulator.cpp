// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator.h"
#include "emulator_cpu.h"

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

static std::array<emulator_cpu, chip_t::cpus_count> cpus_;

void emulator::run(size_t memory_size) {
    memory_size_ = memory_size;

    // 1. Initialize IOCP
    iocp_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

    emulator_cpu::register_message_queue();
    create_cpu_threads();
    WaitForMultipleObjects((DWORD)cpu_threads_.size(), cpu_threads_.data(), TRUE, INFINITE);
}

emulator_cpu &emulator::cpu(size_t id) { return cpus_[id]; }

emulator_cpu &emulator::current_cpu() { return cpu(emulator_arch::current_cpu_id()); }

void emulator::create_cpu_threads() {
    for (size_t i = 0; i < cpu_threads_.size(); i++) {
        cpu_threads_[i] = CreateThread(nullptr, 0, cpu_entry, (LPVOID)i, 0, nullptr);
    }
}

DWORD WINAPI emulator::cpu_entry([[maybe_unused]] LPVOID pcpu_id) {
    auto cpu_id = (size_t)pcpu_id;
    cpu(cpu_id).run(cpu_id, memory_size_);
    return 0;
}
