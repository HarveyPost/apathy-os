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

/* Read a freshly opened regular file into an allocated buffer.
*
* Success: caller owns *data and must FreePool() it when non-Null.
* Empty file: success with *data == NULL and *size == 0.
* Failure: no allocation is returned.
*
* The caller retains ownership of the file handle. */
EFI_STATUS bootfs_read_all(
    EFI_FILE_PROTOCOL *file,
    VOID **data,
    UINTN *size
);

/* Set a regular file's position to an absolute byte offset.
* Offset 0 means the beginning. The caller still owns the handle. */
EFI_STATUS bootfs_seek(
    EFI_FILE_PROTOCOL *file,
    UINT64 offset
);

#endif
