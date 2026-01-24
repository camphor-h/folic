#ifndef __FOLIC_CONSOLE__
#define __FOLIC_CONSOLE__

#include "window.h"
#include "history.h"
#include "keyboard.h"
#include "clipboard/clipboard.h"

typedef enum ConsoleFocusTarget
{
    FOCUS_TOOLBAR = 0,
    FOCUS_TEXTAREA = 1,
    FOCUS_TEXTAREA_SELECTION = 2,
    FOCUS_STATUSBAR = 3
}ConsoleFocusTarget;

typedef struct FoConsole
{
    int w;
    int h;

    ConsoleFocusTarget focusTarget;

    //Window
    FoToolBar* toolBar;
    FoToolBarWidget* toolWidget; //Temporarily used. Usually set NULL representing there is no widget list to render.
    FoTextArea* textArea;
    FoStatusBar* statusBar;

    //Total Buffer
    Buffer* buffer;

    FoClipboard* clipBoard;

    HistoryStack* history;
    HistoryStack* undo;
    
}FoConsole;
FoConsole* createConsole(FoTextFile* textFile);
void freeConsole(FoConsole* console);
void consoleMainLoop(FoConsole* console);
void printWindow(FoWindow* window, bool isImmediate);
void renderDisplay(FoConsole* console);

void setRerenderAll(FoConsole* console);
void folicQuit(FoConsole* console);

int foGetline(FoWindow* window, int line, int startPosX, char* buffer, int maxCharacter);
//ususally this function will return the number of character it received.
//however, if the user press ESC, it will end immediately and return -1

#endif