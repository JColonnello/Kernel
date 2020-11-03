#include <sys/process.h>
#include <stdlib.h>
#include <stdio.h>

char buffer[4096];

const char *stateString(enum ProcessState state)
{
	switch (state) 
	{
		case PROCESS_NONE:
			return "NONE";
		case PROCESS_RUNNING:
			return "RUNNING";
		case PROCESS_PENDING_BLOCK:
			return "PENDING";
		case PROCESS_BLOCKED:
			return "BLOCKED";
		default:
			return "";
	}
}

void main()
{
	struct ProcessInfo *list = (void*)buffer, *current = list;
    size_t size = ps(list, sizeof(buffer));

	printf("%-4s%-5s%-30s%-20s%-8s%-4s\n", "PID", "Prio", "Name", "Stack", "State", "Fgnd");
	while((uintptr_t)current < (uintptr_t)list + size)
	{
		printf("%-4u", current->pid);
		printf("%-5u", current->priority);
		printf("%-30s", current->name);
		printf("0x%-18lX", current->stack);
		printf("%-8s", stateString(current->state));
		printf("%-4s", current->job == JOB_FOREGROUND ? "S" : "N");
		printf("\n");
		current++;
	}
}