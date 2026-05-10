#ifndef __FOLIC_MOUSE__
#define __FOLIC_MOUSE__

#include "../folic.h"
#include "../window.h"
#include "../console.h"

typedef enum FoMouseEvent
{
    MOUSE_CLICK = 1,
    MOUSE_DOUBLE_CLICK = 2,
    MOUSE_SCROLL_UP = 3,
    MOUSE_SCROLL_DOWN = 4,
    MOUSE_NONE = 0
}FoMouseEvent;

typedef struct FoMouseEventResult
{
    FoMouseEvent eventType;
    int clickedX;
    int clickedY;
}FoMouseEventResult;

// Public interface

// Process mouse event from getch() and return result
FoMouseEventResult mouseProcess(int vKey);

// Handle mouse actions (frontend logic)
void mouseHandleTextAreaClick(FoConsole* console, int clickedX, int clickedY);
void mouseHandleScrollUp(FoConsole* console);
void mouseHandleScrollDown(FoConsole* console);

// Platform-specific backend initialization/cleanup
void mouseBackendInit(void);
void mouseBackendFree(void);

#endif
