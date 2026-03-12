org 0x7C00                      ; Tells NASM that the code will be loaded at 0x7C00

mov ah, 0x0E                    ; BIOS teletype function
mov al, 'P'                     ; Character to print
int 0x10                        ; Call BIOS, print P

jmp $                           ; Infinite loop, halts the CPU

times 510 - ($ - $$) db 0       ; Fill the rest of the 512-byte sector with zeros
dw 0xAA55                       ; Boot signature, required for the BIOS to recognize this as a valid boot sector
