#include "isr.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"

// From here: https://wiki.osdev.org/Exceptions
static const char *exception_names[] = {
    "Division error",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved",
    "Triple Fault",
    "FPU Error Interrupt",
    "Reserved",
    "Reserved",
    "Reserved",
};

void isr_handler(struct isr_frame frame)
{
    vga_print_color("EXCEPTION: ", VGA_LIGHT_RED, VGA_BLACK);

    if (frame.int_no < sizeof(exception_names) / sizeof(exception_names[0]))
        vga_print(exception_names[frame.int_no]);
    else
        vga_print("Unknown exception");

    vga_print("\n");

    // Halt - an exception at this point is fatal
    for (;;)
        ;
}

#include "pic.h"

void irq_handler(struct isr_frame frame)
{
    // Sends EOI to the PIC (mandatory!)
    pic_send_eoi(frame.int_no - 32);

    // If interruption is 32 (timer), call the timer tick function
    if (frame.int_no == 32)
        timer_tick();

    // If interruption is 33 (keyboard), call the keyboard handler
    if (frame.int_no == 33)
        keyboard_handler();
}
