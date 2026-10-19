#include "bootinfo.h"

_Noreturn void KERNEL_ABI kernel_main(
    const BOOT_INFO *boot_info
) {
    (void)boot_info;

    for (;;) {
    }
}
