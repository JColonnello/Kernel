#pragma once
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <types.h>

int getcpuinfo(char *id, char *model);
void temp(uint8_t *curr_temp, uint8_t *max_temp);
void date(struct tm *time);
void dumpregs(RegisterStatus *info);
void memuse(size_t *bytesPhysical, size_t *bytesVirtual);
void cpuid(uint32_t data[static 4], uint32_t mode);
bool pipe(int fd[static 2]);
int dup(int fd);
