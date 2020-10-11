#pragma once
#include <pid.h>
#include <lib.h>
#include <scheduler.h>

typedef struct __WaitHandle WaitHandle;
struct __PDNode;

void __freeWaitEntry(WaitHandle *handle, struct __PDNode *node);
void __addWaitEntry(WaitHandle *handle, struct __PDNode *node);
struct __PDNode *__initWaitEntry(WaitHandle *handle);

#define waitEvent(handle, cond) \
do { \
	if(!(cond)) \
		break; \
	struct __PDNode *node = __initWaitEntry((handle)); \
	for(;;) {\
		Scheduler_Disable(); \
		checkProcessSignals(); \
		__addWaitEntry((handle), node); \
		setCurrentState(PROCESS_PENDING_BLOCK); \
		if(!(cond)) { \
			break; \
		} \
		Scheduler_SwitchNext(); \
		Scheduler_Enable(); \
	} \
	setCurrentState(PROCESS_RUNNING); \
	__freeWaitEntry((handle), node); \
	Scheduler_Enable(); \
} while(0)

void releaseOne(WaitHandle *handle);
void releaseAll(WaitHandle *handle);
WaitHandle *WaitHandle_Create();
void WaitHandle_Dispose(WaitHandle *handle);
