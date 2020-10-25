#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <types.h>

#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int open(const char *path, int mode);
int close(int fd);
int dup(int fd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
void yield();
void dumpregs(RegisterStatus *info);
void exit(int status);
