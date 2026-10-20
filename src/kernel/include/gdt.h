#ifndef JAZZ_GDT_H
#define JAZZ_GDT_H

/* Install the kernel GDT with maskable interrupts already disabled. */
void gdt_init(void);

#endif
