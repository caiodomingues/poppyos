#include "vga.h"
#include "idt.h"

extern void isr_install(void);

void kernel_main()
{
    vga_init();
    vga_clear();
    vga_print_color("PoppyOS", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("\n");

    idt_init();
    isr_install();
    vga_print("IDT loaded.\n");
    vga_print("System ready.\n");
}
