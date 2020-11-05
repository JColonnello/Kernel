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
extern void pipe_init();

int64_t __test_sync_global;
char __test_phylo_global[128];

void _init()
{
	memset((void*)&__bss, 0, (&__endOfKernel - &__bss));
	ncClear();
	libInit();
	createConsoleView(0,0,12,80);
	createConsoleView(13,0,12,80);
	initProcesses();
	
	load_idt();
	Scheduler_Init();
	sem_init();
	pipe_init();
	diskInit();

	switchLayout();

	main();
	return;
}