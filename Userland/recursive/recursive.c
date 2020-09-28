#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("Begin %d\n", getpid());
	int pid = execve("userland/recursive.bin", NULL, NULL);
	//for(int i = 0; i < 2000000; i++) ;
	//yield(pid);
	//for(int i = 0; i < 2000000; i++) ;
	printf("Exit %d\n", getpid());
	return 0;
}