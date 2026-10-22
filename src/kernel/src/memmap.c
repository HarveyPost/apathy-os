#include "memmap.h"
#include "memory.h"
#include <stddef.h>

#define UEFI_DESCRIPTOR_VERSION 1
#define UEFI_CONVENTIONAL_MEMORY 7
#define UEFI_PAGE_SIZE UINT64_C(4096)
#define UEFI_MEMORY_RUNTIME (UINT64_C(1) << 63)

typedef struct {
    uint32_t type;
    uint32_t padding;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attributes;
} UEFI_MEMORY_DESCRIPTOR;

_Static_assert(sizeof(UEFI_MEMORY_DESCRIPTOR) == 40,
               "Unexpected memory descriptor size");
_Static_assert(offsetof(UEFI_MEMORY_DESCRIPTOR, physical_start) == 8,
               "Unexpected physical address offset");
_Static_assert(offsetof(UEFI_MEMORY_DESCRIPTOR, number_of_pages) == 24,
               "Unexpected page count offset");
_Static_assert(offsetof(UEFI_MEMORY_DESCRIPTOR, attributes) == 32,
               "Unexpected memory attributes offset");

bool memmap_count_conventional_pages(
    const BOOT_INFO *boot_info, uint64_t *pages
) {
    if (pages == NULL)
        return false;
    *pages = 0;

    if (boot_info == NULL ||
        boot_info->version != BOOT_INFO_VERSION ||
        boot_info->size != sizeof(*boot_info))
        return false;

    uint64_t address = boot_info->memory_map_address;
    uint64_t size = boot_info->memory_map_size;
    uint64_t stride = boot_info->memory_map_descriptor_size;

    if (boot_info->memory_map_descriptor_version != UEFI_DESCRIPTOR_VERSION ||
        address == 0 || size == 0 ||
        stride < sizeof(UEFI_MEMORY_DESCRIPTOR) || size % stride != 0)
        return false;

    if (address > UINTPTR_MAX || size - 1 > UINTPTR_MAX - address)
        return false;

    const unsigned char *map = (const unsigned char *)(uintptr_t)address;
    uint64_t total = 0;

    for (uint64_t offset = 0; offset < size; offset += stride) {
        UEFI_MEMORY_DESCRIPTOR descriptor;
        memcpy(&descriptor, map + offset, sizeof(descriptor));

        if (descriptor.number_of_pages == 0 ||
            descriptor.physical_start % UEFI_PAGE_SIZE != 0 ||
            descriptor.number_of_pages - 1 >
                (UINT64_MAX - descriptor.physical_start) / UEFI_PAGE_SIZE)
            return false;

        if (descriptor.type != UEFI_CONVENTIONAL_MEMORY ||
            (descriptor.attributes & UEFI_MEMORY_RUNTIME) != 0)
            continue;

        if (descriptor.number_of_pages > UINT64_MAX - total)
            return false;

        total += descriptor.number_of_pages;
    }

    *pages = total;
    return true;
}
