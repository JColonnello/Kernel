SUBDIRS := $(wildcard */.)
MAKE := make --no-print-directory

all: bmfs bootloader libc kernel userland image

rebuild: clean all

bmfs:
	$(MAKE) -C BMFS

bootloader:
	$(MAKE) -C Bootloader

kernel:
	$(MAKE) -C Kernel

libc:
	$(MAKE) -C libc

userland:
	$(MAKE) -C Userland

image: kernel bootloader userland
	$(MAKE) -C Image

clean:
	$(MAKE) -C BMFS clean
	$(MAKE) -C Bootloader clean
	$(MAKE) -C Kernel clean
	$(MAKE) -C libc clean
	$(MAKE) -C Userland clean
	$(MAKE) -C Image clean

.PHONY: bootloader image collections kernel userland libc bmfs all clean
