#pragma once

void load_idt();
void setupIDTEntry(uint8_t entry, const void *handler);