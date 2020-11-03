#include <syncro/semaphore.h>
#include <stdio.h>

void main()
{
	int count = sem_list(NULL, 0);
	SemaphoreStatus sems[count];
	sem_list(sems, count);

	printf("%-4s%-7s\n", "ID", "Count");
	for(int i = 0; i < count; i++)
	{
		printf("%-4d%-7d\n", sems[i].id, sems[i].count);
	}
}