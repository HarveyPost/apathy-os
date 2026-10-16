#include "bootfs.h"
#include <efilib.h>

/* A UEFI protocol is an interface containing data and function pointers.
 * HandleProtocol() retrieves an interface from a handle; the GUID argument identifies which interface we want.
 *
 * uefi_call_wrapper() handles the firmware calling convention for the GNU-EFI setup.
 * Its number argument counts the arguments following it.
 *
 * OpenVolume() gives us an open root-directory handle. */

EFI_STATUS bootfs_open_root(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    EFI_FILE_PROTOCOL **root
) {
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem;
    EFI_STATUS status;

    *root = NULL;

    /* Find information about our running application. */
    status = uefi_call_wrapper(
        system_table->BootServices->HandleProtocol,
        3,
        image_handle,
        &LoadedImageProtocol,
        (VOID **)&loaded_image
    );

    if (EFI_ERROR(status))
        return status;

    /* Get the filesystem on the device that loaded us. */
    status = uefi_call_wrapper(
        system_table->BootServices->HandleProtocol,
        3,
        loaded_image->DeviceHandle,
        &FileSystemProtocol,
        (VOID **)&filesystem
    );

    if (EFI_ERROR(status))
        return status;

    /* Open that filesystem's root directory. */
    return uefi_call_wrapper(
        filesystem->OpenVolume,
        2,
        filesystem,
        root
    );
}

EFI_STATUS bootfs_open_readonly(
    EFI_FILE_PROTOCOL *root,
    CHAR16 *path,
    EFI_FILE_PROTOCOL **file
) {
    *file = NULL;
    
    /* root -> Directory from which to open the file
    *  file -> Where firmware writes the new handle
    *  path -> Filename or path
    *  EFI_FILE_MODE_READ -> open for reading
    *  0 -> Attributes are unused when opening an existing file
    * Open() gives us access to the file; readings its contents is a separate operation. */
    return uefi_call_wrapper(
        root->Open,
        5,
        root,
        file,
        path,
        EFI_FILE_MODE_READ,
        0
    );
}
