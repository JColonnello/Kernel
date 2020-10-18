#include <syslib.h>
#include <stddef.h>
#include <console.h>
#include <pid.h>
#include <lib.h>
#include "loader.h"
#include "naiveConsole.h"
#include "stdio.h"
#include <disk.h>
#include "time.h"
#include "types.h"
#include <scheduler.h>
#include <common/processInfo.h>

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
        size_t pages = (addr-1) / PAGE_SIZE - (pd->prgmBreak-1) / PAGE_SIZE;
        if(pages > 0)
        {
            void *begin = (void*)((pd->prgmBreak-1 + PAGE_SIZE) & PAGE_MASK);
            kmap(&begin, NULL, NULL, pages);
        }
        pd->prgmBreak = addr;
        return pd->prgmBreak;
    }
    else
    {
        size_t pages = (pd->prgmBreak-1) / PAGE_SIZE - (addr-1) / PAGE_SIZE;
        if(pages > 0)
        {
            void *begin = (void*)((addr-1 + PAGE_SIZE) & PAGE_MASK);
            kunmap(begin, pages);
        }
        pd->prgmBreak = addr;
        return pd->prgmBreak;
    }
}

extern void _switchPML4(uintptr_t pml4);

int _execve(const char *pathname, char *const argv[], char *const envp[])
{
    ProcessDescriptor *pdnew, *curr = currentProcess();

    size_t pathlen = 0;
    while(pathlen <= 255 && pathname[pathlen] != 0)
        pathlen++;
    if(pathlen > 255)
        return -1;

    int fd = open(pathname, O_RDONLY);
    if(fd < 0)
        return -1;
        
    int pid = createProcess(&pdnew);
    if(pid < 0)
        return -1;
    
    {
        int i = 0;
        while(i < pathlen && pathname[i] != '/')
            i++;
        if(i == pathlen)
            i = 0;
        memcpy(pdnew->name, &pathname[i+1], pathlen-i);
    }

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
    setProcessState(pid, PROCESS_RUNNING);
    return pid;
}

void exit(int status)
{
    ProcessDescriptor *pd = currentProcess();
    for(int i = 0; i < MAX_FD; i++)
        if(pd->fd[i].isOpen)
            close(i);
    exitProcess();
}

void yield()
{
    Scheduler_SwitchNext();
}

static int getpid()
{
    return currentProcess()->pid;
}

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

int getcpuinfo(char *id, char *model)
{
    if(isKernelAddress(id) || isKernelAddress(model))
        return 0;

    int tmp[4];
    cpuVendor(tmp, 0);
    memcpy(id, &tmp[1], 4);
    memcpy(id+4, &tmp[3], 4);
    memcpy(id+8, &tmp[2], 4);
    id[12] = 0;

    cpuVendor(model, 0x80000002);
    cpuVendor(model + 16, 0x80000003);
    cpuVendor(model + 32, 0x80000004);

    return cpuVendor(NULL, 1);
}

extern void temp(uint8_t *curr_temp, uint8_t *max_temp);

size_t ps(struct ProcessInfo *buffer, size_t size)
{
    if(isKernelAddress(buffer))
        return 0;

    return listProcesses(buffer, size);
}

extern RegisterStatus lastRegisterStatus;

void dumpregs(RegisterStatus *info)
{
    *info = lastRegisterStatus;
}

static void memuse(size_t *bytesPhysical, size_t *bytesVirtual)
{
    if(isKernelAddress(bytesPhysical) || isKernelAddress(bytesVirtual))
        return;

    *bytesPhysical = getReservedPagesCount() * PAGE_SIZE;
    *bytesVirtual = getReservedMemoryCount();
}

static int kill(int pid)
{
    if(!isRunning(pid))
        return -1;

    dropProcess(pid);
    return 0;
}

static int ispidrun(int pid)
{
    return isRunning(pid);
}

static enum PdJobStatus setjobstatus(int pid, enum PdJobStatus status)
{
    return setJobStatus(pid, status);
}

static int block(int pid)
{
    ProcessState state = getProcessState(pid);
    switch (state) 
    {
        case PROCESS_RUNNING:
            setProcessState(pid, PROCESS_PENDING_BLOCK);
            break;
        case PROCESS_PENDING_BLOCK:
        case PROCESS_BLOCKED:
            setProcessState(pid, PROCESS_RUNNING);
            break;
        default:
            break;
    }
    return state;
}

#include <syncro/semaphore.h>

Syscall *funcTable[] = 
{
    [0] = (Syscall*)read,
    [1] = (Syscall*)write,
    [2] = (Syscall*)open,
    [3] = (Syscall*)close,
    [12] = (Syscall*)brk,
    [39] = (Syscall*)getpid,
    [59] = (Syscall*)execve,
    [60] = (Syscall*)exit,
    [64] = (Syscall*)yield,
    [400] = (Syscall*)temp,
    [401] = (Syscall*)date,
    [402] = (Syscall*)getcpuinfo,
    [403] = (Syscall*)dumpregs,
    [404] = (Syscall*)ps,
    [405] = (Syscall*)memuse,
    [62] = (Syscall*)kill,
    [406] = (Syscall*)ispidrun,
    [407] = (Syscall*)setjobstatus,
    [408] = (Syscall*)block,
    [409] = (Syscall*)sem_wait,
    [410] = (Syscall*)sem_release,
    [411] = (Syscall*)sem_create,
    [412] = (Syscall*)sem_getId,
    [413] = (Syscall*)sem_open,
    [414] = (Syscall*)sem_close,
};

size_t funcTableSize = sizeof(funcTable) / sizeof(*funcTable);