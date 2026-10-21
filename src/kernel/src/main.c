#include "bootinfo.h"
#include "serial.h"
#include "panic.h"
#include "memory.h"
#include "gdt.h"
#include "idt.h"
#include <stddef.h>

static _Noreturn void kernel_halt(void) {
    for (;;) {
        __asm__ volatile("cli\n\thlt");
    }
}

_Noreturn void KERNEL_ABI kernel_main(const BOOT_INFO *boot_info) {
    if (!serial_init())
        kernel_halt();

    if (!serial_write("[kernel] Apathy kernel entered.\r\n"))
        kernel_halt();

    if (boot_info == NULL ||
        boot_info->version != BOOT_INFO_VERSION ||
        boot_info->size != sizeof(BOOT_INFO)) {
        PANIC("Invalid boot information");
    }

    (void)serial_write("[kernel] Boot information format valid.\r\n");

    gdt_init();
    (void)serial_write("[kernel] GDT and TSS installed.\r\n");

    idt_init();
    (void)serial_write("[kernel] IDT installed.\r\n");

    unsigned char buffer[8];

    if (memset(buffer, 0x1AB, sizeof(buffer)) != buffer)
        PANIC("memset returned the wrong pointer");

    for (size_t i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0xAB)
            PANIC("memset fill failed");
    }
    
    memset(buffer + 2, 0, 4);

    for (size_t i = 0; i < sizeof(buffer); i++) {
        unsigned char expected = (i >= 2 && i < 6) ? 0 : 0xAB;

        if (buffer[i] != expected)
            PANIC("memset partial fill failed");
    }

    (void)serial_write("[kernel] memset checks passed.\r\n");

    const unsigned char source[] = {0x12, 0x00, 0x80, 0xFF};
    unsigned char destination[6];

    memset(destination, 0xAA, sizeof(destination));

    if (memcpy(destination + 1, source, sizeof(source)) != destination + 1)
        PANIC("memcpy returned the wrong pointer");

    for (size_t i = 0; i < sizeof(source); i++) {
        if (destination[i + 1] != source[i])
            PANIC("memcpy copy failed");
    }

    if (destination[0] != 0xAA || destination[5] != 0xAA)
        PANIC("memcpy wrote beyond its destination");

    if (memcpy(destination, source, 0) != destination || destination[0] != 0xAA)
        PANIC("memcpy zero-length copy failed");

    (void)serial_write("[kernel] memcpy checks passed.\r\n");

    char right[] = "ABCDE";
    char left[] = "ABCDE";
    const char expected_right[] = "AABCD";
    const char expected_left[] = "BCDEE";
    
    if (memmove(right + 1, right, 4) != right + 1)
        PANIC("memmove returned the wrong pointer");

    if (memmove(left, left + 1, 4) != left)
        PANIC("memmove returned the wrong pointer");

    if (memmove(right, left, 0) != right)
        PANIC("memmove zero-length return failed");

    if (memmove(left, left, sizeof(left)) != left)
        PANIC("memmove self-copy return failed");

    for (size_t i = 0; i < sizeof(right); i++) {
        if (right[i] != expected_right[i] || left[i] != expected_left[i])
            PANIC("memmove overlap checks failed");
    }

    (void)serial_write("[kernel] memmove checks passed.\r\n");

    const unsigned char a[] = {0x00, 0x80, 0x00};
    const unsigned char b[] = {0x00, 0x7F, 0xFF};
    const unsigned char equal[] = {0x00, 0x80, 0x00};

    if (memcmp(a, equal, sizeof(a)) != 0)
        PANIC("memcmp equality check failed");

    if (memcmp(a, b, sizeof(a)) <= 0)
        PANIC("memcmp greater-than check failed");

    if (memcmp(b, a, sizeof(a)) >= 0)
        PANIC("memcmp less-than check failed");

    if (memcmp(a, b, 1) != 0)
        PANIC("memcmp prefix check failed");

    if (memcmp(a, b, 0) != 0)
        PANIC("memcmp zero-length check failed");

    if (memcmp(a, a, sizeof(a)) != 0)
        PANIC("memcmp self-comparison failed");

    (void)serial_write("[kernel] memcmp checks passed.\r\n");

    kernel_halt();
}
