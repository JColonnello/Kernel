#include "time.h"
#include "keyboard.h"
#include <stdint.h>

static void int_20() {
	timer_handler();
}

static void int_21() 
{
	keyboardHandler();
}


void irqDispatcher(uint64_t irq) {
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			int_21();
			break;
	}
	return;
}