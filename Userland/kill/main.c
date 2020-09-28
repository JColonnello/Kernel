#include <stdio.h>
#include <sys/process.h>

int main(int argc, char **argv)
{
	if(argc < 1)
	{
		printf("Usage: kill [pid]\n");
		return 2;
	}

	int pid;
	sscanf(argv[0], "%d", &pid);
	if(kill(pid) == 0)
	{
		printf("Success\n");
		return 0;
	}
	else
	{
		printf("Failed\n");
		return 1;
	}
	
}