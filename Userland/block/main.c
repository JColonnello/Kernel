#include <sys/process.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
	if(argc < 1)
	{
		printf("Usage: block [pid]\n");
		return 1;
	}

	int pid;
	if(sscanf(argv[0], "%d", &pid))
		printf("%d\n", block(pid));
	return 0;
}