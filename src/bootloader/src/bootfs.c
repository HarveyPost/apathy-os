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

    /* The caller passed &root, so dereferencing that address
     * lets this function change the original value. */
    *root = NULL;

    /* Find information about our running application.
     *
     * image_handle - identifies the running bootloader.
     * *LoadedImageProtocol - points to a GUID identifying the loaded-image protocol. A GUID is a unique identifier that tells the firmware which interface you want.
     * (VOID **)*loaded_image - tells the firmware where to write the resulting interface pointer. */
    status = uefi_call_wrapper(
        system_table->BootServices->HandleProtocol,
        3,
        image_handle,
        &LoadedImageProtocol,
        (VOID **)&loaded_image
    );

    if (EFI_ERROR(status))
        return status;

    /* Get the filesystem on the device that loaded us. 
     * 
     * loaded_image->DeviceHandle - identifies the device from which the bootloader was loaded.
     * &FileSystemProtocol - identifies the simple filesystem protocol.
     * (VOID **)&filesystem - gives the firmware somewhere to write the resulting interface pointer. */
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
    
    /* root - Directory from which to open the file
    *  file - Where firmware writes the new handle
    *  path - Filename or path
    *  EFI_FILE_MODE_READ - open for reading
    *  0 - Attributes are unused when opening an existing file
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

    /* Ask how much memory the metadata needs.
     *
     * file - identifies the open file.
     * &GenericFileInfo - identifies the kid of metadata you want.
     * &info_size - supplies the buffer capacity and receives the required size.
     * NULL - means we haven't provided a buffer yet. */
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
    
    /* Now you can allocate the metadata buffer. */
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

    /* Extract the field we want, in this case FileSize. */
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
    
    /* Without valid addresses, it cannot safely write the results. */
    if (data == NULL || size == NULL)
        return EFI_INVALID_PARAMETER;

    *data = NULL;
    *size = 0;

    if (file == NULL)
        return EFI_INVALID_PARAMETER;

    status = bootfs_get_size(file, &file_size);
    if (EFI_ERROR(status))
        return status;

    /* The file size must fit the allocation and read size type. 
     * The expression builds the largest possible UINTN value:
     *
     * (UINTN)0 creates a zero of type UINTN.
     * ~ flips every bit, producing all ones.
     * (UINT64) converts that limit to the type of file_size for comparison. */
    if (file_size > (UINT64)(~(UINTN)0))
        return EFI_BAD_BUFFER_SIZE;

    length = (UINTN)file_size;

    /* Avoid requesting a zero-byte allocation. */
    if (length == 0)
        return EFI_SUCCESS;

    buffer = AllocatePool(length);
    if (buffer == NULL)
        return EFI_OUT_OF_RESOURCES;
    
    /* Reading loop; need to fill the buffer with the file's bytes. 
     *
     * bytes_read is both an input and an output.
     * Before the call, it contains the maximum number of bytes to read.
     * Afterwards, it contains the actual number read. */
    while (total < length) {
        UINTN remaining = length - total;
        UINTN bytes_read = remaining;

        /* buffer + total chooses where to put the bytes.
         * Because buffer is a UINT8 *, adding one moves forward one byte.
         * If we have already read 100 bytes, the next read writes at buffer + 100. */
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

