#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct FileDescriptorData FileDescriptorData;

typedef struct FileDescriptor
{
    bool isOpen;
    FileDescriptorData *data;
    int (*read)(FileDescriptorData *data, void *buf, size_t count);
    int (*write)(FileDescriptorData *data, const void *buf, size_t count);
    int (*close)(FileDescriptorData *data);
    bool (*dup)(const struct FileDescriptor *fd, struct FileDescriptor *newfd);
} FileDescriptor;
