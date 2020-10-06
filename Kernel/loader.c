#include <loader.h>
#include <lib.h>
#include <pid.h>
#include <naiveConsole.h>
#include <scheduler.h>

extern void load_idt();
extern int main();

void _init()
{
	ncClear();
	libInit();
	ProcessDescriptor *pd = currentProcess();
	pd->pml4 = 0x2000;
	pd->tty = 0;
	pd->fdtSize = initFD(&pd->fd, pd->tty);
	pd->foreground = false;
	pd->state = PROCESS_RUNNING;
	Scheduler_Init();
	load_idt();

	main();
	return;
}