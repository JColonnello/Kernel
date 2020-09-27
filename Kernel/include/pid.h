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

#include <common/processInfo.h>

ProcessDescriptor *currentProcess();
int createProcess(ProcessDescriptor **out);
uintptr_t getKernelStack();
bool isRunning(int pid);
void exitProcess();
void changeFocus(int tty);
size_t listProcesses(struct ProcessInfo *buffer, size_t size);
void contextSwitch(ProcessDescriptor *next);
