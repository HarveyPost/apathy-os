/**
  * @file main.c
  * @author harveypost
  * @date Oct 2026
  * @brief Bootloader entry point and main application
  */

#include <efi.h>
#include <efilib.h>

#include "bootfs.h"
#include "elf64.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    EFI_FILE_PROTOCOL *root = NULL;
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *file = NULL;
    EFI_STATUS close_status;

    VOID *file_data = NULL;
    UINTN file_size = 0;

    InitializeLib(ImageHandle, SystemTable);

    status = bootfs_open_root(ImageHandle, SystemTable, &root);

    if (!EFI_ERROR(status)) {
        Print(L"Boot filesystem root opened successfully.\n");

        status = bootfs_open_readonly(
            root,
            L"\\kernel.elf",
            &file
        );

        if (!EFI_ERROR(status)) {
            Print(L"kernel.elf opened successfully.\n");

            status = bootfs_read_all(file, &file_data, &file_size);

            if (!EFI_ERROR(status)) {
                ELF64_HEADER header;

                Print(L"Loaded %lu bytes into memory.\n", (UINT64)file_size);
                
                status = elf64_validate_image(file_data, file_size, &header);

                if (!EFI_ERROR(status)) {
                    Print(L"ELF headers and segment file bounds accepted.\n");
                    Print(L"Entry point: 0x%lx\n", header.e_entry);
                    Print(
                        L"Program headers: %lu\n",
                        (UINT64)header.e_phnum
                    );
                }

                /* Free the buffer even if validation failed. */
                if (file_data != NULL) {
                    FreePool(file_data);
                    file_data = NULL;
                }
            }

            /* Close the file even if reading failed. */
            close_status = uefi_call_wrapper(file->Close, 1, file);

            if (!EFI_ERROR(status))
                status = close_status;
        }

        /* Close the directory even if opening the file failed. */
        close_status = uefi_call_wrapper(root->Close, 1, root);

        /* Preserve an earlier error instead of replacing it. */
        if (!EFI_ERROR(status))
            status = close_status;
    }

    if (EFI_ERROR(status))
        Print(L"Boot error: %r\n", status);

    Print(L"Press any key to return to UEFI!\n");
    WaitForSingleEvent(SystemTable->ConIn->WaitForKey, 0);

    return status;
}
