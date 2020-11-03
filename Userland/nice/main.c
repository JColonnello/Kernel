#include <stdio.h>
#include <sys/process.h>

int main(int argc, char *args[])
{
	int pid, prio;
	if(argc < 1)
	{
		fprintf(stderr, "Usage: nice <pid> [priority]\n");
	}
	else if(argc < 2)
	{
		sscanf(args[0], "%d", &pid);
		int result = processPriority(pid, -1);
		if(result < 0)
			printf("Bad PID\n");
		else
			printf("Priority: %d\n", result);
	}
	else
	{
		sscanf(args[0], "%d", &pid);
		sscanf(args[1], "%d", &prio);
		int result = processPriority(pid, prio);
		if(result < 0)
			printf("Bad PID\n");
		else
			printf("Set priority to: %d\n", result);
	}

	return 0;
}