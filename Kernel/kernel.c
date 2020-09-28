#include "naiveConsole.h"
#include "pid.h"
#include <stdint.h>
#include <lib.h>
#include <console.h>
#include <loader.h>
#include <syslib.h>
#include <disk.h>

int main()
{	
	createConsoleView(0,0,12,80);
	createConsoleView(13,0,12,80);
	diskInit();

	switchLayout();
	int pidShell = -1;
	int pidCal = -1;

	while(true)
	{
		if(!isRunning(pidShell))
		{
			changeTTY(1);
			currentProcess()->tty = 1;
			pidShell = execve("userland/shell.bin", NULL, NULL);
			yield();
		}
		if(!isRunning(pidCal))
		{
			changeTTY(0);
			currentProcess()->tty = 0;
			pidCal = execve("userland/shell.bin", NULL, NULL);
			yield();
		}
	}
	_halt();
	return 0;
}
