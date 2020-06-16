#include <console.h>
#include "keyboard.h"
#include <types.h>
#include "keyboard.h"
#include "naiveConsole.h"
#include "pid.h"

extern u8 _getKeyCode();

typedef enum
{
    KEYTYPE_SYMBOL = 0,
    KEYTYPE_MOD,
    KEYTYPE_CMD,
} KeyType;

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

static KeyType keyTypes[256] = 
{
    [KEY_CTRL] = KEYTYPE_MOD,
    [KEY_SHIFT] = KEYTYPE_MOD,
    [KEY_ALT] = KEYTYPE_MOD,
    [KEY_RIGHT_SHIFT] = KEYTYPE_MOD,
};

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



static char charMap[256][8] = {
/*                                                         alt
 * scan                       cntrl          alt    alt   cntrl
 * code  base   shift  cntrl  shift   alt   shift  cntrl  shift    spcl flgs
 * ---------------------------------------------------------------------------
 */
/*00*/{  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
/*01*/{ 0x1B,  0x1B,  0x1B,  0x1B,  0x1B,  0x1B,   DBG,  0x1B, },
/*02*/{  '1',   '!',   NOP,   NOP,   '1',   '!',   NOP,   NOP, },
/*03*/{  '2',   '@',  0x00,  0x00,   '2',   '@',  0x00,  0x00, },
/*04*/{  '3',   '#',   NOP,   NOP,   '3',   '#',   NOP,   NOP, },
/*05*/{  '4',   '$',   NOP,   NOP,   '4',   '$',   NOP,   NOP, },
/*06*/{  '5',   '%',   NOP,   NOP,   '5',   '%',   NOP,   NOP, },
/*07*/{  '6',   '^',  0x1E,  0x1E,   '6',   '^',  0x1E,  0x1E, },
/*08*/{  '7',   '&',   NOP,   NOP,   '7',   '&',   NOP,   NOP, },
/*09*/{  '8',   '*',   NOP,   NOP,   '8',   '*',   NOP,   NOP, },
/*0a*/{  '9',   '(',   NOP,   NOP,   '9',   '(',   NOP,   NOP, },
/*0b*/{  '0',   ')',   NOP,   NOP,   '0',   ')',   NOP,   NOP, },
/*0c*/{  '-',   '_',  0x1F,  0x1F,   '-',   '_',  0x1F,  0x1F, },
/*0d*/{  '=',   '+',   NOP,   NOP,   '=',   '+',   NOP,   NOP, },
/*0e*/{ 0x08,  0x08,  0x7F,  0x7F,  0x08,  0x08,  0x7F,  0x7F, },
/*0f*/{ 0x09,  BTAB,   NOP,   NOP,  0x09,  BTAB,   NOP,   NOP, },
/*10*/{  'q',   'Q',  0x11,  0x11,   'q',   'Q',  0x11,  0x11, },
/*11*/{  'w',   'W',  0x17,  0x17,   'w',   'W',  0x17,  0x17, },
/*12*/{  'e',   'E',  0x05,  0x05,   'e',   'E',  0x05,  0x05, },
/*13*/{  'r',   'R',  0x12,  0x12,   'r',   'R',  0x12,  0x12, },
/*14*/{  't',   'T',  0x14,  0x14,   't',   'T',  0x14,  0x14, },
/*15*/{  'y',   'Y',  0x19,  0x19,   'y',   'Y',  0x19,  0x19, },
/*16*/{  'u',   'U',  0x15,  0x15,   'u',   'U',  0x15,  0x15, },
/*17*/{  'i',   'I',  0x09,  0x09,   'i',   'I',  0x09,  0x09, },
/*18*/{  'o',   'O',  0x0F,  0x0F,   'o',   'O',  0x0F,  0x0F, },
/*19*/{  'p',   'P',  0x10,  0x10,   'p',   'P',  0x10,  0x10, },
/*1a*/{  '[',   '{',  0x1B,  0x1B,   '[',   '{',  0x1B,  0x1B, },
/*1b*/{  ']',   '}',  0x1D,  0x1D,   ']',   '}',  0x1D,  0x1D, },
/*1c*/{  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A, },
/*1d*/{ LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR,  LCTR, },
/*1e*/{  'a',   'A',  0x01,  0x01,   'a',   'A',  0x01,  0x01, },
/*1f*/{  's',   'S',  0x13,  0x13,   's',   'S',  0x13,  0x13, },
/*20*/{  'd',   'D',  0x04,  0x04,   'd',   'D',  0x04,  0x04, },
/*21*/{  'f',   'F',  0x06,  0x06,   'f',   'F',  0x06,  0x06, },
/*22*/{  'g',   'G',  0x07,  0x07,   'g',   'G',  0x07,  0x07, },
/*23*/{  'h',   'H',  0x08,  0x08,   'h',   'H',  0x08,  0x08, },
/*24*/{  'j',   'J',  0x0A,  0x0A,   'j',   'J',  0x0A,  0x0A, },
/*25*/{  'k',   'K',  0x0B,  0x0B,   'k',   'K',  0x0B,  0x0B, },
/*26*/{  'l',   'L',  0x0C,  0x0C,   'l',   'L',  0x0C,  0x0C, },
/*27*/{  ';',   ':',   NOP,   NOP,   ';',   ':',   NOP,   NOP, },
/*28*/{ '\'',   '"',   NOP,   NOP,  '\'',   '"',   NOP,   NOP, },
/*29*/{  '`',   '~',   NOP,   NOP,   '`',   '~',   NOP,   NOP, },
/*2a*/{  LSH,   LSH,   LSH,   LSH,   LSH,   LSH,   LSH,   LSH, },
/*2b*/{ '\\',   '|',  0x1C,  0x1C,  '\\',   '|',  0x1C,  0x1C, },
/*2c*/{  'z',   'Z',  0x1A,  0x1A,   'z',   'Z',  0x1A,  0x1A, },
/*2d*/{  'x',   'X',  0x18,  0x18,   'x',   'X',  0x18,  0x18, },
/*2e*/{  'c',   'C',  0x03,  0x03,   'c',   'C',  0x03,  0x03, },
/*2f*/{  'v',   'V',  0x16,  0x16,   'v',   'V',  0x16,  0x16, },
/*30*/{  'b',   'B',  0x02,  0x02,   'b',   'B',  0x02,  0x02, },
/*31*/{  'n',   'N',  0x0E,  0x0E,   'n',   'N',  0x0E,  0x0E, },
/*32*/{  'm',   'M',  0x0D,  0x0D,   'm',   'M',  0x0D,  0x0D, },
/*33*/{  ',',   '<',   NOP,   NOP,   ',',   '<',   NOP,   NOP, },
/*34*/{  '.',   '>',   NOP,   NOP,   '.',   '>',   NOP,   NOP, },
/*35*/{  '/',   '?',   NOP,   NOP,   '/',   '?',   NOP,   NOP, },
/*36*/{  RSH,   RSH,   RSH,   RSH,   RSH,   RSH,   RSH,   RSH, },
/*37*/{  '*',   '*',   '*',   '*',   '*',   '*',   '*',   '*', },
/*38*/{ LALT,  LALT,  LALT,  LALT,  LALT,  LALT,  LALT,  LALT, },
/*39*/{  ' ',   ' ',  0x00,   ' ',   ' ',   ' ',  SUSP,   ' ', },
/*3a*/{  CLK,   CLK,   CLK,   CLK,   CLK,   CLK,   CLK,   CLK, },
/*3b*/{ F( 1), F(13), F(25), F(37), S( 1), S(11), S( 1), S(11),},
/*3c*/{ F( 2), F(14), F(26), F(38), S( 2), S(12), S( 2), S(12),},
/*3d*/{ F( 3), F(15), F(27), F(39), S( 3), S(13), S( 3), S(13),},
/*3e*/{ F( 4), F(16), F(28), F(40), S( 4), S(14), S( 4), S(14),},
/*3f*/{ F( 5), F(17), F(29), F(41), S( 5), S(15), S( 5), S(15),},
/*40*/{ F( 6), F(18), F(30), F(42), S( 6), S(16), S( 6), S(16),},
/*41*/{ F( 7), F(19), F(31), F(43), S( 7), S( 7), S( 7), S( 7),},
/*42*/{ F( 8), F(20), F(32), F(44), S( 8), S( 8), S( 8), S( 8),},
/*43*/{ F( 9), F(21), F(33), F(45), S( 9), S( 9), S( 9), S( 9),},
/*44*/{ F(10), F(22), F(34), F(46), S(10), S(10), S(10), S(10),},
/*45*/{  NLK,   NLK,   NLK,   NLK,   NLK,   NLK,   NLK,   NLK, },
/*46*/{  SLK,   SLK,   SLK,   SLK,   SLK,   SLK,   SLK,   SLK, },
/*47*/{ F(49),  '7',   '7',   '7',   '7',   '7',   '7',   '7', },
/*48*/{ F(50),  '8',   '8',   '8',   '8',   '8',   '8',   '8', },
/*49*/{ F(51),  '9',   '9',   '9',   '9',   '9',   '9',   '9', },
/*4a*/{ F(52),  '-',   '-',   '-',   '-',   '-',   '-',   '-', },
/*4b*/{ F(53),  '4',   '4',   '4',   '4',   '4',   '4',   '4', },
/*4c*/{ F(54),  '5',   '5',   '5',   '5',   '5',   '5',   '5', },
/*4d*/{ F(55),  '6',   '6',   '6',   '6',   '6',   '6',   '6', },
/*4e*/{ F(56),  '+',   '+',   '+',   '+',   '+',   '+',   '+', },
/*4f*/{ F(57),  '1',   '1',   '1',   '1',   '1',   '1',   '1', },
/*50*/{ F(58),  '2',   '2',   '2',   '2',   '2',   '2',   '2', },
/*51*/{ F(59),  '3',   '3',   '3',   '3',   '3',   '3',   '3', },
/*52*/{ F(60),  '0',   '0',   '0',   '0',   '0',   '0',   '0', },
/*53*/{ 0x7F,   '.',   '.',   '.',   '.',   '.',   RBT,   RBT, },
/*54*/{  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
/*55*/{  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
/*56*/{  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
/*57*/{ F(11), F(23), F(35), F(47), S(11), S(11), S(11), S(11),},
/*58*/{ F(12), F(24), F(36), F(48), S(12), S(12), S(12), S(12),},
/*59*/{ 0x0D,  0x0D,  0x0A,  0x0A,  0x0D,  0x0D,  0x0A,  0x0A, },
/*5a*/{ RCTR,  RCTR,  RCTR,  RCTR,  RCTR,  RCTR,  RCTR,  RCTR, },
/*5b*/{  '/',   '/',   '/',   '/',   '/',   '/',   '/',   '/', },
/*5c*/{ NEXT,  PREV,   DBG,   DBG,   NOP,   NOP,   NOP,   NOP, },
/*5d*/{ RALT,  RALT,  RALT,  RALT,  RALT,  RALT,  RALT,  RALT, },
/*5e*/{ F(49), F(49), F(49), F(49), F(49), F(49), F(49), F(49),},
/*5f*/{ F(50), F(50), F(50), F(50), F(50), F(50), F(50), F(50),},
/*60*/{ F(51), F(51), F(51), F(51), F(51), F(51), F(51), F(51),},
/*61*/{ F(53), F(53), F(53), F(53), F(53), F(53), F(53), F(53),},
/*62*/{ F(55), F(55), F(55), F(55), F(55), F(55), F(55), F(55),},
/*63*/{ F(57), F(57), F(57), F(57), F(57), F(57), F(57), F(57),},
/*64*/{ F(58), F(58), F(58), F(58), F(58), F(58), F(58), F(58),},
/*65*/{ F(59), F(59), F(59), F(59), F(59), F(59), F(59), F(59),},
/*66*/{ F(60),PASTE,  F(60), F(60), F(60), F(60), F(60), F(60),},
/*67*/{ F(61), F(61), F(61), F(61), F(61), F(61),  RBT,  F(61),},
/*68*/{  SLK,  SPSC,   SLK,  SPSC,  SUSP,   NOP,  SUSP,   NOP, },
/*69*/{ F(62), F(62), F(62), F(62), F(62), F(62), F(62), F(62),},
/*6a*/{ F(63), F(63), F(63), F(63), F(63), F(63), F(63), F(63),},
/*6b*/{ F(64), F(64), F(64), F(64), F(64), F(64), F(64), F(64),},
/*6c*/{  NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP,   NOP, },
};
#pragma endregion charMap


#define RELEASED 0x80
#define SC_CTRL 0x1d
#define SC_SHIFT 0x2a
#define SC_ALT 0x38
#define SC_RIGHT_SHIFT 0x36
#define SC_CAPS_LOCK 0x3a

static u8 modStatus = 0;
static bool capsLock = false;

extern void cleanInt();

void keyboardHandler()
{
    u8 code = _getKeyCode();
    switch(code)
    {
        case SC_CTRL:
            modStatus |= 2;
            break;
        case SC_CTRL + RELEASED:
            modStatus &= ~2;
            break;
        case SC_ALT:
            modStatus |= 4;
            break;
        case SC_ALT + RELEASED:
            modStatus &= ~4;
            break;
        case SC_SHIFT: case SC_RIGHT_SHIFT:
            modStatus = capsLock ? modStatus & ~1 : modStatus | 1;
            break;
        case SC_CAPS_LOCK:
            capsLock = !capsLock;   //break omitted on purpose
        case SC_SHIFT + RELEASED: case SC_RIGHT_SHIFT + RELEASED:
            modStatus = !capsLock ? modStatus & ~1 : modStatus | 1;
            break;
        default:
        {
            char c = charMap[code][modStatus];
            if(c)
                inputBufferWrite(c);
            else if(code >= scanCodes[KEY_F1] && code <= scanCodes[KEY_F10])
            {
                int view = code - scanCodes[KEY_F1];
                int pid = getByView(view);
                cleanInt();
                contextSwitch(pid);
            }
            break;
        }
    }
}
