BOOTLOADER_PATH=../Bootloader
BMFS=$(BOOTLOADER_PATH)/BMFS/bmfs.bin
BMFS_NEW=$(BOOTLOADER_PATH)/BMFS/bmfs
MBR=$(BOOTLOADER_PATH)/Pure64/bmfs_mbr.sys
PURE64=$(BOOTLOADER_PATH)/Pure64/pure64.sys
OSIMAGENAME=x64BareBonesImage
VMDK=$(OSIMAGENAME).vmdk
QCOW2=$(OSIMAGENAME).qcow2
IMG=$(OSIMAGENAME).img
KERNEL=../Kernel/build/kernel.bin
USERLAND= ../Userland/build
HELP= ../Docs
LAYOUTS= ../Layouts
USERLAND_BIN = $(shell find $(USERLAND) -name "*.bin") $(USERLAND)/list.txt
LAYOUTS_FILES = $(shell find $(LAYOUTS)/* -printf "\"%f\" ")
HELP_FILES = $(shell find $(HELP)/* -name "*.txt" -printf "\"%f\" ")

IMGSIZE=67108864

all: $(IMG) $(USERLAND_BIN) $(VMDK) $(QCOW2)

$(KERNEL):
	cd ../Kernel; make

$(IMG): $(BMFS) $(MBR) $(PURE64) $(KERNEL)
	$(BMFS) $(IMG) initialize $(IMGSIZE) $(MBR) $(PURE64) $(KERNEL) 
	$(BMFS_NEW) -d $(IMG) format -s $(IMGSIZE)
	$(BMFS_NEW) -d $(IMG) mkdir userland
	$(BMFS_NEW) -d $(IMG) mkdir layouts
	$(BMFS_NEW) -d $(IMG) mkdir help
	for f in $(LAYOUTS_FILES); do \
	$(BMFS_NEW) -d $(IMG) cp "$(LAYOUTS)/$$f" "layouts/$$f"; \
	done
	for f in $(HELP_FILES); do \
	$(BMFS_NEW) -d $(IMG) cp "$(HELP)/$$f" "help/$$f"; \
	done
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
