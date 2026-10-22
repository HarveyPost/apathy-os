#ifndef APATHY_MEMMAP_H
#define APATHY_MEMMAP_H

#include "bootinfo.h"
#include <stdbool.h>
#include <stdint.h>

/* Count non-runtime conventional pages. Set *pages to zero on failure. */
bool memmap_count_conventional_pages(
    const BOOT_INFO *boot_info, uint64_t *pages
);

#endif
