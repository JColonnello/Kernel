#include <naiveConsole.h>
#include "time.h"

static unsigned long ticks = 0;

void timer_handler() {
	ncPrintChar('x');
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}
