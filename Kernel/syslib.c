#include "syslib.h"
#include <stddef.h>
#include <console.h>
#include <pid.h>

typedef int (Syscall)(void);

int read(int fd, void *buf, size_t count)
{
    ProcessDescriptor pd = currentProcess();
    if(fd == 0)
    {
        return inputBufferRead(pd.tty, buf, count);
    }
    return 0;
}

int write(int fd, const void *buf, size_t count)
{
    ProcessDescriptor pd = currentProcess();
    if(fd == 1 || fd == 2)
        return viewWrite(pd.tty, buf, count);
    return 0;
}

Syscall *funcTable[] = 
{
    [0] = (Syscall*)read,
    [1] = (Syscall*)write,
};