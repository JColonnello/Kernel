GCC=gcc

LD=ld
AR=ar
ASM=nasm

# GCCFLAGS=-m64 -fno-exceptions -fno-asynchronous-unwind-tables -mno-mmx -mno-sse -mno-sse2 -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -mno-red-zone -Wall -ffreestanding -nostdlib -fno-common -std=c99 -fno-pic -mcmodel=kernel 
GCCFLAGS=-m64 -mno-red-zone -Wall -ffreestanding -std=c99 -fno-pic -mcmodel=kernel
ARFLAGS=rvs
ASMFLAGS=-felf64
LDFLAGS=--warn-common -z max-page-size=0x1000
