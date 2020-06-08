export GCC=$(HOME)/Documents/Arqui/toolchain/opt/cross/bin/amd64-elf-gcc
GPP=g++
LD=ld
AR=ar
ASM=nasm

INC_DIRS := $(CURDIR)/../libc/include
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
export LIB_DIRS := $(CURDIR)/../libc
LIB_FLAGS = $(addprefix -L,$(LIB_DIRS)) 
export GCCFLAGS := $(INC_FLAGS) $(LIB_FLAGS) -m64 -fno-exceptions -std=c99 -Wall -ffreestanding -nostdlib -mno-red-zone
ARFLAGS=rvs
ASMFLAGS=-felf64
LDFLAGS=--warn-common -z max-page-size=0x1000
