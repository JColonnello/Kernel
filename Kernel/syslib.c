#include "syslib.h"
#include <stddef.h>
#include <naiveConsole.h>

int read(int fd, void *buf, size_t count)
{
    ncPrintChar('?');
    return 1;
}

int write(int fd, const void *buf, size_t count)
{
    ncPrintChar('?');
    return 1;
}

int (*funcTable[])(void) = 
{
    [0] = read,
    [1] = write,

};