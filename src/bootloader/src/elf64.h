#ifndef JAZZ_ELF64_H
#define JAZZ_ELF64_H

#include <efi.h>

#define ELF_CLASS_64 2
#define ELF_DATA_LITTLE_ENDIAN 1
#define ELF_VERSION_CURRENT 1
#define ELF_TYPE_EXECUTABLE 2
#define ELF_MACHINE_X86_64 62
#define ELF_PROGRAM_LOAD 1
#define ELF_PROGRAM_DYNAMIC 2
#define ELF_PROGRAM_INTERPRETER 3
#define ELF_PROGRAM_TLS 7
#define ELF_SEGMENT_EXECUTE 1

typedef struct {
    UINT8 e_ident[16];
    UINT16 e_type;
    UINT16 e_machine;
    UINT32 e_version;
    UINT64 e_entry;
    UINT64 e_phoff;
    UINT64 e_shoff;
    UINT32 e_flags;
    UINT16 e_ehsize;
    UINT16 e_phentsize;
    UINT16 e_phnum;
    UINT16 e_shentsize;
    UINT16 e_shnum;
    UINT16 e_shstrndx;
} ELF64_HEADER;

typedef struct {
    UINT32 p_type;
    UINT32 p_flags;
    UINT64 p_offset;
    UINT64 p_vaddr;
    UINT64 p_paddr;
    UINT64 p_filesz;
    UINT64 p_memsz;
    UINT64 p_align;
} ELF64_PROGRAM_HEADER;

_Static_assert(sizeof(ELF64_HEADER) == 64,
               "ELF64 header must be 64 bytes");

/* Validate the fixed header and copy it to header on success.
* This does not validate program headers or load any segments. */
EFI_STATUS elf64_validate_header(
    const VOID *data,
    UINTN size,
    ELF64_HEADER *header
);

_Static_assert(sizeof(ELF64_PROGRAM_HEADER) == 56,
               "ELF64 program header must be 56 bytes");

/* Validate ELF headers, file bounds, loadable memory raanges and entry point.
 * Physical and virtual addresses must match.
 * Does not reserve memory or copy segments. */
EFI_STATUS elf64_validate_image(
    const VOID *data,
    UINTN size,
    ELF64_HEADER *header
);

#endif
