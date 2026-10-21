#ifndef APATHY_IDT_H
#define APATHY_IDT_H

/* Install fatal handlers; maskable interrupts must remain disabled. */
void idt_init(void);

#endif
