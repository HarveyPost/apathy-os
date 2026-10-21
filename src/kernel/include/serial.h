#ifndef APATHY_SERIAL_H
#define APATHY_SERIAL_H

#include <stdbool.h>
#include <stdint.h>

/* Configure and test the COM1 UART for polling output. */
bool serial_init(void);

/* Write a NUL-terminated string using explicit CRLF line endings.
* Failure can occur after part of the string was sent. */
bool serial_write(const char *text);

/* Write 0x followed by exactly sixteen hexadecimal digits. */
bool serial_write_hex64(uint64_t value);

#endif
