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
	int err = main();
	exit(err);
}