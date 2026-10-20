#include "bootinfo.h"
#include "serial.h"
#include "panic.h"
#include "memory.h"
#include <stddef.h>

static _Noreturn void kernel_halt(void) {
    for (;;) {
        __asm__ volatile("cli\n\thlt");
    }
}

_Noreturn void KERNEL_ABI kernel_main(const BOOT_INFO *boot_info) {
    if (!serial_init())
        kernel_halt();

    if (!serial_write("[kernel] Jazz kernel entered.\r\n"))
        kernel_halt();

    if (boot_info == NULL ||
        boot_info->version != BOOT_INFO_VERSION ||
        boot_info->size != sizeof(BOOT_INFO)) {
        PANIC("Invalid boot information");
    }

    (void)serial_write("[kernel] Boot information format valid.\r\n");

    unsigned char buffer[8];

    if (memset(buffer, 0x1AB, sizeof(buffer)) != buffer)
        PANIC("memset returned the wrong pointer");

    for (size_t i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0xAB)
            PANIC("memset fill failed");
    }
    
    memset(buffer + 2, 0, 4);

    for (size_t i = 0; i < sizeof(buffer); i++) {
        unsigned char expected = (i >= 2 && i < 6) ? 0 : 0xAB;

        if (buffer[i] != expected)
            PANIC("memset partial fill failed");
    }

    (void)serial_write("[kernel] memset checks passed.\r\n");

    kernel_halt();
}
