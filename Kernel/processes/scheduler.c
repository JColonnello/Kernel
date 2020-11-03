#include <collections/queue.h>
#include <scheduler.h>
#include <lib.h>
#include <pid.h>

struct SchedEntry
{
	ProcessDescriptor *pd;
	unsigned runs;
};
static Queue *ready, *waiting;
static bool enabled, pending;
static struct SchedEntry current;
static bool resign;

static char prioToRuns(char prio)
{
	return 6 - prio;
}

void Scheduler_Init()
{
	ready = Queue_Create(0, sizeof(struct SchedEntry));
	waiting = Queue_Create(0, sizeof(struct SchedEntry));
	current.pd = currentProcess();
	enabled = true;
	pending = false;
}

void Scheduler_Yield()
{
	resign = true;
	Scheduler_SwitchNext();
}

void Scheduler_AddProcess(ProcessDescriptor *pd)
{
	struct SchedEntry entry = { .pd = pd, .runs = 0 };
	Queue_Enqueue(waiting, &entry);
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
	current.runs++;
	if(current.pd != NULL && isRunning(current.pd->pid))
	{
		switch (current.pd->state) 
		{
			case PROCESS_RUNNING:
				if(current.runs >= prioToRuns(current.pd->priority) || resign)
					Queue_Enqueue(waiting, &current);
				else
					return;
				break;
			case PROCESS_PENDING_BLOCK:
				setCurrentState(PROCESS_BLOCKED);
				break;
			default:
				break;
		}
	}
	resign = false;
	
	if(Queue_Count(ready) == 0)
	{
		if(Queue_Count(waiting) == 0)
		{
			current.pd = NULL;
			goInactive();
			return;
		}
		Queue *tmp = ready;
		ready = waiting;
		waiting = tmp;
	}

	Queue_Dequeue(ready, &current);
	current.runs = 0;
	contextSwitch(current.pd);
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