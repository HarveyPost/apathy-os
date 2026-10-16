/**
  * @file main.c
  * @author harveypost
  * @date Oct 2026
  * @brief Bootloader entry point and main application
  */

#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    /* Main bootloader application status. */
    EFI_STATUS Status;
    /* 
     * The root file system entity.
     * This is the file root from which the kernel binary will be loaded.
    */
    EFI_FILE* root_file_system;
    /* Input key type used to capture user input. */
    EFI_INPUT_KEY input_key;

    return Status;
}
