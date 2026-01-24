#ifndef __FOLIC_WINDOW__
#define __FOLIC_WINDOW__

#include "folic.h"
#include "line.h"
#include "console.h"
#include "message.h"
#include "types.h"
#include "buffer.h"
#include "textfile.h"
#include "sundries.h"

typedef enum cursorPosChangeSignal
{
    CURSOR_MOVE_LEFT = 1,
    CURSOR_MOVE_RIGHT = 2,
    CURSOR_MOVE_UP = 3,
    CURSOR_MOVE_DOWN = 4
}cursorPosChangeSignal;

typedef struct FoCursor
{
    int32_t linePos;
    bool isVisible;
    int lineNumber;
    FoLine* line;
}FoCursor;
FoCursor* createCursor(FoLine* cursorStartLine, int32_t startLinePos);
void freeCursor(FoCursor* cursor);
void cursorPosChange(FoCursor* cursor, cursorPosChangeSignal signal);
void cursorUpdate(FoTextArea* textArea);

void cursorMoveToLine(FoTextArea* textArea, FoLine* line, int32_t pos);

typedef enum FoWindowSignal
{
    //Maybe I should use bitmask to rewrite it?
    NOTHING_TO_RENDER = 0,
    RERENDER_CONTENT = 1,
    RERENDER_FRAME = 2,
    RERENDER_ALL = 3
}FoWindowSignal;

typedef struct FoWindow
{
    WINDOW* win;
    Buffer* buffer;
    FoWindowSignal signal;
    //So you may ask: Hey why we has width and height in buffer, here we still get width and height in window?
    //I admit that it's a little inappropriate, but buffer with width and height can simplify some function call, I think. 
    int x;
    int y;
    int w;
    int h;
}FoWindow;
FoWindow* createWindow(int x, int y, int w, int h);
void freeWindow(FoWindow* window);
void renderToConsole(FoWindow* window, FoConsole* console);

typedef struct FoToolBarWidgetOptionRaw
{
    char* name;
    void (*operation)(FoConsole* console);
}FoToolBarWidgetOptionRaw;

typedef struct FoToolBarWidgetOption
{
    FoString* name;
    void (*operation)(FoConsole* console);
}FoToolBarWidgetOption;

FoToolBarWidgetOption* createToolBarWidgetOption(const char* name, void (*operation)(FoConsole* console));
void freeToolBarWidgetOption(FoToolBarWidgetOption* tbwOption);

typedef struct FoToolBarWidget
{
    FoWindow* window;
    FoWindow* listWindow;
    FoString* name;
    FoVector* options;
    int selectedOption;
}FoToolBarWidget;

FoToolBarWidget* createToolBarWidget(const char* name, int optionAmount, const FoToolBarWidgetOptionRaw* optionRaw, int x, int y, int w, int h);
void freeToolBarWidget(FoToolBarWidget* toolBarWidget);
void toolBarWidgetRender(FoToolBarWidget* tbWidget);

typedef struct FoToolBar
{
    FoWindow* window;
    FoVector* widgets;
    int selectedIndex; //if it == -1. the focus is not on ToolBar
}FoToolBar;
FoToolBar* createToolBar(int x, int y, int w, int h);
void freeToolBar(FoToolBar* toolBar);
void renderToolBar(FoToolBar* toolBar);

typedef struct FoSelectionArea
{
    FoLine* anchorLine;
    FoLine* endLine;
    int32_t anchorLineNumber;
    int32_t anchorLinePos;
    int32_t endLineNumber;
    int32_t endLinePos;
    int32_t selectedAmount;
}FoSelectionArea;
FoSelectionArea* createSelectionArea(FoLine* startLine_, int32_t startLineNumber_, int32_t startLinePos_);
static inline void freeSelectionArea(FoSelectionArea* selectionArea)
{
    free(selectionArea);
}
void updateSelectionAreaToCursor(FoSelectionArea* selectionArea, FoCursor* cursor);
FoString* getSelectionAreaStr(FoSelectionArea* selectionArea);


typedef struct FoTextArea
{
    FoWindow* window;
    FoTextFile* textSource;
    FoLineNumbers* lineNumbers;
    FoLine* topLine;
    FoCursor* cursor;
    HistoryStack* history;
    HistoryStack* undo;
    FoSelectionArea* selectionArea; //If it's set as NULL, that's mean there is no selection area currently.
    
    int scrollOffset; //the offset is bufferCell's offset.
}FoTextArea;
FoTextArea* createTextArea(FoTextFile* textFile, int x, int y, int w, int h); //if TextArea == NULL, it will create a new file
void freeTextArea(FoTextArea* textArea);
void renderTextArea(FoTextArea* textArea);

typedef struct FoLineNumbers
{
    FoWindow* window;
}FoLineNumbers;
FoLineNumbers* createLineNumbers(int x, int y, int w, int h);
void freeLineNumbers(FoLineNumbers* lineNumbers);
void renderLineNumbers(FoLineNumbers* lineNumbers, FoTextArea* textArea);

typedef struct FoStatusBar
{
    FoWindow* window;
    FoMessage* message;
}FoStatusBar;
FoStatusBar* createStatusBar(int x, int y, int w, int h);
void freeStatusBar(FoStatusBar* statusBar);
void renderStatusBar(FoStatusBar* statusBar, FoTextArea* textArea); //it will process message incidentally, I know it's not a good behavior though
static inline void sendMessageToStatusBar(FoStatusBar* statusBar, char* string, int32_t displayTime)
{
    statusBar->message = createMessage(string, displayTime);
}
static inline void sendMessageStrToStatusBar(FoStatusBar* statusBar, FoString* string, int32_t displayTime)
{
    statusBar->message = createMessageStr(string, displayTime);
}

#endif