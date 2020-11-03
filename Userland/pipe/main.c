#include <stdio.h>
#include <syslib.h>

void main()
{
	int count = pipe_list(NULL, 0);
	PipeInfo pipes[count];
	pipe_list(pipes, count);

	printf("%-4s%-7s%s\n", "ID", "Count", "Blocked");
	for(int i = 0; i < count; i++)
	{
		printf("%-4d%-7d", pipes[i].id, pipes[i].count);
		for(int j = 0; j < pipes[i].blockedCount; j++)
			printf("%d ", pipes[i].blocked[j]);
		printf("\n");
	}
}