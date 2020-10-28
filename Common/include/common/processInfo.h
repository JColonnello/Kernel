#pragma once
#include <stdint.h>

enum PdJobStatus
{
    JOB_NONE,
	JOB_FOREGROUND,
	JOB_BACKGROUND,
};

enum ProcessState
{
    PROCESS_NONE,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_PENDING_BLOCK,
};

struct ProcessInfo
{
    char name[64];
    unsigned pid;
    uintptr_t stack;
    enum ProcessState state;
    enum PdJobStatus job;
};
