#pragma once
#include <stddef.h>

int createConsoleView(int startY, int startX, int height, int width);
void viewflush(int id);
int viewWrite(int id, const char *text, size_t n);
void inputBufferWrite(char c);
int inputBufferRead(int id, char *dest, size_t count);
void changeFocus(int id);