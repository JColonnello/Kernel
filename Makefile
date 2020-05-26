SUBDIRS := $(wildcard */.)
MAKE := make --no-print-directory

all:  bootloader kernel userland image

rebuild: clean all

bootloader:
	$(MAKE) -C Bootloader

kernel:
	$(MAKE) -C Kernel

userland:
	$(MAKE) -C Userland

image: kernel bootloader userland
	$(MAKE) -C Image

clean:
	$(MAKE) -C Bootloader clean
	$(MAKE) -C Kernel clean
	$(MAKE) -C Userland clean
	$(MAKE) -C Image clean

.PHONY: bootloader image collections kernel userland all clean
