#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <console.h>
#include "interrupts/idtLoader.h"
#include "loader.h"

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
    //_syscall();
	int id = createConsoleView(2,2,5,20);
	char text[] = "hola";
	for(int i = 0;;i++)
	{
		text[0] = i;
		viewWrite(id, text, sizeof(text));
		for(int j = 0; j < 20000000; j++);
	}
	_halt();

	return 0;
}
