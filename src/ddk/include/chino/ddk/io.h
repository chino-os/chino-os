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
#pragma once
#include "object.h"
#include "utility.h"
#include <chino/io.h>
#include <chino/ps.h>
#include <gsl/gsl-lite.hpp>

namespace chino::io
{
#if defined(_MSC_VER)
#pragma section(".CHDRV$A", long, read) // Begin drivers
#pragma section(".CHDRV$C", long, read) // Drivers
#pragma section(".CHDRV$Z", long, read) // End drivers
#define EXPORT_DRIVER(x) __declspec(allocate(".CHDRV$C")) const ::chino::io::driver *CHINO_CONCAT(_drv_, __COUNTER__) = &x

#pragma section(".CHHDR$A", long, read) // Begin hardware device registrations
#pragma section(".CHHDR$C", long, read) // Hardware device registrations
#pragma section(".CHHDR$Z", long, read) // End hardware device registrations
#define EXPORT_HARDWARE(x) __declspec(allocate(".CHHDR$C")) const ::chino::io::hardware_device_registration *CHINO_CONCAT(_hdr_, __COUNTER__) = &x
#elif defined(__GNUC__)
#define EXPORT_DRIVER(x) __attribute__((unused, section(".chdrv"))) const ::chino::io::driver *CHINO_CONCAT(_drv_, __COUNTER__) = &x
#define EXPORT_HARDWARE(x) __attribute__((unused, section(".chhdr"))) const ::chino::io::hardware_device_registration *CHINO_CONCAT(_hdr_, __COUNTER__) = &x
#else
#error "Unsupported compiler"
#endif

struct driver;
struct device;
struct file;

#define DEFINE_DEV_TYPE(x, v) x = v,
enum class device_type
{
#include "device_types.def"
    COUNT
};
#undef DEFINE_DEV_TYPE

enum class driver_type
{
    hardware,
    console
};

struct hardware_device_registration
{
    const driver &drv;
};

typedef result<void, error_code> (*driver_add_device_t)(const driver &drv, const hardware_device_registration &hdr);
typedef result<void, error_code> (*driver_attach_device_t)(const driver &drv, device &bottom_dev, std::string_view args);
typedef result<file *, error_code> (*driver_open_device_t)(device &dev, std::string_view filename, create_disposition create_disp);
typedef result<void, error_code> (*driver_close_device_t)(file &file);
typedef result<size_t, error_code> (*driver_read_device_t)(file &file, gsl::span<gsl::byte> buffer);
typedef result<void, error_code> (*driver_write_device_t)(file &file, gsl::span<const gsl::byte> buffer);

struct driver_operations
{
    driver_add_device_t add_device;
    driver_attach_device_t attach_device;
    driver_open_device_t open_device;
    driver_close_device_t close_device;
    driver_read_device_t read_device;
    driver_write_device_t write_device;
};

struct driver
{
    driver_type type = driver_type::hardware;
    std::string_view name;
    driver_operations ops;
};

struct device : ob::object
{
    const driver &drv;
    device_type type;
    threading::sched_spinlock syncroot;

    template <class T = uint8_t>
    T &extension() noexcept { return *reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(this) + sizeof(device)); }
};

struct device_extension
{
    device &dev() noexcept { return *reinterpret_cast<device *>(reinterpret_cast<uint8_t *>(this) - sizeof(device)); }
};

struct file : ob::object
{
    device &dev;
    size_t offset;
    threading::sched_spinlock syncroot;

    template <class T = uint8_t>
    T &extension() noexcept { return *reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(this) + sizeof(file)); }
};

struct file_extension
{
    io::file &file() noexcept { return *reinterpret_cast<io::file *>(reinterpret_cast<uint8_t *>(this) - sizeof(device)); }
};

result<device *, error_code> create_device(const driver &drv, device_type type, size_t extension_size) noexcept;
result<device *, error_code> create_device(std::string_view name, const driver &drv, device_type type, size_t extension_size) noexcept;
result<file *, error_code> create_file(device &dev, size_t extension_size) noexcept;

result<file *, error_code> open_file(device &dev, access_mask access, std::string_view filename, create_disposition create_disp = create_disposition::open_existing) noexcept;
result<size_t, error_code> read_file(file &file, gsl::span<gsl::byte> buffer) noexcept;
result<void, error_code> write_file(file &file, gsl::span<const gsl::byte> buffer) noexcept;
result<void, error_code> close_file(file &file) noexcept;
}
