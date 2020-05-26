include Makefile.inc

BUILD_DIR := build
KERNEL=kernel.bin
SOURCES=$(shell find . -name "*.c")
SOURCES_ASM=$(shell find . -name "*.s")
OBJECTS=$(SOURCES:%.c=$(BUILD_DIR)/%.c.o)
OBJECTS_ASM=$(SOURCES_ASM:%.s=$(BUILD_DIR)/%.s.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:%.asm=$(BUILD_DIR)/%.o)
STATICLIBS=

all: $(BUILD_DIR)/$(KERNEL)

$(BUILD_DIR)/$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $@ $^

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT): $(LOADERSRC)
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

MKDIR_P ?= mkdir -p