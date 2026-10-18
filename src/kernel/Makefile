CC := x86_64-elf-gcc

CFLAGS := -I../shared -std=c11 -O2 -g -Wall -Wextra \
          -ffreestanding -mno-red-zone -mgeneral-regs-only \
          -fno-pie -fno-stack-protector \
          -fno-asynchronous-unwind-tables -fno-unwind-tables

LDFLAGS := -nostdlib -static -no-pie -Wl,-T,linker.ld \
           -Wl,--build-id=none -Wl,-z,max-page-size=0x1000

.PHONY: all clean
.DELETE_ON_ERROR:

all: kernel.elf

src/main.o: src/main.c ../shared/bootinfo.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: src/main.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) src/main.o -o $@ -lgcc

clean:
	rm -f src/main.o kernel.elf
