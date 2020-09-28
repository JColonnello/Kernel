#include "lib.h"
#include "naiveConsole.h"
#include <syslib.h>
#include <pid.h>
#include <stdbool.h>
#include <console.h>
#include <common/processInfo.h>
#include <scheduler.h>

static ProcessDescriptor descriptors[MAX_PID] = {0};
static bool inUse[MAX_PID] = {[0] = true};

static int currentPID = 0;

ProcessDescriptor *currentProcess()
{
    return &descriptors[currentPID];
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
    *pd = (ProcessDescriptor) {0};
    pd->pid = i;
    pd->tty = curr->tty;
    pd->pml4 = createPML4();
    pd->fdtSize = initFD(&pd->fd, pd->tty);
    pd->parent = curr;

    inUse[i] = true;
    *out = pd;
    Scheduler_AddProcess(pd);

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

void contextSwitch(ProcessDescriptor *next)
{
    ProcessDescriptor *curr = currentProcess();

    currentPID = next->pid;
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
    descriptors[pid].exitMark = true;
}

void checkProcessSignals()
{
    ProcessDescriptor *pd = currentProcess();
    if(pd->exitMark)
        exitProcess();
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
            .stack = pd->stack
        };
        for (int j = 0; j < sizeof(buffer[count].name) && pd->name[j]; j++)
            buffer[count].name[j] = pd->name[j];
        count++;
        written += sizeof(struct ProcessInfo);
    }
    return written;
}
