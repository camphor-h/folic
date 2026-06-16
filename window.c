#include "window.h"

static const char* toolBarWidgetNames[] = {" File ", " Edit ", " Help "};

static const FoToolBarWidgetOptionRaw toolBarFileOptions[] = {{"New  (C-n)", initTextAreaAndCreateNewFile}, {"Open (C-o)", statusBarLoadFile}, {"Save (C-s)", statusBarSaveToFile}, {"Save as", statusBarSaveAsFile}, {"Quit (C-q)", folicQuit}, {"Goto File Begin", gotoFileBegin}, {"Goto File End", gotoFileEnd}};
static const FoToolBarWidgetOptionRaw toolBarEditOptions[] = {{"Undo (C-z)", undoBehavior}, {"Redo (C-y)", redoBehavior}, {"Find (C-f)", findTargetText}, {"Replace (C-h)", replaceTargetText}, {"Selection", startSelection}, {"Select All (C-a)", textAreaSelectAll}};
static const FoToolBarWidgetOptionRaw toolBarHelpOptions[] = {{"Welcome", openWelcomePage}, {"About", openAboutPage}, {"Update Log", openUpdateLogPage}};

static const FoToolBarWidgetOptionRaw* toolBarWidgetOptionNames[] = {toolBarFileOptions, toolBarEditOptions, toolBarHelpOptions};

static const int toolBarWidgetOptionAmount[] = {sizeof(toolBarFileOptions) / sizeof(toolBarFileOptions[0]), sizeof(toolBarEditOptions) / sizeof(toolBarEditOptions[0]), sizeof(toolBarHelpOptions) / sizeof(toolBarHelpOptions[0])};

static const char newlineT_CR[] = "(Classic Mac CR)";
static const char newlineT_LF[] = "(Unix LF)";
static const char newlineT_CRLF[] = "(MS CRLF)";
FoCursor* createCursor(FoLine* cursorStartLine, int32_t startLinePos)
{
    FoCursor* cursor = malloc(sizeof(FoCursor));
    cursor->line = cursorStartLine;
    cursor->linePos = startLinePos;
    cursor->isVisible = true;
    cursor->lineNumber = 0;
    return cursor;
}
void freeCursor(FoCursor* cursor)
{
    free(cursor);
}
void cursorPosChange(FoCursor* cursor, cursorPosChangeSignal signal) //It only move cursor's logic position. Not display position
{
    //Now you will see is one of the most complex function in the folic project. (Up to 2025.11)
    switch (signal)
    {
        case CURSOR_MOVE_LEFT:
            if (cursor->linePos == 0) //when the cursor is in the first position of line, move to the prev position of prev line (if not the first line)
            {
                if (cursor->line->prev != NULL)
                {
                    cursor->line = cursor->line->prev;
                    cursor->linePos = strGetLength(cursor->line->lineString);
                    cursor->lineNumber--;
                }
            }
            else
            {
                cursor->linePos--;
            }
            break;
        case CURSOR_MOVE_RIGHT:
            if (cursor->linePos == strGetLength(cursor->line->lineString)) //when the cursor is in the prev position of line, move to the first position of prev line (if not the first line)
            {
                if (cursor->line->next != NULL)
                {
                    cursor->line = cursor->line->next;
                    cursor->linePos = 0;
                    cursor->lineNumber++;
                }
            }
            else
            {
                cursor->linePos++;
            }
            break;
        case CURSOR_MOVE_UP:
            if (cursor->line->prev != NULL)
            {
                cursor->line = cursor->line->prev;
                if (strGetLength(cursor->line->lineString) < cursor->linePos)
                {
                    cursor->linePos = strGetLength(cursor->line->lineString);
                }
                cursor->lineNumber--;
            }
            break;
        case CURSOR_MOVE_DOWN:
            if (cursor->line->next != NULL)
            {
                cursor->line = cursor->line->next;
                if (strGetLength(cursor->line->lineString) < cursor->linePos)
                {
                    cursor->linePos = strGetLength(cursor->line->lineString);
                }
                cursor->lineNumber++;
            }
            break;
    }
}
void cursorMoveToLine(FoTextArea* textArea, FoLine* line, int32_t pos)
{
    textArea->cursor->line = line;
    textArea->cursor->lineNumber = getLineNumberWithFirstLine(line, textArea->textSource->firstLine);
    textArea->cursor->linePos = pos;
    textArea->window->signal = RERENDER_ALL;
    textArea->lineNumbers->window->signal = RERENDER_ALL;
}
void cursorUpdate(FoTextArea* textArea)
{
    int lineNumberinWindow = getLineDistance(textArea->cursor->line, textArea->topLine);
    if (lineNumberinWindow >= textArea->window->h) //when the cursor is below the bottom line in window.
    {
        textArea->topLine = textArea->topLine->next;
        textArea->window->signal = RERENDER_CONTENT;
        return;
    }
    else if (lineNumberinWindow < 0) //when the cursor is above the top line in window.
    {
        textArea->topLine = textArea->topLine->prev;
        textArea->window->signal = RERENDER_CONTENT;
        return;
    }
    else //Normally, move cursor
    {
        move(textArea->window->y + lineNumberinWindow, textArea->window->x + strIndexToBufferCellPosX(textArea->cursor->line->lineString, textArea->cursor->linePos, textArea->scrollOffset));
    }
}
/*
void cursorPosChangeUpdate(FoTextArea* textArea, cursorPosChangeSignal signal)
{
    cursorPosChange(textArea->cursor, signal);
    cursorUpdate(textArea);
}
*/

FoWindow* createWindow(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0)
    {
        return NULL;
    }
    FoWindow* window = malloc(sizeof(FoWindow));
    window->win = newwin(h, w, y, x);
    if (window->win == NULL)
    {
        free(window);
        return NULL;
    }
    window->buffer = createBuffer(w, h);
    if (window->buffer == NULL)
    {
        delwin(window->win);
        free(window);
        return NULL;
    }
    window->signal = RERENDER_ALL;
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    return window;
}
void resizeWindow(FoWindow* window, int x, int y, int w, int h)
{
    if (window == NULL || w <= 0 || h <= 0)
    {
        return;
    }
    Buffer* oldBuffer = window->buffer;
    WINDOW* oldWin = window->win;

    Buffer* newBuffer = copyBuffer(oldBuffer, w, h);
    WINDOW* newWin = newwin(h, w, y, x);

    if (newBuffer == NULL || newWin == NULL)
    {
        freeBuffer(newBuffer);
        delwin(newWin);
        window->buffer = NULL;
        window->win = NULL;
        return;
    }

    freeBuffer(oldBuffer);
    delwin(oldWin);

    window->win = newWin;
    window->buffer = newBuffer;
    window->signal = RERENDER_ALL;
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
}
void freeWindow(FoWindow* window)
{
    if (window == NULL)
    {
        return;
    }
    delwin(window->win);
    if (window->buffer != NULL)
    {
        freeBuffer(window->buffer);
    }
    free(window);
}
void renderToConsole(FoWindow* window, FoConsole* console)
{
    for (int y = 0; y < window->h; y++)
    {
        for (int x = 0; x < window->w; x++)
        {
            if (isBufferCellEqual(&(window->buffer->bufferArea[y][x]), &(console->buffer->bufferArea[y + window->y][x + window->x])) == false) //if buffercell not equal
            {
                memcpy(&(console->buffer->bufferArea[y + window->y][x + window->x]), &(window->buffer->bufferArea[y][x]), sizeof(BufferCell));
            }
        }
    }
}



FoToolBarWidgetOption* createToolBarWidgetOption(const char* name, void (*operation)(FoConsole* console))
{
    FoToolBarWidgetOption* option = malloc(sizeof(FoToolBarWidgetOption));
    option->name = createAssignStr(name);
    option->operation = operation;
    return option;
}
void freeToolBarWidgetOption(FoToolBarWidgetOption* tbwOption)
{
    freeStr(tbwOption->name);
    free(tbwOption);
}

FoToolBarWidget* createToolBarWidget(const char* name, int optionAmount, const FoToolBarWidgetOptionRaw* optionRaw, int x, int y, int w, int h)
{
    FoToolBarWidget* tbWidget = malloc(sizeof(FoToolBarWidget));
    tbWidget->name = createAssignStr(name);
    tbWidget->options = createVec(sizeof(FoToolBarWidgetOption*));
    vecReserve(tbWidget->options, optionAmount);
    for (int i = 0; i < optionAmount; i++)
    {
        FoToolBarWidgetOption* temp = createToolBarWidgetOption(optionRaw[i].name, optionRaw[i].operation);
        vecPushBack(tbWidget->options, &temp);
    }
    tbWidget->window = createWindow(x, y, w, h);
    if (tbWidget->window == NULL)
    {
        free(tbWidget);
        return NULL;
    }

    int maxOptionlength = -1;
    for (int i = 0; i < vecGetSize(tbWidget->options); i++)
    {
        if (maxOptionlength < strGetLength((*(FoToolBarWidgetOption**)vecAt(tbWidget->options, i))->name))
        {
            maxOptionlength = strGetLength((*(FoToolBarWidgetOption**)vecAt(tbWidget->options, i))->name);
        }
    }
    if (maxOptionlength == -1)
    {
        return NULL;
    }
    tbWidget->listWindow = createWindow(tbWidget->window->x, tbWidget->window->y + 1, maxOptionlength + 2, vecGetSize(tbWidget->options) + 1);
    if (tbWidget->listWindow == NULL)
    {
        return NULL;
    }

    tbWidget->selectedOption = 0;

    return tbWidget;
}
void freeToolBarWidget(FoToolBarWidget* tbWidget)
{
    if (tbWidget == NULL) return;
    freeStr(tbWidget->name);
    for (int i = 0; i < vecGetSize(tbWidget->options); i++)
    {
        freeToolBarWidgetOption(*(FoToolBarWidgetOption**)vecAt(tbWidget->options, i));
    }
    freeVec(tbWidget->options);
    freeWindow(tbWidget->window);
    freeWindow(tbWidget->listWindow);
    free(tbWidget);
}
void toolBarWidgetRender(FoToolBarWidget* tbWidget)
{
    static char* vFrame = "|";
    static char* hFrame = "-";
    if (tbWidget == NULL)
    {
        return;
    }
    
    if (tbWidget->window == NULL)
    {
        return;
    }

    for (int y = 0; y < vecGetSize(tbWidget->options); y++)
    {
        clearBufferLine(tbWidget->listWindow->buffer, y);
        setBufferLineSpace(tbWidget->listWindow->buffer, y);
        renderCStrToBuffer(tbWidget->listWindow->buffer, 0, y, vFrame);
        renderStrToBuffer(tbWidget->listWindow->buffer, 1, y, (*(FoToolBarWidgetOption**)vecAt(tbWidget->options, y))->name);
        renderCStrToBuffer(tbWidget->listWindow->buffer, tbWidget->listWindow->w - 1, y, vFrame);
    }
    for (int x = 0; x < tbWidget->listWindow->w; x++)
    {
        renderCStrToBuffer(tbWidget->listWindow->buffer, x, tbWidget->listWindow->h - 1, hFrame);
    }

    //Set the selected one highlight -- Blue

    for (int x = 1; x < tbWidget->listWindow->w - 1; x++)
    {
        tbWidget->listWindow->buffer->bufferArea[tbWidget->selectedOption][x].colorPair = BUFFERCELL_COLOR_CYAN_AND_WHITE;
    }
}

/*
static void initToolBar(FoToolBar* toolBar)
{
    //I don't know how to elegantly process the construct of toolBarWidgetOption. So I write this awful function :(
    FoToolBarWidget* widgetFile = *(FoToolBarWidget**)vecAt(toolBar->widgets, 0);
    vecReserve(widgetFile->options, sizeof(toolBarFileOptions) / sizeof(toolBarFileOptions[0]));
    for (int i = 0; i < sizeof(toolBarFileOptions) / sizeof(toolBarFileOptions[0]); i++)
    {
        FoToolBarWidgetOption* temp = createToolBarWidgetOption(toolBarFileOptions[0], NULL);
        vecPushBack(widgetFile->options, &temp);
    }

    FoToolBarWidget* editWidget = *(FoToolBarWidget**)vecAt(toolBar->widgets, 1);
    vecReserve(editWidget->options, sizeof(toolBarEditOptions) / sizeof(toolBarEditOptions[0]));
    for (int i = 0; i < sizeof(toolBarEditOptions) / sizeof(toolBarEditOptions[0]); i++)
    {
        FoToolBarWidgetOption* temp = createToolBarWidgetOption(toolBarEditOptions[0], NULL);
        vecPushBack(editWidget->options, &temp);
    }

    FoToolBarWidget* helpWidget = *(FoToolBarWidget**)vecAt(toolBar->widgets, 2);
    vecReserve(helpWidget->options, sizeof(toolBarHelpOptions) / sizeof(toolBarHelpOptions[0]));
    for (int i = 0; i < sizeof(toolBarHelpOptions) / sizeof(toolBarHelpOptions[0]); i++)
    {
        FoToolBarWidgetOption* temp = createToolBarWidgetOption(toolBarHelpOptions[0], NULL);
        vecPushBack(helpWidget->options, &temp);
    }
}
*/

FoToolBar* createToolBar(int x, int y, int w, int h)
{
    FoToolBar* toolBar = malloc(sizeof(FoToolBar));
    toolBar->window = createWindow(x, y, w, h);
    toolBar->widgets = createVec(sizeof(FoToolBarWidget*));
    int usedWidth = 0;
    int widgetLength;
    vecReserve(toolBar->widgets, sizeof(toolBarWidgetNames) / sizeof(toolBarWidgetNames[0]));
    for (size_t i = 0; i < sizeof(toolBarWidgetNames) / sizeof(toolBarWidgetNames[0]); i++)
    {
        widgetLength = strlen(toolBarWidgetNames[i]) + 1; //1 for '|'
        FoToolBarWidget* widget = createToolBarWidget(toolBarWidgetNames[i], toolBarWidgetOptionAmount[i], toolBarWidgetOptionNames[i], usedWidth, 0, widgetLength, 1);
        vecPushBack(toolBar->widgets, &widget);
        usedWidth += widgetLength;
    }
    //initToolBar(toolBar);
    return toolBar;
}
void freeToolBar(FoToolBar* toolBar)
{
    freeWindow(toolBar->window);
    for (int i = 0; i < vecGetSize(toolBar->widgets); i++)
    {
        FoToolBarWidget* widget = *(FoToolBarWidget**)(FoToolBarWidget*)vecAt(toolBar->widgets, i);
        freeToolBarWidget(widget);
    }
    freeVec(toolBar->widgets);
    free(toolBar);
}
static void renderToolBarFrame(FoToolBar* toolBar)
{
    static char temp = '-';
    for (int x = 0; x < toolBar->window->w; x++)
    {
        renderCharToBuffer(toolBar->window->buffer, x, toolBar->window->h - 1, &temp, 1);
    }
}
static void _renderToolBar(FoToolBar* toolBar)
{
    int usedWidth = 0;
    int length;
    FoToolBarWidget* temp;
    for (int i = 0; i < vecGetSize(toolBar->widgets); i++)
    {
        temp = *(FoToolBarWidget**)vecAt(toolBar->widgets, i);
        length = strlen(strCstr(temp->name));
        renderCStrToBuffer(toolBar->window->buffer, usedWidth, 0, "|");
        usedWidth++;
        renderStrToBuffer(toolBar->window->buffer, usedWidth, 0, temp->name);
        usedWidth += length;
    }
    renderCStrToBuffer(toolBar->window->buffer, usedWidth, 0, "|");
}
void renderToolBar(FoToolBar* toolBar)
{
    if (toolBar->window->signal == NOTHING_TO_RENDER)
    {
        return;
    }
    switch (toolBar->window->signal)
    {
        case NOTHING_TO_RENDER:
            break;
        case RERENDER_ALL:
            renderToolBarFrame(toolBar);
            _renderToolBar(toolBar);
            break;
        case RERENDER_CONTENT:
            _renderToolBar(toolBar);
            //renderCStrToBuffer(toolBar->window->buffer, 0, 0, "|File(F)|Edit(E)|View(V)|Settings(S)|Help(H)|");
            break;
        case RERENDER_FRAME:
            renderToolBarFrame(toolBar);
            break;
    }
}

FoSelectionArea* createSelectionArea(FoLine* startLine_, int32_t startLineNumber_, int32_t startLinePos_)
{
    FoSelectionArea* selectionArea = malloc(sizeof(FoSelectionArea));
    selectionArea->anchorLine = startLine_;
    selectionArea->anchorLineNumber = startLineNumber_;
    selectionArea->anchorLinePos = startLinePos_;
    selectionArea->endLine = startLine_;
    selectionArea->endLineNumber = startLineNumber_;
    selectionArea->endLinePos = startLinePos_;
    selectionArea->selectedAmount = 0;
    return selectionArea;
}
/*
static void selectionAreaSwapStartAndEnd(FoSelectionArea* selectionArea)
{
    FoLine* tempLine = selectionArea->anchorLine;
    int32_t tempLineNumber = selectionArea->anchorLineNumber;
    int32_t tempLinePos = selectionArea->anchorLinePos;
    selectionArea->anchorLine = selectionArea->endLine;
    selectionArea->anchorLineNumber = selectionArea->endLineNumber;
    selectionArea->anchorLinePos = selectionArea->endLinePos;
    selectionArea->endLine = tempLine;
    selectionArea->endLineNumber = tempLineNumber;
    selectionArea->endLinePos = tempLinePos;
}
*/
void getSelectionAreaStartEndLine(FoLine** startLine, FoLine** endLine_,
                                         int32_t* startLineNumber, int32_t* endLineNumber_,
                                         int32_t* startLinePos, int32_t* endLinePos_,
                                         FoSelectionArea* selectionArea)
{
    if (selectionArea->anchorLineNumber < selectionArea->endLineNumber)
    {
        *startLine = selectionArea->anchorLine;
        *endLine_ = selectionArea->endLine;
        *startLineNumber = selectionArea->anchorLineNumber;
        *endLineNumber_ = selectionArea->endLineNumber;
        *startLinePos = selectionArea->anchorLinePos;
        *endLinePos_ = selectionArea->endLinePos;
    }
    else if (selectionArea->anchorLineNumber > selectionArea->endLineNumber)
    {
        *startLine = selectionArea->endLine;
        *endLine_ = selectionArea->anchorLine;
        *startLineNumber = selectionArea->endLineNumber;
        *endLineNumber_ = selectionArea->anchorLineNumber;
        *startLinePos = selectionArea->endLinePos;
        *endLinePos_ = selectionArea->anchorLinePos;
    }
    else
    {
        if (selectionArea->anchorLinePos <= selectionArea->endLinePos)
        {
            *startLine = selectionArea->anchorLine;
            *endLine_ = selectionArea->endLine;
            *startLineNumber = selectionArea->anchorLineNumber;
            *endLineNumber_ = selectionArea->endLineNumber;
            *startLinePos = selectionArea->anchorLinePos;
            *endLinePos_ = selectionArea->endLinePos;
        }
        else
        {
            *startLine = selectionArea->endLine;
            *endLine_ = selectionArea->anchorLine;
            *startLineNumber = selectionArea->endLineNumber;
            *endLineNumber_ = selectionArea->anchorLineNumber;
            *startLinePos = selectionArea->endLinePos;
            *endLinePos_ = selectionArea->anchorLinePos;
        }
    }
}
int32_t calculateSelectedChars(FoSelectionArea* selectionArea)
{
    FoLine* startLine;
    FoLine* endLine;
    int32_t startLineNumber;
    int32_t endLineNumber;
    int32_t startLinePos;
    int32_t endLinePos;
    getSelectionAreaStartEndLine(&startLine, &endLine, &startLineNumber, &endLineNumber,
                                 &startLinePos, &endLinePos, selectionArea);
    if (startLine == endLine)
    {
        return endLinePos - startLinePos;
    }
    int32_t sum = 0;
    int32_t firstLineLength = strGetLength(startLine->lineString);
    sum += firstLineLength - startLinePos;
    FoLine* curLine = startLine->next;
    while (curLine != endLine)
    {
        sum += strGetLength(curLine->lineString) + 1;
        curLine = curLine->next;
    }
    sum += endLinePos + 1;
    return sum;
}
FoString* getSelectionAreaStr(FoSelectionArea* selectionArea)
{
    FoLine* startLine;
    FoLine* endLine;
    int32_t startLineNumber;
    int32_t endLineNumber;
    int32_t startLinePos;
    int32_t endLinePos;
    getSelectionAreaStartEndLine(&startLine, &endLine, &startLineNumber, &endLineNumber,
                                 &startLinePos, &endLinePos, selectionArea);

    FoString* str = createStr();

    if (startLine == endLine)
    {
        //Single line: from startLinePos to endLinePos (exclusive)
        FoString* temp = strSubStr(startLine->lineString, startLinePos, endLinePos - 1);
        if (temp != NULL)
        {
            strAppendStr(str, temp);
            freeStr(temp);
        }
        return str;
    }

    //Multi-line case
    //First line: from startLinePos to end of line
    FoString* temp = strSubStr(startLine->lineString, startLinePos, strGetLength(startLine->lineString) - 1);
    if (temp != NULL)
    {
        strAppendStr(str, temp);
        freeStr(temp);
    }
    //Add newline after first line
    strPushBackAscii(str, '\n');

    //Middle lines: full line content with newline
    FoLine* curLine = startLine->next;
    while (curLine != endLine)
    {
        strAppendStr(str, curLine->lineString);
        strPushBackAscii(str, '\n');
        curLine = curLine->next;
    }

    //Last line: from beginning to endLinePos (exclusive)
    temp = strSubStr(endLine->lineString, 0, endLinePos - 1);
    if (temp != NULL)
    {
        strAppendStr(str, temp);
        freeStr(temp);
    }

    return str;
}
void updateSelectionAreaToCursor(FoSelectionArea* selectionArea, FoCursor* cursor)
{
    selectionArea->endLine = cursor->line;
    selectionArea->endLineNumber = cursor->lineNumber;
    selectionArea->endLinePos = cursor->linePos;
    selectionArea->selectedAmount = calculateSelectedChars(selectionArea);
}

FoTextArea* createTextArea(FoTextFile* textFile, int x, int y, int w, int h)
{
    FoTextArea* textArea = malloc(sizeof(FoTextArea));
    textArea->window = createWindow(x, y, w, h);
    if (textArea->window == NULL)
    {
        free(textArea);
        return NULL;
    }
    if (textFile == NULL)
    {
        textArea->textSource = createTextFile(NULL);
        textArea->topLine = textArea->textSource->firstLine;
    }
    else
    {
        textArea->topLine = textFile->firstLine;
        textArea->textSource = textFile;
    }
    textArea->cursor = createCursor(textArea->textSource->firstLine, 0);
    textArea->history = createHistoryStack();
    textArea->undo = createHistoryStack();
    textArea->scrollOffset = 0;
    textArea->lineNumbers = NULL;
    textArea->selectionArea = NULL;
    return textArea;
}
void freeTextArea(FoTextArea* textArea)
{
    if (textArea == NULL) return;
    freeWindow(textArea->window);
    freeLineNumbers(textArea->lineNumbers);
    freeSelectionArea(textArea->selectionArea);
    freeHistoryStack(textArea->history);
    freeHistoryStack(textArea->undo);
    free(textArea);
}
static void _renderSelectionArea(FoTextArea* textArea)
{
    FoSelectionArea* selectionArea = textArea->selectionArea;
    FoLine* startLine;
    FoLine* endLine;
    int32_t startLineNumber;
    int32_t endLineNumber;
    int32_t startLinePos;
    int32_t endLinePos;
    getSelectionAreaStartEndLine(&startLine, &endLine, &startLineNumber, &endLineNumber,
                                 &startLinePos, &endLinePos, selectionArea);

    int topLineNum = getLineNumberWithFirstLine(textArea->topLine, textArea->textSource->firstLine);
    int bottomLineNum = topLineNum + textArea->window->h - 1;

    if (endLineNumber < topLineNum || startLineNumber > bottomLineNum)
    {
        return;
    }

    int renderStartLine = (startLineNumber > topLineNum) ? startLineNumber : topLineNum;
    int renderEndLine = (endLineNumber < bottomLineNum) ? endLineNumber : bottomLineNum;

    FoLine* curLine = textArea->topLine;
    for (int windowY = 0; windowY < textArea->window->h && curLine != NULL; windowY++)
    {
        int currentLineNum = topLineNum + windowY;

        if (currentLineNum >= renderStartLine && currentLineNum <= renderEndLine)
        {
            int lineSelectStart = 0;
            int lineSelectEnd = strGetLength(curLine->lineString);

            if (currentLineNum == startLineNumber)
            {
                lineSelectStart = startLinePos;
            }

            if (currentLineNum == endLineNumber)
            {
                lineSelectEnd = endLinePos;
            }

            int renderStartIndex = scrollOffsetToStrIndex(curLine->lineString, textArea->scrollOffset);
            if (renderStartIndex == STRING_NPOS)
            {
                curLine = curLine->next;
                continue;
            }

            if (lineSelectStart < renderStartIndex)
            {
                lineSelectStart = renderStartIndex;
            }

            if (lineSelectStart >= lineSelectEnd)
            {
                curLine = curLine->next;
                continue;
            }

            int displayStart = 0;
            int displayEnd = 0;

            for (int i = 0; i < lineSelectStart && i < strGetLength(curLine->lineString); i++)
            {
                displayStart += getUtf8charDisplayWidth(strAt(curLine->lineString, i));
            }

            for (int i = 0; i < lineSelectEnd && i < strGetLength(curLine->lineString); i++)
            {
                displayEnd += getUtf8charDisplayWidth(strAt(curLine->lineString, i));
            }

            displayStart -= textArea->scrollOffset;
            displayEnd -= textArea->scrollOffset;

            if (displayEnd > 0 && displayStart < textArea->window->w)
            {
                int renderStartX = (displayStart > 0) ? displayStart : 0;
                int renderEndX = (displayEnd < textArea->window->w) ? displayEnd : textArea->window->w;

                for (int x = renderStartX; x < renderEndX; x++)
                {
                    if (x >= 0 && x < textArea->window->w)
                    {
                        textArea->window->buffer->bufferArea[windowY][x].colorPair = BUFFERCELL_COLOR_CYAN_AND_WHITE;
                    }
                }
            }
        }
        curLine = curLine->next;
    }
}
static void _renderTextArea(FoTextArea* textArea)
{
    FoLine* curLine = textArea->topLine;
    static FoString* tempStr = NULL;
    int index = 0;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    setBufferSpace(textArea->window->buffer);
    for (int i = 0; i < textArea->window->h; i++)
    {
        if (curLine == NULL)
        {
            break;
        }
        if (strGetLength(curLine->lineString) > 0)
        {
            index = scrollOffsetToStrIndex(curLine->lineString, textArea->scrollOffset);
            if (index != STRING_NPOS)
            {
                tempStr = strSubStr(curLine->lineString, index, strGetLength(curLine->lineString) - 1);
                renderStrToBuffer(textArea->window->buffer, 0, i, tempStr);
                strClear(tempStr);
            }
        }
        curLine = curLine->next;
    }
    
    if (textArea->selectionArea != NULL)
    {
        _renderSelectionArea(textArea);
        textArea->window->signal = RERENDER_CONTENT;
    }

    if (textArea->lineNumbers != NULL && textArea->lineNumbers->window->signal != RERENDER_ALL)
    {
        textArea->lineNumbers->window->signal = RERENDER_CONTENT;
        renderLineNumbers(textArea->lineNumbers, textArea);
    }
    strClear(tempStr);
}
static void moveTextAreaOffset(FoTextArea* textArea)
{
    if (textArea->cursor->line == NULL)
    {
        return;
    }
    FoString* lineStr = textArea->cursor->line->lineString;
    int cursorDisplayPos = 0;
    for (int i = 0; i < textArea->cursor->linePos && i < strGetLength(lineStr); i++)
    {
        cursorDisplayPos += getUtf8charDisplayWidth(strAt(lineStr, i));
    }
    
    int scrollOffset = textArea->scrollOffset;
    if (cursorDisplayPos - scrollOffset >= textArea->window->w)
    {
        textArea->scrollOffset = cursorDisplayPos - textArea->window->w + 1;
        textArea->window->signal = RERENDER_CONTENT;
    }
    else if (cursorDisplayPos < scrollOffset)
    {
        textArea->scrollOffset = cursorDisplayPos;
        textArea->window->signal = RERENDER_CONTENT;
    }

    if (textArea->scrollOffset < 0)
    {
        textArea->scrollOffset = 0;
    }
}
void renderTextArea(FoTextArea* textArea)
{
    moveTextAreaOffset(textArea);
    if (textArea->window->signal == RERENDER_CONTENT || textArea->window->signal == RERENDER_ALL)
    {
        _renderTextArea(textArea);
    }
}

FoLineNumbers* createLineNumbers(int x, int y, int w, int h)
{
    FoLineNumbers* lineNumbers = malloc(sizeof(FoLineNumbers));
    lineNumbers->window = createWindow(x, y, w, h);
    return lineNumbers;
}
void freeLineNumbers(FoLineNumbers* lineNumbers)
{
    freeWindow(lineNumbers->window);
    free(lineNumbers);
}
static void renderLineNumbersFrame(FoLineNumbers* lineNumbers)
{
    static char temp = '|';
    for (int y = 0; y < lineNumbers->window->h; y++)
    {
        renderCharToBuffer(lineNumbers->window->buffer, lineNumbers->window->w - 1, y, &temp, 1);
    }
}
static void _renderLineNumbers(FoLineNumbers* lineNumbers, FoTextArea* textArea)
{
    int topNum = getLineNumberWithFirstLine(textArea->topLine, textArea->textSource->firstLine) + 1;
    int curNum = topNum;
    int prevNum = topNum + lineNumbers->window->h - 1;
    int prevNumLength = getNumberLength(prevNum);
    if (prevNumLength != lineNumbers->window->w - 1) //1 for frame
    {
        int prevLineNumbersW = lineNumbers->window->w;
        resizeWindow(lineNumbers->window, lineNumbers->window->x, lineNumbers->window->y, prevNumLength + 1, lineNumbers->window->h);
        renderLineNumbersFrame(lineNumbers);

        resizeWindow(textArea->window, lineNumbers->window->w, lineNumbers->window->y, prevLineNumbersW + textArea->window->w - lineNumbers->window->w, textArea->window->h);
        textArea->window->signal = RERENDER_CONTENT;
    }
    FoLine* curLine = textArea->topLine;
    char buffer[12];
    //int numLength;
    memset(buffer, '\0', 12);
    for (int i = 0; i < lineNumbers->window->h; i++) //1 for frame
    {
        for (int j = 0; j < lineNumbers->window->w - 1; j++)
        {
            lineNumbers->window->buffer->bufferArea[i][j].data[0] = ' ';
        }
    }
    for (int i = 0; i < lineNumbers->window->h; i++)
    {
        if (curLine == NULL)
        {
            break;
        }
        sprintf(buffer, "%d", curNum);
        //numLength = strlen(buffer);
        renderCStrToBuffer(lineNumbers->window->buffer, 0, i, buffer);
        curLine = curLine->next;
        curNum++;
    }
}
void renderLineNumbers(FoLineNumbers* lineNumbers, FoTextArea* textArea)
{
    switch (lineNumbers->window->signal)
    {
        case NOTHING_TO_RENDER:
            break;
        case RERENDER_ALL:
            renderLineNumbersFrame(lineNumbers);
            _renderLineNumbers(lineNumbers, textArea);
            break;
        case RERENDER_CONTENT:
            _renderLineNumbers(lineNumbers, textArea);
            break;
        case RERENDER_FRAME:
            renderLineNumbersFrame(lineNumbers);
            break;
    }
}

FoStatusBar* createStatusBar(int x, int y, int w, int h)
{
    FoStatusBar* statusBar = malloc(sizeof(FoStatusBar));
    statusBar->window = createWindow(x, y, w, h);
    statusBar->message = NULL;
    setBufferSpace(statusBar->window->buffer);
    return statusBar;
}
void freeStatusBar(FoStatusBar* statusBar)
{
    freeWindow(statusBar->window);
    free(statusBar);
}
static void renderStatusBarFrame(FoStatusBar* statusBar)
{
    static char temp = '-';
    for (int x = 0; x < statusBar->window->w; x++)
    {
        renderCharToBuffer(statusBar->window->buffer, x, 0, &temp, 1);
    }
}
static void renderStatus(FoStatusBar* statusBar, FoTextArea* textArea)
{
    static char* statusCStr = NULL;
    if (statusCStr == NULL)
    {
        statusCStr = malloc(statusBar->window->w + 1);
    }
    memset(statusCStr, ' ', statusBar->window->w);
    int lengthOfFileName = strlen(textArea->textSource->fileName != NULL ? textArea->textSource->fileName : "[New File]");
    memcpy(statusCStr, textArea->textSource->fileName != NULL ? textArea->textSource->fileName : "[New File]", lengthOfFileName);
    if (textArea->textSource->isModified == true)
    {
        statusCStr[lengthOfFileName] = '*';
        lengthOfFileName++;
    }
    char const* newlineT = newlineT_LF;
    switch (textArea->textSource->newlineType)
    {
        case NEWLINE_CR:
            newlineT = newlineT_CR;
            break;
        case NEWLINE_LF:
            newlineT = newlineT_LF;
            break;
        case NEWLINE_CRLF:
            newlineT = newlineT_CRLF;
            break;
        case NEWLINE_MIXED:
            newlineT = newlineT_LF;
            break;
    }
    memcpy(statusCStr + lengthOfFileName + 1, newlineT, strlen(newlineT));
    int lineNum = getLineNumberWithFirstLine(textArea->cursor->line, textArea->textSource->firstLine) + 1;
    int columnNum = textArea->cursor->linePos + 1;
    int renderStartPos = (statusBar->window->w) - getNumberLength(columnNum) - getNumberLength(lineNum) - (sizeof("Line:") - 1) - (sizeof("Column:") - 1) - (sizeof("   ") - 1);
    if (renderStartPos < 0)
    {
        renderStartPos = 0;
    }
    if (textArea->selectionArea != NULL)
    {
        int selectedNum = textArea->selectionArea->selectedAmount;
        renderStartPos -= (sizeof("(Selected:)") - 1);
        renderStartPos -= getNumberLengthZeroSafe(selectedNum);
        if (renderStartPos < 0)
        {
            renderStartPos = 0;
        }
        sprintf(statusCStr + renderStartPos, "Line:%d   Column:%d(Selected:%d)", lineNum, columnNum, selectedNum);
    }
    else
    {
        sprintf(statusCStr + renderStartPos, "Line:%d   Column:%d", lineNum, columnNum);
    }
    renderCStrToBuffer(statusBar->window->buffer, 0, 1, statusCStr);
}
static inline void renderStatusMessage(FoStatusBar* statusBar)
{
    renderStrToBuffer(statusBar->window->buffer, 0, 2, statusBar->message->content);
}
void renderStatusBar(FoStatusBar* statusBar, FoTextArea* textArea)
{
    if (statusBar->message != NULL)
    {
        updateMessageTime(statusBar->message);
        if (statusBar->message->remainTime != 0 && statusBar->message->isRendered == false)
        {
            setBufferLineSpace(statusBar->window->buffer, 2);
            renderStatusMessage(statusBar);
            statusBar->message->isRendered = true;
        }
        else if (statusBar->message->remainTime == 0)
        {
            setBufferLineSpace(statusBar->window->buffer, 2);
            freeMessage(statusBar->message);
            statusBar->message = NULL;
        }
    }
    switch (statusBar->window->signal)
    {
        case NOTHING_TO_RENDER:
            break;
        case RERENDER_FRAME:
            renderStatusBarFrame(statusBar);
            break;
        case RERENDER_CONTENT:
            renderStatus(statusBar, textArea);
            break;
        case RERENDER_ALL:
            renderStatusBarFrame(statusBar);
            renderStatus(statusBar, textArea);
            break;
    }
}