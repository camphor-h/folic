#ifndef __FOLIC_BUFFER__
#define __FOLIC_BUFFER__

#include "folic.h"

#define BUFFERCELL_DATA_LENGTH 4

#define BUFFERCELL_COLOR_CYAN_AND_WHITE 1

//used to print color to console, but now we have ncurses. So I don't need it.

/*
#define BUFFERCELL_COLOR_BLACK           0
#define BUFFERCELL_COLOR_RED             1
#define BUFFERCELL_COLOR_GREEN           2
#define BUFFERCELL_COLOR_YELLOW          3
#define BUFFERCELL_COLOR_BLUE            4
#define BUFFERCELL_COLOR_MAGENTA         5
#define BUFFERCELL_COLOR_CYAN            6
#define BUFFERCELL_COLOR_WHITE           7
#define BUFFERCELL_COLOR_BRIGHT_BLACK    8
#define BUFFERCELL_COLOR_BRIGHT_RED      9
#define BUFFERCELL_COLOR_BRIGHT_GREEN    10
#define BUFFERCELL_COLOR_BRIGHT_YELLOW   11
#define BUFFERCELL_COLOR_BRIGHT_BLUE     12
#define BUFFERCELL_COLOR_BRIGHT_MAGENTA  13
#define BUFFERCELL_COLOR_BRIGHT_CYAN     14
#define BUFFERCELL_COLOR_BRIGHT_WHITE    15

#define BUFFERCELL_FORECOLOR(color)        "\033[38;5;" #color "m"
#define BUFFERCELL_BACKCOLOR(color)        "\033[48;5;" #color "m"
#define BUFFERCELL_RESET            "\033[0m"

#define BUFFERCELL_UNDERLINE      "\033[4m"
#define BUFFERCELL_INVERT_COLOR   "\033[7m"
*/

typedef struct BufferCell
{
    //Maybe I should use bitmask to rewrite it?
    bool isUnderlined;
    bool isInvertColor;
    bool isConnectedNext; //is connected with next BufferCell (used to output wide character)
    bool isConnectedLast; //is connected with next BufferCell (used to output wide character)
    //only the first BufferCell can get output
    //currently it only support 256 colors
    short colorPair;
    char data[BUFFERCELL_DATA_LENGTH];
}BufferCell;

typedef struct Buffer
{
    BufferCell** bufferArea;
    int w;
    int h;
}Buffer;

void initColorPairs();

Buffer* createBuffer(int w, int h);
void freeBuffer(Buffer* buffer);
Buffer* copyBuffer(Buffer* source, int w, int h);

void clearBuffer(Buffer* buffer);
void clearBufferLine(Buffer* buffer, int line);
void setBufferLineSpace(Buffer* buffer, int line);
void setBufferSpace(Buffer* buffer);

bool isBufferCellEqual(BufferCell* bufferCell1, BufferCell* bufferCell2);

void renderCharToBuffer(Buffer* buffer, int x, int y, char* char_, int length);
void renderCStrToBuffer(Buffer* buffer, int x, int y, char* cStr); //ASCII C str only!
void renderStrToBuffer(Buffer* buffer, int x, int y, FoString* str);
int32_t scrollOffsetToStrIndex(FoString* str, int scrollOffset);
int32_t scrollOffsetToDisplayStrIndex(FoString* str, int scrollOffset);
int strIndexToBufferCellPosX(FoString* str, int index, int scrollOffset);
#endif