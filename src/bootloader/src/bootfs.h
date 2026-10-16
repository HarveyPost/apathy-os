#ifndef JAZZ_BOOTFS_H
#define JAZZ_BOOTFS_H

#include <efi.h>

/* On success, the caller receives a root handle and must close it. */

/* The function returns an EFI_STATUS describing success or failure.
* The opened directory is returned separately through root.
*
* root has two starts because the function needs to change the caller's pointer.
* We'll pass the address of that pointer. */
EFI_STATUS bootfs_open_root(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    EFI_FILE_PROTOCOL **root
);

#endif
