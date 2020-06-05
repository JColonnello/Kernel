#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <console.h>
#include "naiveConsole.h"

extern int _syscall(int,void*,size_t);

int main()
{	
	/*
	ncPrint("[Kernel Main]");
	ncNewline();
	ncPrint("  Sample code module at 0x");
	ncPrintHex((uint64_t)sampleCodeModuleAddress);
	ncNewline();
	ncPrint("  Calling the sample code module returned: ");
	ncPrintHex(((EntryPoint)sampleCodeModuleAddress)());
	ncNewline();
	ncNewline();

	ncPrint("  Sample data module at 0x");
	ncPrintHex((uint64_t)sampleDataModuleAddress);
	ncNewline();
	ncPrint("  Sample data module contents: ");
	ncPrint((char*)sampleDataModuleAddress);
	ncNewline();

	ncPrint("[Finished]");
	*/
	createConsoleView(0,0,25,80);
	diskInit();
	show_file("test");


	_halt();

	return 0;
}
