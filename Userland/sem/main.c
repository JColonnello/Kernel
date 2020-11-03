#include <syncro/semaphore.h>
#include <stdio.h>

void main()
{
	int count = sem_list(NULL, 0);
	SemaphoreStatus sems[count];
	sem_list(sems, count);

	printf("%-4s%-7s%s\n", "ID", "Count", "Blocked");
	for(int i = 0; i < count; i++)
	{
		printf("%-4d%-7d", sems[i].id, sems[i].count);
		for(int j = 0; j < sems[i].blockedCount; j++)
			printf("%d ", sems[i].blocked[j]);
		printf("\n");
	}
}