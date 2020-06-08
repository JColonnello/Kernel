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
	ProcessDescriptor desc = (ProcessDescriptor)
	{
		.pml4 = 0x2000,
		.tty = 0
	};
	createProcess(&desc);
	main();
	return;
}