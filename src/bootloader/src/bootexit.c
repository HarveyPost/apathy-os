#include "bootexit.h"
#include <efilib.h>

EFI_STATUS bootexit_services(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map,
    BOOT_INFO *boot_info,
    BOOLEAN *attempted
) {
    EFI_STATUS status;

    if (attempted == NULL)
        return EFI_INVALID_PARAMETER;

    *attempted = FALSE;

    if (image_handle == NULL || system_table == NULL ||
        map == NULL || boot_info == NULL)
        return EFI_INVALID_PARAMETER;

    for (UINTN attempt = 0; attempt < 4; attempt++) {
        status = bootmem_capture(system_table, map);
        if (EFI_ERROR(status))
            return status;

        /* Capture may have replaced the map buffer. */
        boot_info->memory_map_address = (uint64_t)(uintptr_t)map->buffer;
        boot_info->memory_map_size = map->size;
        boot_info->memory_map_descriptor_size = map->descriptor_size;
        boot_info->memory_map_descriptor_version = map->descriptor_version;
        
        /* No logging or other firmware work between capture and exit. */
        *attempted = TRUE;

        /* Checks that map->key identifies its current memory map.
        * If it is stale, the call returns and tries up to 4 times. */
        status = uefi_call_wrapper(
            system_table->BootServices->ExitBootServices,
            2,
            image_handle,
            map->key
        );

        if (status != EFI_INVALID_PARAMETER)
            return status;
    }

    return EFI_ABORTED;
}
