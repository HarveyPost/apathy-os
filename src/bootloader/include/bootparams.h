#ifndef APATHY_BOOTPARAMS_H
#define APATHY_BOOTPARAMS_H

#include <efi.h>
#include "bootinfo.h"

typedef struct {
    EFI_PHYSICAL_ADDRESS base;
    UINTN pages;
    BOOT_INFO *info;
} BOOT_PARAMETERS;

EFI_STATUS bootparams_allocate(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_PARAMETERS *parameters
);

EFI_STATUS bootparams_release(
    EFI_SYSTEM_TABLE *system_table,
    BOOT_PARAMETERS *parameters
);

#endif
