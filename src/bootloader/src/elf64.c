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

    if (size < sizeof(candidate))
        return EFI_LOAD_ERROR;

    if (data == NULL)
        return EFI_INVALID_PARAMETER;

    /* Copy the header's first 64 bits into candidate.
     *
     * Arguments are destination, source and byte count.
     * Copying into a local structure gives an approximately aligned object whos members we can inspect. */
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

    UINT64 previous_end = 0;
    BOOLEAN entry_valid = FALSE;

    if (header == NULL)
        return EFI_INVALID_PARAMETER;

    status = elf64_validate_header(data, size, &candidate);
    if (EFI_ERROR(status))
        return status;


    /*** Next, check the program header table. ***/

    if (candidate.e_phnum == 0)
        return EFI_LOAD_ERROR;

    /* Extended program-header numbering is not supported yet. */
    if (candidate.e_phnum == 0xffff)
        return EFI_UNSUPPORTED;

    /* The program-header structure is 56 bytes.
     * Each table entry must match the size the code expects. */
    if (candidate.e_phentsize != sizeof(ELF64_PROGRAM_HEADER))
        return EFI_LOAD_ERROR;
    
    /* Check where the table starts.
     * The loader requires it to start after the fixed header and within the file's bounds. */
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
        
        /* Copy the entry into a local structure. */
        uefi_call_wrapper(
            CopyMemC,
            3,
            &segment,
            bytes + offset,
            sizeof(segment)
        );

        /* Features the loader doesn't implement:
         * 1. dynamic linking information.
         * 2. a requested interpreter.
         * 3. thread-local storage. */
        if (segment.p_type == ELF_PROGRAM_DYNAMIC ||
            segment.p_type == ELF_PROGRAM_INTERPRETER ||
            segment.p_type == ELF_PROGRAM_TLS)
            return EFI_UNSUPPORTED;

        if (segment.p_type != ELF_PROGRAM_LOAD)
            continue;

        /* The destination must have room for all the file bytes.
         *
         * p_filesz - how many bytes come from the file.
         * p_memsz - how many bytes the segment occupies in memory. */
        if (segment.p_filesz > segment.p_memsz)
            return EFI_LOAD_ERROR;

        /* Check a segment's source bytes actually exist inside the file buffer.
         *
         * p_offset - where the segment's bytes begin in the file.
         * p-filesz - how many bytes to copy from there.
         * size - the size of the complete file buffer. */
        if (segment.p_filesz != 0 &&
            (segment.p_offset > (UINT64)size ||
             segment.p_filesz > (UINT64)size - segment.p_offset))
            return EFI_LOAD_ERROR;

        /* Empty loadable segments need no destination memory. */
        if (segment.p_memsz == 0)
            continue;

        /* Initially, physical and virtual addresses must match.
         * Supporting different values would require arranging suitable address mappings. */
        if (segment.p_paddr != segment.p_vaddr)
            return EFI_UNSUPPORTED;

        /* Check for overflow when calculating the segment's end address. */
        if (segment.p_memsz > (~(UINT64)0) - segment.p_vaddr)
            return EFI_LOAD_ERROR;

        /* Alignment must be 0, 1 or a power of two.
         * For alignment greater than 1, the virtual address and file offset
         * must have the same remainder when divided by the alignment. */
        if (segment.p_align > 1 &&
            ((segment.p_align & (segment.p_align - 1)) != 0 ||
             segment.p_vaddr % segment.p_align !=
             segment.p_offset % segment.p_align))
            return EFI_LOAD_ERROR;

        /* Require sorted, non-overlapping loadable byte ranges. */
        if (segment.p_vaddr < previous_end)
            return EFI_LOAD_ERROR;

        previous_end = segment.p_vaddr + segment.p_memsz;

        /* Check whether the kernel's entry point is inside a suitable segment.
         *
         * p_filesz instead of p_memsz as it requires the entry point to refer to file-backed executable bytes, rather than the additional zero-filled storage. */
        if ((segment.p_flags & ELF_SEGMENT_EXECUTE) != 0 &&
            candidate.e_entry >= segment.p_vaddr &&
            candidate.e_entry - segment.p_vaddr < segment.p_filesz)
            entry_valid = TRUE;

        ++load_count;
    }

    if (load_count == 0 || !entry_valid)
        return EFI_LOAD_ERROR;

    *header = candidate;
    return EFI_SUCCESS;
}
