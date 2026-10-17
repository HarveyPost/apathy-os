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
            L"\\test.txt",
            &file
        );

        if (!EFI_ERROR(status)) {
            Print(L"test.txt opened successfully.\n");

            status = bootfs_read_all(file, &file_data, &file_size);

            if (!EFI_ERROR(status)) {
                UINT8 sample[8];
                UINTN bytes_read = sizeof(sample);

                /* Use offset 5, or EOF if the file is shorter. */
                UINT64 offset = file_size < 5 ? file_size : 5;
                
                Print(L"Loaded %lu bytes into memory.\n", (UINT64)file_size);

                if (file_data != NULL) {
                    FreePool(file_data);
                    file_data = NULL;
                }

                status = bootfs_seek(file, offset);

                if (!EFI_ERROR(status))
                    status = bootfs_read(file, sample, &bytes_read);

                if (!EFI_ERROR(status)) {
                    Print(
                        L"Read %lu bytes at offset %lu:\n",
                        (UINT64)bytes_read,
                        offset
                    );

                    for (UINTN i = 0; i < bytes_read; i++)
                        Print(L"%02x ", (UINT32)sample[i]);

                    Print(L"\n");
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
        Print(L"Filesystem error: %r\n", status);

    Print(L"Press any key to return to UEFI!\n");
    WaitForSingleEvent(SystemTable->ConIn->WaitForKey, 0);

    return status;
}
