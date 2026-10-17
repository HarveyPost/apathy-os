IMAGE := ../fat.img
OVMF := /usr/share/edk2/x64/OVMF.4m.fd

.DELETE_ON_ERROR:
.PHONY: all bootloader kernel image run clean

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
	qemu-system-x86_64 -bios $(OVMF) -drive format=raw,file=$(IMAGE) -net none

clean:
	rm -f $(IMAGE) bootloader/src/main.o bootloader/hello.so bootloader/hello.efi bootloader/src/bootfs.o bootloader/src/elf64.o bootloader/src/kernelload.o
	$(MAKE) -C kernel clean
