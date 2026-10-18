#include "bootserial.h"
#include <efilib.h>

EFI_STATUS bootserial_open(
    EFI_SYSTEM_TABLE *system_table,
    EFI_SERIAL_IO_PROTOCOL **serial
) {
    EFI_STATUS status;
    EFI_SERIAL_IO_PROTOCOL *candidate;

    if (serial == NULL)
        return EFI_INVALID_PARAMETER;

    *serial = NULL;

    if (system_table == NULL)
        return EFI_INVALID_PARAMETER;

    /* &SerialIoProtocol - the GUID identifying the serial I/O protocol.
     * NULL - no protocol-notification registration key; perform an ordinary lookup.
     * &candidate - receives the discovered interface. */
    status = uefi_call_wrapper(
        system_table->BootServices->LocateProtocol,
        3,
        &SerialIoProtocol,
        NULL,
        (VOID **)&candidate
    );

    if (EFI_ERROR(status))
        return status;

    /* The combination of eight data bits, no parity and one stop bit is commonly called 8N1.
     * 
     * candidate - the serial interface to configure.
     * 115200 - requested speed: 115,200 baud.
     * 0 - use the device's default receive-buffer depth.
     * 100000 - timeout per character: 100,000 microseconds, or 0.1 seconds.
     * NoParity - don't use a parity bit.
     * 8 - send eight data bits per character.
     * OneStopBit - Use one stop bit. */
    status = uefi_call_wrapper(
        candidate->SetAttributes,
        7,
        candidate,
        (UINT64)115200,
        0,
        100000,
        NoParity,
        8,
        OneStopBit
    );

    if (EFI_ERROR(status))
        return status;

    *serial = candidate;
    return EFI_SUCCESS;
}

EFI_STATUS bootserial_write(
    EFI_SERIAL_IO_PROTOCOL *serial,
    const char *text
) {
    UINTN remaining = 0;

    if (serial == NULL || text == NULL)
        return EFI_INVALID_PARAMETER;

    while (text[remaining] != '\0')
        remaining++;

    while (remaining != 0) {
        UINTN written = remaining;
        EFI_STATUS status;

        /* Written starts as the number of bytes you want to send.
         * After the call, written reports how many bytes were actually transmitted. */
        status = uefi_call_wrapper(
            serial->Write,
            3,
            serial,
            &written,
            (VOID *)text
        );

        if (EFI_ERROR(status))
            return status;

        if (written == 0 || written > remaining)
            return EFI_DEVICE_ERROR;

        text += written;
        remaining -= written;
    }
    
    return EFI_SUCCESS;
}
