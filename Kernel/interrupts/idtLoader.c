#include <stdint.h>
#include <naiveConsole.h>
#include "idtLoader.h"
#include <syslib.h>
#include "keyboard.h"
#include "time.h"
#include <scheduler.h>
#include <pid.h>
#include <lib.h>

#define endOfArray(x) (sizeof(x)/ sizeof (*(x)) - 1)

extern void setupIDTHandlers();
extern void defaultException();
extern void defaultInterrupt();
extern void picMasterMask(uint8_t mask);
extern void picSlaveMask(uint8_t mask);
extern int syscallHandler();

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

    setupIDTEntry(0x20, Scheduler_SwitchNext);
    setupIDTEntry(0x21, keyboardHandler);
    setupIDTEntry (0x80, syscallHandler);
    // setupIDTEntry (0x00, (uint64_t)&_exception0Handler);

	picMasterMask(0xFC); 
	picSlaveMask(0xFF);
  
	_sti();
}

static const char *exc_string[] = 
{
    [0] = "00 - DE",
    [1] = "01 - DB",
    [2] = "02     ",
    [3] = "03 - BP",
    [4] = "04 - OF",
    [5] = "05 - BR",
    [6] = "06 - UD",
    [7] = "07 - NM",
    [8] = "08 - DF",
    [9] = "09     ",	
    [10] = "10 - TS",
    [11] = "11 - NP",
    [12] = "12 - SS",
    [13] = "13 - GP",
    [14] = "14 - PF",
    [15] = "15     ",
    [16] = "16 - MF",
    [17] = "17 - AC",
    [18] = "18 - MC",
    [19] = "19 - XM",
    [20] = "20 - VE",
},
*reg_string[] =
{
    "  A:",
    "  B:",
    "  C:",
    "  D:",
    "  8:",
    "  9:",
    " 10:",
    " 11:",
    " 12:",
    " 13:",
    " 14:",
    " 15:",
    " DI:",
    " SI:",
    " BP:",
    " SP:",
};
int reg_offsets[] = 
{
    offsetof(RegisterStatus,rax),
    offsetof(RegisterStatus,rbx),
    offsetof(RegisterStatus,rcx),
    offsetof(RegisterStatus,rdx),
    offsetof(RegisterStatus,r8),
    offsetof(RegisterStatus,r9),
    offsetof(RegisterStatus,r10),
    offsetof(RegisterStatus,r11),
    offsetof(RegisterStatus,r12),
    offsetof(RegisterStatus,r13),
    offsetof(RegisterStatus,r14),
    offsetof(RegisterStatus,r15),
    offsetof(RegisterStatus,rdi),
    offsetof(RegisterStatus,rsi),
    offsetof(RegisterStatus,rbp),
    offsetof(RegisterStatus,rsp),
};

void defaultException(RegisterStatus *registers)
{
    Scheduler_Disable();
    ncNewline();
    ncPrint("ColOS - Exception ");
    ncPrint(exc_string[registers->irqNumber]);
    ncPrint(" @ 0x");
    ncPrintPointer(registers->rip);
    ncPrint(" - EC= ");
    ncPrintHex(registers->errorCode);
    ncPrint(" - PID= ");
    ncPrintDec(getCurrentPid());
    ncNewline();
    if(registers->irqNumber == 14) // Page fault
    {
        ncPrint("ADDR: 0x");
        ncPrintPointer(_cr2());
    }
    ncNewline();
    for(int i = 0; i < sizeof(reg_string)/sizeof(*reg_string); i++)
    {
        uint64_t *reg = (void*)(registers) + reg_offsets[i];
        ncPrint(reg_string[i]);
        ncPrintPointer(*reg);
    }

    exit(0);
}