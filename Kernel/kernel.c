#include "naiveConsole.h"
#include "pid.h"
#include <stdint.h>
#include <lib.h>
#include <loader.h>
#include <syslib.h>
#include <console.h>

int main()
{	
	int pidShell = -1;
	int pidCal = -1;

	while(true)
	{
		if(!isRunning(pidShell))
		{
			changeTTY(1);
			currentProcess()->tty = 1;
			pidShell = execve("userland/shell.bin", NULL, NULL);
		}
		if(!isRunning(pidCal))
		{
			changeTTY(0);
			currentProcess()->tty = 0;
			pidCal = execve("userland/shell.bin", NULL, NULL);
		}
		setCurrentState(PROCESS_PENDING_BLOCK);
		yield();
	}
	_halt();
	return 0;
}
