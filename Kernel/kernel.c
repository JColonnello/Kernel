#include "naiveConsole.h"
#include "pid.h"
#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <console.h>
#include <loader.h>
#include <syslib.h>
#include <disk.h>

extern int _syscall(int,void*,size_t);
static void * sampleCodeModuleAddress = (void*)0x400000;
typedef int (*EntryPoint)();

int main()
{	
	int pages = 20;
	createConsoleView(0,0,25,80);
	kmap(&sampleCodeModuleAddress, NULL, NULL, pages);
	diskInit();
	
	int pid = execve("userland/0000-sampleCodeModule.bin", NULL, NULL);
	contextSwitch(pid);
	ncPrint("Listo\n");

	_halt();
	return 0;
}
