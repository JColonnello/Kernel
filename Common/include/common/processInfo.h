#pragma once
#include <stdint.h>

struct ProcessInfo
{
    unsigned pid;
    uintptr_t stack;
};
