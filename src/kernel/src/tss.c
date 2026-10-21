#include "tss.h"
#include <stddef.h>

_Static_assert(sizeof(TSS) == 104, "Unexpected TSS size");
_Static_assert(offsetof(TSS, rsp) == 4, "Unexpected TSS RSP offset");
_Static_assert(offsetof(TSS, ist) == 36, "Unexpected TSS IST offset");
_Static_assert(offsetof(TSS, io_map_base) == 102, "Unexpected TSS I/O map offset");

#define EXCEPTION_STACK_SIZE (16 * 1024)

static unsigned char double_fault_stack[EXCEPTION_STACK_SIZE]
    __attribute__((aligned(4096)));
static unsigned char nmi_stack[EXCEPTION_STACK_SIZE]
    __attribute__((aligned(4096)));
static unsigned char machine_check_stack[EXCEPTION_STACK_SIZE]
    __attribute__((aligned(4096)));

static TSS tss __attribute__((aligned(16)));

const TSS *tss_init(void) {
    tss = (TSS) {
        .io_map_base = sizeof(TSS), /* Beyond the limit: no I/O bitmap. */
    };

    tss.ist[TSS_IST_DOUBLE_FAULT - 1] =
        (uint64_t)(uintptr_t)(double_fault_stack + sizeof(double_fault_stack));
    tss.ist[TSS_IST_NMI - 1] =
        (uint64_t)(uintptr_t)(nmi_stack + sizeof(nmi_stack));
    tss.ist[TSS_IST_MACHINE_CHECK - 1] =
        (uint64_t)(uintptr_t)(machine_check_stack + sizeof(machine_check_stack));

    return &tss;
}
