#include "bootparams.h"
#include <efilib.h>

_Static_assert(sizeof(BOOT_INFO) <= EFI_PAGE_SIZE,
               "Boot information must fit in one page");

EFI_STATUS bootparams_allocate(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_PARAMETERS *parameters
) {
    EFI_PHYSICAL_ADDRESS address = 0;
    EFI_STATUS status;

    if (system_table == NULL || parameters == NULL ||
        parameters->pages != 0)
        return EFI_INVALID_PARAMETER;

    *parameters = (BOOT_PARAMETERS){0};

    status = uefi_call_wrapper(
        system_table->BootServices->AllocatePages,
        4,
        AllocateAnyPages,
        EfiLoaderData,
        1,
        &address
    );

    if (EFI_ERROR(status))
        return status;

    parameters->base = address;
    parameters->pages = 1;
    parameters->info = (BOOT_INFO *)(UINTN)address;

    uefi_call_wrapper(
        system_table->BootServices->SetMem,
        3,
        parameters->info,
        EFI_PAGE_SIZE,
        0
    );

    return EFI_SUCCESS;
}

EFI_STATUS bootparams_release(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_PARAMETERS *parameters
) {
    EFI_STATUS status;

    if (system_table == NULL || parameters == NULL)
        return EFI_INVALID_PARAMETER;

    if (parameters->pages == 0)
        return EFI_SUCCESS;

    status = uefi_call_wrapper(
        system_table->BootServices->FreePages,
        2,
        parameters->base,
        parameters->pages
    );

    if (!EFI_ERROR(status))
        *parameters = (BOOT_PARAMETERS){0};

    return status;
}
