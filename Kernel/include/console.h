#pragma once
#include <stddef.h>
#include <naiveConsole.h>

#define MAX_VIEWS 12

int createConsoleView(int startY, int startX, int height, int width);
void viewflush(int id);
int viewWrite(int id, const char *text, size_t n);
void inputBufferWrite(char c);
int inputBufferRead(int id, char *dest, size_t count);
void changeTTY(int id);