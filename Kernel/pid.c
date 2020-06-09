#include "lib.h"
#include "naiveConsole.h"
#include <syslib.h>
#include <pid.h>
#include <stdbool.h>
#include <console.h>

ProcessDescriptor descriptors[MAX_PID] = {0};
bool inUse[MAX_PID] = {[0] = true};
unsigned byViews[MAX_VIEWS];

int currentPID = 0;

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
    pd->pid = i;
    pd->tty = curr->tty;
    pd->pml4 = createPML4();
    pd->fdtSize = initFD(&pd->fd, pd->tty);
    pd->parent = curr;

    inUse[i] = true;
    *out = pd;
    return pd->pid;
}

int getByView(int view)
{
    return (view < MAX_VIEWS) ? byViews[view] : -1;
}

uintptr_t getKernelStack()
{
    return descriptors[0].stack;
}

extern void _switch(uintptr_t pml4, uintptr_t newStack, uintptr_t *stackSave);

void contextSwitch(int pid)
{
    if(pid >= MAX_PID && pid > 0)
        return;
    
    ProcessDescriptor *curr = currentProcess(), *next = &descriptors[pid];
    if(next->parent != curr)
        return;

    byViews[next->tty] = next->pid;
    currentPID = next->pid;
    _switch(next->pml4, next->stack, &curr->stack);
}

extern void _abandon(uintptr_t pml4, uintptr_t newStack);

void exitProcess()
{
    ProcessDescriptor *pd = currentProcess(), *parent = pd->parent;
    //Drop PML4
    //kfree(pd->fd);
    inUse[pd->pid] = false;
    if(parent == NULL)
    {
        ncPrint("Kernel exit. Halting\n");
        _halt();
    }
    else
    {
        byViews[parent->tty] = parent->pid;
        currentPID = parent->pid;
        _abandon(parent->pml4, parent->stack);
    }
}