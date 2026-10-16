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

EFI_STATUS bootfs_read(
    EFI_FILE_PROTOCOL *file,
    VOID *buffer,
    UINTN *size
) {
    return uefi_call_wrapper(
        file->Read,
        3,
        file,
        size,
        buffer
    );
}

EFI_STATUS bootfs_get_size(
    EFI_FILE_PROTOCOL *file,
    UINT64 *size
) {
    EFI_STATUS status;
    UINTN info_size = 0;
    EFI_FILE_INFO *info;

    if (file == NULL || size == NULL)
        return EFI_INVALID_PARAMETER;

    *size = 0;

    /* Ask how much memory the metadata needs. */
    status = uefi_call_wrapper(
        file->GetInfo,
        4,
        file,
        &GenericFileInfo,
        &info_size,
        NULL
    );

    /* This is the expected result when supplying no buffer. */
    if (status != EFI_BUFFER_TOO_SMALL) {
        if (EFI_ERROR(status))
            return status;
        
        return EFI_DEVICE_ERROR;
    }

    info = AllocatePool(info_size);

    if (info == NULL)
        return EFI_OUT_OF_RESOURCES;

    /* Retrieve the metadata into the allocated buffer. */
    status = uefi_call_wrapper(
        file->GetInfo,
        4,
        file,
        &GenericFileInfo,
        &info_size,
        info
    );

    if (!EFI_ERROR(status))
        *size = info->FileSize;
    
    FreePool(info);

    return status;
}

EFI_STATUS bootfs_read_all(
    EFI_FILE_PROTOCOL *file,
    VOID **data,
    UINTN *size
) {
    EFI_STATUS status;
    UINT64 file_size;
    UINTN length;
    UINTN total = 0;
    UINT8 *buffer;

    if (data == NULL || size == NULL)
        return EFI_INVALID_PARAMETER;

    *data = NULL;
    *size = 0;

    if (file == NULL)
        return EFI_INVALID_PARAMETER;

    status = bootfs_get_size(file, &file_size);
    if (EFI_ERROR(status))
        return status;

    /* The file size must fit the allocation and read size type. */
    if (file_size > (UINT64)(~(UINTN)0))
        return EFI_BAD_BUFFER_SIZE;

    length = (UINTN)file_size;

    /* Avoid requesting a zero-byte allocation. */
    if (length == 0)
        return EFI_SUCCESS;

    buffer = AllocatePool(length);
    if (buffer == NULL)
        return EFI_OUT_OF_RESOURCES;

    while (total < length) {
        UINTN remaining = length - total;
        UINTN bytes_read = remaining;

        status = bootfs_read(file, buffer + total, &bytes_read);
        if (EFI_ERROR(status))
            goto fail;

        /* EOF before the advertised file size is incomplete data. */
        if (bytes_read == 0) {
            status = EFI_END_OF_FILE;
            goto fail;
        }

        if (bytes_read > remaining) {
            status = EFI_DEVICE_ERROR;
            goto fail;
        }
        
        total += bytes_read;
    }

    /* Transfer ownership only once the complete read succeeds. */
    *data = buffer;
    *size = length;
    return EFI_SUCCESS;

fail:
    FreePool(buffer);
    return status;
}
