#ifndef JAZZ_GDT_H
#define JAZZ_GDT_H

#define GDT_KERNEL_CODE_SELECTOR 0x08

/* Install the kernel GDT with maskable interrupts already disabled. */
void gdt_init(void);

#endif
