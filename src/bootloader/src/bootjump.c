#include "bootjump.h"

/* naked prevents GCC from generating its normal function entry and exit
* instructions. That gives this routine control of the stack from its
* first instruction.
*
* Under our System V calling convention, these arguments arrive in:
* entry - RDI
* boot_info - RSI
* stack_top - RDX
*
* The instructions then:
* cli disables maskable interrupts until the kernel prepares its interrupt handling.
* cld clears the direction flag, as C code expects.
* Save the new entry address in RAX.
* Put the new stack top in RSP and align it to 16 bytes.
* Clear the RBP, the frame-pointer register.
* Move boot_info into RDI, making it the kernel's first argument.
* Call the kernel through RAX. 
*
* If the kernel unexpectedly returns, the final loop disables interrupts and halts
* instead of attempting to return through the abandoned firmware stack. */
_Noreturn __attribute__((naked)) void KERNEL_ABI bootjump(
    KERNEL_ENTRY entry,
    const BOOT_INFO *boot_info,
    uint64_t stack_top
) {
    __asm__(
        "cli\n\t"
        "cld\n\t"

        "mov %rdi, %rax\n\t"
        "mov %rdx, %rsp\n\t"
        "and $-16, %rsp\n\t"
        "xor %ebp, %ebp\n\t"
        
        "mov %rsi, %rdi\n\t"
        "call *%rax\n\t"

        "1:\n\t"
        "cli\n\t"
        "hlt\n\t"
        "jmp 1b\n\t"
    );
}
