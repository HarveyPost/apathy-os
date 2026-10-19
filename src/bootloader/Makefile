ARCH = $(shell uname -m | sed s,i[3456789]86,ia32,)

OBJS = src/main.o src/bootfs.o src/elf64.o src/kernelload.o src/bootserial.o src/bootmem.o src/bootstack.o src/bootparams.o src/bootjump.o
TARGET = hello.efi

EFIINC = /usr/include/efi
EFIINCS = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIB = /usr/lib
EFILIB = /usr/lib
EFI_CRT_OBJS = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS = $(EFILIB)/elf_$(ARCH)_efi.lds

CFLAGS = -I../shared $(EFIINCS) -fno-stack-protector -fpic \
		 -fshort-wchar -mno-red-zone -Wall

ifeq ($(ARCH),x86_64)
	CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS)

all: $(TARGET)

src/main.o src/bootfs.o: src/bootfs.h
src/elf64.o src/kernelload.o: src/elf64.h
src/main.o src/kernelload.o: src/kernelload.h
src/main.o src/bootserial.o: src/bootserial.h
src/main.o src/bootmem.o: src/bootmem.h
src/main.o: ../shared/bootinfo.h
src/main.o src/bootstack.o: src/bootstack.h
src/main.o src/bootparams.o: src/bootparams.h ../shared/bootinfo.h
src/bootjump.o: src/bootjump.h ../shared/bootinfo.h

hello.so: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic \
			-j .dynsym -j .rel -j .rela -j .reloc \
			--output-target=efi-app-$(ARCH) --subsystem=10 $< $@
