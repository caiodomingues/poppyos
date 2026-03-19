#ifndef IDT_H
#define IDT_H

#include "types.h"

// IDT entry (8 bytes, format defined by Intel)
struct idt_entry
{
    uint16_t base_low;  // Handler address, bits 0-15
    uint16_t selector;  // Code segment selector (0x08, same as GDT)
    uint8_t zero;       // Always zero
    uint8_t flags;      // Type + privilege + present
    uint16_t base_high; // Handler address, bits 16-31
} __attribute__((packed));

// Pointer to LIDT (same idea as GDT_DESCRIPTOR)
struct idt_ptr
{
    uint16_t limit; // Limit of the IDT
    uint32_t base;
} __attribute__((packed)); // This tells the compiler not to add any padding between fields, ensuring the structure is exactly 8 bytes, which is required for the IDT entry format.

void idt_init(void);
void idt_set_gate(int n, uint32_t handler);

#endif