bits 32

extern isr_handler    ; C function that treats the interruptions

; Macro that generates a stub for interrupts that do NOT push an error code
%macro ISR_NO_ERR 1
global isr%1
isr%1:
    push 0            ; fake error code (to keep the stack uniform)
    push %1           ; interrupt number
    jmp isr_common
%endmacro

; Macro that generates a stub for interrupts that ALREADY push an error code
%macro ISR_ERR 1
global isr%1
isr%1:
    push %1           ; interrupt number (error code is already on the stack)
    jmp isr_common
%endmacro

; CPU exceptions (0-31)
ISR_NO_ERR 0    ; Division by zero
ISR_NO_ERR 1    ; Debug
ISR_NO_ERR 2    ; NMI
ISR_NO_ERR 3    ; Breakpoint
ISR_NO_ERR 4    ; Overflow
ISR_NO_ERR 5    ; Bound range exceeded
ISR_NO_ERR 6    ; Invalid opcode
ISR_NO_ERR 7    ; Device not available
ISR_ERR    8    ; Double fault
ISR_NO_ERR 9    ; Coprocessor segment overrun
ISR_ERR    10   ; Invalid TSS
ISR_ERR    11   ; Segment not present
ISR_ERR    12   ; Stack-segment fault
ISR_ERR    13   ; General protection fault
ISR_ERR    14   ; Page fault
ISR_NO_ERR 15   ; Reserved
ISR_NO_ERR 16   ; x87 floating-point exception
ISR_ERR    17   ; Alignment check
ISR_NO_ERR 18   ; Machine check
ISR_NO_ERR 19   ; SIMD floating-point exception
ISR_NO_ERR 20
ISR_NO_ERR 21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_NO_ERR 30
ISR_NO_ERR 31

; Common code for all stubs
isr_common:
    pusha              ; stores EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    call isr_handler   ; calls C handler

    popa               ; restores registers
    add esp, 8         ; removes error code + interrupt number from stack
    iret               ; returns from interrupt 