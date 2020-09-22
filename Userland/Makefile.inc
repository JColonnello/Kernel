export GCC=gcc
GPP=g++
LD=ld
AR=ar
ASM=nasm

INC_DIRS := $(CURDIR)/../libc/include
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
export LIB_DIRS := $(CURDIR)/../libc
LIB_FLAGS = $(addprefix -L,$(LIB_DIRS)) 
export GCCFLAGS := $(INC_FLAGS) $(LIB_FLAGS) -mno-red-zone -Wall -ffreestanding -std=c99 #-fexec-charset=cp437
ARFLAGS=rvs
ASMFLAGS=-felf64
LDFLAGS=--warn-common -z max-page-size=0x1000
