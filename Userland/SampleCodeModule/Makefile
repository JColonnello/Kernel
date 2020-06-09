MODULE=../0000-sampleCodeModule.bin
SOURCES=$(wildcard *.c)
OBJS := $(SOURCES:%.c=%.c.o)

all: $(MODULE)

$(MODULE): $(OBJS) $(LIB_DIRS)/libc.a $(LIB_DIRS)/crt0.o
	$(LD) $(LDFLAGS) -T sampleCodeModule.ld -o $@.elf $(OBJS) -l:libc.a
	objcopy -O binary $@.elf $@

%.c.o: %.c
	$(GCC) $(GCCFLAGS) -g -c $^ -o $@

clean:
	rm -rf *.o $(MODULE)*

.PHONY: all clean
