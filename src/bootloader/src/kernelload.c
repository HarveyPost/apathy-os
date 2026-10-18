#include "kernelload.h"
#include "elf64.h"
#include <efilib.h>

EFI_STATUS kernelload_reserve(
    EFI_SYSTEM_TABLE *system_table,
    const VOID *data,
    UINTN size,
    KERNEL_IMAGE *image
) {
    EFI_STATUS status;
    ELF64_HEADER header;
    const UINT8 *bytes = data;
    UINT64 first = ~(UINT64)0;
    UINT64 last = 0;
    UINT64 page_count;
    EFI_PHYSICAL_ADDRESS address;

    if (system_table == NULL || image == NULL)
        return EFI_INVALID_PARAMETER;

    /* Avoid overwriting an existing allocation record. */
    if (image->pages != 0)
        return EFI_INVALID_PARAMETER;

    *image = (KERNEL_IMAGE){0};

    status = elf64_validate_image(data, size, &header);
    if (EFI_ERROR(status))
        return status;

    for (UINTN i = 0; i < header.e_phnum; i++) {
        ELF64_PROGRAM_HEADER segment;
        UINTN offset = (UINTN)header.e_phoff + i * sizeof(segment);
        UINT64 end;

        uefi_call_wrapper(
            CopyMemC,
            3,
            &segment,
            bytes + offset,
            sizeof(segment)
        );

        if (segment.p_type != ELF_PROGRAM_LOAD ||
            segment.p_memsz == 0)
            continue;

        /* Validation already checked this addition for overflow. */
        end = segment.p_paddr + segment.p_memsz;

        /* Reserve one continuous span covering both segments and the gap between. */
        if (segment.p_paddr < first)
            first = segment.p_paddr;

        if (end > last)
            last = end;
    }
    
    /* Round the allocation outwards to whole pages.
     * UEFI's page-allocation service works in units of 4096 bytes,
     * so the allocation must start on a page boundary and cover
     * a whole number of pages.
     *
     * First, round the starting address down. */
    first &= ~((UINT64)EFI_PAGE_MASK);

    /* Next, round the exclusive end address up. */
    if (last > (~(UINT64)0) - EFI_PAGE_MASK)
        return EFI_LOAD_ERROR;
    last = (last + EFI_PAGE_MASK) & ~((UINT64)EFI_PAGE_MASK);

    /* Page zero is outside this loader's supported placements. */
    if (first == 0)
        return EFI_UNSUPPORTED;

    page_count = (last - first) / EFI_PAGE_SIZE;

    if (page_count > (UINT64)(~(UINTN)0))
        return EFI_BAD_BUFFER_SIZE;

    /* Allocate the pages calculated. */
    address = first;

    /* AllocateAddress - allocate at the exact address supplied through address.
     * EfiLoaderCode - classify the allocated memory as loader code.
     * (UINTN)page_count - how many consecutive pages to allocate.
     * &address - a pointer through which the firmware receives the requested address and returns the allocation address. */
    status = uefi_call_wrapper(
        system_table->BootServices->AllocatePages,
        4,
        AllocateAddress,
        EfiLoaderCode,
        (UINTN)page_count,
        &address
    );

    if (EFI_ERROR(status))
        return status;

    /* On success, fill the caller's KERNEL_IMAGE record. */
    image->base = address;
    image->pages = (UINTN)page_count;
    image->entry = header.e_entry;

    return EFI_SUCCESS;
}

EFI_STATUS kernelload_release(
    EFI_SYSTEM_TABLE *system_table,
    KERNEL_IMAGE *image
) {
    EFI_STATUS status;

    if (system_table == NULL || image == NULL)
        return EFI_INVALID_PARAMETER;

    /* Zero pages means the record owns no allocation, so there is nothing to release. */
    if (image->pages == 0)
        return EFI_SUCCESS;

    status = uefi_call_wrapper(
        system_table->BootServices->FreePages,
        2,
        image->base,
        image->pages
    );

    if (!EFI_ERROR(status))
        *image = (KERNEL_IMAGE){0};

    return status;
}

EFI_STATUS kernelload_load(
    EFI_SYSTEM_TABLE *system_table,
    const VOID *data,
    UINTN size,
    KERNEL_IMAGE *image
) {
    EFI_STATUS status;
    ELF64_HEADER header;
    const UINT8 *bytes = data;

    status = kernelload_reserve(system_table, data, size, image);
    if (EFI_ERROR(status))
        return status;

    /* Reservation has already validated this header and its table.
     * Copy the ELF header into its own local variable. */
    uefi_call_wrapper(
        CopyMemC,
        3,
        &header,
        data,
        sizeof(header)
    );

    /* Zero segment tails, gaps and page padding before copying as
     * allocation alone doesn't guarantee that the memory contains zeroes. */
    uefi_call_wrapper(
        system_table->BootServices->SetMem,
        3,
        (VOID *)(UINTN)image->base,
        image->pages * EFI_PAGE_SIZE,
        0
    );

    /* Copy each loadable segment into the allocated memory. */
    for (UINTN i = 0; i < header.e_phnum; i++) {
        ELF64_PROGRAM_HEADER segment;
        UINTN offset = (UINTN)header.e_phoff + i * sizeof(segment);

        uefi_call_wrapper(
            CopyMemC,
            3,
            &segment,
            bytes + offset,
            sizeof(segment)
        );
        
        if (segment.p_type != ELF_PROGRAM_LOAD ||
            segment.p_filesz == 0)
            continue;

        /* Destination: segment.p_paddr,
         * Source: the file buffer plus segment.p_offset,
         * Amount: segment.p_filesz bytes */
        uefi_call_wrapper(
            CopyMemC,
            3,
            (VOID *)(UINTN)segment.p_paddr,
            bytes + (UINTN)segment.p_offset,
            (UINTN)segment.p_filesz
        );
    }

    return EFI_SUCCESS;
}
