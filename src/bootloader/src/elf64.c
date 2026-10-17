#include "elf64.h"
#include <efilib.h>

EFI_STATUS elf64_validate_header (
    const VOID *data,
    UINTN size,
    ELF64_HEADER *header
) {
    ELF64_HEADER candidate;

    if (header == NULL)
        return EFI_INVALID_PARAMETER;

    /* Check the buffer length before reading anything from it. */
    if (size < sizeof(candidate))
        return EFI_LOAD_ERROR;

    if (data == NULL)
        return EFI_INVALID_PARAMETER;

    uefi_call_wrapper(
        CopyMemC,
        3,
        &candidate,
        data,
        sizeof(candidate)
    );

    /* Check the ELF signature. */
    if (candidate.e_ident[0] != 0x7f ||
        candidate.e_ident[1] != 'E' ||
        candidate.e_ident[2] != 'L' ||
        candidate.e_ident[3] != 'F')
        return EFI_LOAD_ERROR;

    /* Check class, byte order and identification version. */
    if (candidate.e_ident[4] != ELF_CLASS_64 ||
        candidate.e_ident[5] != ELF_DATA_LITTLE_ENDIAN ||
        candidate.e_ident[6] != ELF_VERSION_CURRENT)
        return EFI_LOAD_ERROR;

    /* Check the executable format supported by this loader. */
    if (candidate.e_type != ELF_TYPE_EXECUTABLE ||
        candidate.e_machine != ELF_MACHINE_X86_64 ||
        candidate.e_version != ELF_VERSION_CURRENT ||
        candidate.e_ehsize != sizeof(candidate))
        return EFI_LOAD_ERROR;

    *header = candidate;
    return EFI_SUCCESS;
}

