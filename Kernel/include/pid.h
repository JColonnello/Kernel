#pragma once
#include <stdint.h>
#include <stddef.h>
#include <syslib.h>

#define MAX_PID 64

typedef struct ProcessDescriptor ProcessDescriptor;

struct ProcessDescriptor
{
    uintptr_t pml4;
    unsigned pid;
    int tty;
    uintptr_t binaryEnd;
    uintptr_t prgmBreak;
    uintptr_t stack;
    FileDescriptor *fd;
    size_t fdtSize;
    ProcessDescriptor *parent;
};

ProcessDescriptor *currentProcess();
int createProcess(ProcessDescriptor **out);
int getByView(int view);
uintptr_t getKernelStack();
void contextSwitch(int pid);
bool isRunning(int pid);
void exitProcess();
