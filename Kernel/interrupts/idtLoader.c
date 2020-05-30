#include <stdint.h>
#include <naiveConsole.h>
#include "idtLoader.h"
#include "interrupts.h"
#include "keyboard.h"
#include "time.h"

extern void setupIDTHandlers();
extern void defaultException();
extern void defaultInterrupt();

int (*exceptionTable[32])(void) = 
{
    [0 ... 31] = defaultException
}; 

void (*irqTable[256-32])(void) = 
{
    [0 ... 255-32] = defaultInterrupt
};

void setupIDTEntry(uint8_t entry, const void *handler)
{
    if(entry < 0x20)
        exceptionTable[entry] = handler;
    else
        irqTable[entry - 0x20] = handler;
}

void load_idt() {
    setupIDTHandlers();

    // setupIDTEntry(0x20, timer_handler);
    setupIDTEntry(0x21, keyboardHandler);

	//Solo interrupcion timer tick habilitadas
	picMasterMask(0xFD); 
	picSlaveMask(0xFF);
  
	_sti();
}