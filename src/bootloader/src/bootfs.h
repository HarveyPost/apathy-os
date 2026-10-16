#ifndef JAZZ_BOOTFS_H
#define JAZZ_BOOTFS_H

#include <efi.h>


/* The function returns an EFI_STATUS describing success or failure.
* The opened directory is returned separately through root.
*
* root has two starts because the function needs to change the caller's pointer.
* We'll pass the address of that pointer.
*
* On success, the caller receives a root handle and must close it. */
EFI_STATUS bootfs_open_root(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    EFI_FILE_PROTOCOL **root
);


/* Opens an existing file for reading.
 *
* On success, the caller must close the returned handle. */
EFI_STATUS bootfs_open_readonly(
    EFI_FILE_PROTOCOL *root,
    CHAR16 *path,
    EFI_FILE_PROTOCOL **file
);


/* size: buffer capacity on entry, bytes read on successful return.
* The caller supplies and owns the buffer. */
EFI_STATUS bootfs_read(
    EFI_FILE_PROTOCOL *file,
    VOID *buffer,
    UINTN *size
);

/* Use UEFI's GetInfo() operation to obtain an EFI_FILE_INFO structure, then extract its FileSize field. */
EFI_STATUS bootfs_get_size(
    EFI_FILE_PROTOCOL *file,
    UINT64 *size
);

#endif
