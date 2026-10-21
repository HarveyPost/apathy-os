#ifndef APATHY_MEMORY_H
#define APATHY_MEMORY_H

#include <stddef.h>

/* Fill count bytes with the unsigned-char value of value. */
void *memset(void *destination, int value, size_t count);

/* Copy count bytes between non-overlapping regions. */
void *memcpy(void *destination, const void *source, size_t count);

/* Copy count bytes; source and destination may overlap. */
void *memmove(void *destination, const void *source, size_t count);

/* Compare count bytes; return negative, zero, or positive. */
int memcmp(const void *left, const void *right, size_t count);

#endif
