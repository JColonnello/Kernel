BOOTLOADER_PATH=../Bootloader
BMFS=$(BOOTLOADER_PATH)/BMFS/bmfs.bin
BMFS_NEW=$(BOOTLOADER_PATH)/BMFS/bmfs
MBR=$(BOOTLOADER_PATH)/Pure64/bmfs_mbr.sys
MP=../Toolchain/ModulePacker/mp.bin
PURE64=$(BOOTLOADER_PATH)/Pure64/pure64.sys
OSIMAGENAME=x64BareBonesImage
VMDK=$(OSIMAGENAME).vmdk
QCOW2=$(OSIMAGENAME).qcow2
IMG=$(OSIMAGENAME).img
KERNEL=../Kernel/build/kernel.bin
USERLAND= ../Userland/build
USERLAND_BIN = $(shell find $(USERLAND) -name *.bin) $(USERLAND)/list.txt

IMGSIZE=67108864

all: $(IMG) $(USERLAND_BIN) $(VMDK) $(QCOW2)

$(KERNEL):
	cd ../Kernel; make

$(IMG): $(BMFS) $(MBR) $(PURE64) $(KERNEL)
	$(BMFS) $(IMG) initialize $(IMGSIZE) $(MBR) $(PURE64) $(KERNEL) 
	$(BMFS_NEW) -d $(IMG) format -s $(IMGSIZE)
	$(BMFS_NEW) -d $(IMG) mkdir userland
	@touch .image

$(USERLAND_BIN): $(USERLAND)/%: .image
	@echo Copying $@...
	$(BMFS_NEW) -d $(IMG) cp $@ userland/$*
	@touch $@

$(VMDK): $(IMG)
	qemu-img convert -f raw -O vmdk $(IMG) $(VMDK) 

$(QCOW2): $(IMG)
	qemu-img convert -f raw -O qcow2 $(IMG) $(QCOW2)

clean:
	rm -rf $(IMG) $(VMDK) $(QCOW2) *.bin

.PHONY: all clean
