// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/hal/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>
#include <chino/version.h>
#include <format>
// #include <lyra/lyra.hpp>
#include <memory>
#include <string>

#ifdef WIN32
#include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

using namespace chino::os::hal;
using namespace chino::os::kernel;

#define SYSTEM_PATH ""
#define KERNEL_FILENAME "chino.kernel"
#define HAL_STARTUP_STR "hal_startup"

#ifdef WIN32
#define DYNLIB_PREFIX
#define DYNLIB_EXT ".dll"
#elif defined(__unix__) || defined(__APPLE__)
#define DYNLIB_PREFIX "lib"
#ifdef __unix__
#define DYNLIB_EXT ".so"
#else
#define DYNLIB_EXT ".dylib"
#endif
#endif

#define KERNEL_FILEPATH SYSTEM_PATH DYNLIB_PREFIX KERNEL_FILENAME DYNLIB_EXT

namespace {
typedef void (*hal_startup_t)(size_t);

inline constexpr size_t default_memory_size = 1024 * 1024 * 64; // 64MB

hal_startup_t load_hal_startup() {
#ifdef WIN32
    auto kernel_lib = LoadLibraryExA(KERNEL_FILEPATH, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!kernel_lib) {
        ExitProcess(-1);
        // throw std::runtime_error(std::format("Unable to load " KERNEL_FILENAME ": {:#08X}", GetLastError()));
    }

    auto kernel_entryp = reinterpret_cast<hal_startup_t>(GetProcAddress(kernel_lib, HAL_STARTUP_STR));
    if (!kernel_entryp) {
        ExitProcess(-1);
        // throw std::runtime_error(std::format("Unable to load kernel entrypoint: {:#08X}", GetLastError()));
    }
#else
    auto kernel_lib = dlopen(KERNEL_FILEPATH, RTLD_NOW);
    if (!kernel_lib) {
        throw std::runtime_error(std::string("Unable to load " KERNEL_FILENAME ": ") + dlerror());
    }

    auto kernel_entryp = reinterpret_cast<hal_startup_t>(dlsym(kernel_lib, HAL_STARTUP_STR));
    if (!kernel_entryp) {
        throw std::runtime_error(dlerror());
    }
#endif
    return kernel_entryp;
}

void print(std::string_view str) {
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str.data(), str.size(), nullptr, nullptr);
}

} // namespace

extern "C" int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                              _In_ int nShowCmd) {
    bool show_version = false;
    size_t memory_size = default_memory_size;

    // auto cli = lyra::cli();
    // cli.add_argument(lyra::opt(show_version).name("-v").name("--version").help("show version"));
    // cli.add_argument(
    //     lyra::opt(memory_size, "memory size").optional().name("-m").name("--memory_size").help("set memory size"));
    //
    // cli.parse({argc, argv});

    if (show_version) {
        print("Chino OS emulator " CHINO_VERSION CHINO_VERSION_SUFFIX " Build " __DATE__ "\n");
        print("Copyright (c) SunnyCase.\n");
        return 0;
    }

    auto hal_startup = load_hal_startup();
    hal_startup(memory_size);
}

extern "C" void __cdecl WinMainCRTStartup(void) {
    int mainret;
    LPSTR lpszCommandLine;
    STARTUPINFO StartupInfo;

    lpszCommandLine = (LPSTR)GetCommandLine();

    if (*lpszCommandLine == '"') {
        lpszCommandLine++;
        while (*lpszCommandLine && (*lpszCommandLine != '"'))
            lpszCommandLine++;

        if (*lpszCommandLine == '"')
            lpszCommandLine++;
    } else {
        while (*lpszCommandLine > ' ')
            lpszCommandLine++;
    }

    while (*lpszCommandLine && (*lpszCommandLine <= ' '))
        lpszCommandLine++;

    StartupInfo.dwFlags = 0;
    GetStartupInfo(&StartupInfo);

    mainret = WinMain(GetModuleHandle(NULL), NULL, lpszCommandLine,
                      StartupInfo.dwFlags & STARTF_USESHOWWINDOW ? StartupInfo.wShowWindow : SW_SHOWDEFAULT);

    ExitProcess(mainret);
}
