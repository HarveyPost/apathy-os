#ifndef JAZZ_BOOTMEM_H
#define JAZZ_BOOTMEM_H

#include <efi.h>

typedef struct {
    VOID *buffer;
    UINTN capacity;
    UINTN size;
    UINTN key;
    UINTN descriptor_size;
    UINT32 descriptor_version;
} BOOT_MEMORY_MAP;

/* Zero-initialised before first use.
* A later capture can refresh the existing buffer.
* The record owns its buffer even on failure: always release it
* before returning to firmware.
* Map contents are valid only after success. */
EFI_STATUS bootmem_capture(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map
);

/* Requires active boot services.
* An empty record is a successful no-op.
* A failed release leaves ownership recorded. */
EFI_STATUS bootmem_release(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map
);

#endif
