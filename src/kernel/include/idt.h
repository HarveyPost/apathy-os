#ifndef JAZZ_IDT_H
#define JAZZ_IDT_H

/* Install fatal handlers; maskable interrupts must remain disabled. */
void idt_init(void);

#endif
