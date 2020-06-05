#include <syslib.h>
#include <stddef.h>
#include <console.h>
#include <pid.h>
#include <lib.h>
#include "naiveConsole.h"
#include "stdio.h"

#define MAX_FD 64
typedef int (Syscall)(void);

int read(int fd, void *buf, size_t count)
{
    ProcessDescriptor pd = currentProcess();
    FileDescriptor desc;

    if(fd < pd.fdtSize && (desc = pd.fd[fd]).read != NULL)
    {
        return desc.read(desc.data, buf, count);
    }
    else
        return -1;
}

int write(int fd, const void *buf, size_t count)
{
    ProcessDescriptor pd = currentProcess();
    FileDescriptor desc;
    if(fd < pd.fdtSize && (desc = pd.fd[fd]).write != NULL)
    {
        return desc.write(desc.data, buf, count);
    }
    else
        return -1;
}

size_t initFD(FileDescriptor **fdt)
{
    size_t size = MAX_FD;
    *fdt = kcalloc(size, sizeof(FileDescriptor));
    openStdio(*fdt);

    return size;
}

Syscall *funcTable[] = 
{
    [0] = (Syscall*)read,
    [1] = (Syscall*)write,
};