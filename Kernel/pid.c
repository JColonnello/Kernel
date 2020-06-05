#include "lib.h"
#include <syslib.h>
#include <pid.h>
#include <stdbool.h>

ProcessDescriptor descriptors[MAX_PID];
bool inUse[MAX_PID];
int currentPID;

ProcessDescriptor currentProcess()
{
    return descriptors[currentPID];
}

int createProcess(ProcessDescriptor *pd)
{
    int i;
    for(i = 0; i < MAX_PID; i++)
        if(!inUse[i])
            break;
    if(i == MAX_PID)
        return -1;

    pd->pid = i;
    if(pd->fd == NULL)
        pd->fdtSize = initFD(&pd->fd);
    
    descriptors[i] = *pd;
    return i;
}

void exitProcess(ProcessDescriptor *pd)
{
    kfree(pd->fd);
}