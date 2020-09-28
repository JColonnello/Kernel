#pragma once
#include <stdint.h>

enum PdJobStatus
{
    JOB_NONE,
	JOB_FOREGROUND,
	JOB_BACKGROUND,
};

struct ProcessInfo
{
    char name[64];
    unsigned pid;
    uintptr_t stack;
};
