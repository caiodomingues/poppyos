#include "vga.h"
#include "idt.h"
#include "pic.h"
#include "shell.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "isr.h"

extern void isr_install(void);

void kernel_main()
{
    vga_init();
    vga_clear();
    vga_print_color("PoppyOS\n", VGA_LIGHT_GREEN, VGA_BLACK);

    idt_init();
    isr_install();
    pic_remap();
    vga_print("IDT loaded.\n");
    vga_print("PIC remapped.\n");

    paging_init();
    vga_print("Paging enabled.\n");

    pmm_init();
    vga_print("Physical memory manager initialized.\n");

    heap_init();
    vga_print("Heap initialized.\n");

    timer_init();
    vga_print("Timer initialized.\n");

    // Enable interrupts
    asm volatile("sti");

    vga_print("Interrupts enabled.\n");
    vga_print("System ready.\n");

    shell_init();

    // Halt the CPU to save power until the next interrupt occurs
    for (;;)
    {
        asm volatile("hlt"); // Halts the CPU until the next interrupt
    }
}