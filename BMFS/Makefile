SOURCES := $(shell find lib/ -name *.c)
OBJS := $(SOURCES:%.c=build/%.o)
INC_DIR := include
LIB := libbmfs.a

$(LIB): $(OBJS)
	$(AR) rcs -o $@ $(OBJS)

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -fno-stack-protector -g -c -o $@ -I$(INC_DIR) $^

clean:
	rm -r build $(LIB)

.PHONY: clean