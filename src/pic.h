#ifndef PIC_H
#define PIC_H

#define PIC1_CMD 0x20  // Master command port
#define PIC1_DATA 0x21 // Master data port
#define PIC2_CMD 0xA0  // Slave command port
#define PIC2_DATA 0xA1 // Slave data port

#define PIC_EOI 0x20 // End of Interrupt code

void pic_remap(void);
void pic_send_eoi(int irq);

#endif