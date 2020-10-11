#include <collections/queue.h>
#include <scheduler.h>
#include <lib.h>

static Queue *ready, *waiting;
static bool enabled, pending;

void Scheduler_Init()
{
	ready = Queue_Create(0, sizeof(ProcessDescriptor*));
	waiting = Queue_Create(0, sizeof(ProcessDescriptor*));
	enabled = true;
	pending = false;
}

void Scheduler_AddProcess(const ProcessDescriptor *pd)
{
	Queue_Enqueue(waiting, &pd);
}

void Scheduler_SwitchNext()
{
	if(!enabled)
	{
		pending = true;
		return;
	}
	_cli();
	pending = false;
	ProcessDescriptor *next, *curr = currentProcess();
	if(isRunning(curr->pid) && curr->pid != INACTIVE_PID)
	{
		switch (curr->state) 
		{
			case PROCESS_RUNNING:
				Queue_Enqueue(waiting, &curr);
				break;
			case PROCESS_PENDING_BLOCK:
				setCurrentState(PROCESS_BLOCKED);
				break;
			default:
				break;
		}
	}
	
	if(Queue_Count(ready) == 0)
	{
		if(Queue_Count(waiting) == 0)
		{
			goInactive();
			return;
		}
		Queue *tmp = ready;
		ready = waiting;
		waiting = tmp;
	}

	Queue_Dequeue(ready, &next);
	contextSwitch(next);
}

void Scheduler_Disable()
{
	enabled = false;
}

void Scheduler_Enable()
{
	enabled = true;
	if(pending)
		Scheduler_SwitchNext();
}