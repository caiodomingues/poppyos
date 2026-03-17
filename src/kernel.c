#include "vga.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "paging.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "shell.h"

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

    pmm_init();
    paging_init();
    vga_print("Paging enabled.\n");

    heap_init();
    vga_print("Heap initialized.\n");

    timer_init(100);
    vga_print("Timer initialized.\n");

    task_init();
    vga_print("Task system initialized.\n");

    asm volatile("sti");
    vga_print("Interrupts enabled.\n");

    vga_print("System ready.\n\n");
    shell_init();

    for (;;)
    {
        asm volatile("hlt");
    }
}