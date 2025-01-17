#pragma once
#include <stddef.h>
#include <common/processInfo.h>

size_t ps(struct ProcessInfo *buffer, size_t size);
int kill(int pid);
int ispidrun(int pid);
void yield();
enum PdJobStatus setjobstatus(int pid, enum PdJobStatus status);
int block(int pid);
int processPriority(int pid, int prio);
