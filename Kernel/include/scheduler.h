#pragma once
#include <pid.h>

void Scheduler_AddProcess(const ProcessDescriptor *pd);
void Scheduler_Init();
void Scheduler_SwitchNext();
void Scheduler_Disable();
void Scheduler_Enable();
