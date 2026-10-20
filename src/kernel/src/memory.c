#include "memory.h"

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
