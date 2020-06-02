#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include "interrupts/idtLoader.h"

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
	load_idt();
	ncClear();
    //_syscall();
	uint64_t *buf = kmap(NULL, NULL, NULL, 1);
	ncPrintPointer(buf);
	ncNewline();
	*buf = 0xdeadc0de;
	ncPrintHex(*buf);
	_halt();

	return 0;
}
