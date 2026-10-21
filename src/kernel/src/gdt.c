#include "gdt.h"
#include <stdint.h>

/* Must occupy exactly ten bytes:
 * two-byte limit followed immediately by an eight-byte address.
 * packed prevents padding between them. */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} GDT_POINTER;

_Static_assert(sizeof(GDT_POINTER) == 10, "Unexpected GDTR layout");

/* The actual table.
 * Static because the CPU needs it to remain available after the function returns. */
static uint64_t gdt[] __attribute__((aligned(16))) = {
    0x0000000000000000, /* Null descriptor. */
    0x00AF9B000000FFFF, /* 0x08: ring 0, readable 64-bit code. */
    0x00CF93000000FFFF, /* 0x10: ring 0, writeable data. */
};

static __attribute__((naked)) void gdt_load(const GDT_POINTER *pointer) {
    __asm__(
        /* lgdt loads the new table's address and limit into GDTR.
         * It doesn't copy the entire table.
         * RDI contains the pointer argument. */
        "lgdt (%rdi)\n\t"

        /* This constructs a far-return destination on the stack:
         * Code selector: 0x08.
         * Instruction address: the upcoming label 1:. 
         *
         * leaq calculates that label's address.
         * lretq then transfers execution there while reloading CS, the code-segment register.
         * This is necessary as loading GDTR alone doesn't refresh the current code segment. */
        "pushq $0x08\n\t"
        "leaq 1f(%rip), %rax\n\t"
        "pushq %rax\n\t"
        "lretq\n\t"

        "1:\n\t"
        /* mov instructions load data selector 0x10 into DS, ES, and SS */
        "mov $0x10, %ax\n\t"
        "mov %ax, %ds\n\t"
        "mov %ax, %es\n\t"
        "mov %ax, %ss\n\t"
        /* The final ordinary ret returns to gdt_init. */
        "ret\n\t"
    );
}

void gdt_init(void) {
        const GDT_POINTER pointer = {
            /* The table occupies 24 bytes, so its limit is 23:
            * the offset of its last byte. */
            .limit = sizeof(gdt) - 1,
            .base = (uint64_t)(uintptr_t)gdt,
        };

        gdt_load(&pointer);
    }
