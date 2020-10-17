#include "lib.h"
#include "naiveConsole.h"
#include <syslib.h>
#include <pid.h>
#include <stdbool.h>
#include <console.h>
#include <common/processInfo.h>
#include <scheduler.h>

static ProcessDescriptor descriptors[MAX_PID] = {0};
static bool inUse[MAX_PID] = {[INACTIVE_PID] = true, [KERNEL_PID] = true};
uintptr_t inacStack[PAGE_SIZE];

static int currentPID = KERNEL_PID;

extern void Scheduler_AddProcess(const ProcessDescriptor *pd);


ProcessDescriptor *currentProcess()
{
    return &descriptors[currentPID];
}

extern void inactiveProcess();
void initProcesses()
{
    ProcessDescriptor *kernel = &descriptors[KERNEL_PID];
    *kernel = (struct ProcessDescriptor)
    {
        .pid = KERNEL_PID,
        .pml4 = 0x2000,
        .tty = 0,
        .foreground = false,
        .state = PROCESS_RUNNING,
        .name = "kernel",
    };
    kernel->fdtSize = initFD(&kernel->fd, kernel->tty);
    ProcessDescriptor *inactive = &descriptors[INACTIVE_PID];
    *inactive = (struct ProcessDescriptor)
    {
        .state = PROCESS_RUNNING,
        .stack = (uintptr_t)inacStack + sizeof(inacStack),
        .pid = INACTIVE_PID,
        .name = "inactive",
    };
    inactive->stack -= sizeof(void*);
    *(void (**)())inactive->stack = inactiveProcess;
}

int createProcess(ProcessDescriptor **out)
{
    int i;
    for(i = 0; i < MAX_PID; i++)
        if(!inUse[i])
            break;
    if(i == MAX_PID)
        return -1;

    ProcessDescriptor *curr = currentProcess(), *pd = &descriptors[i];
    *pd = (ProcessDescriptor) 
    {
        .pml4 = createPML4(),
        .pid = i,
        .tty = curr->tty,
        .parent = curr,
        .foreground = true,
    };
    pd->fdtSize = initFD(&pd->fd, pd->tty),

    inUse[i] = true;
    *out = pd;

    return pd->pid;
}

bool isRunning(int pid)
{
    if(pid >= MAX_PID || pid < 0)
        return false;
    return inUse[pid];
}

uintptr_t getKernelStack()
{
    return (uintptr_t)kmalloc(PAGE_SIZE * 4) + PAGE_SIZE * 4;
}

void freeKernelStack(uintptr_t stack)
{
    kfree((void*)(stack - PAGE_SIZE * 4));
}

extern void _switch(uintptr_t pml4, uintptr_t *newStack, uintptr_t *stackSave);

void goInactive()
{
    if(currentPID == INACTIVE_PID)
        return;
    ProcessDescriptor *curr = currentProcess();
    ProcessDescriptor *next = &descriptors[INACTIVE_PID];
    
    currentPID = INACTIVE_PID;
	outb(0x20, 0x20);
    //No need to switch PML4
    _switch(curr->pml4, &next->stack, &curr->stack);
}

void contextSwitch(ProcessDescriptor *next)
{
    ProcessDescriptor *curr = currentProcess();

    currentPID = next->pid;
	outb(0x20, 0x20);
    _switch(next->pml4, &next->stack, &curr->stack);
}

extern void _dropAndLeave();

void exitProcess()
{
    ProcessDescriptor *pd = currentProcess(), *parent = pd->parent;
    //Drop PML4
    _cli();
    kfree(pd->fd);
    inUse[pd->pid] = false;
    if(parent == NULL)
    {
        ncPrint("Kernel exit. Halting\n");
        _halt();
    }
    else
    {
        _dropAndLeave();
    }
}

void dropProcess(int pid)
{
    if(pid >= MAX_PID || pid < 0)
        return;
    
    ProcessDescriptor *pd = &descriptors[pid];
    pd->exitMark = true;
    if(pd->state == PROCESS_BLOCKED)
        Scheduler_AddProcess(pd);
}

void checkProcessSignals()
{
    ProcessDescriptor *pd = currentProcess();
    if(pd->exitMark)
        exit(0);
}

size_t listProcesses(struct ProcessInfo *buffer, size_t size)
{
    int count = 0;
    size_t written = 0;
    for(int i = 0; i < MAX_PID; i++)
    {
        if(!inUse[i])
            continue;
        if(size < written + sizeof(struct ProcessInfo))
            break;

        ProcessDescriptor *pd = &descriptors[i];
        buffer[count] = (struct ProcessInfo)
        {
            .pid = pd->pid,
            .stack = pd->stack,
            .state = pd->state,
        };
        for (int j = 0; j < sizeof(buffer[count].name) && pd->name[j]; j++)
            buffer[count].name[j] = pd->name[j];
        count++;
        written += sizeof(struct ProcessInfo);
    }
    return written;
}

enum PdJobStatus setJobStatus(int pid, enum PdJobStatus status)
{
    if(pid >= MAX_PID || pid < 0)
        return JOB_NONE;

    ProcessDescriptor *pd = &descriptors[pid];
    switch (status) 
    {
        case JOB_NONE:
            break;
        case JOB_FOREGROUND:
            pd->foreground = true;
            break;
        case JOB_BACKGROUND:
            pd->foreground = false;
            break;
        default:
            return JOB_NONE;
    }
    return pd->foreground ? JOB_FOREGROUND : JOB_BACKGROUND;
}

void setCurrentState(ProcessState state)
{
    setProcessState(currentPID, state);
}

void setProcessState(int pid, ProcessState state)
{
    if(!isRunning(pid))
        return;

    ProcessDescriptor *pd = &descriptors[pid];
    if((pd->state == PROCESS_BLOCKED || pd->state == PROCESS_NONE)
        && state == PROCESS_RUNNING)
    {
        Scheduler_AddProcess(pd);
    }
    pd->state = state;
}

ProcessState getProcessState(int pid)
{
    if(!isRunning(pid))
        return PROCESS_NONE;

    return descriptors[pid].state;
}

unsigned getCurrentPid()
{
    return currentPID;
}