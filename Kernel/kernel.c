#include "naiveConsole.h"
#include "pid.h"
#include <stdint.h>
#include <lib.h>
#include <loader.h>
#include <syslib.h>
#include <console.h>
#include <io/stdio.h>

static void closeStdio()
{
	for(int i = 0; i < 3; i++)
		close(i);
}

int main()
{	
	int pidShell = -1;
	int pidCal = -1;

	FileDescriptor *table = currentProcess()->fd;

	while(true)
	{
		if(!isRunning(pidShell))
		{
			currentProcess()->tty = 0;
			openStdio(table, 0);
			pidShell = execve("userland/shell.bin", NULL, NULL);
			closeStdio();
		}
		if(!isRunning(pidCal))
		{
			currentProcess()->tty = 1;
			openStdio(table, 1);
			pidCal = execve("userland/shell.bin", NULL, NULL);
			closeStdio();
		}
		setCurrentState(PROCESS_PENDING_BLOCK);
		yield();
	}
	_halt();
	return 0;
}
