#pragma once
#include <stddef.h>

int createConsoleView(int startY, int startX, int height, int width);
void viewflush(int id);
void viewWrite(int id, const char *text, size_t n);
