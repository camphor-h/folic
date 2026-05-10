#include "mouse.h"

void mouseBackendInit(void)
{
    // No platform-specific initialization needed
}

void mouseBackendFree(void)
{
    // No platform-specific cleanup needed
}

FoMouseEventResult mouseProcess(int vKey)
{
    FoMouseEventResult result;
    result.eventType = MOUSE_NONE;
    result.clickedX = 0;
    result.clickedY = 0;

    if (vKey == KEY_MOUSE)
    {
        MEVENT event;
        if (getmouse(&event) == OK)
        {
            result.clickedX = event.x;
            result.clickedY = event.y;

            if (event.bstate & BUTTON1_PRESSED)
            {
                result.eventType = MOUSE_CLICK;
            }
            else if (event.bstate & BUTTON4_PRESSED)
            {
                result.eventType = MOUSE_SCROLL_UP;
            }
            else if (event.bstate & BUTTON5_PRESSED)
            {
                result.eventType = MOUSE_SCROLL_DOWN;
            }
        }
    }

    return result;
}

void mouseHandleTextAreaClick(FoConsole* console, int clickedX, int clickedY)
{
    if (console == NULL || console->textArea == NULL)
    {
        return;
    }

    FoTextArea* textArea = console->textArea;

    // Check if click is within text area bounds
    int textAreaStartX = textArea->window->x;
    int textAreaStartY = textArea->window->y;
    int textAreaEndX = textAreaStartX + textArea->window->w;
    int textAreaEndY = textAreaStartY + textArea->window->h;

    if (clickedX < textAreaStartX || clickedX >= textAreaEndX ||
        clickedY < textAreaStartY || clickedY >= textAreaEndY)
    {
        return;
    }

    // Calculate relative position within text area
    int relX = clickedX - textAreaStartX;
    int relY = clickedY - textAreaStartY;

    // Find the clicked line
    FoLine* targetLine = textArea->topLine;
    int lineIndex = 0;
    while (targetLine && lineIndex < relY)
    {
        targetLine = targetLine->next;
        lineIndex++;
    }

    if (targetLine == NULL)
    {
        return;
    }

    // Calculate clicked position in the line (character offset)
    int charIndex = relX + textArea->scrollOffset;
    if (charIndex > strGetLength(targetLine->lineString))
    {
        charIndex = strGetLength(targetLine->lineString);
    }

    // Move cursor to clicked position
    cursorMoveToLine(textArea, targetLine, charIndex);

    // If in selection mode, update selection area
    if (console->focusTarget == FOCUS_TEXTAREA_SELECTION && textArea->selectionArea)
    {
        updateSelectionAreaToCursor(textArea->selectionArea, textArea->cursor);
    }

    textArea->window->signal = RERENDER_CONTENT;
}

void mouseHandleScrollUp(FoConsole* console)
{
    if (console == NULL || console->textArea == NULL)
    {
        return;
    }
    cursorPosChange(console->textArea->cursor, CURSOR_MOVE_UP);
}

void mouseHandleScrollDown(FoConsole* console)
{
    if (console == NULL || console->textArea == NULL)
    {
        return;
    }
    cursorPosChange(console->textArea->cursor, CURSOR_MOVE_DOWN);
}
