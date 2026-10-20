IMAGE := ../fat.img
OVMF := /usr/share/edk2/x64/OVMF.4m.fd

.DELETE_ON_ERROR:
.PHONY: all bootloader kernel image run debug gdb clean

all: image

bootloader:
	$(MAKE) -C bootloader

kernel:
	$(MAKE) -C kernel

# Create the filesystem only when the image doesn't exist yet.
$(IMAGE):
	dd if=/dev/zero of=$(IMAGE) bs=1k count=1440
	mformat -i $(IMAGE) -f 1440 ::
	mmd -i $(IMAGE) ::/EFI
	mmd -i $(IMAGE) ::/EFI/BOOT

# Copy the latest application into the existing image.
image: bootloader kernel $(IMAGE)
	mcopy -o -i $(IMAGE) bootloader/hello.efi ::/EFI/BOOT/BOOTX64.EFI
	mcopy -o -i $(IMAGE) bootloader/files/test.txt ::test.txt
	mcopy -o -i $(IMAGE) kernel/kernel.elf ::kernel.elf

run: image
	qemu-system-x86_64 -bios $(OVMF) -drive format=raw,file=$(IMAGE) -net none -display none -serial stdio -monitor none

debug: image
	qemu-system-x86_64 -bios $(OVMF) -drive format=raw,file=$(IMAGE) -net none -display none -serial stdio -monitor none -S -gdb tcp:127.0.0.1:1234 -no-reboot -no-shutdown

gdb:
	gdb -q kernel/kernel.elf -ex 'target remote 127.0.0.1:1234' -ex 'hbreak kernel_main' -ex 'hbreak panic_at'

clean:
	rm -f $(IMAGE) bootloader/src/main.o bootloader/hello.so bootloader/hello.efi bootloader/src/bootfs.o bootloader/src/elf64.o bootloader/src/kernelload.o bootloader/src/bootserial.o bootloader/src/bootmem.o bootloader/src/bootstack.o bootloader/src/bootparams.o bootloader/src/bootjump.o bootloader/src/bootexit.o
	$(MAKE) -C kernel clean
