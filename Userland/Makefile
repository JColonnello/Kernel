GCC=$(HOME)/Documents/Arqui/toolchain/opt/cross/bin/amd64-elf-gcc
LD=ld
AR=ar
ASM=nasm

INC_DIRS := $(CURDIR)/../../libc/include
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
GCCFLAGS= $(INC_FLAGS) -m64 -fno-exceptions -fno-asynchronous-unwind-tables -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -mno-red-zone -Wall -ffreestanding -nostdlib -fno-common -std=c99
ARFLAGS=rvs
ASMFLAGS=-felf64
LDFLAGS=--warn-common -z max-page-size=0x1000

MODULES := $(shell find * -maxdepth 0 -type d -not -name build)
DIR_NAME := $(shell basename $(CURDIR))
BUILD_DIR := ../build/$(DIR_NAME)
BINARIES := $(MODULES:%=%.bin)
SOURCES := $(shell find * -type f -name *.c)
SOURCES_ASM := $(shell find * -type f -name *.s)
OBJS := $(SOURCES:%=$(BUILD_DIR)/%.o) $(SOURCES_ASM:%=$(BUILD_DIR)/%.o) 

all: $(BINARIES) list

list:
	echo $(BINARIES) | tr ' ' '\n' | sed 's/.bin//g' > build/list.txt

binary: $(BUILD_DIR).bin

$(BUILD_DIR).bin: $(OBJS)
	$(LD) $(LDFLAGS) -T ../link.ld -o $@.elf $(OBJS) -l:libc.a
	objcopy -O binary $@.elf $@

%.bin:
	$(MAKE) -C $* -f ../Makefile binary

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(GCC) $(GCCFLAGS) -g $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: sampleCodeModule all clean list
MKDIR_P ?= mkdir -p
