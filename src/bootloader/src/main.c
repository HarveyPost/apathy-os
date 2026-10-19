#include <efi.h>
#include <efilib.h>

#include "bootfs.h"
#include "kernelload.h"
#include "bootserial.h"
#include "bootmem.h"
#include "bootinfo.h"
#include "bootstack.h"
#include "bootparams.h"

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
                KERNEL_STACK stack = {0};
                BOOT_MEMORY_MAP memory_map = {0};
                BOOT_PARAMETERS parameters = {0};
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

                    status = bootstack_allocate(SystemTable, &stack);

                    if (!EFI_ERROR(status)) {
                        Print(L"Kernel stack size: %lu bytes\n",
                              (UINT64)stack.pages * EFI_PAGE_SIZE);
                        Print(L"Kernel stack base: 0x%lx\n", stack.base);
                        Print(L"Kernel stack top: 0x%lx\n",
                              stack.base + (UINT64)stack.pages * EFI_PAGE_SIZE);

                        status = bootparams_allocate(SystemTable, &parameters);
                        if (!EFI_ERROR(status))
                            status = bootmem_capture(SystemTable, &memory_map);
                    }

                    if (!EFI_ERROR(status)) {
                        *parameters.info = (BOOT_INFO) {
                            .version = BOOT_INFO_VERSION,
                            .size = sizeof(BOOT_INFO),

                            .memory_map_address = 
                                (uint64_t)(uintptr_t)memory_map.buffer,
                            .memory_map_size = memory_map.size,
                            .memory_map_descriptor_size =
                                memory_map.descriptor_size,
                            .memory_map_descriptor_version = 
                                memory_map.descriptor_version,

                            .kernel_base = kernel.base,
                            .kernel_size = 
                                (uint64_t)kernel.pages * EFI_PAGE_SIZE,

                            .stack_base = stack.base,
                            .stack_size = 
                                (uint64_t)stack.pages * EFI_PAGE_SIZE,
                        };

                        Print(L"Boot information address: 0x%lx\n",
                              parameters.base);
                        Print(L"Boot information allocation: %lu bytes\n",
                              (UINT64)parameters.pages * EFI_PAGE_SIZE);
                        Print(L"Boot information version: %u\n",
                              parameters.info->version);
                        Print(L"Boot information size: %u bytes\n",
                              parameters.info->size);
                        Print(L"Kernel allocation: %lu bytes\n",
                              (UINT64)parameters.info->kernel_size);
                        Print(L"Boot stack allocation: %lu bytes\n",
                              (UINT64)parameters.info->stack_size);
                    }

                    if (!EFI_ERROR(status)) {
                        Print(L"Memory map captured.\n");
                        Print(L"Memory descriptors: %lu\n",
                              (UINT64)(memory_map.size / memory_map.descriptor_size));
                        Print(L"Descriptor stride: %lu bytes\n",
                              (UINT64)memory_map.descriptor_size);
                        Print(L"Descriptor version: %lu\n",
                              (UINT64)memory_map.descriptor_version);
                    }
                }

                release_status = bootmem_release(SystemTable, &memory_map);

                if (!EFI_ERROR(status))
                    status = release_status;

                release_status = bootparams_release(SystemTable, &parameters);

                if (!EFI_ERROR(status))
                    status = release_status;

                release_status = bootstack_release(SystemTable, &stack);

                if (!EFI_ERROR(status))
                    status = release_status;

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
