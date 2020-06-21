#pragma once
#include <types.h>

void load_idt();
void setupIDTEntry(uint8_t entry, const void *handler);