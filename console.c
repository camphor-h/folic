#include "console.h"
#include "syntax.h"
FoConsole* createConsole(FoTextFile* textFile)
{
    FoConsole* console = malloc(sizeof(FoConsole));
    if (console == NULL)
    {
        return NULL;
    }
    getmaxyx(stdscr, console->h, console->w);
    console->buffer = createBuffer(console->w, console->h);
    if (console->buffer == NULL)
    {
        free(console);
        return NULL;
    }
    console->toolBar = createToolBar(0, 0, console->w, 2);
    console->toolWidget = NULL;
    console->textArea = createTextArea(textFile, 3, 2, console->w - 3, console->h - 4 - 2);
    console->textArea->lineNumbers = createLineNumbers(0, 2, 3, console->h - 4 - 2); //4 for status bar. 2 for toolbar
    console->statusBar = createStatusBar(0, console->h - 4, console->w, 4);
    console->history = createHistoryStack();
    console->undo = createHistoryStack();
    console->focusTarget = FOCUS_TEXTAREA;
    console->clipBoard = createClipboard();
    console->syntaxHighlighter = createSyntaxHighlighter(NULL);
    move(console->textArea->window->y, console->textArea->window->x);
    return console;
}
void freeConsole(FoConsole* console)
{
    freeToolBar(console->toolBar);
    freeTextArea(console->textArea);
    freeStatusBar(console->statusBar);
    freeBuffer(console->buffer);
    freeClipboard(console->clipBoard);
    freeSyntaxHighlighter(console->syntaxHighlighter);
    free(console);
}
void renderAll(FoConsole* console)
{
    cursorUpdate(console->textArea);
    renderToolBar(console->toolBar);
    renderLineNumbers(console->textArea->lineNumbers, console->textArea);
    renderTextArea(console->textArea);
    syntaxHighlightTextArea(console->syntaxHighlighter, console->textArea);
    renderStatusBar(console->statusBar, console->textArea);
    renderDisplay(console);
    cursorUpdate(console->textArea);
}
static void printBufferCell(FoWindow* window, int x, int y)
{
    static short cellPair = 0;
    if (window->buffer->bufferArea[y][x].colorPair != cellPair)
    {
        wattroff(window->win, COLOR_PAIR(cellPair));
        cellPair = window->buffer->bufferArea[y][x].colorPair;
        wattron(window->win, COLOR_PAIR(cellPair));
    }
    mvwprintw(window->win, y, x, "%s", window->buffer->bufferArea[y][x].data);
}
void printWindow(FoWindow* window, bool isImmediate)
{
    if (window == NULL || window->win == NULL || window->buffer == NULL)
    {
        return;
    }
    if (window->w <= 0 || window->h <= 0)
    {
        return;
    }
    if (window->signal != NOTHING_TO_RENDER)
    {
        werase(window->win);
        for (int y = 0; y < window->h; y++)
        {
            for (int x = 0; x < window->w; x++)
            {
                if (window->buffer->bufferArea[y][x].isConnectedNext != true)
                {
                    printBufferCell(window, x, y);
                }
            }
        }
        wrefresh(window->win);
    }
    window->signal = (isImmediate == false ? NOTHING_TO_RENDER : window->signal);
}
void renderDisplay(FoConsole* console)
{
    curs_set(0);
    printWindow(console->toolBar->window, false);
    printWindow(console->textArea->lineNumbers->window, false);
    printWindow(console->textArea->window, false);
    printWindow(console->statusBar->window, true);

    if (console->toolWidget != NULL && console->focusTarget == FOCUS_TOOLBAR)
    {
        toolBarWidgetRender(console->toolWidget);
        printWindow(console->toolWidget->listWindow, true);
    }

    if (console->focusTarget != FOCUS_TOOLBAR)
    {
        curs_set(1);
    }
}

void consoleMainLoop(FoConsole* console)
{
    refresh();
    while (1)
    {
        //render
        renderAll(console);
        refresh();
        //keyboard check
        keyManage(console);
        //control
        
    }
}

void setRerenderAll(FoConsole* console)
{
    console->toolBar->window->signal = RERENDER_ALL;
    console->textArea->window->signal = RERENDER_ALL;
    console->statusBar->window->signal = RERENDER_ALL;
    if (console->toolWidget != NULL)
    {
        console->toolWidget->listWindow->signal = RERENDER_ALL;
    }
}

void folicQuit(FoConsole* console)
{
    console->toolWidget = NULL;
    freeConsole(console);
    endwin();
    exit(0);
}
int foGetline(FoWindow* window, int line, int startPosX, char* buffer, int maxCharacter)
{
    wrefresh(window->win);
    int prevX;
    int prevY;
    getyx(stdscr, prevY, prevX);
    int prevCursor = curs_set(1);

    int bufferSize = (maxCharacter > 1024) ? maxCharacter : 1024;
    char* inputBuffer = malloc(bufferSize);
    inputBuffer[0] = '\0';
    int inputed = 0;
    int inputPos = 0;
    int temp;
    int displayOffset = 0;

    while (1)
    {
        int displayLength = window->w - startPosX;
        if (inputed > displayLength)
        {
            if (inputPos < displayOffset)
            {
                displayOffset = inputPos;
            }
            else if (inputPos > displayOffset + displayLength - 1)
            {
                displayOffset = inputPos - displayLength + 1;
            }
        }
        else
        {
            displayOffset = 0;
        }

        int visibleLength = inputed - displayOffset;
        if (visibleLength > displayLength)
        {
            visibleLength = displayLength;
        }

        move(line + window->y, startPosX + window->x);
        clrtoeol();
        printw("%.*s", visibleLength, inputBuffer + displayOffset);
        move(line + window->y, startPosX + window->x + inputPos - displayOffset);
        wrefresh(window->win);

        temp = getch();
        if (temp == '\n' || temp == KEY_ENTER)
        {
            break;
        }
        else if (temp == KEY_LEFT && inputPos > 0)
        {
            inputPos--;
        }
        else if (temp == KEY_RIGHT && inputPos < inputed)
        {
            inputPos++;
        }
        else if (temp == KEY_BACKSPACE && inputPos > 0)
        {
            memmove(inputBuffer + inputPos - 1, inputBuffer + inputPos, inputed - inputPos + 1);
            inputed--;
            inputPos--;
        }
        else if (temp == KEY_DC && inputPos < inputed)
        {
            memmove(inputBuffer + inputPos, inputBuffer + inputPos + 1, inputed - inputPos);
            inputed--;
        }
        else if (temp == KEY_HOME || temp == 1) //KEY_HOME or Ctrl+A
        {
            inputPos = 0;
        }
        else if (temp == KEY_END || temp == 5) //KEY_END or Ctrl+E
        {
            inputPos = inputed;
        }
        else if (temp == 27) //ESC
        {
            free(inputBuffer);
            return -1;
        }
        else if (temp >= 32 && temp <= 126)
        {
            if (inputed + 1 >= bufferSize)
            {
                bufferSize *= 2;
                inputBuffer = realloc(inputBuffer, bufferSize);
            }
            if (inputPos < inputed)
            {
                memmove(inputBuffer + inputPos + 1, inputBuffer + inputPos, inputed - inputPos + 1);
            }
            inputBuffer[inputPos] = temp;
            inputed++;
            inputPos++;
        }
    }

    inputBuffer[inputed] = '\0';
    if (inputed < maxCharacter)
    {
        memcpy(buffer, inputBuffer, inputed + 1);
    }
    else
    {
        memcpy(buffer, inputBuffer, maxCharacter - 1);
        buffer[maxCharacter - 1] = '\0';
    }
    free(inputBuffer);
    curs_set(prevCursor);
    move(prevY, prevX);
    return inputed;
}