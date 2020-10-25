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
	int pidShell1 = -1;
	int pidShell2 = -1;

	FileDescriptor *table = currentProcess()->fd;

	while(true)
	{
		if(!isRunning(pidShell1))
		{
			currentProcess()->tty = 0;
			setJobStatus(KERNEL_PID, JOB_FOREGROUND);
			openStdio(table, 0);
			pidShell1 = execve("userland/shell.bin", NULL, NULL);
			setJobStatus(KERNEL_PID, JOB_BACKGROUND);
			closeStdio();
		}
		if(!isRunning(pidShell2))
		{
			currentProcess()->tty = 1;
			setJobStatus(KERNEL_PID, JOB_FOREGROUND);
			openStdio(table, 1);
			pidShell2 = execve("userland/shell.bin", NULL, NULL);
			setJobStatus(KERNEL_PID, JOB_BACKGROUND);
			closeStdio();
		}
		setCurrentState(PROCESS_PENDING_BLOCK);
		yield();
	}
	_halt();
	return 0;
}
