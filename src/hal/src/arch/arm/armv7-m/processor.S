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

.syntax unified
.fpu softvfp
.thumb

.global arm_start_schedule
.global arm_thread_thunk
.global arm_yield

.section .text.arm_start_schedule,"ax",%progbits
arm_start_schedule:
    ldmia r0!, {r4-r11, lr}
    ldr sp, [r0]
    cpsie i
    bx lr
.size arm_start_schedule, .-arm_start_schedule

// r4: start
// r5: arg0
// r6: arg1
.section .text.arm_thread_thunk,"ax",%progbits
arm_thread_thunk:
    mov r0, r5
    mov r1, r6
    bx r4
.size arm_thread_thunk, .-arm_thread_thunk
    
// r0: old_ctx
// r1: new_ctx
.section .text.arm_yield,"ax",%progbits
arm_yield:
    stmia r0!, {r4-r11, lr}
    str sp, [r0]

    ldmia r1!, {r4-r11, lr}
    ldr sp, [r1]
    bx lr

.size arm_yield, .-arm_yield