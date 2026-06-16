#ifndef __FOLIC_LINE__
#define __FOLIC_LINE__

#include "folic.h"
#include "window.h"
#include "types.h"

typedef struct FoLine
{
    struct FoLine* next;
    struct FoLine* prev;
    FoString* lineString;
}FoLine;

int getLineNumberWithCurrentLine(FoLine* line);
int getLineNumberWithFirstLine(FoLine* line, FoLine* firstLine);
int getLineNumberInWindow(FoTextArea* FoTextArea, FoLine* line);

int getLineDistance(FoLine* target, FoLine* source);

FoLine* getLineWithFirstLine(int index, FoLine* firstLine);
FoLine* getLastLine(FoLine* line);

void clearLine(FoLine* line);

int getLineIndentation(FoLine* line);

void inputToLine(FoLine* line, FoString* sourceStr, int32_t position);
void inputToCursorLine(FoTextArea* textArea, FoString* sourceStr);
FoString* backspaceToCursorLine(FoTextArea* textArea);
//if you want just input string to the line that the cursor situated. I recommended you use it. Because it will change the position of cursor automatically.

//Raw input layer - direct insert/delete without any special handling
void rawInsert(FoTextArea* textArea, FoString* sourceStr);
FoString* rawDelete(FoTextArea* textArea, int32_t count);

//Keyboard input layer - handles indentation and other special logic, then calls raw input
void keyboardInput(FoTextArea* textArea, FoString* sourceStr);

FoLine* createLine();
FoLine* lineAppend(FoLine* curNode);
void lineRemove(FoLine* curNode);
FoLine* lineInsertBelow(FoLine* prevLine);
FoLine* lineInsertAbove(FoLine* nextLine);
void freeAllLineWithFirstLine(FoLine* firstLine);
void freeAllLineWithCurrentLine(FoLine* curLine);
static inline int getNumberLength(int number)
{
    return (int)(log10(number) + 1);
}
static inline int getNumberLengthZeroSafe(int number)
{
    return (number != 0 ? (int)(log10(number) + 1) : 1);
}



typedef struct FoLineRaw //Only for the find and replace function.
{
    FoLine* line;
    int32_t pos;
}FoLineRaw;

FoLineRaw findWithCurrentLine(FoLine* line, const char* target, int32_t startIndex);

#endif