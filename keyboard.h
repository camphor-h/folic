#ifndef __FOLIC_KEYBOARD__
#define __FOLIC_KEYBOARD__
#include "folic.h"
typedef enum
{
    KEY_INFO_INPUT = 0,
    KEY_INFO_FUNCTION = 1
}KeyInfoType;
typedef enum
{
    INPUTSTR_APPEND = 0, //Append
    INPUTSTR_APPEND_SEND = 1,
    INPUTSTR_CLEAR = 2, //Discard the buffer string
    INPUTSTR_SEND = 3 //Send to inputToLine (and clear)
}InputStrSignal;

//Here is the virtual key mapping table and key info. They are legacy from the previous version of my code

typedef struct FoKeyInfo
{
    KeyInfoType infoType;
    uint32_t keyCode;
}FoKeyInfo;
typedef struct FoSingleInputKey
{
    int vkCode;
    char normal;
    char shifted;
}FoSingleInputKey;

static const FoSingleInputKey keyMapping[] =
{
    {0x30, '0', ')'}, {0x31, '1', '!'}, {0x32, '2', '@'},
    {0x33, '3', '#'}, {0x34, '4', '$'}, {0x35, '5', '%'},
    {0x36, '6', '^'}, {0x37, '7', '&'}, {0x38, '8', '*'},
    {0x39, '9', '('},

    {0x41, 'a', 'A'}, {0x42, 'b', 'B'}, {0x43, 'c', 'C'},
    {0x44, 'd', 'D'}, {0x45, 'e', 'E'}, {0x46, 'f', 'F'},
    {0x47, 'g', 'G'}, {0x48, 'h', 'H'}, {0x49, 'i', 'I'},
    {0x4A, 'j', 'J'}, {0x4B, 'k', 'K'}, {0x4C, 'l', 'L'},
    {0x4D, 'm', 'M'}, {0x4E, 'n', 'N'}, {0x4F, 'o', 'O'},
    {0x50, 'p', 'P'}, {0x51, 'q', 'Q'}, {0x52, 'r', 'R'},
    {0x53, 's', 'S'}, {0x54, 't', 'T'}, {0x55, 'u', 'U'},
    {0x56, 'v', 'V'}, {0x57, 'w', 'W'}, {0x58, 'x', 'X'},
    {0x59, 'y', 'Y'}, {0x5A, 'z', 'Z'},

    {0xBA, ';', ':'}, 
    {0xBB, '=', '+'},
    {0xBC, ',', '<'},
    {0xBD, '-', '_'},
    {0xBE, '.', '>'},
    {0xBF, '/', '?'},
    {0xC0, '`', '~'},
    {0xDB, '[', '{'},
    {0xDC, '\\', '|'},
    {0xDD, ']', '}'},
    {0xDE, '\'', '"'},
    {0x20, ' ', ' '}
};

void keyManage(FoConsole* console);
void singleKeyProcess(FoConsole* console, int key);
void inputTextProcess(FoConsole* console);
void prepareInputStr(FoConsole* console, InputStrSignal signal, FoString* source); //When you decide not to append str. Please send NULL as the source FoString.

void functionKey(FoConsole* console, int key);
#endif