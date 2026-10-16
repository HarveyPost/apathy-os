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

    CHAR8 buffer[128];
    UINTN bytes_read;
    UINT64 total_read = 0;

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

            Print(L"Contents:\n");
            
            for (;;) {
                /* Reset capacity before every read. */
                bytes_read = sizeof(buffer) - 1;

                status = bootfs_read(file, buffer, &bytes_read);

                if (EFI_ERROR(status))
                    break;

                /* A successful read of zero bytes means EOF. */
                if (bytes_read == 0)
                    break;
                
                buffer[bytes_read] = '\0';

                Print(L"%a", buffer);
                total_read += bytes_read;
            }
        
            if (!EFI_ERROR(status))
                    Print(L"Read %lu bytes in total.\n", total_read);
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
