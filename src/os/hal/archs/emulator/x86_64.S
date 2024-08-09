# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_thread_start

.extern emulator_restore_irq
.extern emulator_irq_state

STACK_TOP_OFFSET    = 0x08
STACK_BOTTOM_OFFSET = 0x10
IRQ_STATE_OFFSET    = 0x20

.text
.align 16

# rcx = new_thread
# after restore:
#   rcx = thread_main_thunk
#   rdx = entry_point
#   r8  = entry_arg
emulator_thread_start:
    mov     STACK_TOP_OFFSET(%rcx),     %rsp

    # set stack limit
    mov     %gs:0x30,   %rax                        # TIB
    mov     %rsp,       8(%rax)                     # StackBase
    mov     STACK_BOTTOM_OFFSET(%rcx),  %rdx        # StackLimit
    mov     %rdx,       16(%rax)

    mov     IRQ_STATE_OFFSET(%rcx),     %rcx
    call    emulator_restore_irq        # restore irq

    # restore context
    addq    $8*2,       %rsp            # skip rax, rbx
    pop     %rcx
    pop     %rdx
    addq    $8*11,      %rsp            # skip rbp, rsi, rdi, r8-r15
    pop     %rax
    jmp     *%rax
    