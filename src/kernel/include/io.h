#ifndef JAZZ_IO_H
#define JAZZ_IO_H

#include <stdint.h>

/* x86 has a separate I/O-port address space.
* A port number identifies a hardware interface; it isn't an ordinary RAM address.
*
* outb writes one byte to an I/O port.
* Here, the assembly uses extended asm, allowing GCC to connect C values to
* instruction operands:
* %0 represents value.
* %1 represents port.
* "a"(value) puts the byte in AL, the low eight bits of RAX.
* "Nd"(port) allows either an eight-bit immediate port number or the DX register. COM1's 0x3F8 requires DX.
*
* volatile tells GCC this operation has important side effects.
* The "memory" clobber contrains compiler memory optimisations around it;
* it doesn't emit a CPU memory-fence instruction. */

static inline void io_out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1"
                     : : "a"(value), "Nd"(port) : "memory");
}

static inline uint8_t io_in8(uint16_t port) {
    uint8_t value;

    __asm__ volatile("inb %1, %0"
                     : "=a"(value) : "Nd"(port) : "memory");

    return value;
}

#endif
