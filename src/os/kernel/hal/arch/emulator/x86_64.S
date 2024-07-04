# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_restore_context
.global emulator_start_schedule

.text
.align 16

# rcx = context
emulator_restore_context:
    mov 0(%rcx), %rax
    mov 8(%rcx), %rbx
    jmp .

# rcx = thread
# rdx = entry_point
# r8 = entry_arg
emulator_start_schedule:
    mov 8*3(%rcx), %rdx
    mov 8*4(%rcx), %rsp
    mov 8*8(%rcx), %r8
    mov 8*16(%rcx), %rax    # rip
    push $0                 # avoid unwind
    mov 8*2(%rcx), %rcx
    jmp *%rax
