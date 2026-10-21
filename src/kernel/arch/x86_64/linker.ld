OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_main)

SECTIONS
{
    /* Begin placing the kernel at 1 MiB. */
    . = 0x100000;
    
    /* Place executable code on a 4096-byte boundary. */
    .text ALIGN(4096) : {
        *(.text .text.*)
    }
    
    /* Read-only data. Typically string literals and constant tables. */
    .rodata ALIGN(4096) : {
        *(.rodata .rodata.*)
    }

    /* Writable variables with initial values. */
    .data ALIGN(4096) : {
        *(.data .data.*)
    }

    /* Zero-initialised storage, typically including global or static variables. */
    .bss ALIGN(4096) (NOLOAD) : {
        *(COMMON)
        *(.bss .bss.*)
    }
}
