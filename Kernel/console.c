#include <console.h>
#include <stddef.h>
#include <loader.h>
#include <lib.h>
#include <naiveConsole.h>
#include <stdbool.h>
#include <syncro/wait.h>

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
    bool flushInput;
    bool eof;
    CharEntry *lines;
    size_t lineEnd;
    size_t lineCount;
    size_t lineCursor;
    size_t maxLines;
    char *input;
    size_t inputStart;
    size_t inputCount;
    size_t maxInput;
    WaitHandle *handle;
} ConsoleView;

static CharEntry (*video)[25][80] = (void*)&__vga;
static ConsoleView views[MAX_VIEWS];
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
    view.lines = kmalloc(bufferPages * PAGE_SIZE);
    view.maxLines = (bufferPages * PAGE_SIZE) / (view.width * sizeof(CharEntry));
    for(int i = 0; i < (view.width * view.maxLines); i++)
    {
        view.lines[i] = (CharEntry){ .color.code = defaultColor, .symbol = 0 };
    }
    view.input = kmalloc(PAGE_SIZE);
    view.maxInput = PAGE_SIZE;
    view.handle = WaitHandle_Create();

    for(int i = startY; i < startY + height; i++)
        if(i >= 0 && i < 25)
        {
            if(startX-1 >= 0)
                (*video)[i][startX-1].symbol = (char)186;
            if(startX+width < 80)
                (*video)[i][startX+width].symbol = (char)186;
        }
    for(int j = startX; j < startX + width; j++)
        if(j >= 0 && j < 80)
        {
            if(startY-1 >= 0)
                (*video)[startY-1][j].symbol = (char)205;
            if(startY+height < 25)
                (*video)[startY+height][j].symbol = (char)205;
        }
    if(startX-1 >= 0)
    {
        if(startY-1 >= 0)
            (*video)[startY-1][startX-1].symbol = (char)201;
        if(startY+height < 25)
            (*video)[startY+height][startX-1].symbol = (char)200;
    }
    if(startX+width < 80)
    {
        if(startY-1 >= 0)
            (*video)[startY-1][startX+width].symbol = (char)187;
        if(startY+height < 25)
            (*video)[startY+height][startX+width].symbol = (char)188;
    }
    
    views[id] = view;

    return id;
}

void update_cursor(int x, int y)
{
	uint16_t pos = y * 80 + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void viewflush(int id)
{
    ConsoleView view = views[id];
    const CharEntry (*buf)[view.maxLines][view.width] = (void*)view.lines;

    int lines = (view.height < view.lineCount) ? view.height : view.lineCount;
    int outY = view.startY, bufY = view.lineEnd - lines + 1;
    for(int i = 0; i < lines; i++)
    {
        if(outY >= 25) outY -= 25;
        if(outY < 0) outY += 25;
        if(bufY >= 25) bufY -= 25;
        if(bufY < 0) bufY += 25;

        for(int j = 0; j < view.width; j++)
            (*video)[outY][view.startX+j] = (*buf)[bufY][j];
        outY++;
        bufY++;
    }
    if(id == focusedView)
        update_cursor(view.startX + view.lineCursor, outY-1);
}

void viewLF(int id)
{
    ConsoleView *view = &views[id];
    if(view->lines == NULL)
        return;
    CharEntry (*buf)[view->maxLines][view->width] = (void*)view->lines;
    
    view->lineEnd++;
    view->lineCursor = 0;
    view->lineCount++;
    if(view->lineCount >= view->maxLines)
            view->lineCount = view->maxLines;
    if(view->lineEnd >= view->maxLines)
        view->lineEnd %= view->maxLines;
    
    for(int j = 0; j < view->width; j++)
        (*buf)[view->lineEnd][j] = (CharEntry){ .color.code = defaultColor, .symbol = 0 };
}

int viewWrite(int id, const char *text, size_t n)
{
    ConsoleView *view = &views[id];
    if(view->lines == NULL)
        return 0;
    CharEntry (*buf)[view->maxLines][view->width] = (void*)view->lines;

    for(int i = 0; i < n; i++, text++)
    {
        if(view->lineCursor >= view->width)
            viewLF(id);
        
        unsigned char c = *text;
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
                case '\b':
                    if(view->lineCursor == 0)
                    {
                        if(view->lineCount > 0)
                        {
                            view->lineCursor = view->width - 1;
                            if(view->lineEnd == 0)
                                view->lineEnd = view->maxLines - 1;
                            else
                                view->lineEnd--;
                            view->lineCount--;
                        }
                    }
                    else
                        view->lineCursor--;
                    (*buf)[view->lineEnd][view->lineCursor].symbol = 0;
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
    static int canDelete = 0;

    if(c == '\b')
    {
        if(canDelete > 0)
        {
            view->inputCount--;
            canDelete--;
            viewWrite(focusedView, &c, 1);
        }
    }
    else if(c == '\e')
    {
        char buf[canDelete];
        memset(buf, '\b', sizeof(buf));
        viewWrite(focusedView, buf, sizeof(buf));
        view->inputCount -= canDelete;
        canDelete = 0;
    }
    else if(view->inputCount < view->maxInput)
    {
        canDelete++;
        view->input[(view->inputStart + view->inputCount++) % view->maxInput] = c;
        if(c == '\n')
        {
            canDelete=0;
            view->flushInput = true;
        }
        viewWrite(focusedView, &c, 1);
        releaseOne(view->handle);
    }
}

int inputBufferRead(int id, char *dest, size_t count)
{
    ProcessDescriptor *pd = currentProcess();
    ConsoleView *view = &views[pd->tty];
    int i;
    
    if(!pd->foreground)
        return 0;
    if(view->input == NULL)
        return 0;
    bool done = false;
    if(view->eof && !view->flushInput)
        done = true;
    for(i = 0; i < count && !done;)
    {
        waitEvent(view->inputCount >= (count - i) || view->inputCount >= view->maxInput || view->flushInput,
                    view->handle);

        if(view->inputCount > 0)
        {
            dest[i++] = view->input[view->inputStart++];
            if(view->inputStart == view->maxInput)
                view->inputStart = 0;
            view->inputCount--;
        }
        else if(view->flushInput)
        {
            done = true;
            view->flushInput = false;
        }
    }
    if(i == 0)
        view->eof = false;
    return i;
}


void changeTTY(int id)
{
    if(views[id].lines != NULL)
        focusedView = id;

    viewflush(id);
}

void ttyEOF()
{
    ConsoleView *view = &views[focusedView];

    view->eof = true;
    view->flushInput = true;
    releaseOne(view->handle);
}

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

static char buffer[64];

void ncWrite(const char *buf, size_t n)
{
	viewWrite(focusedView, buf, n);
}

void ncPrint(const char * string)
{
	int i = 0;
    while(string[i] != 0)
        i++;
    ncWrite(string, i);
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

void ncPrintPointer(uintptr_t value)
{
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