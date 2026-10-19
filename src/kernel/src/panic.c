#include "panic.h"
#include "serial.h"
#include <stddef.h>

static void panic_write_line(uint32_t line) {
    char buffer[11]; /* Ten decimal digits plus terminating NUL. */
    unsigned index = sizeof(buffer) - 1;

    buffer[index] = '\0';

    do {
        buffer[--index] = (char)('0' + line % 10);
        line /= 10;
    } while (line != 0);

    (void)serial_write(&buffer[index]);
}

_Noreturn void panic_at(
    const char *message,
    const char *file,
    uint32_t line
) {
    __asm__ volatile("cli" : : : "memory");

    (void)serial_write("\r\n[panic] ");
    (void)serial_write(message != NULL ? message : "Unspecified error");

    (void)serial_write("\r\n[panic] ");
    (void)serial_write(file != NULL ? file : "Unspecified error");
    (void)serial_write(":");
    panic_write_line(line);
    
    (void)serial_write("\r\n[panic] System halted.\r\n");

    for (;;) {
        __asm__ volatile("cli\n\thlt");
    }

}
