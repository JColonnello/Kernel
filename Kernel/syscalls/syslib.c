#include <syslib.h>
#include <stddef.h>
#include <console.h>
#include <pid.h>
#include <lib.h>
#include "loader.h"
#include "naiveConsole.h"
#include "stdio.h"
#include <disk.h>

#define MAX_FD 128
typedef int (Syscall)(void);

int read(int fd, void *buf, size_t count)
{
    ProcessDescriptor *pd = currentProcess();

    if(fd < pd->fdtSize)
    {
        FileDescriptor desc= pd->fd[fd];
        if(desc.isOpen && desc.read != NULL)
            return desc.read(desc.data, buf, count);
    }
    return -1;
}

int write(int fd, const void *buf, size_t count)
{
    ProcessDescriptor *pd = currentProcess();

    if(fd < pd->fdtSize)
    {
        FileDescriptor desc= pd->fd[fd];
        if(desc.isOpen && desc.write != NULL)
            return desc.write(desc.data, buf, count);
    }
    return -1;
}

int open(const char *path, int mode)
{
    ProcessDescriptor *pd = currentProcess();
    FileDescriptor desc;

    int i;
    for(i = 0; i < pd->fdtSize && pd->fd[i].isOpen; i++) ;
    if(i == pd->fdtSize)
        return -1;

    int err = openFile(&desc, path, mode);
    if(err != 0)
        return err;

    pd->fd[i] = desc;
    pd->fd[i].isOpen = true;
    return i;
}

int close(int fd)
{
    ProcessDescriptor *pd = currentProcess();

    if(fd < pd->fdtSize)
    {
        FileDescriptor desc= pd->fd[fd];
        if(desc.isOpen && desc.close != NULL)
        {
            int err = desc.close(desc.data);
            pd->fd[fd].isOpen = false;
            return err;
        }
    }
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

extern void _execve();
extern void _switchPML4(uintptr_t pml4);

int execve(const char *pathname, char *const argv[], char *const envp[])
{
    ProcessDescriptor *pdnew, *curr = currentProcess();

    size_t pathlen = 0;
    while(pathname[pathlen] != 0)
        pathlen++;
    if(pathlen > 255)
        return -1;

    int pid = createProcess(&pdnew);
    if(pid < 0)
        return -1;
    
    int fd = open(pathname, O_RDONLY);
    if(fd < 0)
        return -1;
        
    size_t argLen = 2;
    char defkargs[2] = {0};
    char *kargs = defkargs;
    if(argv != NULL)
    {
        argLen = 1;
        char *const * tmpargv = argv;
        while(*tmpargv != NULL)
        {
            char *str = *tmpargv;
            do
                argLen++;
            while(*str++ != 0);
            tmpargv++;
        }
        //double 0 at the end
        argLen++;
        kargs = kmalloc(argLen);
        char *ptr = kargs+1;
        tmpargv = argv;
        while(*tmpargv != NULL)
        {
            char *str = *tmpargv;
            do
                *ptr++ = *str;
            while(*str++ != 0);
            tmpargv++;
        }
        kargs[0] = 0;
        kargs[argLen-1] = 0;
    }
    
    _switchPML4(pdnew->pml4);
    
    size_t size = 0;
    void *exePos = (void*)0x400000;
    int rd;
    void *pgrmBreak = (void*)exePos;
    do 
    {
        int pageStep = 4;
        size_t step = pageStep * PAGE_SIZE;
        void *buf = (exePos + size);
        while(buf + step > pgrmBreak)
        {
            kmap(&pgrmBreak, NULL, NULL, pageStep);
            pgrmBreak += pageStep * PAGE_SIZE;
        }
        rd = read(fd, buf, step);
        size += rd;
    } while (rd > 0);
    close(fd);
    pdnew->binaryEnd = (uintptr_t)exePos + size;
    pdnew->prgmBreak = (uintptr_t)pgrmBreak;

    size_t stackPages = 32;
    uintptr_t stackEnd = (uintptr_t)exePos;
    void *stackStart = (void*)(stackEnd - stackPages * PAGE_SIZE);
    kmap(&stackStart, NULL, NULL, stackPages);

    //Prepare context, store return address
    stackEnd -= sizeof(void*);
    *(void**)(stackEnd) = exePos;
    pdnew->stack = stackEnd;

    char *argLoc = (void*)0x100000;
    kmap((void**)&argLoc, NULL, NULL, argLen / 0x1000 + 1);
    memcpy(argLoc, kargs, argLen);
    if(argv != NULL)
        kfree(kargs);
    
    _switchPML4(curr->pml4);
    return pid;
}

int exit(int status)
{
    ProcessDescriptor *pd = currentProcess();
    for(int i = 0; i < MAX_FD; i++)
        if(pd->fd[i].isOpen)
            close(i);
    exitProcess();
}

static void wait(int pid)
{
    contextSwitch(pid);
}

static int getpid()
{
    return currentProcess()->pid;
}

struct tm
{
    int tm_sec;   /* 0-60 */
    int tm_min;   /* 0-59 */
    int tm_hour;  /* 0-23 */
    int tm_mday;  /* 1-31 */
    int tm_mon;   /* 0-11 */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* 0-6 */
    int tm_yday;  /* 0-365 */
    int tm_isdst; /* >0 DST, 0 no DST, <0 information unavailable */
};

size_t initFD(FileDescriptor **fdt, int tty)
{
    size_t size = MAX_FD;
    *fdt = kcalloc(size, sizeof(FileDescriptor));
    openStdio(*fdt, tty);
    (*fdt)[0].isOpen = true;
    (*fdt)[1].isOpen = true;
    (*fdt)[2].isOpen = true;

    return size;
}

Syscall *funcTable[] = 
{
    [0] = (Syscall*)read,
    [1] = (Syscall*)write,
    [2] = (Syscall*)open,
    [3] = (Syscall*)close,
    [12] = (Syscall*)brk,
    [39] = (Syscall*)getpid,
    [59] = (Syscall*)_execve,
    [60] = (Syscall*)exit,
    [64] = (Syscall*)wait,
};

size_t funcTableSize = sizeof(funcTable) / sizeof(*funcTable);