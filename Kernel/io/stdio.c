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

static int closeFD(FileDescriptorData *data)
{
    kfree(data);
    return 0;
}

static bool dupTTY(const FileDescriptor *fd, struct FileDescriptor *newfd)
{
    FileDescriptorData *newdata = kmalloc(sizeof(FileDescriptorData));
    if(newdata == NULL)
        return false;

    memcpy(newdata, fd->data, sizeof(FileDescriptorData));
    memcpy(newfd, fd, sizeof(struct FileDescriptor));
    newfd->data = newdata;

    return true;
}


void openStdio(FileDescriptor *table, int tty)
{
    //To not overcomplicate things, is better if we allocate
    //a diferent structure for every descriptor

    //stdin
    table[0] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .read = readTTY,
        .close = closeFD,
        .dup = dupTTY,
        .isOpen = true,
    };
    //stdout
    table[1] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .write = writeTTY,
        .close = closeFD,
        .dup = dupTTY,
        .isOpen = true,
    };
    //stderr
    table[2] = (FileDescriptor)
    {
        .data = kmalloc(sizeof(FileDescriptorData)),
        .write = writeTTY,
        .close = closeFD,
        .dup = dupTTY,
        .isOpen = true,
    };

    table[0].data->tty = tty;
    table[1].data->tty = tty;
    table[2].data->tty = tty;
}