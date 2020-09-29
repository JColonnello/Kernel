#include <stdio.h>
#include <stdlib.h>

void main()
{
	int pid = getpid();
	for(;;)
	{
		for(int i = 0; i < 20000000; i++) ;
		printf("%d\n", pid);
	}
}