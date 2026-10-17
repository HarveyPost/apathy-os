Jazz is a personal project to learn how operating systems work by building one from scratch. The aim is a terminal based x86-64 operating system written in C, with a custom UEFI bootloader and kernel.

The project covers how a computer boots, manages memory, handles hardware, runs programs and stores files. The intended interface is a command line shell with text input and output.

Development takes place on Arch Linux, using QEMU and OVMF to run the system. The focus is on understanding the code and keeping the design small and manageable.


# How the bootloader begins running

1. QEMU starts the virtual cmputer
2. OVMF runs - this is the virtual computer's UEFI firmware
3. The firmware loads your BOOTX64.EFI application
4. GNU-EFI startup code prepares the application
5. The efi_main() function runs

The bootloader starts life as a UEFI application. This lets it use firmware services to display text, read files and allocate memory.

The first C function is in main.c (src/bootloader/src/main.c:14):
EFI_Status efi_main(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
)

There are 3 things to understand here:
- EFI_STATUS is the function's return type. It reports whether the application succeed or encountered an error.
- ImageHandle identifies your currently running application. Treat it as an opaque identifier: you pass it to firmware services to ask about your application. Later, the code uses it to find the device from which the bootloader was loaded.
- SystemTable points to a structure provided by the firmware. That structure gives the application access to firmware facilities, including console input, console output and boot services.
