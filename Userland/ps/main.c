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
			return "PENDING BLOCK";
		case PROCESS_BLOCKED:
			return "BLOCKED";
	}
}

void main()
{
	struct ProcessInfo *list = (void*)buffer, *current = list;
    size_t size = ps(list, sizeof(buffer));

	printf("%-4s%-40s%-20s%s\n", "PID", "Name", "Stack", "State");
	while((uintptr_t)current < (uintptr_t)list + size)
	{
		printf("%-4u", current->pid);
		printf("%-40s", current->name);
		printf("0x%-18lX", current->stack);
		printf("%s", stateString(current->state));
		printf("\n");
		current++;
	}
}