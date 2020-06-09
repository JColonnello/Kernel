#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

extern void *_brk(void *addr);

static void *currbrk = NULL;

int brk(void *addr)
{
    //Initialize currbrk
    if(currbrk == NULL)
    {
        //Force fail to get program break
        currbrk = _brk(0);
    }
    void *newseg = _brk(addr);
    if(newseg == currbrk)
    {
        errno = ENOMEM;
        return -1;
    }
    currbrk = addr;
    return 0;
}

void *sbrk(intptr_t increment)
{
    //Initialize currbrk
    if(currbrk == NULL)
    {
        //Force fail to get program break
        currbrk = _brk(0);
    }

    if(increment == 0)
        return currbrk;
    //Check for overflow
    if((increment > 0 && ((uintptr_t)currbrk + increment) < (uintptr_t)currbrk) ||
        (increment < 0 && (uintptr_t)currbrk < -increment))
    {
        errno = ENOMEM;
        return (void*) -1;
    }
    void *prevbrk = currbrk;
    if(brk(currbrk + increment) < 0)
        return (void*) -1;

    return prevbrk;
}