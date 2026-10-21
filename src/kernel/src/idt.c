#include "idt.h"
#include "gdt.h"
#include "panic.h"
#include <stddef.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} IDT_ENTRY;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} IDT_POINTER;

typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} INTERRUPT_FRAME;

_Static_assert(sizeof(IDT_ENTRY) == 16, "Unexpected IDT entry layout");
_Static_assert(sizeof(IDT_POINTER) == 10, "Unexpected IDTR layout");

static IDT_ENTRY idt[256] __attribute__((aligned(16)));

static __attribute__((interrupt)) void unhandled_interrupt(
    INTERRUPT_FRAME *frame
) {
    (void)frame;
    PANIC("Unhandled interrupt or exception");
}

static __attribute__((interrupt)) void unhandled_error(
    INTERRUPT_FRAME *frame, uint64_t error_code
) {
    (void)frame;
    (void)error_code;
    PANIC("CPU exception with an error code");
}

static __attribute__((interrupt)) void breakpoint_handler(
    INTERRUPT_FRAME *frame
) {
    (void)frame;
    PANIC("Breakpoint exception");
}

static void idt_set_gate(unsigned vector, uintptr_t address) {
    idt[vector] = (IDT_ENTRY) {
        .offset_low = (uint16_t)address,
        .selector = GDT_KERNEL_CODE_SELECTOR,
        .ist = 0,
        .attributes = 0x8E,
        .offset_middle = (uint16_t)(address >> 16),
        .offset_high = (uint32_t)(address >> 32),
        .reserved = 0,
    };
}

void idt_init(void) {
    for (unsigned vector = 0; vector < 256; vector++) {
        idt_set_gate(vector, (uintptr_t)unhandled_interrupt);
    }

    const uint8_t error_vectors[] = {
        8, 10, 11, 12, 13, 14, 17, 21, 29, 30
    };

    for (size_t i = 0; i < sizeof(error_vectors) / sizeof(error_vectors[0]); i ++) {
        idt_set_gate(error_vectors[i], (uintptr_t)unhandled_error);
    }

    idt_set_gate(3, (uintptr_t)breakpoint_handler);

    const IDT_POINTER pointer = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)(uintptr_t)idt,
    };

    __asm__ volatile("lidt %0" : : "m"(pointer) : "memory");
}
