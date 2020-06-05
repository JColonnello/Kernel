#pragma once
#include <stddef.h>

typedef struct FileDescriptorData FileDescriptorData;

typedef struct
{
    FileDescriptorData *data;
    int (*read)(FileDescriptorData *data, void *buf, size_t count);
    int (*write)(FileDescriptorData *data, const void *buf, size_t count);
    void (*close)(FileDescriptorData *data);
} FileDescriptor;

int syscallHandler();
size_t initFD(FileDescriptor **fdt);
