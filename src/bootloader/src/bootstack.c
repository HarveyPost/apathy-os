#include "bootstack.h"
#include <efilib.h>

EFI_STATUS bootstack_allocate(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_STACK *stack
) {
    EFI_PHYSICAL_ADDRESS address = 0;
    EFI_STATUS status;

    if (system_table == NULL || stack == NULL || stack->pages != 0)
        return EFI_INVALID_PARAMETER;

    *stack = (KERNEL_STACK){0};

    /* Difference from loading the kernel is AllocateAnyPages.
     * Kernel segments have addresses specified by the ELF
     * executable, so we request their exact placements using
     * AllocateAddress. The stack has no predetermined address.
     * The firmware can choose an avaialable range. */
    status = uefi_call_wrapper(
        system_table->BootServices->AllocatePages,
        4,
        AllocateAnyPages,
        EfiLoaderData,
        KERNEL_STACK_PAGES,
        &address
    );

    if (EFI_ERROR(status))
        return status;

    stack->base = address;
    stack->pages = KERNEL_STACK_PAGES;

    /* Fill all 64 KiB with Zeroes. */
    uefi_call_wrapper(
        system_table->BootServices->SetMem,
        3,
        (VOID *)(UINTN)stack->base,
        stack->pages * EFI_PAGE_SIZE,
        0
    );

    return EFI_SUCCESS;
}

EFI_STATUS bootstack_release(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_STACK *stack
) {
    EFI_STATUS status;

    if (system_table == NULL || stack == NULL)
        return EFI_INVALID_PARAMETER;

    if (stack->pages == 0)
        return EFI_SUCCESS;

    status = uefi_call_wrapper(
        system_table->BootServices->FreePages,
        2,
        stack->base,
        stack->pages
    );

    if (!EFI_ERROR(status))
        *stack = (KERNEL_STACK){0};

    return status;
}
