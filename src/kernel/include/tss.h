#ifndef APATHY_TSS_H
#define APATHY_TSS_H

#include <stdint.h>

/* Layout comes from x86-64 architecture. */
typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    /* Stack pointers for privilege levels 0, 1 and 2. */
    uint64_t rsp[3];
    uint64_t reserved1;
    /* Seven alternative stack pointers.
     * An IDT entry can select one, allowing an exception handler to run on a dedicated stack. */
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    /* The byte offset, relative to the TSS start, of an optional bitmap controlling port acces. */
    uint16_t io_map_base;
} TSS;

/* Prepare the initial CPU's TSS before loading its task register. */
const TSS *tss_init(void);

#endif
