#ifndef APATHY_BOOTMEM_H
#define APATHY_BOOTMEM_H

#include <efi.h>

/* A UEFI memory map describes the physical address ranges and their purpose.
 * It lets an OS distinguish available memory from memory occupied
 * by firmware, loaded programs or reserved resources.
 *
 * buffer - points to the returned memory descriptors.
 * capacity - allocated buffer space, in bytes.
 * size - bytes actually occupied by the returned map.
 * key - firmware token identifying the captured map.
 * descriptor_size - byte distance between consecutive descriptors.
 * descriptor_version - version of the descriptor format. */
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
