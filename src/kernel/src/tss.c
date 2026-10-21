#include "tss.h"
#include <stddef.h>

_Static_assert(sizeof(TSS) == 104, "Unexpected TSS size");
_Static_assert(offsetof(TSS, rsp) == 4, "Unexpected TSS RSP offset");
_Static_assert(offsetof(TSS, ist) == 36, "Unexpected TSS IST offset");
_Static_assert(offsetof(TSS, io_map_base) == 102, "Unexpected TSS I/O map offset");

static TSS tss __attribute__((aligned(16)));

const TSS *tss_init(void) {
    tss = (TSS) {
        .io_map_base = sizeof(TSS), /* Beyond the limit: no I/O bitmap. */
    };

    return &tss;
}
