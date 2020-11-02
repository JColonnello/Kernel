include ../Userland/Makefile.inc

BUILD_DIR := build
LIB=libc.a
SOURCES=$(shell find src -name "*.c")
SOURCES_ASM=$(shell find src -name "*.s")
OBJECTS=$(SOURCES:%.c=$(BUILD_DIR)/%.c.o)
OBJECTS_ASM=$(SOURCES_ASM:%.s=$(BUILD_DIR)/%.s.o)
INC_DIRS = ./include ./include/pdclib ../Common/include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
DEFS = _PDCLIB_BUILD _PDCLIB_STATIC_DEFINE LACKS_SYS_TYPES_H \
		LACKS_SYS_MMAN_H LACKS_FCNTL_H LACKS_UNISTD_H USE_LOCKS=0 \
		EXEC_PAGESIZE=4096 LACKS_SYS_PARAM_H __STDC_NO_THREADS__ HAVE_MMAP=0 \
		USE_DEV_RANDOM=0
override CFLAGS += $(addprefix -D,$(DEFS)) -mcmodel=large

all: $(LIB) crt0.o

crt0.o: crt0.c
	$(GCC) -g $(CFLAGS) $(INC_FLAGS) -c $< -o $@

$(LIB): $(OBJECTS) $(OBJECTS_ASM)
	$(AR) rcs -o $@ $^

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(GCC) -g $(CFLAGS) $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(MKDIR_P) $(dir $@)
	$(ASM) -g $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(LIB) crt0.o

.PHONY: all clean

MKDIR_P ?= mkdir -p