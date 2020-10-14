#pragma once
#include <stdint.h>
#include <time.h>

typedef struct
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t : 64;       //Reserved
    uint64_t rip;
    uint64_t : 64;       //Reserved
} RegisterStatus;

int getcpuinfo(char *id, char *model);
void temp(uint8_t *curr_temp, uint8_t *max_temp);
void date(struct tm *time);
void dumpregs(RegisterStatus *info);
void memuse(size_t *bytesPhysical, size_t *bytesVirtual);
void cpuid(uint32_t data[static 4], uint32_t mode);
