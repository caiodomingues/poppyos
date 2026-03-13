org 0x7C00                      ; Tells NASM that the code will be loaded at 0x7C00

xor ax, ax                      ; Clear AX register (sets it to 0)
mov ds, ax                      ; Set DS segment to 0 (pointing to the start of memory)
mov es, ax                      ; Set ES segment to 0 as well (for string operations)

; 1 - Store the boot drive (BIOS left in DL) for later use
mov [BOOT_DRIVE], dl

; 2 - Configure stack
mov bp, 0x7C00                  ; Base pointer for stack (reference)
mov sp, bp                      ; Stack top, grows downwards (0x7C00)

; 3 - Prints the message from stage 1
mov si, MSG_STAGE1
call print                      ; Now we have a working stack, we can use call/ret

; 4 - Loads stage 2 from disk
mov ah, 0x02                    ; Function: read sectors
mov al, 32                       ; How many sectors to read (32 sectors = 16KB)
mov ch, 0                       ; Cylinder 0
mov cl, 2                       ; Sector 2 (right after the boot sector)
mov dh, 0                       ; Head 0
mov dl, [BOOT_DRIVE]            ; Boot drive (the one we've stored earlier)
mov bx, 0x7E00                  ; Target memory address to load stage 2
int 0x13                        ; Call BIOS disk interrupt (reads disk)

jc disk_error                   ; If error occurs, carry flag (CF) will be set as 1

jmp 0x7E00                      ; Jump to the loaded stage 2

print:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

disk_error:
    mov si, MSG_ERROR
    call print
    jmp $

BOOT_DRIVE db 0
MSG_STAGE1 db 'Stage 1 OK', 13, 10, 0
MSG_ERROR db 'Disk error!', 0

times 510 - ($ - $$) db 0
dw 0xAA55
