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

EFI_STATUS elf64_validate_image(
    const VOID *data,
    UINTN size,
    ELF64_HEADER *header
) {
    EFI_STATUS status;
    ELF64_HEADER candidate;
    const UINT8 *bytes = data;
    UINTN load_count = 0;
    UINTN table_offset;

    if (header == NULL)
        return EFI_INVALID_PARAMETER;

    status = elf64_validate_header(data, size, &candidate);
    if (EFI_ERROR(status))
        return status;

    if (candidate.e_phnum == 0)
        return EFI_LOAD_ERROR;

    /* Extended program-header numbering is not supported yet. */
    if (candidate.e_phnum == 0xffff)
        return EFI_UNSUPPORTED;

    if (candidate.e_phentsize != sizeof(ELF64_PROGRAM_HEADER))
        return EFI_LOAD_ERROR;

    if (candidate.e_phoff < sizeof(ELF64_HEADER) ||
        candidate.e_phoff > (UINT64)size)
        return EFI_LOAD_ERROR;

    /* Check the complete table without overflowing a size calculation. */
    if ((UINT64)candidate.e_phnum >
        ((UINT64)size - candidate.e_phoff) / sizeof(ELF64_PROGRAM_HEADER))
        return EFI_LOAD_ERROR;

    table_offset = (UINTN)candidate.e_phoff;

    for (UINTN i = 0; i < candidate.e_phnum; i++) {
        ELF64_PROGRAM_HEADER segment;
        UINTN offset = table_offset + i * sizeof(segment);

        uefi_call_wrapper(
            CopyMemC,
            3,
            &segment,
            bytes + offset,
            sizeof(segment)
        );

        if (segment.p_type != ELF_PROGRAM_LOAD)
            continue;

        if (segment.p_filesz > segment.p_memsz)
            return EFI_LOAD_ERROR;

        /* A segment with no file bytes needs no file-data range. */
        if (segment.p_filesz != 0 &&
            (segment.p_offset > (UINT64)size ||
             segment.p_filesz > (UINT64)size - segment.p_offset))
            return EFI_LOAD_ERROR;

        if (segment.p_memsz != 0)
            ++load_count;
    }

    if (load_count == 0)
        return EFI_LOAD_ERROR;

    *header = candidate;
    return EFI_SUCCESS;
}
