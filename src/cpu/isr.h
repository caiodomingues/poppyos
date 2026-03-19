#ifndef ISR_H
#define ISR_H

#include "types.h"

// Struct that represents the CPU state when the interrupt arrived
struct isr_frame
{
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // push
    uint32_t int_no, err_code;                       // pushed by the stub
};

void isr_handler(struct isr_frame frame);

#endif