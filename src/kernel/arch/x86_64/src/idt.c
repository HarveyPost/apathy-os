#include "arch/idt.h"
#include "arch/gdt.h"
#include "panic.h"
#include "serial.h"
#include "arch/tss.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed)) {
    uint16_t offset_low; /* offset bits 0..15 */
    uint16_t selector; /* a code segment selector in GDT or LDT */
    uint8_t ist; /* bits 0..2 holds Interrupt Stack Table offset, rest of bits zero */
    uint8_t attributes; /* gate type, descriptor privilege level (DPL), and present (P) fields */
    uint16_t offset_middle; /* offset bits 16..31 */
    uint32_t offset_high; /* offset bits 32..63 */
    uint32_t reserved; /* Must be zero */
} IDT_ENTRY;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} IDT_POINTER;

typedef struct {
    uint64_t rip; /* saved instruction pointer */
    uint64_t cs; /* code selector */
    uint64_t rflags; /* flags */
    uint64_t rsp; /* stack pointer */
    uint64_t ss; /* stack selector */
} INTERRUPT_FRAME;

_Static_assert(sizeof(IDT_ENTRY) == 16, "Unexpected IDT entry layout");
_Static_assert(sizeof(IDT_POINTER) == 10, "Unexpected IDTR layout");

static IDT_ENTRY idt[256] __attribute__((aligned(16)));

static _Noreturn void exception_panic(
    const char *message,
    const INTERRUPT_FRAME *frame,
    bool has_error_code,
    uint64_t error_code
) {
    /* rip is the saved instruction address. */
    (void)serial_write("\r\n[exception] RIP: ");
    (void)serial_write_hex64(frame->rip);
    /* rsp is the saved stack pointer. */
    (void)serial_write("\r\n[exception] RSP: ");
    (void)serial_write_hex64(frame->rsp);

    if (has_error_code) {
        (void)serial_write("\r\n[exception] Error code: ");
        (void)serial_write_hex64(error_code);
    }

    (void)serial_write("\r\n");
    PANIC(message);
}

static __attribute__((interrupt)) void unhandled_interrupt(
    INTERRUPT_FRAME *frame
) {
    exception_panic("Unhandled interrupt or exception", frame, false, 0);
}

static __attribute__((interrupt)) void unhandled_error(
    INTERRUPT_FRAME *frame, uint64_t error_code
) {
    exception_panic("CPU exception with an error code", frame, true, error_code);
}

static __attribute__((interrupt)) void breakpoint_handler(
    INTERRUPT_FRAME *frame
) {
    exception_panic("Breakpoint exception", frame, false, 0);
}

static __attribute__((interrupt)) void double_fault_handler(
    INTERRUPT_FRAME *frame, uint64_t error_code
) {
    exception_panic("Double fault", frame, true, error_code);
}

static __attribute__((interrupt)) void nmi_handler(
    INTERRUPT_FRAME *frame
) {
    exception_panic("Non-maskable interrupt", frame, false, 0);
}

static __attribute__((interrupt)) void machine_check_handler(
    INTERRUPT_FRAME *frame
) {
    exception_panic("Machine check", frame, false, 0);
}

static void idt_set_gate(unsigned vector, uintptr_t address, uint8_t ist) {
    idt[vector] = (IDT_ENTRY) {
        .offset_low = (uint16_t)address,
        .selector = GDT_KERNEL_CODE_SELECTOR,
        .ist = ist,
        .attributes = 0x8E, /* creates a present, ring-0 interrupt gate. */
        .offset_middle = (uint16_t)(address >> 16),
        .offset_high = (uint32_t)(address >> 32),
        .reserved = 0,
    };
}

void idt_init(void) {
    for (unsigned vector = 0; vector < 256; vector++) {
        idt_set_gate(vector, (uintptr_t)unhandled_interrupt, 0);
    }

    const uint8_t error_vectors[] = {
        8, 10, 11, 12, 13, 14, 17, 21, 29, 30
    };

    for (size_t i = 0; i < sizeof(error_vectors) / sizeof(error_vectors[0]); i ++) {
        idt_set_gate(error_vectors[i], (uintptr_t)unhandled_error, 0);
    }

    idt_set_gate(3, (uintptr_t)breakpoint_handler, 0);

    idt_set_gate(8, (uintptr_t)double_fault_handler, TSS_IST_DOUBLE_FAULT);
    idt_set_gate(2, (uintptr_t)nmi_handler, TSS_IST_NMI);
    idt_set_gate(18, (uintptr_t)machine_check_handler, TSS_IST_MACHINE_CHECK);

    const IDT_POINTER pointer = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)(uintptr_t)idt,
    };

    /* lidt installs these values into IDTR, making this table active. */
    __asm__ volatile("lidt %0" : : "m"(pointer) : "memory");
}
