#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <console.h>
#include "interrupts/idtLoader.h"
#include "loader.h"
#include "naiveConsole.h"

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
	char *test = kmalloc(0x10);
	memcpy(test, "prueba\n", 8);
	ncPrint(test);
	ncPrintPointer(test);
	ncNewline();
	char *test2 = kmalloc(0x20);
	ncPrintPointer(test2);
	ncNewline();
	kfree(test);
	kfree(test2);
	test = kmalloc(0x30);
	ncPrintPointer(test);
	kfree(test);

	_halt();

	return 0;
}
