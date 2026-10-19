#ifndef JAZZ_BOOTJUMP_H
#define JAZZ_BOOTJUMP_H

#include "bootinfo.h"

/* Call only after successfully exiting UEFI boot services.
* entry, boot_info and the stack must remain mapped and valid. */
_Noreturn void KERNEL_ABI bootjump(
    KERNEL_ENTRY entry,
    const BOOT_INFO *boot_info,
    uint64_t stack_top
);

#endif
