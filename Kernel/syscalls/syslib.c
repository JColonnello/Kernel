#include <syslib.h>
#include <stddef.h>
#include <console.h>
#include <pid.h>
#include <lib.h>
#include "loader.h"
#include "naiveConsole.h"
#include "stdio.h"
#include <disk.h>

#define MAX_FD 64
typedef int (Syscall)(void);

int read(int fd, void *buf, size_t count)
{
    ProcessDescriptor *pd = currentProcess();
    FileDescriptor desc;

    if(fd < pd->fdtSize && (desc = pd->fd[fd]).read != NULL)
    {
        return desc.read(desc.data, buf, count);
    }
    else
        return -1;
}

int write(int fd, const void *buf, size_t count)
{
    ProcessDescriptor *pd = currentProcess();
    FileDescriptor desc;
    if(fd < pd->fdtSize && (desc = pd->fd[fd]).write != NULL)
    {
        return desc.write(desc.data, buf, count);
    }
    else
        return -1;
}

int open(const char *path, int mode)
{
    ProcessDescriptor *pd = currentProcess();
    FileDescriptor desc;

    int i;
    for(i = 0; i < pd->fdtSize && pd->fd[i].data != NULL; i++) ;
    if(i == pd->fdtSize)
        return -1;
    int err = openFile(&desc, path, mode);
    if(err != 0)
        return err;
    pd->fd[i] = desc;
    return i;
}

int close(int fd)
{
    ProcessDescriptor *pd = currentProcess();
    FileDescriptor desc;
    if(fd < pd->fdtSize && (desc = pd->fd[fd]).close != NULL)
    {
        return desc.close(desc.data);
    }
    else
        return -1;
}

static uintptr_t brk(uintptr_t addr)
{
    ProcessDescriptor *pd = currentProcess();
    if(addr == 0 || addr < pd->binaryEnd || addr >= (uintptr_t)&__startOfUniverse)
        return pd->prgmBreak;
    if(addr == pd->prgmBreak)
        return pd->prgmBreak;
    
    if(addr > pd->prgmBreak)
    {
        size_t pages = addr / PAGE_SIZE - pd->prgmBreak / PAGE_SIZE;
        if(pages > 0)
        {
            void *begin = (void*)(pd->prgmBreak & PAGE_MASK);
            kmap(&begin, NULL, NULL, pages);
        }
        pd->prgmBreak = addr;
        return pd->prgmBreak;
    }
    else
    {
        size_t pages = pd->prgmBreak / PAGE_SIZE - addr / PAGE_SIZE;
        if(pages > 0)
        {
            void *begin = (void*)((addr + PAGE_SIZE) & PAGE_MASK);
            kunmap(begin, pages);
        }
        pd->prgmBreak = addr;
        return pd->prgmBreak;
    }
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
    [2] = (Syscall*)open,
    [3] = (Syscall*)close,
    [12] = (Syscall*)brk
};

size_t funcTableSize = sizeof(funcTable) / sizeof(*funcTable);