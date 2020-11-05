SOURCES := $(shell find lib/ -name "*.c")
AR=gcc-ar
RANLIB=gcc-ranlib
LD=gcc
OBJS := $(SOURCES:%.c=build/%.o)
override CFLAGS += -m64 -mno-red-zone -ffreestanding -std=c99 -mcmodel=large -O3 -flto
INC_DIR := include
LIB := libbmfs.a

$(LIB): $(OBJS)
	$(AR) rcs -o $@ $(OBJS)

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g -c -o $@ -I$(INC_DIR) $^

clean:
	rm -rf build $(LIB)

.PHONY: clean