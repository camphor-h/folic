#include "line.h"
int getLineNumberWithCurrentLine(FoLine* line)
{
    if (line == NULL)
    {
        return 0;
    }
    FoLine* curLine = line;
    int index = 0;
    while (curLine->prev != NULL) //rewind
    {
        curLine = curLine->prev;
        index++;
    }
    return index;
}
int getLineNumberWithFirstLine(FoLine* line, FoLine* firstLine)
{
    FoLine* curLine = firstLine;
    int index = 0;
    while (curLine != line)
    {
        if (curLine->next != NULL)
        {
            curLine = curLine->next;
            ++index;
        }
    }
    return index;
}

int getLineNumberInWindow(FoTextArea* FoTextArea, FoLine* line) //get current line in window
{
    FoLine* uLine = FoTextArea->topLine;
    for (int i = 0; i < FoTextArea->window->h; i++)
    {
        if (uLine == line)
        {
            return i; 
        }
        else if (uLine->next != NULL)
        {
            uLine = uLine->next;
        }
        else
        {
            break;
        }
    }
    return -1;
}

int getLineDistance(FoLine* target, FoLine* source)
{
    //I have to declare one thing: it's not similar to getLineNumber~() serias function, which are used to get line number (absolute line number)
    //but this is to get the "distance" from source line (relative line number)
    if (target == NULL || source == NULL)
    {
        return 0;
    }
    int indexUp = 0;
    FoLine* curLineUp = source;
    int indexDown = 0;
    FoLine* curLineDown = source;
    while (curLineUp != NULL || curLineDown != NULL)
    {
        if (curLineUp == target)
        {
            return indexUp;
        }
        else if (curLineDown == target)
        {
            return indexDown;
        }

        if (curLineUp != NULL)
        {
            curLineUp = curLineUp->prev;
            indexUp--;
        }

        if (curLineDown != NULL)
        {
            curLineDown = curLineDown->next;
            indexDown++;
        }
    }
    return 0;
}

FoLine* getLineWithFirstLine(int index, FoLine* firstLine)
{
    int count = 0;
    FoLine* curLine = firstLine;
    while (curLine != NULL)
    {
        if (count == index)
        {
            return curLine;
        }
        curLine = curLine->next;
        count++;
    }
    return NULL; //not found
}
FoLine* getLastLine(FoLine* line)
{
    FoLine* curLine = line;
    while (curLine->next != NULL)
    {
        curLine = curLine->next;
    }
    return curLine;
}

void clearLine(FoLine* line)
{
    freeStr(line->lineString);
    line->lineString = createStr();
}

int getLineIndentation(FoLine* line)
{
    int indentationCount = 0;
    for (int i = 0; i < strGetLength(line->lineString); i++)
    {
        if (strAt(line->lineString, i)->data[0] == ' ' || strAt(line->lineString, i)->data[0] == 9) //9 == Tab
        {
            indentationCount++;
        }
        else
        {
            break;
        }
    }
    return indentationCount;
}

void inputToLine(FoLine* line, FoString* sourceStr, int32_t position)
{
    //In the following code, The code will handle with situation differently
    //for the inputting position that at the end of the line. I will use strAppendStr()
    //for the inputting position that in the line. I will use strInsertStr()
    //To do it like this is to optimizie the program.
    //However, I haven't tested whether processing inputting differently can really make the code fast...
    if (position == strGetLength(line->lineString))
    {
        strAppendStr(line->lineString, sourceStr);
    }
    else
    {
        strInsertStr(line->lineString, position, sourceStr);
    }
}
void inputToCursorLine(FoTextArea* textArea, FoString* sourceStr) //input to line that the cursor currently situated.
{
    static FoString* tempStr = NULL;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    utf8char* temp;
    for (int i = 0; i < strGetLength(sourceStr); i++)
    {
        temp = strAt(sourceStr, i);
        if (temp->data[0] == '\n')
        {
            int indentation = getLineIndentation(textArea->cursor->line);
            if (strGetLength(tempStr) > 0)
            {
                inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
                textArea->cursor->linePos += strGetLength(tempStr);
                strClear(tempStr);
            }
            if (textArea->cursor->linePos < strGetLength(textArea->cursor->line->lineString))
            {
                FoString* remained = strSplit(textArea->cursor->line->lineString, textArea->cursor->linePos, strGetLength(textArea->cursor->line->lineString) - 1);
                strAppendStr(tempStr, remained);
                freeStr(remained);
            }
            textArea->cursor->line = lineInsertBelow(textArea->cursor->line);
            textArea->cursor->lineNumber++;
            textArea->cursor->linePos = 0;
            if (indentation > 0)
            {
                for (int i = 0; i < indentation; i++)
                {
                    strPushBackAscii(textArea->cursor->line->lineString, ' ');
                }
            }
            textArea->cursor->linePos = indentation;
            inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
            strClear(tempStr);
        }
        else
        {
            strPushBack(tempStr, *temp);
        }
    }
    inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
    textArea->cursor->linePos += strGetLength(tempStr);
    strClear(tempStr);
    textArea->window->signal = RERENDER_CONTENT;
}
FoString* backspaceToLine(FoLine* line, int32_t position) //need a temp FoString to receive it.
{
    if (position > strGetLength(line->lineString) || position == 0)
    {
        return NULL;
    }
    return strSplit(line->lineString, position - 1, position - 1);
}
FoString* backspaceToCursorLine(FoTextArea* textArea)
{
    static FoString* tempStr = NULL;
    static FoString* deleted = NULL;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    else
    {
        strClear(tempStr);
    }

    if (deleted == NULL)
    {
        deleted = createStr();
    }
    else
    {
        strClear(deleted);
    }

    if (textArea->cursor->linePos == 0 && textArea->cursor->line->prev != NULL)
    {
        int prevLineLength = strGetLength(textArea->cursor->line->prev->lineString);
        inputToLine(textArea->cursor->line->prev, textArea->cursor->line->lineString, prevLineLength);
        textArea->cursor->line = textArea->cursor->line->prev;
        textArea->cursor->lineNumber--;
        lineRemove(textArea->cursor->line->next);
        textArea->cursor->linePos = prevLineLength;
        strPushBackAscii(deleted, '\n');
        textArea->lineNumbers->window->signal = RERENDER_CONTENT;
    }
    else if (textArea->cursor->linePos > 0)
    {
        if (textArea->cursor->linePos >= 4 && strMatch(textArea->cursor->line->lineString, textArea->cursor->linePos - 4, textArea->cursor->linePos - 1, "    "))
        {
            tempStr = strSplit(textArea->cursor->line->lineString, textArea->cursor->linePos - 4, textArea->cursor->linePos - 1);
            strAppendStr(deleted, tempStr);
            strClear(tempStr);
            textArea->cursor->linePos -= 4;
        }
        else
        {
            tempStr = backspaceToLine(textArea->cursor->line, textArea->cursor->linePos);
            strAppendStr(deleted, tempStr);
            textArea->cursor->linePos--;
        }
    }
    textArea->window->signal = RERENDER_CONTENT;
    return deleted;
}

FoLine* createLine()
{
    //This is a newer function, so it didn't get much use.
    FoLine* newNode = (FoLine*)malloc(sizeof(FoLine));
    newNode->lineString = createStr();
    return newNode;
}
FoLine* lineAppend(FoLine* curNode) //if the curNode is NULL, it will create the head node of line.
{
    FoLine* newNode = (FoLine*)malloc(sizeof(FoLine));
    if (newNode == NULL)
    {
        exit(1);
    }
    newNode->lineString = createStr();
    newNode->next = NULL;
    if (curNode == NULL)
    {
        newNode->prev = NULL;
        return newNode;
    }
    newNode->prev = curNode;
    curNode->next = newNode;
    return newNode;
}
void lineRemove(FoLine* curNode)
{
    if (curNode == NULL)
    {
        return;
    }

    if (curNode->prev != NULL)
    {
        curNode->prev->next = curNode->next;
    }
    if (curNode->next != NULL)
    {
        curNode->next->prev = curNode->prev;
    }

    freeStr(curNode->lineString);
    free(curNode);
}
FoLine* lineInsertBelow(FoLine* prevLine)
{
    if (prevLine == NULL)
    {
        return lineAppend(prevLine);
    }
    FoLine* line = createLine();
    line->prev = prevLine;
    line->next = prevLine->next;
    prevLine->next = line;
    if (line->next != NULL)
    {
        line->next->prev = line;
    }
    return line;
}
FoLine* lineInsertAbove(FoLine* nextLine)
{
    if (nextLine == NULL)
    {
        return lineAppend(NULL);
    }
    FoLine* line = createLine();
    line->next = nextLine;
    line->prev = nextLine->prev;
    nextLine->prev = line;
    if (line->prev != NULL)
    {
        line->prev->next = line;
    }
    return line;
}
void freeAllLineWithFirstLine(FoLine* firstLine)
{
    FoLine* curLine = firstLine;
    while (curLine != NULL)
    {
        FoLine* next = curLine->next;
        if (curLine->lineString != NULL)
        {
            freeStr(curLine->lineString);
        }
        free(curLine);
        curLine = next;
    }
}
void freeAllLineWithCurrentLine(FoLine* curLine)
{
    if (curLine == NULL)
    {
        return;
    }
    while (curLine != NULL && curLine->prev != NULL)
    {
        curLine = curLine->prev;
    }
    freeAllLineWithFirstLine(curLine);
}

FoLineRaw findWithCurrentLine(FoLine* line, const char* target, int32_t startIndex)
{
    FoLine* curLine = line;
    int32_t pos = 0;
    while (curLine != NULL)
    {
        pos = strFindWithStartIndex(curLine->lineString, target, startIndex);
        if (pos != STRING_NPOS)
        {
            return (FoLineRaw){curLine, pos};
        }
        curLine = curLine->next;
        startIndex = 0;
    }
    return (FoLineRaw){NULL, 0};
}

//Raw input layer - direct insert without any special handling
void rawInsert(FoTextArea* textArea, FoString* sourceStr)
{
    if (sourceStr == NULL || strGetLength(sourceStr) == 0)
    {
        return;
    }
    
    static FoString* tempStr = NULL;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    else
    {
        strClear(tempStr);
    }
    
    utf8char* temp;
    for (int i = 0; i < strGetLength(sourceStr); i++)
    {
        temp = strAt(sourceStr, i);
        if (temp->data[0] == '\n')
        {
            if (strGetLength(tempStr) > 0)
            {
                inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
                textArea->cursor->linePos += strGetLength(tempStr);
                strClear(tempStr);
            }
            if (textArea->cursor->linePos < strGetLength(textArea->cursor->line->lineString))
            {
                FoString* remained = strSplit(textArea->cursor->line->lineString, textArea->cursor->linePos, strGetLength(textArea->cursor->line->lineString) - 1);
                strAppendStr(tempStr, remained);
                freeStr(remained);
            }
            textArea->cursor->line = lineInsertBelow(textArea->cursor->line);
            textArea->cursor->lineNumber++;
            textArea->cursor->linePos = 0;
            strClear(tempStr);
        }
        else
        {
            strPushBack(tempStr, *temp);
        }
    }
    inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
    textArea->cursor->linePos += strGetLength(tempStr);
    strClear(tempStr);
    textArea->window->signal = RERENDER_CONTENT;
}

//Raw delete layer - direct delete without any special handling
FoString* rawDelete(FoTextArea* textArea, int32_t count)
{
    if (count <= 0)
    {
        return NULL;
    }
    
    static FoString* deleted = NULL;
    if (deleted == NULL)
    {
        deleted = createStr();
    }
    else
    {
        strClear(deleted);
    }
    
    for (int32_t i = 0; i < count; i++)
    {
        if (textArea->cursor->linePos == 0 && textArea->cursor->line->prev != NULL)
        {
            int prevLineLength = strGetLength(textArea->cursor->line->prev->lineString);
            inputToLine(textArea->cursor->line->prev, textArea->cursor->line->lineString, prevLineLength);
            textArea->cursor->line = textArea->cursor->line->prev;
            textArea->cursor->lineNumber--;
            lineRemove(textArea->cursor->line->next);
            textArea->cursor->linePos = prevLineLength;
            strPushBackAscii(deleted, '\n');
            textArea->lineNumbers->window->signal = RERENDER_CONTENT;
        }
        else if (textArea->cursor->linePos > 0)
        {
            FoString* removed = backspaceToLine(textArea->cursor->line, textArea->cursor->linePos);
            if (removed != NULL)
            {
                strAppendStr(deleted, removed);
                freeStr(removed);
                textArea->cursor->linePos--;
            }
        }
    }
    
    strReverse(deleted);
    textArea->window->signal = RERENDER_CONTENT;
    return deleted;
}

//Keyboard input layer - handles indentation and other special logic
void keyboardInput(FoTextArea* textArea, FoString* sourceStr)
{
    if (sourceStr == NULL || strGetLength(sourceStr) == 0)
    {
        return;
    }

    static FoString* tempStr = NULL;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    else
    {
        strClear(tempStr);
    }

    utf8char* temp;
    for (int i = 0; i < strGetLength(sourceStr); i++)
    {
        temp = strAt(sourceStr, i);
        if (temp->data[0] == '\n')
        {
            int indentation = getLineIndentation(textArea->cursor->line);
            //Insert accumulated content before newline
            if (strGetLength(tempStr) > 0)
            {
                inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
                textArea->cursor->linePos += strGetLength(tempStr);
                strClear(tempStr);
            }
            //Split remaining content at cursor position
            if (textArea->cursor->linePos < strGetLength(textArea->cursor->line->lineString))
            {
                FoString* remained = strSplit(textArea->cursor->line->lineString, textArea->cursor->linePos, strGetLength(textArea->cursor->line->lineString) - 1);
                strAppendStr(tempStr, remained);
                freeStr(remained);
            }
            //Create new line below
            textArea->cursor->line = lineInsertBelow(textArea->cursor->line);
            textArea->cursor->lineNumber++;
            textArea->cursor->linePos = 0;
            //Apply indentation to new line
            if (indentation > 0)
            {
                for (int j = 0; j < indentation; j++)
                {
                    strPushBackAscii(textArea->cursor->line->lineString, ' ');
                }
            }
            textArea->cursor->linePos = indentation;
            //Insert remained content after indentation
            if (strGetLength(tempStr) > 0)
            {
                inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
            }
            strClear(tempStr);
        }
        else
        {
            char c = temp->data[0];
            strPushBack(tempStr, *temp);
            
            //Auto-close brackets and quotes
            char closeChar = '\0';
            if (c == '(') closeChar = ')';
            else if (c == '[') closeChar = ']';
            else if (c == '{') closeChar = '}';
            else if (c == '<') closeChar = '>';
            else if (c == '"') closeChar = '"';
            else if (c == '\'') closeChar = '\'';
            
            if (closeChar != '\0')
            {
                //Flush accumulated content first
                if (strGetLength(tempStr) > 0)
                {
                    inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
                    textArea->cursor->linePos += strGetLength(tempStr);
                    strClear(tempStr);
                }
                //Insert closing character
                strPushBackAscii(tempStr, closeChar);
                inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
                textArea->cursor->linePos += strGetLength(tempStr);
                //Move cursor back between the brackets
                textArea->cursor->linePos--;
                strClear(tempStr);
            }
        }
    }
    //Insert remaining accumulated content
    if (strGetLength(tempStr) > 0)
    {
        inputToLine(textArea->cursor->line, tempStr, textArea->cursor->linePos);
        textArea->cursor->linePos += strGetLength(tempStr);
        strClear(tempStr);
    }
    textArea->window->signal = RERENDER_CONTENT;
}