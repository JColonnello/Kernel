#pragma once
#include <stdint.h>
#include <stddef.h>
#define MAX_PID 64

typedef struct
{
    uintptr_t pml4;
    unsigned pid;
} ProcessDescriptor;

ProcessDescriptor currentProcess();
int createProcess(ProcessDescriptor *pd);
