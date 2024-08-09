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

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Section Attributes
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#pragma section(".CRT$XCA", long, read) // First C++ Initializer
#pragma section(".CRT$XCAA", long, read) // Startup C++ Initializer
#pragma section(".CRT$XCZ", long, read) // Last C++ Initializer

#pragma section(".CRT$XDA", long, read) // First Dynamic TLS Initializer
#pragma section(".CRT$XDZ", long, read) // Last Dynamic TLS Initializer

#pragma section(".CRT$XIA", long, read) // First C Initializer
#pragma section(".CRT$XIAA", long, read) // Startup C Initializer
#pragma section(".CRT$XIAB", long, read) // PGO C Initializer
#pragma section(".CRT$XIAC", long, read) // Post-PGO C Initializer
#pragma section(".CRT$XIC", long, read) // CRT C Initializers
#pragma section(".CRT$XIYA", long, read) // VCCorLib Threading Model Initializer
#pragma section(".CRT$XIYAA", long, read) // XAML Designer Threading Model Override Initializer
#pragma section(".CRT$XIYB", long, read) // VCCorLib Main Initializer
#pragma section(".CRT$XIZ", long, read) // Last C Initializer

#pragma section(".CRT$XLA", long, read) // First Loader TLS Callback
#pragma section(".CRT$XLC", long, read) // CRT TLS Constructor
#pragma section(".CRT$XLD", long, read) // CRT TLS Terminator
#pragma section(".CRT$XLZ", long, read) // Last Loader TLS Callback

#pragma section(".CRT$XPA", long, read) // First Pre-Terminator
#pragma section(".CRT$XPB", long, read) // CRT ConcRT Pre-Terminator
#pragma section(".CRT$XPX", long, read) // CRT Pre-Terminators
#pragma section(".CRT$XPXA", long, read) // CRT stdio Pre-Terminator
#pragma section(".CRT$XPZ", long, read) // Last Pre-Terminator

#pragma section(".CRT$XTA", long, read) // First Terminator
#pragma section(".CRT$XTZ", long, read) // Last Terminator

#pragma section(".CRTMA$XCA", long, read) // First Managed C++ Initializer
#pragma section(".CRTMA$XCZ", long, read) // Last Managed C++ Initializer

#pragma section(".CRTVT$XCA", long, read) // First Managed VTable Initializer
#pragma section(".CRTVT$XCZ", long, read) // Last Managed VTable Initializer

#pragma section(".rdata$T", long, read)

#pragma section(".rtc$IAA", long, read) // First RTC Initializer
#pragma section(".rtc$IZZ", long, read) // Last RTC Initializer

#pragma section(".rtc$TAA", long, read) // First RTC Terminator
#pragma section(".rtc$TZZ", long, read) // Last RTC Terminator

#define _CRTALLOC(x) __declspec(allocate(x))

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// "Special" Data
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#ifndef _M_CEE
typedef void(__cdecl *_PVFV)(void);
typedef int(__cdecl *_PIFV)(void);

extern _CRTALLOC(".CRT$XIA") _PIFV __xi_a[]; // First C Initializer
extern _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[]; // Last C Initializer
extern _CRTALLOC(".CRT$XCA") _PVFV __xc_a[]; // First C++ Initializer
extern _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[]; // Last C++ Initializer
extern _CRTALLOC(".CRT$XPA") _PVFV __xp_a[]; // First Pre-Terminator
extern _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[]; // Last Pre-Terminator
extern _CRTALLOC(".CRT$XTA") _PVFV __xt_a[]; // First Terminator
extern _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[]; // Last Terminator
#endif

// Call C constructors
static int _initterm_e(_PIFV *pfbegin, _PIFV *pfend)
{
    int ret = 0;

    // walk the table of function pointers from the bottom up, until
    // the end is encountered.  Do not skip the first entry.  The initial
    // value of pfbegin points to the first valid entry.  Do not try to
    // execute what pfend points to.  Only entries before pfend are valid.
    while (pfbegin < pfend && ret == 0)
    {
        // if current table entry is non-NULL, call thru it.
        if (*pfbegin != 0)
            ret = (**pfbegin)();
        ++pfbegin;
    }

    return ret;
}

// Call C++ constructors
static void _initterm(_PVFV *pfbegin, _PVFV *pfend)
{
    // walk the table of function pointers from the bottom up, until
    // the end is encountered.  Do not skip the first entry.  The initial
    // value of pfbegin points to the first valid entry.  Do not try to
    // execute what pfend points to.  Only entries before pfend are valid.
    while (pfbegin < pfend)
    {
        // if current table entry is non-NULL, call thru it.
        if (*pfbegin != 0)
            (**pfbegin)();
        ++pfbegin;
    }
}
