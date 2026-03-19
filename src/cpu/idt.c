#include "idt.h"

struct idt_entry idt[256]; // IDT can have up to 256 entries
struct idt_ptr idt_ptr;    // Pointer to the IDT

// Zero out the table and loads the CPU at the end
void idt_init(void)
{
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1; // Size of IDT in bytes - 1
    idt_ptr.base = (uint32_t)&idt;                      // Base address of the IDT

    // Clear the IDT by setting all entries to zero
    for (int i = 0; i < 256; i++)
    {
        idt[i].base_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].flags = 0;
        idt[i].base_high = 0;
    }

    // Special assembly instruction to load the IDT (lidt)
    __asm__ volatile("lidt (%0)" : : "r"(&idt_ptr));
}

// Fills the entry N with the handler address
void idt_set_gate(int n, uint32_t handler)
{
    idt[n].base_low = handler & 0xFFFF;          // Lower 16 bits of handler address
    idt[n].selector = 0x08;                      // Code segment selector (0x08, same as GDT)
    idt[n].zero = 0;                             // Always zero
    idt[n].flags = 0x8E;                         // Present, ring 0, 32-bit interrupt gate
    idt[n].base_high = (handler >> 16) & 0xFFFF; // Upper 16 bits of handler address
}
