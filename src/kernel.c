#include "vga.h"

void kernel_main()
{
    // Initialize VGA text mode and clear the screen
    vga_init();
    vga_clear();

    vga_print_color("PoppyOS", VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("\n");
    vga_print("Kernel loaded successfully.");
}
