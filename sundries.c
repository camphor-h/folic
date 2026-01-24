#include "sundries.h"

#ifdef DOC_PATH
static const char* WELCOME_DOC_PATH = DOC_PATH "/welcome.txt";
static const char* ABOUT_DOC_PATH = DOC_PATH "/about.txt";
static const char* UPDATE_LOG_DOC_PATH = DOC_PATH "/update.txt";
#else
static const char* WELCOME_DOC_PATH = "./doc/welcome.txt";
static const char* ABOUT_DOC_PATH = "./doc/about.txt";
static const char* UPDATE_LOG_DOC_PATH = "./update.txt";
#endif

void statusBarSaveToFile(FoConsole* console)
{
    if (console->textArea->textSource->filePath == NULL) //New File
    {
        char* path = calloc(console->statusBar->window->w + 1, sizeof(char));
        mvwprintw(console->statusBar->window->win, 2, 0, "Please input the file path.");
        if (foGetline(console->statusBar->window, 3, 0, path, console->statusBar->window->w) == -1)
        {
            free(path);
            return;
        }
        setTextFilePath(console->textArea->textSource, path);
        free(path);
    }
    writeFile(console->textArea->textSource);
    sendMessageToStatusBar(console->statusBar, "----------File Saved----------", 5);
}
void statusBarLoadFile(FoConsole* console)
{
    char* path = calloc(console->statusBar->window->w + 1, sizeof(char));
    FILE* tryer;
    mvwprintw(console->statusBar->window->win, 2, 0, "Please input the file path.");
    if (foGetline(console->statusBar->window, 3, 0, path, console->statusBar->window->w) == -1)
    {
        free(path);
        return;
    }
    tryer = fopen(path, "r");
    if (tryer == NULL)
    {
        sendMessageToStatusBar(console->statusBar, "----------ERROR: No such file----------", 5);
    }
    else
    {
        fclose(tryer);
        freeTextFile(console->textArea->textSource);
        console->textArea->textSource = createTextFile(path);
        console->textArea->topLine = console->textArea->textSource->firstLine;
        console->textArea->scrollOffset = 0;
        cursorMoveToLine(console->textArea, console->textArea->textSource->firstLine, 0);
        clearHistoryStack(console->history);
        clearHistoryStack(console->undo);
    }
    setRerenderAll(console);
    free(path);
}
void openTargetFile(FoConsole* console, const char* path)
{
    if (console->textArea->textSource->isModified)
    {
        switch (statusBarYOrNCancel(console, "Save current file?"))
        {
            case SELECTION_YES:
                statusBarSaveToFile(console);
                break;
            case SELECTION_NO:
                break;
            case SELECTION_CANCEL:
                return;
        };
    }
    freeTextFile(console->textArea->textSource);
    console->textArea->textSource = createTextFile(path);
    console->textArea->topLine = console->textArea->textSource->firstLine;
    console->textArea->scrollOffset = 0;
    cursorMoveToLine(console->textArea, console->textArea->textSource->firstLine, 0);
    clearHistoryStack(console->history);
    clearHistoryStack(console->undo);
    setRerenderAll(console);
}
void statusBarSaveAsFile(FoConsole* console)
{
    char* path = calloc(console->statusBar->window->w + 1, sizeof(char));
    mvwprintw(console->statusBar->window->win, 2, 0, "Please input the file path.");
    if (foGetline(console->statusBar->window, 3, 0, path, console->statusBar->window->w) == -1)
    {
        free(path);
        return;
    }
    free(console->textArea->textSource->fileName);
    setTextFilePath(console->textArea->textSource, path);
    free(path);
    writeFile(console->textArea->textSource);
    sendMessageToStatusBar(console->statusBar, "----------File Saved----------", 5);
}

void initTextAreaAndCreateNewFile(FoConsole* console)
{
    openTargetFile(console, NULL);
}

void openWelcomePage(FoConsole* console)
{
    openTargetFile(console, WELCOME_DOC_PATH);
}

void openAboutPage(FoConsole* console)
{
    openTargetFile(console, ABOUT_DOC_PATH);
}
void openUpdateLogPage(FoConsole* console)
{
    openTargetFile(console, UPDATE_LOG_DOC_PATH);
}

void gotoTargetLine(FoConsole* console)
{
    int32_t lineNumberToGo;
    char* lineNumberStr = calloc(console->statusBar->window->w + 1, sizeof(char));
    mvwprintw(console->statusBar->window->win, 2, 0, "Please input the line Number you want to go to:");
    foGetline(console->statusBar->window, 3, 0, lineNumberStr, console->statusBar->window->w);
    lineNumberToGo = atoi(lineNumberStr);
    FoLine* lineToGo = getLineWithFirstLine(lineNumberToGo - 1, console->textArea->textSource->firstLine);
    if (lineToGo == NULL)
    {
        sendMessageToStatusBar(console->statusBar, "----------ERROR: No target line----------", 5);
    }
    else
    {
        console->textArea->topLine = lineToGo;
        cursorMoveToLine(console->textArea, lineToGo, 0);
    }
    setRerenderAll(console);
    free(lineNumberStr);
}

static inline void immediateCleanWindowLine(FoWindow* window, int line) //You should only use the function in statusBar control!!!
{
    mvwprintw(window->win, line, 0, "%*s", window->w, " ");
}

/*OK I admit that the two function above is very nasty. Honestly, I hope I */
void findTargetText(FoConsole* console)
{
    static FoString* tempStr = NULL;
    if (tempStr == NULL)
    {
        tempStr = createStr();
    }
    else
    {
        strClear(tempStr);
    }

    char* targetText = calloc(console->statusBar->window->w + 1, sizeof(char));
    mvwprintw(console->statusBar->window->win, 2, 0, "Find:");
    if (foGetline(console->statusBar->window, 3, 0, targetText, console->statusBar->window->w) == -1)
    {
        free(targetText);
        return;
    }
    strAssign(tempStr, targetText);
    FoLineRaw find;
    int tempKey;
    do
    {
        find = findWithCurrentLine(console->textArea->cursor->line, targetText, console->textArea->cursor->linePos);
        if (find.line == NULL)
        {
            immediateCleanWindowLine(console->statusBar->window, 2);
            mvwprintw(console->statusBar->window->win, 2, 0, "Find Nothing, press end to break");
            wrefresh(console->statusBar->window->win);
            while (getch() != 27);
            break;
        }
        else
        {
            console->textArea->topLine = find.line;
            cursorMoveToLine(console->textArea, find.line, find.pos + strGetLength(tempStr));
            cursorUpdate(console->textArea);
            console->textArea->window->signal = RERENDER_CONTENT;
            console->textArea->lineNumbers->window->signal = RERENDER_CONTENT;
            renderLineNumbers(console->textArea->lineNumbers, console->textArea);
            renderTextArea(console->textArea);
            printWindow(console->textArea->lineNumbers->window, false);
            printWindow(console->textArea->window, false);
            cursorUpdate(console->textArea);
            immediateCleanWindowLine(console->statusBar->window, 2);
            mvwprintw(console->statusBar->window->win, 2, 0, "Use Enter Key to find next one, or Esc to break.");
            wrefresh(console->statusBar->window->win);
            tempKey = getch();
            if (tempKey == '\n')
            {
                continue;
            }
            else if (tempKey == 27)
            {
                break;
            }
        }
    }while(find.line != NULL);
    
    free(targetText);
}

void replaceTargetText(FoConsole* console)
{
    static FoString* tempSourceStr = NULL;
    static FoString* tempTargetStr = NULL;
    if (tempSourceStr == NULL)
    {
        tempSourceStr = createStr();
    }
    else
    {
        strClear(tempSourceStr);
    }
    if (tempTargetStr == NULL)
    {
        tempTargetStr = createStr();
    }
    else
    {
        strClear(tempTargetStr);
    }

    char* sourceText = calloc(console->statusBar->window->w + 1, sizeof(char));
    char* targetText = calloc(console->statusBar->window->w + 1, sizeof(char));
    mvwprintw(console->statusBar->window->win, 2, 0, "Replace - Find:");
    if (foGetline(console->statusBar->window, 3, 0, sourceText, console->statusBar->window->w) == -1)
    {
        free(sourceText);
        return;
    }
    immediateCleanWindowLine(console->statusBar->window, 2);
    mvwprintw(console->statusBar->window->win, 2, 0, "Replace - Replace:");
    immediateCleanWindowLine(console->statusBar->window, 3);
    if (foGetline(console->statusBar->window, 3, 0, targetText, console->statusBar->window->w) == -1)
    {
        free(sourceText);
        free(targetText);
        return;
    }
    strAssign(tempSourceStr, sourceText);
    strAssign(tempTargetStr, targetText);
    FoLineRaw find;
    int tempKey;
    bool all = false;
    do
    {
        find = findWithCurrentLine(console->textArea->cursor->line, sourceText, console->textArea->cursor->linePos);
        if (find.line == NULL)
        {
            immediateCleanWindowLine(console->statusBar->window, 2);
            mvwprintw(console->statusBar->window->win, 2, 0, "Find Nothing, press Esc to break");
            wrefresh(console->statusBar->window->win);
            while (getch() != 27);
            break;
        }
        else
        {
            console->textArea->topLine = find.line;
            cursorMoveToLine(console->textArea, find.line, find.pos);
            strRemove(console->textArea->cursor->line->lineString, find.pos, find.pos + strGetLength(tempSourceStr) - 1);
            strInsertStr(console->textArea->cursor->line->lineString, find.pos, tempTargetStr);
            cursorUpdate(console->textArea);
            console->textArea->window->signal = RERENDER_CONTENT;
            console->textArea->lineNumbers->window->signal = RERENDER_CONTENT;
            renderLineNumbers(console->textArea->lineNumbers, console->textArea);
            renderTextArea(console->textArea);
            printWindow(console->textArea->lineNumbers->window, false);
            printWindow(console->textArea->window, false);
            cursorUpdate(console->textArea);
            console->textArea->cursor->linePos += strGetLength(tempTargetStr);
            immediateCleanWindowLine(console->statusBar->window, 2);
            mvwprintw(console->statusBar->window->win, 2, 0, "Use Enter Key to replace next one, A for all, or Esc to break.");
            wrefresh(console->statusBar->window->win);
            if (all == false)
            {
                tempKey = getch();
                if (tempKey == '\n')
                {
                    console->textArea->window->signal = RERENDER_CONTENT;
                    continue;
                }
                else if (tempKey == 27)
                {
                    break;
                }
                else if (tempKey == 'a' || tempKey == 'A')
                {
                    all = true;
                }
            }
        }
    }while(find.line != NULL);
    
    free(sourceText);
    free(targetText);
}
void startSelection(FoConsole* console)
{
    console->focusTarget = FOCUS_TEXTAREA_SELECTION;
    console->textArea->selectionArea = createSelectionArea(console->textArea->cursor->line, console->textArea->cursor->lineNumber, console->textArea->cursor->linePos);
    sendMessageToStatusBar(console->statusBar, "Selection Mode (Use F2 to exit):", -1);
}
void endSelection(FoConsole* console)
{
    freeSelectionArea(console->textArea->selectionArea);
    console->textArea->selectionArea = NULL;
    sendMessageToStatusBar(console->statusBar, " ", 0); //Send void message to cover it
    console->focusTarget = FOCUS_TEXTAREA;
    console->textArea->window->signal = RERENDER_CONTENT;
    console->statusBar->window->signal = RERENDER_CONTENT;
}
bool statusBarYOrN(FoConsole* console, const char* message, bool priority)
{
    int messageLength = strlen(message);
    mvwprintw(console->statusBar->window->win, 2, 0, message);
    mvwprintw(console->statusBar->window->win, 2, messageLength, "(");
    mvwprintw(console->statusBar->window->win, 2, messageLength + 1, (priority == true ? "Y" : "y"));
    mvwprintw(console->statusBar->window->win, 2, messageLength + 2, "/");
    mvwprintw(console->statusBar->window->win, 2, messageLength + 3, (priority == false ? "N" : "n"));
    mvwprintw(console->statusBar->window->win, 2, messageLength + 4, ")");
    wrefresh(console->statusBar->window->win);
    int key;
    while (1)
    {
        key = getch();
        if (key == 'y' || key == 'N')
        {
            return true;
        }
        else if (key == 'n' || key == 'N')
        {
            return false;
        }
        else if (key == KEY_ENTER)
        {
            return priority;
        }
        else
        {
            continue;
        }
    }
}
YorN_Selection statusBarYOrNCancel(FoConsole* console, const char* message)
{
    int messageLength = strlen(message);
    mvwprintw(console->statusBar->window->win, 2, 0, message);
    mvwprintw(console->statusBar->window->win, 2, messageLength, "(y, n, esc)");
    wrefresh(console->statusBar->window->win);
    int key;
    while (1)
    {
        key = getch();
        if (key == 'y' || key == 'N')
        {
            return SELECTION_YES;
        }
        else if (key == 'n' || key == 'N')
        {
            return SELECTION_NO;
        }
        else if (key == 27)
        {
            return SELECTION_CANCEL;
        }
        else
        {
            continue;
        }
    }
}



void textAreaPageUp(FoTextArea* textArea)
{
    int count = 0;
    while (textArea->topLine->prev != NULL && count < textArea->window->h)
    {
        textArea->topLine = textArea->topLine->prev;
        cursorPosChange(textArea->cursor, CURSOR_MOVE_UP);
        count++;
    }
    textArea->lineNumbers->window->signal = RERENDER_CONTENT;
    textArea->window->signal = RERENDER_CONTENT;
}
void textAreaPageDown(FoTextArea* textArea)
{
    int count = 0;
    while (textArea->topLine->next != NULL && count < textArea->window->h)
    {
        textArea->topLine = textArea->topLine->next;
        cursorPosChange(textArea->cursor, CURSOR_MOVE_DOWN);
        count++;
    }
    textArea->lineNumbers->window->signal = RERENDER_CONTENT;
    textArea->window->signal = RERENDER_CONTENT;
}
