#ifndef JAZZ_BOOTSTACK_H
#define JAZZ_BOOTSTACK_H

#include <efi.h>

#define KERNEL_STACK_PAGES 16

/* Tracks where allocation begins and how many pages it owns. */
typedef struct {
    EFI_PHYSICAL_ADDRESS base;
    UINTN pages;
} KERNEL_STACK;

/* Zero-initialise before first use.
* Owns pages after successful allocation. */
EFI_STATUS bootstack_allocate(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_STACK *stack
);

EFI_STATUS bootstack_release(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_STACK *stack
);

#endif
