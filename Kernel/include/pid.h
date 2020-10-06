#pragma once
#include <stdint.h>
#include <stddef.h>
#include <syslib.h>

#define MAX_PID 64

typedef struct ProcessDescriptor ProcessDescriptor;

typedef enum
{
    PROCESS_NONE,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_PENDING_BLOCK,
} ProcessState;

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

#include <common/processInfo.h>

ProcessDescriptor *currentProcess();
int createProcess(ProcessDescriptor **out);
uintptr_t getKernelStack();
bool isRunning(int pid);
void exitProcess();
size_t listProcesses(struct ProcessInfo *buffer, size_t size);
void contextSwitch(ProcessDescriptor *next);
void dropProcess(int pid);
enum PdJobStatus setJobStatus(int pid, enum PdJobStatus status);
void setCurrentState(ProcessState state);
void setProcessState(int pid, ProcessState state);
unsigned getCurrentPid();
void checkProcessSignals();
ProcessState getProcessState(int pid);
