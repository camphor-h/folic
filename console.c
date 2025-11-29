#include "console.h"
FoConsole* createConsole(FoTextFile* textFile)
{
    FoConsole* console = malloc(sizeof(FoConsole));
    getmaxyx(stdscr, console->h, console->w);
    //mvwprintw(stdscr, 0, 0, "width = %d, height = %d", console->w, console->h);getch();
    console->buffer = createBuffer(console->w, console->h);
    console->toolBar = createToolBar(0, 0, console->w, 2);
    console->toolWidget = NULL;
    console->textArea = createTextArea(textFile, 3, 2, console->w - 3, console->h - 4 - 2);
    console->textArea->lineNumbers = createLineNumbers(0, 2, 3, console->h - 4 - 2); //4 for status bar. 2 for toolbar
    console->statusBar = createStatusBar(0, console->h - 4, console->w, 4);
    console->history = createHistoryStack();
    console->undo = createHistoryStack();
    console->focusTarget = FOCUS_TEXTAREA;
    move(console->textArea->window->y, console->textArea->window->x);
    return console;
}
void freeConsole(FoConsole* console)
{
    freeToolBar(console->toolBar);
    freeTextArea(console->textArea);
    freeStatusBar(console->statusBar);
    freeBuffer(console->buffer);
    free(console);
}
void renderAll(FoConsole* console)
{
    cursorUpdate(console->textArea);
    renderToolBar(console->toolBar);
    renderLineNumbers(console->textArea->lineNumbers, console->textArea);
    renderTextArea(console->textArea);
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
    if (window->signal != NOTHING_TO_RENDER)
    {
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
    move(line + window->y, startPosX + window->x);

    maxCharacter = startPosX + maxCharacter + 1 > window->w ? window->w : maxCharacter;
    int inputed = 0;
    int inputPos = inputed;
    int temp;
    buffer[0] = '\0';
    while (inputed < maxCharacter - 1)
    {
        move(line + window->y, startPosX + window->x);
        clrtoeol();
        printw("%s", buffer);
        move(line + window->y, startPosX + window->x + inputPos);
        wrefresh(window->win);

        temp = getch();
        if (temp == '\n' || temp == KEY_ENTER)
        {
            break;
        }
        else if (temp == KEY_LEFT && inputed > 0 && inputPos > 0)
        {
            inputPos--;
        }
        else if (temp == KEY_RIGHT && inputPos < inputed)
        {
            inputPos++;
        }
        else if (temp == KEY_BACKSPACE && inputed != 0 && inputPos != 0)
        {
            memmove(buffer + inputPos - 1, buffer + inputPos, inputed - inputPos + 1);
            inputed--;
            inputPos--;
        }
        else if (temp == KEY_DC && inputPos < inputed)
        {
            memmove(buffer + inputPos, buffer + inputPos + 1, inputed - inputPos);
            inputed--;
        }
        else if (temp == KEY_HOME)
        {
            inputPos = 0;
        }
        else if (temp == KEY_END)
        {
            inputPos = inputed;
        }
        else if (temp == 27) //ESC
        {
            return -1;
        }
        else if (temp >= 32 && temp <= 126)
        {
            if (inputPos < inputed)
            {
                memmove(buffer + inputPos + 1, buffer + inputPos, inputed - inputPos + 1);
            }
            buffer[inputPos] = temp;
            inputed++;
            inputPos++;
        }
    }
    buffer[inputed] = '\0';
    curs_set(prevCursor);
    move(prevY, prevX);
    return inputed;
}