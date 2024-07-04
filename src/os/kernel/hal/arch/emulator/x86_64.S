# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_restore_context

.text
.align 16

# rcx = context
emulator_restore_context:
    mov 0(%rcx), %rax
    mov 8(%rcx), %rbx
    jmp .
