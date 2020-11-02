include Makefile.inc

BUILD_DIR := build
KERNEL=kernel.bin
SOURCES=$(shell find . -name "*.c")
SOURCES_ASM=$(shell find . -name "*.s")
OBJECTS=$(SOURCES:%.c=$(BUILD_DIR)/%.c.o)
OBJECTS_ASM=$(SOURCES_ASM:%.s=$(BUILD_DIR)/%.s.o)
LOADERSRC=loader.asm
INC_DIRS = ./include ../BMFS/include ../Common/include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
BMFS_LIB = ../BMFS/libbmfs.a
DEP_FLAGS := -MMD -MP

LOADEROBJECT=$(LOADERSRC:%.asm=$(BUILD_DIR)/%.o)
STATICLIBS=

all: $(BUILD_DIR)/$(KERNEL)

$(BUILD_DIR)/$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(BMFS_LIB)
	$(LD) $(LDFLAGS) $(CFLAGS) -g -T kernel.ld -o $@.elf $^
	objcopy -O binary $@.elf $@

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(GCC) $(CFLAGS) $(DEP_FLAGS) -g $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT): $(LOADERSRC)
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

-include $(OBJECTS:%.o=%.d)

MKDIR_P ?= mkdir -p