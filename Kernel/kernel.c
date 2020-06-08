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
	//ata_lba_read(sampleCodeModuleAddress,0x110A,10);
	diskInit();
	int fd = open("userland/0000-sampleCodeModule.bin", O_RDONLY);
	int len = read(fd, sampleCodeModuleAddress, pages * PAGE_SIZE);
	EntryPoint module = sampleCodeModuleAddress;
	ProcessDescriptor *pd = currentProcess();
	pd->binaryEnd = (uintptr_t)sampleCodeModuleAddress + len;
	pd->prgmBreak = (uintptr_t)sampleCodeModuleAddress + pages * PAGE_SIZE;

	module();
	ncPrint("Listo\n");
	_halt();

	return 0;
}
