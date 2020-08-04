#include <console.h>
#include "keyboard.h"
#include <types.h>
#include "keyboard.h"
#include "naiveConsole.h"
#include "pid.h"
#include <lib.h>
#include "idtLoader.h"

extern u8 _getKeyCode();

typedef enum
{
    MOD_FLAG_SHIFT = 1,
    MOD_FLAG_CTRL = 2,
    MOD_FLAG_ALT = 4
} ModFlag;

typedef enum
{
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ENTER,
    KEY_CTRL,
    KEY_SHIFT,
    KEY_RIGHT_SHIFT,
    KEY_NUM_PROD,
    KEY_ALT,
    KEY_SPACE,
    KEY_CAPS_LOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_PAUSE,
    KEY_SCROLL_LOCK,
    KEY_NUM_7,
    KEY_NUM_8,
    KEY_NUM_9,
    KEY_NUM_MINUS,
    KEY_NUM_4,
    KEY_NUM_5,
    KEY_NUM_6,
    KEY_NUM_ADD,
    KEY_NUM_1,
    KEY_NUM_2,
    KEY_NUM_3,
    KEY_NUM_0,
    KEY_NUM_DEL,
    KEY_SYS_REQ,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
} Key;

static u8 scanCodes[256] = 
{
    [KEY_ESC] = 0x01,
    [KEY_BACKSPACE] = 0x0e,
    [KEY_TAB] = 0x0f,
    [KEY_ENTER] = 0x1c,
    [KEY_CTRL] = 0x1d,
    [KEY_SHIFT] = 0x2a,
    [KEY_RIGHT_SHIFT] = 0x36,
    [KEY_NUM_PROD] = 0x37,
    [KEY_ALT] = 0x38,
    [KEY_SPACE] = 0x39,
    [KEY_CAPS_LOCK] = 0x3a,
    [KEY_F1] = 0x3b,
    [KEY_F2] = 0x3c,
    [KEY_F3] = 0x3d,
    [KEY_F4] = 0x3e,
    [KEY_F5] = 0x3f,
    [KEY_F6] = 0x40,
    [KEY_F7] = 0x41,
    [KEY_F8] = 0x42,
    [KEY_F9] = 0x43,
    [KEY_F10] = 0x44,
    [KEY_PAUSE] = 0x45,
    [KEY_SCROLL_LOCK] = 0x46,
    [KEY_NUM_7] = 0x47,
    [KEY_NUM_8] = 0x48,
    [KEY_NUM_9] = 0x49,
    [KEY_NUM_MINUS] = 0x4a,
    [KEY_NUM_4] = 0x4b,
    [KEY_NUM_5] = 0x4c,
    [KEY_NUM_6] = 0x4d,
    [KEY_NUM_ADD] = 0x4e,
    [KEY_NUM_1] = 0x4f,
    [KEY_NUM_2] = 0x50,
    [KEY_NUM_3] = 0x51,
    [KEY_NUM_0] = 0x52,
    [KEY_NUM_DEL] = 0x53,
    [KEY_SYS_REQ] = 0x54,
    [KEY_F11] = 0x57,
    [KEY_F12] = 0x58,
    [KEY_F13] = 0x7c,
    [KEY_F14] = 0x7d,
    [KEY_F15] = 0x7e,
    [KEY_F16] = 0x7f,
    [KEY_F17] = 0x80,
    [KEY_F18] = 0x81,
    [KEY_F19] = 0x82,
    [KEY_F20] = 0x83,
    [KEY_F21] = 0x84,
    [KEY_F22] = 0x85,
    [KEY_F23] = 0x86,
    [KEY_F24] = 0x87,
};

#pragma region charMap

#define NOP 0
#define BTAB 0
#define LCTR 0
#define LSH 0
#define RSH 0
#define DBG 0
#define LALT 0
#define SUSP 0
#define CLK 0
#define NLK 0
#define SLK 0
#define F(x) 0
#define S(x) 0
#define RALT 0
#define RCTR 0
#define NEXT 0
#define PREV 0
#define RBT 0
#define SPSC 0
#define PASTE 0



const char charMapOrig[256][8] = {
/*                                                         alt
 * scan                       cntrl          alt    alt   cntrl
 * code  base   shift  cntrl  shift   alt   shift  cntrl  shift    spcl flgs
 * ---------------------------------------------------------------------------
 */
    [0x00] = {  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
    [0x01] = { 0x1B,  0x1B,  0x1B,  0x1B,  0x1B,  0x1B,   DBG,  0x1B, },
    [0x0e] = { 0x08,  0x08,  0x7F,  0x7F,  0x08,  0x08,  0x7F,  0x7F, },
    [0x0f] = { 0x09,  BTAB,   NOP,   NOP,  0x09,  BTAB,   NOP,   NOP, },
    [0x1c] = {  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A, },
    [0x1d] = { LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR, },
    [0x2a] = {  LSH,   LSH,   LSH,   LSH,   LSH,   LSH,   LSH,   LSH, },
    [0x36] = {  RSH,   RSH,   RSH,   RSH,   RSH,   RSH,   RSH,   RSH, },
    [0x37] = {  '*',   '*',   '*',   '*',   '*',   '*',   '*',   '*', },
    [0x38] = { LALT,  LALT,  LALT,  LALT,  LALT,  LALT,  LALT,  LALT, },
    [0x39] = {  ' ',   ' ',  0x00,   ' ',   ' ',   ' ',  SUSP,   ' ', },
    [0x3a] = {  CLK,   CLK,   CLK,   CLK,   CLK,   CLK,   CLK,   CLK, },
    [0x45] = {  NLK,   NLK,   NLK,   NLK,   NLK,   NLK,   NLK,   NLK, },
    [0x46] = {  SLK,   SLK,   SLK,   SLK,   SLK,   SLK,   SLK,   SLK, },
    [0x47] = { F(49),  '7',   '7',   '7',   '7',   '7',   '7',   '7', },
    [0x48] = { F(50),  '8',   '8',   '8',   '8',   '8',   '8',   '8', },
    [0x49] = { F(51),  '9',   '9',   '9',   '9',   '9',   '9',   '9', },
    [0x4a] = { F(52),  '-',   '-',   '-',   '-',   '-',   '-',   '-', },
    [0x4b] = { F(53),  '4',   '4',   '4',   '4',   '4',   '4',   '4', },
    [0x4c] = { F(54),  '5',   '5',   '5',   '5',   '5',   '5',   '5', },
    [0x4d] = { F(55),  '6',   '6',   '6',   '6',   '6',   '6',   '6', },
    [0x4e] = { F(56),  '+',   '+',   '+',   '+',   '+',   '+',   '+', },
    [0x4f] = { F(57),  '1',   '1',   '1',   '1',   '1',   '1',   '1', },
    [0x50] = { F(58),  '2',   '2',   '2',   '2',   '2',   '2',   '2', },
    [0x51] = { F(59),  '3',   '3',   '3',   '3',   '3',   '3',   '3', },
    [0x52] = { F(60),  '0',   '0',   '0',   '0',   '0',   '0',   '0', },
    [0x53] = { 0x7F,   '.',   '.',   '.',   '.',   '.',   RBT,   RBT, },
    [0x54] = {  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
};

static char charMap[256][8];
static bool isDeadKey[256][8];

#define DEAD_KEY_TABLE_SIZE 16
#define DEAD_KEY_TABLE_CONV_SIZE 16
typedef struct
{
    uint8_t deadKey;
    uint8_t tableEntries;
    struct
    {
        uint8_t orig;
        uint8_t dest;
    } table[DEAD_KEY_TABLE_CONV_SIZE];
} DeadKeyEntry;
static DeadKeyEntry deadKeyTables[DEAD_KEY_TABLE_SIZE];
static uint8_t deadKeyCount = 0;
#pragma endregion charMap

RegisterStatus lastRegisterStatus;

static void saveregs(const RegisterStatus *registers)
{
    lastRegisterStatus = *registers;
}

#define RELEASED 0x80
#define SC_CTRL 0x1d
#define SC_SHIFT 0x2a
#define SC_ALT 0x38
#define SC_RIGHT_SHIFT 0x36
#define SC_CAPS_LOCK 0x3a

static u8 modStatus = 0;
static bool capsLock = false;

void loadLayout(const char *file)
{
    int fd = open(file, O_RDONLY);
    if(fd < 0)
    {
        ncPrint("File not found\n");
        ncPrintDec(-fd);
        return;
    }

    memcpy(charMap, charMapOrig, sizeof(charMapOrig));

    char magic[4];
    read(fd, magic, 4);
    char name[32];
    for(int i = 0; i < sizeof(name); i++)
    {
        read(fd, &name[i], 1);
        if(name[i] == 0)
            break;
    }
    uint8_t codeCount;
    read(fd, &codeCount, 1);
    memset(isDeadKey, 0, sizeof(isDeadKey));
    for(int i = 0; i < codeCount; i++)
    {
        uint8_t scanCode;
        read(fd, &scanCode, 1);
        uint8_t cap;
        read(fd, &cap, 1);
        uint8_t statesCount;
        read(fd, &statesCount, 1);
        for(int j = 0; j < statesCount; j++)
        {
            uint8_t shiftState;
            read(fd, &shiftState, 1);
            if(shiftState & 0x80)
            {
                shiftState &= ~0x80;
                isDeadKey[scanCode][shiftState] = true;
            }
            uint8_t ch;
            read(fd, &ch, 1);
            charMap[scanCode][shiftState] = ch;
        }
    }
    read(fd, &deadKeyCount, 1);
    for(int i = 0; i < deadKeyCount && i < DEAD_KEY_TABLE_SIZE; i++)
    {
        DeadKeyEntry *entry = &deadKeyTables[i];
        read(fd, &entry->deadKey, 1);
        read(fd, &entry->tableEntries, 1);
        for(int j = 0; j < entry->tableEntries && j < DEAD_KEY_TABLE_CONV_SIZE; j++)
        {
            read(fd, &entry->table[j].orig, 1);
            read(fd, &entry->table[j].dest, 1);
        }
    }
    close(fd);
}

void switchLayout()
{
    static char *layouts[] = { "layouts/US", "layouts/Latin American" };
    static uint8_t current = 1;
    static uint8_t layoutCount = sizeof(layouts) / sizeof(*layouts);

    current++;
    current %= layoutCount;

    loadLayout(layouts[current]);
}

static uint8_t dead = 0;
void keyboardHandler(RegisterStatus *registers)
{
    u8 code = _getKeyCode();
    switch(code)
    {
        case SC_CTRL:
            modStatus |= MOD_FLAG_CTRL;
            break;
        case SC_CTRL + RELEASED:
            modStatus &= ~MOD_FLAG_CTRL;
            break;
        case SC_ALT:
            modStatus |= MOD_FLAG_ALT;
            break;
        case SC_ALT + RELEASED:
            modStatus &= ~MOD_FLAG_ALT;
            break;
        case SC_SHIFT: case SC_RIGHT_SHIFT:
            modStatus = capsLock ? modStatus & ~MOD_FLAG_SHIFT : modStatus | MOD_FLAG_SHIFT;
            break;
        case SC_CAPS_LOCK:
            capsLock = !capsLock;   //break omitted on purpose
        case SC_SHIFT + RELEASED: case SC_RIGHT_SHIFT + RELEASED:
            modStatus = !capsLock ? modStatus & ~MOD_FLAG_SHIFT : modStatus | MOD_FLAG_SHIFT;
            break;
        default:
        {
            if(code >= RELEASED)
                break;
            
            uint8_t c = charMap[code][modStatus];
            if(dead != 0)
            {
                int i;
                for(i = 0; i < deadKeyCount; i++)
                {
                    if(deadKeyTables[i].deadKey == dead)
                        break;
                }
                if(i == deadKeyCount)
                    break;
                
                for(int j = 0; j < deadKeyTables[i].tableEntries; j++)
                {
                    if(deadKeyTables[i].table[j].orig == c)
                    {
                        c = deadKeyTables[i].table[j].dest;
                        break;
                    }
                }
                dead = 0;
            }
            else if(isDeadKey[code][modStatus])
            {
                dead = c;
                break;
            }

            // Special commands
            // Function keys
            if(code >= scanCodes[KEY_F1] && code <= scanCodes[KEY_F10])
            {
                int view = code - scanCodes[KEY_F1];
                //Clean interrupt before switch
                outb(0x20, 0x20);
                changeFocus(view);
            }
            // Ctrl + Alt + Supr
            else if(code == scanCodes[KEY_NUM_DEL] && (modStatus & MOD_FLAG_CTRL) && (modStatus & MOD_FLAG_ALT))
            {
                saveregs(registers);
            }
            // Alt + Space
            else if(code == scanCodes[KEY_SPACE] && (modStatus & MOD_FLAG_ALT))
            {
                switchLayout();
            }
            else if(c)
                inputBufferWrite(c);
            break;
        }
    }
}
