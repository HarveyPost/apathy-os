#ifndef JAZZ_BOOTEXIT_H
#define JAZZ_BOOTEXIT_H

#include "bootmem.h"
#include "bootinfo.h"

/* Call once for the shutdown sequence.
* attempted becomes TRUE before the first ExitBootServices call.
* After an attempt, do not resume ordinary firmware calls.
* EFI_SUCCESS means boot services have ended: retain allocations. */
EFI_STATUS bootexit_services(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map,
    BOOT_INFO *boot_info,
    BOOLEAN *attempted
);

#endif
