#include <stdint.h>
#include <naiveConsole.h>
#include "idtLoader.h"
#include "interrupts.h"
#include "../syslib.h"
#include "keyboard.h"
#include "time.h"

#define endOfArray(x) (sizeof(x)/ sizeof (*(x)) - 1)

extern void setupIDTHandlers();
extern void defaultException();
extern void defaultInterrupt();

void (*exceptionTable[0x20])(void);

void (*irqTable[0x30-0x20])(void);

void (*intTable[0x100-0x30])(void);

void setupIDTEntry(uint8_t entry, const void *handler)
{
    if(entry < 0x20)
        exceptionTable[entry] = handler;
    else if(entry < 0x30)
        irqTable[entry - 0x20] = handler;
    else
        intTable[entry - 0x30] = handler;
}

void load_idt() {
    setupIDTHandlers();

    // setupIDTEntry(0x20, timer_handler);
    setupIDTEntry(0x21, keyboardHandler);
    setupIDTEntry (0x80, syscallHandler);
    // setupIDTEntry (0x00, (uint64_t)&_exception0Handler);

	picMasterMask(0xFD); 
	picSlaveMask(0xFF);
  
	_sti();
}