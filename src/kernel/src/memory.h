#ifndef JAZZ_MEMORY_H
#define JAZZ_MEMORY_H

#include <stddef.h>

/* Fill count bytes with the unsigned-char value of value. */
void *memset(void *destination, int value, size_t count);

/* Copy count bytes between non-overlapping regions. */
void *memcpy(void *destination, const void *source, size_t count);

/* Copy count bytes; source and destination may overlap. */
void *memmove(void *destination, const void *source, size_t count);

#endif
