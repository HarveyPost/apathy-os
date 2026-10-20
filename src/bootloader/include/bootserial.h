#ifndef JAZZ_BOOTSERIAL_H
#define JAZZ_BOOTSERIAL_H

#include <efi.h>

EFI_STATUS bootserial_open(
    EFI_SYSTEM_TABLE *system_table,
    EFI_SERIAL_IO_PROTOCOL **serial
);

EFI_STATUS bootserial_write(
    EFI_SERIAL_IO_PROTOCOL *serial,
    const char *text
);

#endif
