#include <collections/queue.h>
#include <scheduler.h>
#include <lib.h>

Queue *ready, *waiting;

void Scheduler_Init()
{
	ready = Queue_Create(0, sizeof(ProcessDescriptor*));
	waiting = Queue_Create(0, sizeof(ProcessDescriptor*));
}

void Scheduler_AddProcess(const ProcessDescriptor *pd)
{
	Queue_Enqueue(waiting, &pd);
}

void Scheduler_SwitchNext()
{
	_cli();
	ProcessDescriptor *next, *curr = currentProcess();
	if(isRunning(curr->pid))
		Queue_Enqueue(waiting, &curr);
	
	if(Queue_Count(ready) == 0)
	{
		Queue *tmp = ready;
		ready = waiting;
		waiting = tmp;
	}

	Queue_Dequeue(ready, &next);
	//Clean interrupt before switch
	outb(0x20, 0x20);
	contextSwitch(next);
}