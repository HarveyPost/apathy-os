#ifndef JAZZ_BOOTINFO_H
#define JAZZ_BOOTINFO_H

#include <stdint.h>

#define BOOT_INFO_VERSION 1

/* version, size - Identify the boot-information format.
* memory_map_address, memory_map_size - Locate the captured map.
* Descriptor size and version - Describe how to interpret its entries.
* reserved - An explicitly reserved four-byte slot, currently zero.
* kernel_base, kernel_size - Identify the allocation occupied by the kernel. */
typedef struct {
    uint32_t version;
    uint32_t size;

    uint64_t memory_map_address;
    uint64_t memory_map_size;
    uint64_t memory_map_descriptor_size;
    uint32_t memory_map_descriptor_version;
    uint32_t reserved;

    uint64_t kernel_base;
    uint64_t kernel_size;
} BOOT_INFO;

_Static_assert(sizeof(BOOT_INFO) == 56,
               "Unexpected boot information layout");

#endif
