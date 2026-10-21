#ifndef APATHY_PANIC_H
#define APATHY_PANIC_H

#include <stdint.h>

/* Best-effort serial diagnostics, then halt permanently. */
_Noreturn void panic_at(
    const char *message,
    const char *file,
    uint32_t line
);

/* Macros supplies filename and linenumber. */
#define PANIC(message) panic_at((message), __FILE__, __LINE__)

#endif
