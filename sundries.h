#ifndef __FOLIC_SUNDRIES__
#define __FOLIC_SUNDRIES__

#include "folic.h"
#include "console.h"

typedef enum YorN_Selection
{
    SELECTION_NO = 0,
    SELECTION_YES = 1,
    SELECTION_CANCEL = 2
}YorN_Selection;

void statusBarSaveToFile(FoConsole* console);
void statusBarLoadFile(FoConsole* console);
void statusBarSaveAsFile(FoConsole* console);

void openTargetFile(FoConsole* console, const char* path);
void initTextAreaAndCreateNewFile(FoConsole* console);
void openWelcomePage(FoConsole* console);
void openAboutPage(FoConsole* console);
void openUpdateLogPage(FoConsole* console);
void gotoTargetLine(FoConsole* console);
void findTargetText(FoConsole* console);
void replaceTargetText(FoConsole* console);
void startSelection(FoConsole* console);
void endSelection(FoConsole* console);
bool statusBarYOrN(FoConsole* console, const char* message, bool priority); //y(Y) - true n(N) - false
YorN_Selection statusBarYOrNCancel(FoConsole* console, const char* message);

void textAreaPageUp(FoTextArea* textArea);
void textAreaPageDown(FoTextArea* textArea);

void textAreaSelectAll(FoConsole* console);

#endif