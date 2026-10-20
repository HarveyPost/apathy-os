CC := x86_64-elf-gcc

OBJS := src/main.o src/serial.o src/panic.o src/memory.o src/gdt.o

CFLAGS := -Iinclude -I../shared -std=c11 -O2 -g -Wall -Wextra \
          -ffreestanding -mno-red-zone -mgeneral-regs-only \
          -fno-pie -fno-stack-protector \
          -fno-asynchronous-unwind-tables -fno-unwind-tables

LDFLAGS := -nostdlib -static -no-pie -Wl,-T,linker.ld \
           -Wl,--build-id=none -Wl,-z,max-page-size=0x1000

.PHONY: all clean
.DELETE_ON_ERROR:

all: kernel.elf

src/main.o: ../shared/bootinfo.h include/serial.h include/panic.h include/memory.h include/gdt.h
src/serial.o: include/serial.h include/io.h
src/panic.o: include/panic.h include/serial.h
src/memory.o: include/memory.h
src/gdt.o: include/gdt.h
src/memory.o: CFLAGS += -fno-tree-loop-distribute-patterns

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ -lgcc

clean:
	rm -f $(OBJS) kernel.elf
