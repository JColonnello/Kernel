GCC=gcc

LD=gcc
AR=ar
ASM=nasm

# CFLAGS=-m64 -fno-exceptions -fno-asynchronous-unwind-tables -mno-mmx -mno-sse -mno-sse2 -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -mno-red-zone -Wall -ffreestanding -nostdlib -fno-common -std=c99 -fno-pic -mcmodel=kernel 
override CFLAGS+=-m64 -mno-red-zone -Wall -ffreestanding -nostdlib -std=c11 -fno-pic -mcmodel=kernel -O3 -march=nehalem -flto #-fexec-charset=cp437
override ARFLAGS+=rvs
override ASMFLAGS+=-felf64
override LDFLAGS+= -static -Wl,--build-id=none
