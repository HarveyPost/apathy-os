#include "serial.h"
#include "arch/io.h"
#include <stddef.h>

#define COM1 0x3F8
#define POLL_LIMIT 100000

static bool ready;

static bool serial_wait(uint8_t mask) {
    for (unsigned attempt = 0; attempt < POLL_LIMIT; attempt++) {
        /* Accesses the Line Status Register.
         * Its bits describe the UART's current state.
         * Repeately reads that status until data is available or it reaches 100k attempts. */
        if (io_in8(COM1 + 5) & mask)
            return true;
    }
    
    return false;
}

/* Initialise and test the UART:
 * Selects 8N1, configures the baud divisor, enables buffers, then uses internal loopback to check that a transmitted 0xAE can be read back. */
bool serial_init(void) {
    ready = false;
    
    /* Selects the Line Control Register.
     * Eight data bits per character, no parity bit, one stop bit, DLAB cleared. */
    io_out8(COM1 + 3, 0x03);
    /* With DLAB cleared, this accesses the Interrupt Enable Register.
     * Writing zero disables UART-generated interrupts. */
    io_out8(COM1 + 1, 0x00);
    /* 0x83 is 0x03 with bit 7 set. That bit is DLAB, the Divisor Latch Access Bit.
     * DLAB changes what the first two port offsets access:
     * COM1 + 0 : DLAB cleared receive/transmit data, DLAB set Baud-divisor low byte.
     * COM1 + 1 : DLAB cleared interrupt enables, DLAB set Baud-divisor high byte.
     * This lets the hardware expose different registers through the same port numbers. */
    io_out8(COM1 + 3, 0x83);
    /* Together, these low and high bytes form the divisor 0x0001. */
    io_out8(COM1 + 0, 0x01); /* Divisor low byte: 115200 baud on QEMU. */
    io_out8(COM1 + 1, 0x00); /* Divisor high byte. */
    /* Clear DLAB again while retaining 8N1. */
    io_out8(COM1 + 3, 0x03); /* Clear DLAB; restore data-register access. */
    /* Access the FIFO Control Register.
     * 0x07 sets three bits:
     * Bit 0 - enable FIFOs
     * Bit 1 - clear the receive FIFO
     * Bit 2 - clear the transmit FIFO 
     * This enables buffering and discards any old queue data. */
    io_out8(COM1 + 2, 0x07);

    /* Enable internal loopback.
     * This is the Modem Control Register.
     * Setting its loopback bit connects the UART's transmit path internally to its receive path.
     * Our driver can send a byte and check whether it receives the same value. */
    io_out8(COM1 + 4, 0x10);
    io_out8(COM1 + 0, 0xAE); /* Send a test byte internally. 0xAE is a chosen text pattern, not a special command. */

    bool passed = serial_wait(0x01) && io_in8(COM1 + 0) == 0xAE;

    io_out8(COM1 + 4, 0x03); /* Leave loopback; assert DTR and RTS. */
    ready = passed;
    return ready;
}
/*
 * Transmit text by polling:
 * Repeatedly checks the UART's status before writing each character.
 * The polling limit prevents an endless wait, but represents iterations. */
bool serial_write(const char *text) {
    if (!ready || text == NULL)
        return false;

    while (*text != '\0') {
        /* Reuses the status-polling helper, but checks a different bit:
        * 0x01 -> received data is available.
        * 0x20 -> transmit holding register is empty.
        * The 0x20 condition (with UART's transmit FIFO enabled) indicates its transmit FIFO is empty. Our driver waits for that condition before providing another byte. */
        if (!serial_wait(0x20))
            return false;
        
        /* Writes the current character to the UART's transmit-data register and advances the string pointer. */
        io_out8(COM1, (uint8_t)*text++);
    }

    return true;
}

bool serial_write_hex64(uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    char buffer[19]; /* Prefix, sixteen digits, terminating NUL. */

    buffer[0] = '0';
    buffer[1] = 'x';

    for (unsigned i = 0; i < 16; i++) {
        unsigned shift = 60 - i * 4;
        buffer[i + 2] = digits[(value >> shift) & 0xF];
    }

    buffer[18] = '\0';
    return serial_write(buffer);
}
