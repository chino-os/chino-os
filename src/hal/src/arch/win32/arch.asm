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
PUBLIC win32_yield

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
    
; RCX: old_ctx
; RDX: new_ctx
win32_yield:
    mov [rcx], rbx
    mov [rcx + 08h], rbp
    mov [rcx + 10h], rdi
    mov [rcx + 18h], rsi
    mov [rcx + 20h], rsp
    mov [rcx + 28h], r12
    mov [rcx + 30h], r13
    mov [rcx + 38h], r14
    mov [rcx + 40h], r15

    movdqa [rcx + 50h], xmm6
    movdqa [rcx + 60h], xmm7
    movdqa [rcx + 70h], xmm8
    movdqa [rcx + 80h], xmm9
    movdqa [rcx + 90h], xmm10
    movdqa [rcx + 0A0h], xmm11
    movdqa [rcx + 0B0h], xmm12
    movdqa [rcx + 0C0h], xmm13
    movdqa [rcx + 0D0h], xmm14
    movdqa [rcx + 0E0h], xmm15

    mov rbx, [rdx]
    mov rbp, [rdx + 08h]
    mov rdi, [rdx + 10h]
    mov rsi, [rdx + 18h]
    mov rsp, [rdx + 20h]
    mov r12, [rdx + 28h]
    mov r13, [rdx + 30h]
    mov r14, [rdx + 38h]
    mov r15, [rdx + 40h]

    movdqa xmm6, [rdx + 50h]
    movdqa xmm7, [rdx + 60h]
    movdqa xmm8, [rdx + 70h]
    movdqa xmm9, [rdx + 80h]
    movdqa xmm10, [rdx + 90h]
    movdqa xmm11, [rdx + 0A0h]
    movdqa xmm12, [rdx + 0B0h]
    movdqa xmm13, [rdx + 0C0h]
    movdqa xmm14, [rdx + 0D0h]
    movdqa xmm15, [rdx + 0E0h]
    ret

; RBX: start
; RDI: arg0
; RSI: arg1
win32_thread_thunk:
    mov rcx, rdi
    mov rdx, rsi
    jmp rbx

END
