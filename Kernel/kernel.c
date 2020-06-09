#include "naiveConsole.h"
#include "pid.h"
#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <console.h>
#include <loader.h>
#include <syslib.h>
#include <disk.h>

int main()
{	
	createConsoleView(0,0,25,80);
	diskInit();
	
	int before = getReservedPagesCount();

	int pid = execve("userland/0000-sampleCodeModule.bin", NULL, NULL);
	contextSwitch(pid);
	ncPrint("Listo\n");

	pid = execve("userland/0000-sampleCodeModule.bin", NULL, NULL);
	contextSwitch(pid);
	ncPrint("Listo\n");

	int after = getReservedPagesCount();
	ncPrintDec(before);
	ncNewline();
	ncPrintDec(after);

	_halt();
	return 0;
}
