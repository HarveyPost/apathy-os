#ifndef JAZZ_KERNELLOAD_H
#define JAZZ_KERNELLOAD_H

#include <efi.h>

typedef struct {
    EFI_PHYSICAL_ADDRESS base;
    UINTN pages;
    UINT64 entry;
} KERNEL_IMAGE;

/* Pass a zero-initialised image.
* On success, it owns the reserved pages.
* Release them while UEFI boot services are active. */
EFI_STATUS kernelload_reserve(
    EFI_SYSTEM_TABLE *system_table,
    const VOID *data,
    UINTN size,
    KERNEL_IMAGE *image
);

/* An empty image is a sucessful no-op.
* On failure, ownership remains recorded in image. */
EFI_STATUS kernelload_release(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_IMAGE *image
);


/* Validate, reserve, zero and populate the kernel's memory image.
* Pass a zero-initiliased image. The caller owns successful allocations
* and retains ownership of the source file buffer. */
EFI_STATUS kernelload_load(
    EFI_SYSTEM_TABLE *system_table,
    const VOID *data,
    UINTN size,
    KERNEL_IMAGE *image
);

#endif
