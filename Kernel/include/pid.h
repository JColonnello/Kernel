#pragma once
#include <stdint.h>
#include <stddef.h>
#include <syslib.h>

#define MAX_PID 64

typedef struct
{
    uintptr_t pml4;
    unsigned pid;
    int tty;
    uintptr_t binaryEnd;
    uintptr_t prgmBreak;
    FileDescriptor *fd;
    size_t fdtSize;
} ProcessDescriptor;

ProcessDescriptor *currentProcess();
int createProcess(ProcessDescriptor *pd);
