#include "pic.h"
#include "ports.h"

// Small delay to give the PIC time to process
static void io_wait(void)
{
    port_out(0x80, 0); // Port 0x80 is used as a "waste" port for delay
}

void pic_remap(void)
{
    // Save the current masks
    uint8_t mask1 = port_in(PIC1_DATA);
    uint8_t mask2 = port_in(PIC2_DATA);

    // ICW1: start the initialization sequence
    port_out(PIC1_CMD, 0x11);
    io_wait();
    port_out(PIC2_CMD, 0x11);
    io_wait();

    // ICW2: set the offset (where the IRQs are mapped)
    port_out(PIC1_DATA, 0x20);
    io_wait(); // master: IRQ 0-7 -> interrupts 32-39
    port_out(PIC2_DATA, 0x28);
    io_wait(); // slave:  IRQ 8-15 -> interrupts 40-47

    // ICW3: tells the master that there is a slave on IRQ 2
    port_out(PIC1_DATA, 0x04);
    io_wait(); // bit 2 = IRQ 2
    port_out(PIC2_DATA, 0x02);
    io_wait(); // slave connected to IRQ 2 (identity)

    // ICW4: 8086 mode
    port_out(PIC1_DATA, 0x01);
    io_wait();
    port_out(PIC2_DATA, 0x01);
    io_wait();

    // Restore the masks
    port_out(PIC1_DATA, mask1);
    port_out(PIC2_DATA, mask2);
}

void pic_send_eoi(int irq)
{
    if (irq >= 8)
        port_out(PIC2_CMD, PIC_EOI); // if it came from the slave, notify the slave as well
    port_out(PIC1_CMD, PIC_EOI);     // always notify the master
}