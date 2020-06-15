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
#include <chino/arch/win32/target.h>
#include <chino/ddk/io.h>

using namespace chino;
using namespace chino::io;

namespace
{
struct win32_fs_device_registration : hardware_device_registration
{
    std::string_view path;
};

struct win32_fs_file : public file_extension
{
    HANDLE handle;
};

class win32_fs_dev : public device_extension
{
public:
    win32_fs_dev(std::string_view root)
    {
        CreateDirectoryA(root.data(), nullptr);
        auto error = GetLastError();
        if (error != ERROR_SUCCESS && error != ERROR_ALREADY_EXISTS)
            panic("cannot create win32 root directory");

        auto len = GetFullPathNameA(root.data(), 0, nullptr, nullptr);
        auto path = (char *)malloc(len);
        assert(GetFullPathNameA(root.data(), len, path, nullptr) == len - 1);
        root_ = { path, len - 1 };
    }

    result<file *, error_code> open(std::string_view filename, create_disposition create_disp) noexcept
    {
        auto len = root_.length() + 1 + filename.length() + 1;
        auto fullname = (char *)malloc(len);
        snprintf(fullname, len, "%s\\%s", root_.data(), filename.data());
        auto handle = CreateFileA(fullname, GENERIC_READ | GENERIC_WRITE, 0, nullptr, (DWORD)create_disp, 0, nullptr);
        free(fullname);
        if (handle == INVALID_HANDLE_VALUE)
            return err(error_code::not_found);
        try_var(f, io::create_file(dev(), sizeof(win32_fs_file)));
        f->extension<win32_fs_file>().handle = handle;
        return ok(f);
    }

    result<size_t, error_code> read(file &file, gsl::span<gsl::byte> buffer) noexcept
    {
        DWORD read = 0;
        auto h = file.extension<win32_fs_file>().handle;
        if (ReadFile(file.extension<win32_fs_file>().handle, buffer.data(), buffer.length_bytes(), &read, nullptr))
            return ok<size_t>(read);
        return err(error_code::io_error);
    }

    result<void, error_code> write(file &file, gsl::span<const gsl::byte> buffer) noexcept
    {
        if (WriteFile(file.extension<win32_fs_file>().handle, buffer.data(), buffer.length_bytes(), nullptr, nullptr))
            return ok();
        return err(error_code::io_error);
    }

private:
    std::string_view root_;
};

result<void, error_code> fs_add_device(const driver &drv, const hardware_device_registration &hdr)
{
    auto &reg = static_cast<const win32_fs_device_registration &>(hdr);
    try_var(fs, create_device(drv, device_type::fs, sizeof(win32_fs_dev)));
    new (&fs->extension()) win32_fs_dev(reg.path);
    return ok();
}

result<file *, error_code> fs_open_device(device &dev, std::string_view filename, create_disposition create_disp)
{
    return dev.extension<win32_fs_dev>().open(filename, create_disp);
}

result<void, error_code> fs_close_device(file &file)
{
    if (CloseHandle(file.extension<win32_fs_file>().handle))
        return ok();
    return err(error_code::unknown);
}

result<size_t, error_code> fs_read_device(file &file, gsl::span<gsl::byte> buffer)
{
    return file.dev.extension<win32_fs_dev>().read(file, buffer);
}

result<void, error_code> fs_write_device(file &file, gsl::span<const gsl::byte> buffer)
{
    return file.dev.extension<win32_fs_dev>().write(file, buffer);
}

const driver fs_drv = {
    .name = "win32-fs",
    .ops = { .add_device = fs_add_device, .open_device = fs_open_device, .close_device = fs_close_device, .read_device = fs_read_device, .write_device = fs_write_device },
};
#include <win32-fs.inl>
}
