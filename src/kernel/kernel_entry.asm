bits 32

extern kernel_main      ; tells NASM to look for the kernel_main function in another file

call kernel_main        ; call the kernel_main function to start the kernel
jmp $                   ; infinite loop to prevent the kernel from exiting