org 0x7C00                      ; Tells NASM that the code will be loaded at 0x7C00

mov si, msg                     ; Points SI to the first byte of string (msg)

loop:
    lodsb                       ; AL = byte that SI points to, then SI++
    cmp al, 0                   ; Is this byte 0?
    je halt                     ; je = jump if equal, in this case, if the string has ended, we jumps to the halt label
    mov ah, 0x0E                ; teletype function
    int 0x10                    ; prints the character in AL
    jmp loop                    ; jumps back to the start of the loop, essentially loading the next character of the string until it reaches the null terminator (0)

halt:
    jmp $                       ; halt :D

msg db 'PoppyOS', 0             ; string to be printed, null-terminated (0 at the end)

times 510 - ($ - $$) db 0       ; Fill the rest of the 512-byte sector with zeros
dw 0xAA55                       ; Boot signature, required for the BIOS to recognize this as a valid boot sector
