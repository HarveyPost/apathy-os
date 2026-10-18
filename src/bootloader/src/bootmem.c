#include "bootmem.h"
#include <efilib.h>

EFI_STATUS bootmem_capture(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map
) {
    if (system_table == NULL || map == NULL)
        return EFI_INVALID_PARAMETER;

    map->size = 0;
    map->key = 0;
    map->descriptor_size = 0;
    map->descriptor_version = 0;

    /* Allocation can change the map, so allow bounded retries. */
    for (UINTN attempt = 0; attempt < 4; attempt++) {
        UINTN size = map->capacity;
        UINTN key = 0;
        UINTN descriptor_size = 0;
        UINT32 version = 0;
        EFI_STATUS status;

        status = uefi_call_wrapper(
            system_table->BootServices->GetMemoryMap,
            5,
            &size,
            map->buffer,
            &key,
            &descriptor_size,
            &version
        );

        if (status == EFI_BUFFER_TOO_SMALL) {
            UINTN capacity;
            VOID *buffer = NULL;

            if (descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR))
                return EFI_DEVICE_ERROR;

            /* Leave space for eight extra descriptors. */
            if (descriptor_size > ((~(UINTN)0) - size) / 8)
                return EFI_BAD_BUFFER_SIZE;

            capacity = size + 8 * descriptor_size;
            
            status = bootmem_release(system_table, map);
            if (EFI_ERROR(status))
                return status;

            status = uefi_call_wrapper(
                system_table->BootServices->AllocatePool,
                3,
                EfiLoaderData,
                capacity,
                &buffer
            );

            if (EFI_ERROR(status))
                return status;

            map->buffer = buffer;
            map->capacity = capacity;
            continue;
        }

        if (EFI_ERROR(status))
            return status;

        if (descriptor_size < sizeof(EFI_MEMORY_DESCRIPTOR) ||
            size == 0 || size > map->capacity ||
            size % descriptor_size != 0)
            return EFI_DEVICE_ERROR;

        if (version != EFI_MEMORY_DESCRIPTOR_VERSION)
            return EFI_UNSUPPORTED;

        map->size = size;
        map->key = key;
        map->descriptor_size = descriptor_size;
        map->descriptor_version = version;

        return EFI_SUCCESS;
    }
    
    return EFI_ABORTED;
}

EFI_STATUS bootmem_release(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_MEMORY_MAP *map
) {
    EFI_STATUS status;

    if (system_table == NULL || map == NULL)
        return EFI_INVALID_PARAMETER;

    if (map->buffer == NULL)
        return EFI_SUCCESS;

    status = uefi_call_wrapper(
        system_table->BootServices->FreePool,
        1,
        map->buffer
    );

    if (!EFI_ERROR(status))
        *map = (BOOT_MEMORY_MAP){0};

    return status;
}
