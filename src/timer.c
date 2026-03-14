#include "timer.h"
#include "ports.h"

static uint32_t ticks = 0;

void timer_init(uint32_t frequency)
{
    // The internal timer frequency is typically 1_193_182 Hz. To achieve the desired frequency, we calculate the divisor.
    uint32_t divisor = 1193182 / frequency;

    // Send the command byte to the PIT command port (0x43).
    port_out(0x43, 0x36); // Command byte: 0x36 sets the PIT to mode 3 (square wave generator) and selects channel 0.

    // Send the divisor to the PIT data port (0x40). The divisor is sent as two bytes: low byte first, then high byte.
    port_out(0x40, divisor & 0xFF);        // Send the low byte of the divisor.
    port_out(0x40, (divisor >> 8) & 0xFF); // Send the high byte of the divisor.

    // The timer will now generate interrupts at the specified frequency. Each time an interrupt occurs, the timer_interrupt_handler will be called, which increments the ticks variable.
}

uint32_t timer_get_ticks()
{
    return ticks;
}

void timer_tick(void)
{
    ticks++;
}
