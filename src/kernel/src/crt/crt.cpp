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
#include <cstring>
#include <memory>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <chino/memory.h>
#include <chino/io.h>

using namespace chino;
using namespace chino::memory;

#define STDIN_FILENO 0 /* standard input file descriptor */
#define STDOUT_FILENO 1 /* standard output file descriptor */
#define STDERR_FILENO 2 /* standard error file descriptor */

#define FILENO_OFFSET 3

static handle_t tohandle(int fno) noexcept
{
    if (fno < 0)
        return handle_t::invalid();
    else if (fno < FILENO_OFFSET)
        return io::get_std_handle((io::std_handles)fno);
    return handle_t { uint16_t(fno - FILENO_OFFSET) };
}

extern "C"
{
    __attribute__((weak)) void *__dso_handle = 0;

    void *malloc(size_t n)
    {
        auto ret = heap_alloc(n);
        if (ret.is_ok())
            return ret.unwrap();
        return nullptr;
    }

    void free(void *p)
    {
        heap_free(p);
    }

    void *realloc(void *p, size_t n)
    {
        auto ret = heap_realloc(p, n);
        if (ret.is_ok())
            return ret.unwrap();
        return nullptr;
    }

    void *calloc(size_t num, size_t size)
    {
        const auto n = num * size;
        auto p = malloc(n);
        if (p)
            memset(p, 0, n);
        return p;
    }

    void *_malloc_r(struct _reent *, size_t n)
    {
        return malloc(n);
    }

    void _free_r(struct _reent *, void *p)
    {
        free(p);
    }

    void *_realloc_r(struct _reent *, void *p, size_t n)
    {
        return realloc(p, n);
    }

    void *_calloc_r(struct _reent *, size_t num, size_t size)
    {
        return calloc(num, size);
    }

    int _close(int file) __attribute__((alias("close")));
    int _fstat(int file, struct stat *st) __attribute__((alias("fstat")));
    int _getpid() __attribute__((alias("getpid")));
    int _isatty(int file) __attribute__((alias("isatty")));
    int _kill(int pid, int sig) __attribute__((alias("kill")));
    int _lseek(int file, int ptr, int dir) __attribute__((alias("lseek")));
    int _open(const char *name, int flags, ...) __attribute__((alias("open")));
    int _read(int file, char *ptr, int len) __attribute__((alias("read")));
    int _write(int file, char *ptr, int len) __attribute__((alias("write")));
    int _gettimeofday(struct timeval *__restrict p, void *__restrict tz) __attribute__((alias("gettimeofday")));

    void _exit(int)
    {
        while (1)
            ;
    }

    int close(int file)
    {
        errno = ENOSYS;
        return -1;
    }

    char **environ; /* pointer to array of char * strings that define the current environment variables */
    int execve(const char *name, char *const *argv, char *const *env);
    int fork();

    int fstat(int file, struct stat *st)
    {
        errno = ENOSYS;
        return -1;
    }

    int getpid()
    {
        errno = ENOSYS;
        return -1;
    }

    int isatty(int file)
    {
        errno = ENOSYS;
        return 0;
    }

    int kill(int pid, int sig)
    {
        //g_Logger->PutString("kill\n");
        errno = ENOSYS;
        return -1;
    }

    int link(char *old, char *newl);

    int lseek(int file, int ptr, int dir)
    {
        errno = ENOSYS;
        return -1;
    }

    int open(const char *name, int flags, ...)
    {
        errno = ENOSYS;
        return -1;
    }

    int read(int file, char *ptr, int len)
    {
        auto handle = tohandle(file);
        if (handle == handle_t::invalid())
        {
            errno = EBADF;
            return -1;
        }
        else
        {
            auto ret = io::read(handle, { reinterpret_cast<gsl::byte *>(ptr), size_t(len) });
            if (ret.is_ok())
                return (int)ret.unwrap();
            else
            {
                errno = EIO;
                return -1;
            }
        }
    }

    caddr_t sbrk(int incr);
    int stat(const char *file, struct stat *st);
    clock_t times(struct tms *buf);
    int unlink(char *name);
    int wait(int *status);

    int write(int file, char *ptr, int len)
    {
        auto handle = tohandle(file);
        if (handle == handle_t::invalid())
        {
            errno = EBADF;
            return -1;
        }
        else
        {
            if (io::write(handle, { reinterpret_cast<const gsl::byte *>(ptr), size_t(len) }).is_ok())
                return len;
            else
            {
                errno = EIO;
                return -1;
            }
        }
    }

    int gettimeofday(struct timeval *__restrict p, void *__restrict tz)
    {
        errno = ENOSYS;
        return -1;
    }

    void *_sbrk(ptrdiff_t incr)
    {
        return nullptr;
    }
}
