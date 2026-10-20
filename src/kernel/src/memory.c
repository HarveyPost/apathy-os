#include "memory.h"

#include <stdint.h>

void *memset(void *destination, int value, size_t count) {
    unsigned char *bytes = destination;

    for (size_t i = 0; i < count; i++) {
        bytes[i] = (unsigned char) value;
    }
    
    return destination;
}

void *memcpy(void *destination, const void *source, size_t count) {
    unsigned char *output = destination;
    const unsigned char *input = source;

    for (size_t i = 0; i < count; i++) {
        output[i] = input[i];
    }
    
    return destination;
}

void *memmove(void *destination, const void *source, size_t count) {
    unsigned char *output = destination;
    const unsigned char *input = source;

    if (count == 0 || destination == source)
        return destination;

    if ((uintptr_t)output < (uintptr_t)input) {
        for (size_t i = 0; i < count; i++) {
            output[i] = input[i];
        }
    } else {
        for (size_t i = count; i > 0; i--) {
            output[i - 1] = input[i - 1];
        }
    }

    return destination;
}

int memcmp(const void *left, const void *right, size_t count) {
    const unsigned char *a = left;
    const unsigned char *b = right;

    for (size_t i = 0; i < count; i++) {
        if (a[i] != b[i])
            return (int)a[i] - (int)b[i];
    }

    return 0;
}
