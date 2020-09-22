#include <sys/process.h>
#include <stdlib.h>
#include <stdio.h>

char buffer[4096];

void main()
{
	struct ProcessInfo *list = (void*)buffer, *current = list;
    size_t size = ps(list, sizeof(buffer));

	printf("%u\n", size);
	while((uintptr_t)current < (uintptr_t)list + size)
	{
		printf("Process %u:\n", current->pid);
		printf("\tStack: 0x%Xl\n", current->stack);
		current++;
	}
}