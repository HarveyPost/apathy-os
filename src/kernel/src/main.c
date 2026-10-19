#include "bootinfo.h"
#include "serial.h"
#include "panic.h"
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
    kernel_halt();
}
