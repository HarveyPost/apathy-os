/**
  * @file main.c
  * @author harveypost
  * @date Oct 2026
  * @brief Bootloader entry point and main application
  */

#include <efi.h>
#include <efilib.h>

#include "bootfs.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    EFI_FILE_PROTOCOL *root = NULL;
    EFI_STATUS status;

    InitializeLib(ImageHandle, SystemTable);

    status = bootfs_open_root(ImageHandle, SystemTable, &root);

    if (!EFI_ERROR(status)) {
        Print(L"Boot filesystem root opened successfully.\n");

        /* We have finished with this handle for this checkpoint. */
        status = uefi_call_wrapper(root->Close, 1, root);
    }

    if (EFI_ERROR(status))
        Print(L"Filesystem error: %r\n", status);

    Print(L"Press any key to return to UEFI!\n");
    WaitForSingleEvent(SystemTable->ConIn->WaitForKey, 0);

    return EFI_SUCCESS;
}
