bits 32
global context_switch

; void context_switch(uint32_t* old_esp, uint32_t new_esp)
;
; Saves all registers on the current stack,
; stores the current ESP in *old_esp,
; switches to new_esp,
; restores registers from the new stack,
; ret jumps to the EIP that was on the new stack.

context_switch:
    pusha                     ; saves 8 registers on the current stack (32 bytes)

    mov eax, [esp + 36]       ; first argument: &old_esp
                              ; 32 (pusha) + 4 (return addr) = offset 36
    mov [eax], esp            ; saves the current ESP in *old_esp

    mov eax, [esp + 40]       ; second argument: new_esp
                              ; 36 + 4 = offset 40
    mov esp, eax              ; SWITCH STACK, from here we are in the new task

    popa                      ; restores registers from the new stack
    ret                       ; pops the EIP from the new stack, jumps there