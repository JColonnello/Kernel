GCC=gcc
LD=ld
AR=ar
ASM=nasm

INC_DIRS := $(CURDIR)/../../libc/include $(CURDIR)/../../Common/include
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
override CFLAGS+= $(INC_FLAGS) -m64 -mno-red-zone -Wall -ffreestanding -std=c99 # -fexec-charset=cp437
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
	$(GCC) $(CFLAGS) -g $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(MKDIR_P) $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf build

.PHONY: sampleCodeModule all clean list
MKDIR_P ?= mkdir -p
