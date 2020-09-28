#pragma once
#include <stdint.h>

struct ProcessInfo
{
    char name[64];
    unsigned pid;
    uintptr_t stack;
};
