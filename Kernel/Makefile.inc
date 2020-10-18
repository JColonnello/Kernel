GCC=gcc

LD=gcc
AR=ar
ASM=nasm

# GCCFLAGS=-m64 -fno-exceptions -fno-asynchronous-unwind-tables -mno-mmx -mno-sse -mno-sse2 -fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -mno-red-zone -Wall -ffreestanding -nostdlib -fno-common -std=c99 -fno-pic -mcmodel=kernel 
GCCFLAGS=-m64 -mno-red-zone -Wall -ffreestanding -nostdlib -std=c11 -fno-pic -mcmodel=kernel -O3 -march=nehalem -flto #-fexec-charset=cp437
ARFLAGS=rvs
ASMFLAGS=-felf64
LDFLAGS= -static -Wl,--build-id=none
