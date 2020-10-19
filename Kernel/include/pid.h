#pragma once
#include <stdint.h>
#include <stddef.h>
#include <syslib.h>
#include <common/processInfo.h>

#define MAX_PID 64
#define KERNEL_PID 1
#define INACTIVE_PID 0

typedef struct ProcessDescriptor ProcessDescriptor;
typedef enum ProcessState ProcessState;

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
    char name[64];
    bool exitMark;
    bool foreground;
    ProcessState state;
    ProcessDescriptor *parent;
};

void initProcesses();
ProcessDescriptor *currentProcess();
int createProcess(ProcessDescriptor **out);
uintptr_t getKernelStack();
bool isRunning(int pid);
void exit();
size_t listProcesses(struct ProcessInfo *buffer, size_t size);
void contextSwitch(ProcessDescriptor *next);
void goInactive();
void dropProcess(int pid);
enum PdJobStatus setJobStatus(int pid, enum PdJobStatus status);
void setCurrentState(ProcessState state);
void setProcessState(int pid, ProcessState state);
unsigned getCurrentPid();
void checkProcessSignals();
ProcessState getProcessState(int pid);
