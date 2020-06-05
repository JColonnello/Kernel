#include "stdio.h"
#include "lib.h"
#include "syslib.h"
#include <console.h>

struct FileDescriptorData
{
    int tty;
};

static int readTTY(FileDescriptorData *data, void *buf, size_t count)
{
    return inputBufferRead(data->tty, buf, count);
}

static int writeTTY(FileDescriptorData *data, const void *buf, size_t count)
{
    return viewWrite(data->tty, buf, count);
}

static void closeFD(FileDescriptorData *data)
{
    kfree(data);
}

void openStdio(FileDescriptor *table)
{
    //To not overcomplicate things, is better if we allocate
    //a diferent structure for every descriptor

    //stdin
    table[0] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .read = readTTY,
        .close = closeFD
    };
    //stdout
    table[1] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .write = writeTTY,
        .close = closeFD
    };
    //stderr
    table[2] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .write = writeTTY,
        .close = closeFD
    };
}