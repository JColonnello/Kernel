#include <loader.h>
#include <lib.h>
#include <pid.h>
#include <naiveConsole.h>
#include <scheduler.h>
#include <disk.h>
#include <console.h>

extern void load_idt();
extern int main();
extern void sem_init();

int64_t __test_sync_global;

void _init()
{
	ncClear();
	libInit();
	initProcesses();
	createConsoleView(0,0,12,80);
	createConsoleView(13,0,12,80);
	
	load_idt();
	Scheduler_Init();
	sem_init();
	diskInit();

	switchLayout();

	main();
	return;
}