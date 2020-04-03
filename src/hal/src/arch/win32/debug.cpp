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
#include <chino/io.h>
#include <chino/memory.h>
#include <new>
#include <stdexcept>

extern "C" int _fltused = 0x9875;

using namespace chino;

void ::operator delete(void *ptr, size_t size)
{
}

_STD_BEGIN
    [[noreturn]] _CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL
    _Xout_of_range(
        _In_z_ const char *_Message)
{ // report an out_of_range error
    _THROW(out_of_range(_Message));
}
_STD_END

static FILE *tofile(handle_t handle) noexcept
{
    return reinterpret_cast<FILE *>(handle.value);
}

static FILE *tofile(result<handle_t, error_code> handle) noexcept
{
    if (handle.is_ok())
        return reinterpret_cast<FILE *>(handle.unwrap().value);
    else
        return nullptr;
}

static handle_t tohandle(FILE *fp) noexcept
{
    if (fp)
        return { reinterpret_cast<uint16_t>(fp) };
    else
        return handle_t::invalid();
}

extern "C"
{
    int __cdecl puts(char const *_Buffer)
    {
        char buffer[258];
        auto len = strlen(_Buffer);
        if (len > 256)
            return EOF;

        strcpy(buffer, _Buffer);
        buffer[len] = '\n';
        buffer[len + 1] = 0;
        if (io::write(io::get_std_handle(io::std_handles::out), { reinterpret_cast<const gsl::byte *>(buffer), len + 1 }).is_ok())
            return len + 1;
        return EOF;
    }

    FILE *__cdecl __acrt_iob_func(unsigned _Ix)
    {
        auto handle = io::get_std_handle((io::std_handles)_Ix);
        return tofile(handle);
    }

    int __cdecl __stdio_common_vfprintf(
        _In_ unsigned __int64 _Options,
        _Inout_ FILE *_Stream,
        _In_z_ _Printf_format_string_params_(2) char const *_Format,
        _In_opt_ _locale_t _Locale,
        va_list _ArgList)
    {
        char buffer[258];
        auto ret = vsprintf_s(buffer, _Format, _ArgList);
        if (ret < 0)
            return ret;

        if (io::write(io::get_std_handle(io::std_handles::out), { reinterpret_cast<const gsl::byte *>(buffer), size_t(ret) }).is_ok())
            return ret;
        return EOF;
    }

    int __cdecl putchar(
        _In_ int _Character)
    {
        char ch = _Character;
        if (io::write(io::get_std_handle(io::std_handles::out), gsl::byte_span(ch)).is_ok())
            return 1;
        return EOF;
    }

    int __cdecl getchar(void)
    {
        char ch;
        if (io::read(io::get_std_handle(io::std_handles::out), gsl::byte_span(ch)).is_ok())
            return ch;
        return EOF;
    }

    void *__cdecl malloc(size_t size)
    {
        auto ret = memory::heap_alloc(size);
        if (ret.is_ok())
            return ret.unwrap();
        return nullptr;
    }

    void *__cdecl realloc(void *ptr, size_t size)
    {
        auto ret = memory::heap_realloc(ptr, size);
        if (ret.is_ok())
            return ret.unwrap();
        return nullptr;
    }

    void __cdecl free(void *ptr)
    {
        memory::heap_free(ptr);
    }

    int __cdecl fflush(
        _Inout_opt_ FILE *_Stream)
    {
        return 0;
    }

    char *__cdecl strdup(
        _In_opt_z_ char const *_String)
    {
        if (_String == nullptr)
            return nullptr;

        size_t const size = strlen(_String) + 1;
        char *const memory = static_cast<char *>(malloc(size));
        if (memory == nullptr)
            return nullptr;

        strcpy(memory, _String);
        return memory;
    }

    FILE *__cdecl fopen(
        _In_z_ char const *_FileName,
        _In_z_ char const *_Mode)
    {
        auto handle = io::open({ .name = _FileName, .desired_access = access_mask::generic_all });
        return tofile(handle);
    }

    char *__cdecl fgets(
        _Out_writes_z_(_MaxCount) char *_Buffer,
        _In_ int _MaxCount,
        _Inout_ FILE *_Stream)
    {
        if (!_Buffer || _MaxCount < 1)
            return nullptr;

        auto handle = tohandle(_Stream);
        size_t idx = 0;

        while (idx < _MaxCount - 1)
        {
            char ch;
            auto ret = io::read(handle, gsl::byte_span(ch));

            if (ret.is_ok())
            {
                if (ret.unwrap() == 0 || ch == 0)
                    break;
                else
                    _Buffer[idx++] = ch;

                if (ch == '\n')
                    break;
            }
            else
            {
                return nullptr;
            }
        }

        if (idx == 0)
            return nullptr;
        _Buffer[idx] = 0;
        return _Buffer;
    }

    int __cdecl fputs(
        _In_z_ char const *_Buffer,
        _Inout_ FILE *_Stream)
    {
        if (!_Buffer)
            return EOF;

        auto handle = tohandle(_Stream);
        auto len = strlen(_Buffer);
        if (io::write(handle, { reinterpret_cast<const gsl::byte *>(_Buffer), len }).is_ok())
            return len;
        return EOF;
    }

    int __cdecl fclose(
        _Inout_ FILE *_Stream)
    {
        auto ret = io::close(tohandle(_Stream));
        if (ret.is_ok())
            return 0;
        return -1;
    }

    void _exit(int code)
    {
    }

    void __chkstk()
    {
    }
}
