#pragma once
#include <lib.h>

typedef struct __WaitHandle WaitHandle;
struct __PDNode;

void __freeWaitEntry(WaitHandle *handle, struct __PDNode *node);
void __addWaitEntry(WaitHandle *handle, struct __PDNode *node);
void __freeWaitEntries(struct __PDNode *node, ...);
void __addWaitEntries(struct __PDNode *node, ...);
struct __PDNode *__initWaitEntry(WaitHandle *handle);

#include <scheduler.h>
#include <pid.h>
#define waitEvent(cond, handle) \
do { \
	if((cond)) \
		break; \
	struct __PDNode *node = __initWaitEntry((handle)); \
	for(;;) {\
		Scheduler_Disable(); \
		checkProcessSignals(); \
		__addWaitEntry((handle), node); \
		setCurrentState(PROCESS_PENDING_BLOCK); \
		if((cond)) { \
			break; \
		} \
		Scheduler_SwitchNext(); \
		Scheduler_Enable(); \
	} \
	setCurrentState(PROCESS_RUNNING); \
	__freeWaitEntry((handle), node); \
	Scheduler_Enable(); \
} while(0)

#define waitEvents(cond, ...) \
do { \
	if((cond)) \
		break; \
	struct __PDNode *node = __initWaitEntry((handle)); \
	for(;;) {\
		Scheduler_Disable(); \
		checkProcessSignals(); \
		__addWaitEntries(node, __VA_ARGS__, NULL); \
		setCurrentState(PROCESS_PENDING_BLOCK); \
		if((cond)) { \
			break; \
		} \
		Scheduler_SwitchNext(); \
		Scheduler_Enable(); \
	} \
	setCurrentState(PROCESS_RUNNING); \
	__freeWaitEntries(node, __VA_ARGS__, NULL); \
	Scheduler_Enable(); \
} while(0)

void releaseOne(WaitHandle *handle);
void releaseAll(WaitHandle *handle);
WaitHandle *WaitHandle_Create();
void WaitHandle_Dispose(WaitHandle *handle);
