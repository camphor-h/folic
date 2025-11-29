#include "keyboard.h"

#define KEY_CTRL(x) ((x) & 0x1F)
#define KEY_ALT(x) ((x) + 0x1000)

static void keyWithMode(FoConsole* console, int vKey, int keyTryer)
{
    static int widgetIndex = 0;
    static int optionIndex = 0;
    if (vKey == 27)
    {
        if (console->focusTarget == FOCUS_TEXTAREA)
        {
            console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, widgetIndex);
            widgetIndex = 0;
            console->toolWidget->selectedOption = 0;
            console->focusTarget = FOCUS_TOOLBAR;
            setRerenderAll(console);
        }
        else if (console->focusTarget == FOCUS_TOOLBAR)
        {
            console->toolWidget = NULL;
            console->focusTarget = FOCUS_TEXTAREA;
            widgetIndex = 0;
            optionIndex = 0;
            setRerenderAll(console);
        }
        return;
    }

    if (console->focusTarget == FOCUS_TEXTAREA)
    {
        if (keyTryer == ERR) //if not get something new. Enter the single key process
        {
            //set it to blocking
            singleKeyProcess(console, vKey);
        }
        else //put it back;
        {
            ungetch(keyTryer);
            ungetch(vKey);
            inputTextProcess(console);
        }
    }
    else if (console->focusTarget == FOCUS_TOOLBAR)
    {
        timeout(-1);
        switch (vKey)
        {
            case KEY_UP:
                optionIndex = getLastIndex(optionIndex, 0, vecGetSize(console->toolWidget->options) - 1);
                console->toolWidget->selectedOption = optionIndex;
                break;
            case KEY_DOWN:
                optionIndex = getNextIndex(optionIndex, 0, vecGetSize(console->toolWidget->options) - 1);
                console->toolWidget->selectedOption = optionIndex;
                break;
            case KEY_LEFT:
                optionIndex = 0;
                console->toolWidget->selectedOption = 0;
                widgetIndex = getLastIndex(widgetIndex, 0, vecGetSize(console->toolBar->widgets) - 1);
                console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, widgetIndex);
                setRerenderAll(console);
                break;
            case KEY_RIGHT:
                optionIndex = 0;
                console->toolWidget->selectedOption = 0;
                widgetIndex = getNextIndex(widgetIndex, 0, vecGetSize(console->toolBar->widgets) - 1);
                console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, widgetIndex);
                setRerenderAll(console);
                break;
            case '\n':
                curs_set(1);
                (*(FoToolBarWidgetOption**)vecAt(console->toolWidget->options, console->toolWidget->selectedOption))->operation(console);
                console->toolWidget = NULL;
                console->focusTarget = FOCUS_TEXTAREA;
                widgetIndex = 0;
                optionIndex = 0;
                setRerenderAll(console);
                break;
        }
    }
}
/*
static void altKey(FoConsole* console, int vKey)
{
    switch (vKey)
    {
        case KEY_ALT('f'):
            console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, 0);
            console->focusTarget = FOCUS_TOOLBAR;
            break;
        case KEY_ALT('e'):
            console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, 1);
            console->focusTarget = FOCUS_TOOLBAR;
            break;
        case KEY_ALT('h'):
            console->toolWidget = *(FoToolBarWidget**)vecAt(console->toolBar->widgets, 2);
            console->focusTarget = FOCUS_TOOLBAR;
            break;
        default:
            console->focusTarget = (console->focusTarget == FOCUS_TEXTAREA) ? FOCUS_TOOLBAR : FOCUS_TEXTAREA;
            break;
    }
}
*/
void keyManage(FoConsole* console)
{
    static int vKey;
    static int keyTryer;
    /*The default behavior of getch() is blocking. However, by set the timeout(). You could change it to non-blocking
    **Here, we get a tempKey by blocking type getch(). Then we temporarily change to non-blocking type. and try to get another one
    **If we get another one. And even more. That's mean that the user is copying or pasting. Or use input method.*/

    //I have to say that it's definitely not easy. You have to think for a while to get this idea.
    //Emmm... Maybe I should set the getch() as non-blocking at the first time?
    vKey = getch();
    timeout(0); //set it to non-blocking immediately.
    keyTryer = getch();
    timeout(-1);
    keyWithMode(console, vKey, keyTryer);
}
void singleKeyProcess(FoConsole* console, int key)
{
    static FoString* singleCharStr = NULL;
    if (singleCharStr == NULL)
    {
        singleCharStr = createStr();
    }

    if (key == KEY_BACKSPACE)
    {
            strPushBackAscii(singleCharStr, 8); //ASCII 8 == BS
            prepareInputStr(console, INPUTSTR_APPEND_SEND, singleCharStr);
            strClear(singleCharStr);
    }
    else if (key >= KEY_CODE_YES || key == KEY_CTRL('s') || key == KEY_CTRL('z') || key == KEY_CTRL('y') || key == KEY_CTRL('o') || key == KEY_CTRL('n') || key == KEY_CTRL('f') || 
                                    key == KEY_CTRL('h') || key == KEY_CTRL('g') || key == KEY_CTRL('q')) //if is a functional key
    {
        functionKey(console, key);
    }
    else if (key == 9)
    {
        strAppend(singleCharStr, "    "); //I like to map tab key to 4 spaces;
        prepareInputStr(console, INPUTSTR_APPEND_SEND, singleCharStr);
        strClear(singleCharStr);
    }
    else //if is just a input about single ASCII character
    {
        strPushBackAscii(singleCharStr, key);
        prepareInputStr(console, INPUTSTR_APPEND_SEND, singleCharStr);
        strClear(singleCharStr);
    }
}
void inputTextProcess(FoConsole* console)
{
    timeout(0);
    int keyTryer;
    uint8_t tempUtf8charBuffer[4];
    memset(tempUtf8charBuffer, '\0', 4);
    static FoString* bufferStr = NULL;
    if (bufferStr == NULL)
    {
        bufferStr = createStr();
    }
    while (1)
    {
        keyTryer = getch();
        if (keyTryer == ERR)
        {
            prepareInputStr(console, INPUTSTR_APPEND_SEND, bufferStr);
            break;
        }
        else
        {
            int length = getUtf8Length(keyTryer);
            tempUtf8charBuffer[0] = (uint8_t)keyTryer;
            for (int i = 1; i < length; i++)
            {
                keyTryer = getch();
                if (keyTryer == ERR) //if it gets wrong, throw the data it has received
                {
                    memset(tempUtf8charBuffer + i, '\0', 4 - i);
                    break;
                }
                tempUtf8charBuffer[i] = (uint8_t)keyTryer;
            }
            utf8char* tempUtf8char = createUtf8char(tempUtf8charBuffer);
            strPushBack(bufferStr, *tempUtf8char);
            freeUtf8char(tempUtf8char);
            memset(tempUtf8charBuffer, '\0', 4);
        }
    }
    strClear(bufferStr);
    timeout(-1);
}
void prepareInputStr(FoConsole* console, InputStrSignal signal, FoString* source)
{
    static FoString* inputStr = NULL;
    if (inputStr == NULL)
    {
        inputStr = createStr();
    }
    switch (signal)
    {
        case INPUTSTR_APPEND:
            if (source != NULL)
            {
                strAppendStr(inputStr, source);
            }
            break;
        case INPUTSTR_APPEND_SEND:
            if (source != NULL)
            {
                strAppendStr(inputStr, source);
            }
            historyStackPush(console->history, behaviorManage(inputStr, console));
            strClear(inputStr);
            break;
        case INPUTSTR_SEND:
            historyStackPush(console->history, behaviorManage(inputStr, console));
            strClear(inputStr);
            break;
        case INPUTSTR_CLEAR:
            strClear(inputStr);
            break;
    }
}

void functionKey(FoConsole* console, int key)
{
    switch (key)
    {
        case KEY_UP:
            cursorPosChange(console->textArea->cursor, CURSOR_MOVE_UP);
            break;
        case KEY_DOWN:
            cursorPosChange(console->textArea->cursor, CURSOR_MOVE_DOWN);
            break;
        case KEY_LEFT:
            cursorPosChange(console->textArea->cursor, CURSOR_MOVE_LEFT);
            break;
        case KEY_RIGHT:
            cursorPosChange(console->textArea->cursor, CURSOR_MOVE_RIGHT);
            break;
        case KEY_CTRL('s'):
            statusBarSaveToFile(console);
            break;
        case KEY_CTRL('o'):
            statusBarLoadFile(console);
            break;
        case KEY_CTRL('z'):
            undoBehavior(console);
            break;
        case KEY_CTRL('y'):
            redoBehavior(console);
            break;
        case KEY_CTRL('n'):
            initTextAreaAndCreateNewFile(console);
            break;
        case KEY_CTRL('f'):
            findTargetText(console);
            break;
        case KEY_CTRL('h'):
            replaceTargetText(console);
            break;
        case KEY_CTRL('g'):
            gotoTargetLine(console);
            break;
        case KEY_CTRL('q'):
            folicQuit(console);
            break;
        case KEY_HOME:
            console->textArea->cursor->linePos = 0;
            break;
        case KEY_END:
            console->textArea->cursor->linePos = strGetLength(console->textArea->cursor->line->lineString);
            break;
            /*
        case KEY_CTRL(KEY_HOME):
            cursorMoveToLine(console->textArea, console->textArea->textSource->firstLine, 0);
            break;
        case KEY_CTRL(KEY_END):
            FoLine* lastLine = getLastLine(console->textArea->cursor->line);
            cursorMoveToLine(console->textArea, lastLine, strGetLength(lastLine->lineString));
            break;
            */
    }
}