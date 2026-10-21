#include "arch/gdt.h"
#include "arch/tss.h"
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
    0,                  /* 0x18: TSS descriptor, low half. */
    0,                  /*       TSS descriptor, high half. */
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

static void gdt_set_tss(const TSS *tss) {
    uint64_t base = (uint64_t)(uintptr_t)tss;
    uint64_t limit = sizeof(*tss) - 1;

    gdt[3] = (limit & 0xFFFF)
           | ((base & 0xFFFFFF) << 16)
           | (UINT64_C(0x89) << 40)
           | (((limit >> 16) & 0xF) << 48)
           | (((base >> 24) & 0xFF) << 56);

    gdt[4] = base >> 32;
}

void gdt_init(void) {
    const TSS *tss = tss_init();
        gdt_set_tss(tss);

    const GDT_POINTER pointer = {
        /* The table occupies 40 bytes, so its limit is 39:
        * the offset of its last byte. */
        .limit = sizeof(gdt) - 1,
        .base = (uint64_t)(uintptr_t)gdt,
    };

    gdt_load(&pointer);

    uint16_t selector = GDT_TSS_SELECTOR;
    /* ltr means load task register, telling the CPU which TSS descriptor to use.
    * The GDT stays writable because loading the task register sets the descriptor's busy bit. */
    __asm__ volatile("ltr %0" : : "r"(selector) : "memory");
}

