; MIT License
;
; Copyright (c) 2020 SunnyCase
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

PUBLIC win32_start_schedule
PUBLIC win32_thread_thunk

.code

win32_start_schedule:
    mov rbx, [rcx]
    mov rbp, [rcx + 08h]
    mov rdi, [rcx + 10h]
    mov rsi, [rcx + 18h]
    mov rsp, [rcx + 20h]
    mov r12, [rcx + 28h]
    mov r13, [rcx + 30h]
    mov r14, [rcx + 38h]
    mov r15, [rcx + 40h]

    movdqa xmm6, [rcx + 50h]
    movdqa xmm7, [rcx + 60h]
    movdqa xmm8, [rcx + 70h]
    movdqa xmm9, [rcx + 80h]
    movdqa xmm10, [rcx + 90h]
    movdqa xmm11, [rcx + 0A0h]
    movdqa xmm12, [rcx + 0B0h]
    movdqa xmm13, [rcx + 0C0h]
    movdqa xmm14, [rcx + 0D0h]
    movdqa xmm15, [rcx + 0E0h]
    ret

; RBX: start
; RDI: arg0
; RSI: arg1
win32_thread_thunk:
    mov rcx, rdi
    mov rdx, rsi
    jmp rbx

END
