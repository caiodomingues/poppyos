#ifndef PORTS_H
#define PORTS_H

#include "types.h"

// Reads one byte from an I/O port
static inline uint8_t port_in(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Writes one byte to an I/O port
static inline void port_out(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

#endif