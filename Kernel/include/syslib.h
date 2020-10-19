#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <types.h>

typedef struct FileDescriptorData FileDescriptorData;

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02

typedef struct FileDescriptor
{
    bool isOpen;
    FileDescriptorData *data;
    int (*read)(FileDescriptorData *data, void *buf, size_t count);
    int (*write)(FileDescriptorData *data, const void *buf, size_t count);
    int (*close)(FileDescriptorData *data);
    bool (*dup)(const struct FileDescriptor *fd, struct FileDescriptor *newfd);
} FileDescriptor;

int syscallHandler();
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int open(const char *path, int mode);
int close(int fd);
int dup(int fd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
void yield();
void dumpregs(RegisterStatus *info);
void exit(int status);
