#include <loader.h>
#include <lib.h>
#include <pid.h>
#include <naiveConsole.h>

extern void load_idt();
extern int main();

void _init()
{
	ncClear();
	load_idt();
	libInit();
	ProcessDescriptor *pd = currentProcess();
	pd->pml4 = 0x2000;
	pd->tty = 0;
	pd->fdtSize = initFD(&pd->fd, pd->tty);

	main();
	return;
}