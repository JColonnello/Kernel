#include "interrupts/interrupts.h"
#include <console.h>
#include <stddef.h>
#include <loader.h>
#include <lib.h>
#include <naiveConsole.h>

typedef enum
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE,
} Color;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    char symbol;
    union
    {
        struct
        {
            Color foreground : 4;
            Color background : 4;
        };
        char code;
    } color;
} CharEntry;
#pragma pack(pop)

typedef struct 
{
    int startY;
    int startX;
    int height;
    int width;
    CharEntry *lines;
    size_t lineEnd;
    size_t lineCount;
    size_t lineCursor;
    size_t maxLines;
    char *input;
    size_t inputStart;
    size_t inputCount;
    size_t maxInput;
} ConsoleView;

static CharEntry (*video)[25][80] = &__vga + 0x18000;
static ConsoleView views[8];
static const char defaultColor = 0x07;
int focusedView = 0;

void ncClear()
{
    CharEntry *p = (CharEntry*)video;
    for(int i = 0; i < sizeof(*video); i++)
        *p++ = (CharEntry){ .color.code = defaultColor, .symbol = 0 };
}

int createConsoleView(int startY, int startX, int height, int width)
{
    int id;
    const size_t bufferPages = 1;
    for(id = 0; id < sizeof(views)/sizeof(*views); id++)
        if(views[id].lines == NULL)
            break;
    
    ConsoleView view = (ConsoleView)
    {
        .startY = startY,
        .startX = startX,
        .height = height,
        .width = width,
        .lineCount = 1
    };
    view.lines = kmap(NULL, NULL, NULL, bufferPages);
    view.maxLines = (bufferPages * 4096) / (view.width * sizeof(CharEntry));
    for(int i = 0; i < (view.width * view.maxLines); i++)
    {
        view.lines[i] = (CharEntry){ .color.code = defaultColor, .symbol = 0 };
    }
    view.input = kmap(NULL, NULL, NULL, 1);
    view.maxInput = 4096;

    views[id] = view;

    return id;
}

void viewflush(int id)
{
    ConsoleView view = views[id];
    CharEntry (*buf)[view.maxLines][view.width] = (void*)view.lines;

    int lines = (view.height < view.lineCount) ? view.height : view.lineCount;
    for(int i = 0; i < lines; i++)
    {
        int bufLine = (view.lineEnd - lines + i + 1) % view.maxLines;
        for(int j = 0; j < view.width; j++)
        {
            (*video)[view.startY + i][view.startX + j] = (*buf)[bufLine][j];
        }
    }

}

void viewLF(int id)
{
    ConsoleView *view = &views[id];
    CharEntry (*buf)[view->maxLines][view->width] = (void*)view->lines;
    
    view->lineEnd++;
    view->lineCursor = 0;
    view->lineCount++;
    for(int j = 0; j < view->width; j++)
        (*buf)[view->lineEnd][j] = (CharEntry){ .color.code = defaultColor, .symbol = 0 };

    if(view->lineCount > view->maxLines)
            view->lineCount = view->maxLines;
    if(view->lineEnd > view->maxLines)
        view->lineEnd %= view->maxLines;
}

int viewWrite(int id, const char *text, size_t n)
{
    ConsoleView *view = &views[id];
    CharEntry (*buf)[view->maxLines][view->width] = (void*)view->lines;

    for(int i = 0; i < n; i++, text++)
    {
        if(view->lineCursor >= view->width)
            viewLF(id);
        
        char c = *text;
        //Control character
        if(c < 0x20)
        {
            switch (c) {
                case '\n':
                    viewLF(id);
                    break;
                case '\t':
                    view->lineCursor += 4 - view->lineCursor % 4;
                    break;
                case '\r':
                    view->lineCursor = 0;
                    break;
            }
        }
        else
            (*buf)[view->lineEnd][view->lineCursor++].symbol = *text;
    }
    viewflush(id);

    return n;
}

void inputBufferWrite(char c)
{
    ConsoleView *view = &views[focusedView];

    if(view->inputCount < view->maxInput)
        view->input[(view->inputStart + view->inputCount++) % view->maxInput] = c;

    viewWrite(focusedView, &c, 1);
}

int inputBufferRead(int id, char *dest, size_t count)
{
    ConsoleView *view = &views[focusedView];
    int i;
    
    if(view->input == NULL)
        return 0;

    for(i = 0; i < count; i++, view->inputCount--)
    {
        while(view->inputCount == 0)
            _hlt();

        dest[i] = view->input[view->inputStart++];
        if(view->inputStart == view->maxInput)
            view->inputStart = 0;
    }

    return i;
}


void changeFocus(int id)
{
    if(views[id].lines != NULL)
        focusedView = id;
}

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

static char buffer[64] = { '0' };

void ncWrite(const char *buf, size_t n)
{
	viewWrite(focusedView, buf, n);
}

void ncPrint(const char * string)
{
	int i;

	for (i = 0; string[i] != 0; i++)
		ncPrintChar(string[i]);
}

void ncPrintChar(char character)
{
	viewWrite(focusedView, &character, 1);
}

void ncNewline()
{
	viewLF(focusedView);
}

void ncPrintDec(uint64_t value)
{
	ncPrintBase(value, 10);
}

void ncPrintHex(uint64_t value)
{
	ncPrintBase(value, 16);
}

void ncPrintBin(uint64_t value)
{
	ncPrintBase(value, 2);
}

void ncPrintBase(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    ncPrint(buffer);
}

void ncPrintPointer(void *pointer)
{
	uintptr_t value = (uintptr_t)pointer;
	//Calculate characters for each digit
	for(int i = 15; i >= 0; i--, value >>= 4)
	{
		uint32_t remainder = value % 16;
		buffer[i] = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
	}

	// Terminate string in buffer.
	buffer[16] = 0;
    ncPrint(buffer);
}

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}