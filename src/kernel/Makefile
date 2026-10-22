CC := x86_64-elf-gcc

ARCH_DIR := arch/x86_64
LINKER_SCRIPT := $(ARCH_DIR)/linker.ld

OBJS := src/main.o src/serial.o src/panic.o src/memory.o src/memmap.o \
		$(ARCH_DIR)/src/gdt.o $(ARCH_DIR)/src/idt.o $(ARCH_DIR)/src/tss.o
DEPS := $(OBJS:.o=.d)

CFLAGS := -Iinclude -I$(ARCH_DIR)/include -I../shared \
		  -std=c11 -O2 -g -Wall -Wextra \
          -ffreestanding -mno-red-zone -mgeneral-regs-only \
          -fno-pie -fno-stack-protector \
          -fno-asynchronous-unwind-tables -fno-unwind-tables

LDFLAGS := -nostdlib -static -no-pie -Wl,-T,$(LINKER_SCRIPT) \
           -Wl,--build-id=none -Wl,-z,max-page-size=0x1000

.PHONY: all clean
.DELETE_ON_ERROR:

all: kernel.elf

src/memory.o: CFLAGS += -fno-tree-loop-distribute-patterns

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

kernel.elf: $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ -lgcc

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) kernel.elf
