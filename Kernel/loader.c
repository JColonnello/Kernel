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
	initProcesses();
	Scheduler_Init();
	load_idt();

	main();
	return;
}