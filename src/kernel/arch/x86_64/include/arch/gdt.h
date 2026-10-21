#ifndef APATHY_GDT_H
#define APATHY_GDT_H

#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_TSS_SELECTOR 0x18

/* Install the kernel GDT with maskable interrupts already disabled. */
void gdt_init(void);

#endif
