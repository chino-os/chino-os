# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_start_schedule
.global emulator_dispatch_irq
.global emulator_dispatch_irq_end
.global emulator_yield

.extern emulator_restore_irq
.extern emulator_irq_state
.extern emulator_in_irq_handler
.extern io_handle_irq
.extern ps_switch_task

STACK_TOP_OFFSET    = 0x08
STACK_BOTTOM_OFFSET = 0x10

.text
.align 16

# rcx = new_thread
# after restore:
#   rcx = thread_main_thunk
#   rdx = entry_point
#   r8  = entry_arg
emulator_start_schedule:
    mov     STACK_TOP_OFFSET(%rcx),     %rsp

    # set stack limit
    mov     %gs:0x30,   %rax                        # TIB
    mov     %rsp,       8(%rax)                     # StackBase
    mov     STACK_BOTTOM_OFFSET(%rcx),  %rdx        # StackLimit
    mov     %rdx,       16(%rax)

    pop     %rcx
    call    emulator_restore_irq        # restore irq

    # restore context
    addq    $8*2,       %rsp            # skip rax, rbx
    pop     %rcx
    pop     %rdx
    addq    $8*11,      %rsp            # skip rbp, rsi, rdi, r8-r15
    pop     %rax
    jmp     *%rax
    
emulator_dispatch_irq:
    # 1. save context
    xchg    %r15,       8*2(%rsp)       # syscall.arg
    xchg    %r14,       8*1(%rsp)       # syscall.number
    xchg    %r13,       8*0(%rsp)       # irq number
    push    %r12
    push    %r11
    push    %r10
    push    %r9
    push    %r8
    push    %rdi
    push    %rsi
    push    %rbp
    push    %rdx
    push    %rcx
    push    %rbx
    push    %rax
    lea     emulator_irq_state(%rip),       %rax
    xor     %rbx,       %rbx
    lock xchg           (%rax),     %rbx
    push    %rbx                                    # store irq state

    lea     chino_current_threads(%rip),    %rax
    mov     (%rax),     %rax
    mov     %rsp,       STACK_TOP_OFFSET(%rax)      # store stack_top

    # 2. call handler
    mov     %r13,       %rcx
    mov     %r14,       %rdx
    mov     %r15,       %r8
    call    io_handle_irq

    # 3. restore context
    lea     chino_current_threads(%rip),    %rax
    mov     (%rax),     %rax
    mov     STACK_TOP_OFFSET(%rax),         %rsp    # load stack_top

    # set stack limit
    mov     %gs:0x30,   %rbx                        # TIB
    mov     %rsp,       8(%rbx)                     # StackBase
    mov     STACK_BOTTOM_OFFSET(%rax),      %rdx    # StackLimit
    mov     %rdx,       16(%rbx)

    pop     %rcx
    call    emulator_restore_irq        # restore irq

    lea     emulator_in_irq_handler(%rip),  %rax
    movw    $0,         (%rax)

    pop     %rax
    pop     %rbx
    pop     %rcx
    pop     %rdx
    pop     %rbp
    pop     %rsi
    pop     %rdi
    pop     %r8
    pop     %r9
    pop     %r10
    pop     %r11
    pop     %r12
    pop     %r13
    pop     %r14
    pop     %r15
    ret
emulator_dispatch_irq_end:
    
emulator_yield:
    # 1. save context
    push    %r15
    push    %r14
    push    %r13
    push    %r12
    push    %r11
    push    %r10
    push    %r9
    push    %r8
    push    %rdi
    push    %rsi
    push    %rbp
    push    %rdx
    push    %rcx
    push    %rbx
    push    %rax
    lea     emulator_irq_state(%rip),       %rax
    mov     (%rax),     %rax
    push    %rax                                    # store irq state

    lea     chino_current_threads(%rip),    %rax
    mov     (%rax),     %rax
    mov     %rsp,       STACK_TOP_OFFSET(%rax)      # store stack_top

    # 2. switch task
    call    ps_switch_task

    # 3. restore context
    lea     chino_current_threads(%rip),    %rax
    mov     (%rax),     %rax
    mov     STACK_TOP_OFFSET(%rax),         %rsp    # load stack_top

    # set stack limit
    mov     %gs:0x30,   %rbx                        # TIB
    mov     %rsp,       8(%rbx)                     # StackBase
    mov     STACK_BOTTOM_OFFSET(%rax),      %rdx    # StackLimit
    mov     %rdx,       16(%rbx)

    pop     %rcx
    call    emulator_restore_irq        # restore irq

    pop     %rax
    pop     %rbx
    pop     %rcx
    pop     %rdx
    pop     %rbp
    pop     %rsi
    pop     %rdi
    pop     %r8
    pop     %r9
    pop     %r10
    pop     %r11
    pop     %r12
    pop     %r13
    pop     %r14
    pop     %r15
    ret
