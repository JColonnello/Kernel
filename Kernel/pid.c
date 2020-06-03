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

    pd->pid = i;
    descriptors[i] = *pd;
    return i;
}