#include <efi.h>
#include <efilib.h>

#include "bootfs.h"
#include "kernelload.h"
#include "bootserial.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_FILE_PROTOCOL *root = NULL;
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *file = NULL;
    EFI_STATUS close_status;

    VOID *file_data = NULL;
    UINTN file_size = 0;

    EFI_SERIAL_IO_PROTOCOL *serial = NULL;
    EFI_STATUS serial_status;


    InitializeLib(ImageHandle, SystemTable);

    serial_status = bootserial_open(SystemTable, &serial);

    if (!EFI_ERROR(serial_status))
        serial_status = bootserial_write(
            serial,
            "[boot] Jazz bootloader started.\n"
        );

    if (EFI_ERROR(serial_status))
        Print(L"Serial logging unavailable: %r\n", serial_status);

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
                KERNEL_IMAGE kernel = {0};
                EFI_STATUS release_status;

                Print(L"Read %lu kernel file bytes.\n", (UINT64)file_size);

                status = kernelload_load(
                    SystemTable,
                    file_data,
                    file_size,
                    &kernel
                );

                /* The loaded image has its own allocation. */
                if (file_data != NULL) {
                    FreePool(file_data);
                    file_data = NULL;
                }

                if (!EFI_ERROR(status)) {
                    if (serial != NULL)
                        (VOID)bootserial_write(serial, "[boot] Kernel loaded. \n");
                    Print(L"Kernel segments copied and zero-initialised.\n");
                    Print(L"Kernel base: 0x%lx\n", kernel.base);
                    Print(L"Kernel pages: %lu\n", (UINT64)kernel.pages);
                    Print(L"Entry point: 0x%lx\n", kernel.entry);
                }

                release_status = kernelload_release(SystemTable, &kernel);

                if (!EFI_ERROR(status))
                    status = release_status;

                if (!EFI_ERROR(status))
                    Print(L"Kernel pages released.\n");
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

    if (EFI_ERROR(status)) {
        Print(L"Boot error: %r\n", status);

        if (serial != NULL)
            (VOID)bootserial_write(serial, "[boot] Boot failed.\n");
    }

    Print(L"Press any key to return to UEFI!\n");
    WaitForSingleEvent(SystemTable->ConIn->WaitForKey, 0);

    return status;
}
