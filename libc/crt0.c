#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char bss;
extern char endOfBinary;

extern int main();
extern int brk(void *addr);

void _start() {
	//Clean BSS
	brk(&endOfBinary);
	memset(&bss, 0, &endOfBinary - &bss);

	char *argv[64];
    char *argLoc = (void*)0x100000;
    int argc = 0;
    for(int i = 1; argLoc[i-1] != 0 || argLoc[i] != 0; i++)
    {
        if(argLoc[i-1] == 0)
        {
            argv[argc] = &argLoc[i];
            argc++;
        }
    }

	int err = main(argc, argv);
	exit(err);
}