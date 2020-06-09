#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("Begin %d\n", getpid());
	int pid = execve("userland/0000-sampleCodeModule.bin", NULL, NULL);
	for(int i = 0; i < 20000000; i++) ;
	wait(pid);
	for(int i = 0; i < 20000000; i++) ;
	printf("Exit %d\n", getpid());
	return 0;
}