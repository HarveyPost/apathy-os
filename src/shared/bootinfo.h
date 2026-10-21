#ifndef APATHY_BOOTINFO_H
#define APATHY_BOOTINFO_H

#include <stdint.h>

#define BOOT_INFO_VERSION 2
/* ABI means Application Binary Interface. Here, the attribute
 * selects the System V calling convention: the machine-level
 * rules governing aargument passing, register preservation and
 * stack alignment. */
#define KERNEL_ABI __attribute__((sysv_abi))

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

    uint64_t stack_base;
    uint64_t stack_size;
} BOOT_INFO;

_Static_assert(sizeof(BOOT_INFO) == 72,
               "Unexpected boot information layout");

typedef void (KERNEL_ABI *KERNEL_ENTRY) (const BOOT_INFO *boot_info);

#endif
