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
#include <Windows.h>
#include <process.h>
#include <stdint.h>
#include <stdio.h>

static void read_main(void *p_port) {
    HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE port = (HANDLE)p_port;

    DWORD read = 0;
    uint8_t read_buf[256];
    while (ReadFile(h_stdin, read_buf, 256, &read, nullptr)) {
        if (!WriteFile(port, read_buf, read, nullptr, nullptr))
            break;
    }
}

int main(int argc, char *argv[]) {
    char port_name[MAX_PATH] = "\\\\.\\";
    if (argc == 1) {
        printf("Input port name: ");
        scanf("%s", port_name + 4);
    } else {
        strcat(port_name, argv[1]);
    }

    auto port = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (port == INVALID_HANDLE_VALUE) {
        printf("Cannot open %s\n", port_name);
        exit(1);
    }

    HANDLE h_stdin, h_stdout;
    h_stdin = GetStdHandle(STD_INPUT_HANDLE);
    h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    SetConsoleMode(h_stdin, ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT);
    SetConsoleMode(h_stdout, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    _beginthread(read_main, 0, port);

    DCB dcb{sizeof(DCB)};
    GetCommState(port, &dcb);
    dcb.BaudRate = 38400;
    dcb.ByteSize = 8;
    dcb.StopBits = 0;
    dcb.Parity = 0;
    SetCommState(port, &dcb);
    EscapeCommFunction(port, CLRDTR);

    DWORD written = 0;
    uint8_t write_buf[256];
    while (ReadFile(port, write_buf, 256, &written, nullptr)) {
        if (written) {
            if (!WriteFile(h_stdout, write_buf, written, nullptr, nullptr))
                break;
        }
    }

    return 0;
}
