#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <types.h>

typedef struct FileDescriptorData FileDescriptorData;

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02

typedef struct
{
    bool isOpen;
    FileDescriptorData *data;
    int (*read)(FileDescriptorData *data, void *buf, size_t count);
    int (*write)(FileDescriptorData *data, const void *buf, size_t count);
    int (*close)(FileDescriptorData *data);
} FileDescriptor;

int syscallHandler();
size_t initFD(FileDescriptor **fdt, int tty);
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int open(const char *path, int mode);
int close(int fd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
void wait();
void dumpregs(RegisterStatus *info);
