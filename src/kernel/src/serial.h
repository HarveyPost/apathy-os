#ifndef JAZZ_SERIAL_H
#define JAZZ_SERIAL_H

#include <stdbool.h>

/* Configure and test the COM1 UART for polling output. */
bool serial_init(void);

/* Write a NUL-terminated string using explicit CRLF line endings.
* Failure can occur after part of the string was sent. */
bool serial_write(const char *text);

#endif
