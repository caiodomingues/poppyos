org 0x7E00

mov si, MSG_STAGE2
call print

cli                                     ; 1. Disable interruptions

lgdt [GDT_DESCRIPTOR]                   ; 2. Loads GDT address into CPU

mov eax, cr0                            ; 3. Reads CRO
or eax, 1                               ;    Sets PE bit (bit 0)
mov cr0, eax                            ;    Writes back, now in protected mode :D

jmp 0x08:protected_mode                 ; 4. Far jump to the code segment (clears the pipeline)

print:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

MSG_STAGE2 db 'Stage 2 loaded - switching to protected mode...', 13, 10, 0

; ==================================================================================
; GDT
; ==================================================================================

GDT_START:

GDT_NULL:                           ; entry 0: null descriptor (required)
    dd 0x0                          ; dd = defines double
    dd 0x0

GDT_CODE:                           ; entry 1: code segment
    dw 0xFFFF                       ; limit (bits 0-15) 
    dw 0x0000                       ; base (bits 0-15)
    db 0x00                         ; base (bits 16-23)
    db 10011010b                    ; access byte: present=1, ring=00, type=1, exec=1, direction=0, rw=1, accessed=0
    db 11001111b                    ; flags + limit: granularity=1, 32-bit=1, 64-bit=0, available=0, limit (bits 16-19)=1111
    db 0x00                         ; base (bits 24-31)

GDT_DATA:                           ; entry 2: data segment
    dw 0xFFFF                       ; limit (bits 0-15)
    dw 0x0000                       ; base (bits 0-15)
    db 0x00                         ; base (bits 16-23)
    db 10010010b                    ; access byte: same as code, but exec=0, rw=1 (writable)
    db 11001111b                    ; flags + limit: same as code
    db 0x00                         ; base (bits 24-31)

GDT_END:

GDT_DESCRIPTOR:
    dw GDT_END - GDT_START - 1      ; GDT size - 1
    dd GDT_START                    ; GDT address

; ==================================================================================
; Protected mode code (32 bits)
; ==================================================================================

bits 32                             ; from here, NASM generates 32-bit code

; Updates all segment registers to data segment
protected_mode:
    mov ax, 0x10                    ; 0x10 = entry offset (2, data) at GDT: 2 * 8 = 16 = 0x10 
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x9000                 ; stack in high address
    mov esp, ebp

    ; Prints directly at video memory using VGA (int 0x10 doesn't work anymore!)
    mov byte [0xB8000], 'O'         ; char
    mov byte [0xB8001], 0x0A        ; attribute: green ov black :D

    jmp $
