# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_restore_context

.text
.align 16

# rcx = context
emulator_restore_context:
    mov 8*3(%rcx), %rdx
    mov 8*4(%rcx), %rsp
    mov 8*8(%rcx), %r8
    mov 8*16(%rcx), %rax    # rip
    mov 8*2(%rcx), %rcx
    jmp *%rax
