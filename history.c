#include "history.h"

HistoryStack* createHistoryStack()
{
    HistoryStack* historys = malloc(sizeof(HistoryStack));
    historys->curIndex = 0;
    historys->curSize = 0;
    for (int i = 0; i < HISTORY_STACK_MAX; i++)
    {
        historys->behaviors[i] = NULL;
    }
    return historys;
}
void freeHistoryStack(HistoryStack* historys)
{
    clearHistoryStack(historys);
    free(historys);
}
void historyStackPush(HistoryStack* historys, Behavior* behavior)
{
    if (historys->behaviors[historys->curIndex] != NULL)
    {
        freeBehavior(historys->behaviors[historys->curIndex]);
    }
    historys->behaviors[historys->curIndex] = behavior;
    historys->curIndex = historyStackGetNextIndex(historys);
    if (historys->curSize < HISTORY_STACK_MAX)
    {
        historys->curSize++;
    }
}
void historyStackPop(HistoryStack* historys)
{
    if (historys == NULL || historys->curSize <= 0)
    {
        return;
    }
    if (historys->behaviors[historys->curIndex] != NULL)
    {
        freeBehavior(historys->behaviors[historys->curIndex]);
        historys->behaviors[historys->curIndex] = NULL;
    }
    historys->curIndex = historyStackGetLastIndex(historys);
    historys->curSize--;
}
int historyStackGetNextIndex(HistoryStack* historys)
{
    if (historys == NULL)
    {
        return -1;
    }

    if (historys->curIndex < HISTORY_STACK_MAX - 1)
    {
        return historys->curIndex + 1;
    }
    else
    {
        return 0;
    }
}
int historyStackGetLastIndex(HistoryStack* historys)
{
    if (historys == NULL)
    {
        return -1;
    }

    if (historys->curIndex > 0)
    {
        return historys->curIndex - 1;
    }
    else
    {
        return HISTORY_STACK_MAX - 1;
    }
}
Behavior* historyStackTop(HistoryStack* historys)
{
    int index = historyStackGetLastIndex(historys);
    if (historys->behaviors[index] == NULL)
    {
        return NULL;
    }
    return (historys->behaviors[index]);
}

void clearHistoryStack(HistoryStack* historys)
{
    while (historys->curSize > 0)
    {
        historyStackPop(historys);
    }
    historys->curIndex = 0;
    historys->curSize = 0;
}

Behavior* createBehavior()
{
    Behavior* newBehavior = malloc(sizeof(Behavior));
    newBehavior->behaviorString = createStr();
    return newBehavior;
}
Behavior* createBehaviorCopy(Behavior* behavior)
{
    if (behavior == NULL)
    {
        return NULL;
    }
    Behavior* newBehavior = malloc(sizeof(Behavior));
    newBehavior->behaviorString = createCopyStr(behavior->behaviorString);
    newBehavior->lineIndex = behavior->lineIndex;
    newBehavior->pos = behavior->pos;
    newBehavior->bKind = behavior->bKind;
    return newBehavior;
}
void freeBehavior(Behavior* behavior)
{
    if (behavior == NULL)
    {
        return;
    }
    freeStr(behavior->behaviorString);
    free(behavior);
}

Behavior* behaviorManage(FoString* target, FoConsole* console)
{
    static FoString* deleted = NULL;
    Behavior* behavior = createBehavior();
    if (deleted == NULL)
    {
        deleted = createStr();
    }
    else
    {
        strClear(deleted);
    }

    behaviorKind bKind = strAt(target, 0)->data[0] == 8 ? BEHAVIOR_REMOVE : BEHAVIOR_INSERT; //ASCII 8 == BS

    if (bKind == BEHAVIOR_INSERT)
    {
        inputToCursorLine(console->textArea, target);
        behavior->lineIndex = console->textArea->cursor->lineNumber;
        behavior->pos = console->textArea->cursor->linePos;
        behavior->bKind = BEHAVIOR_INSERT;
        strCopy(behavior->behaviorString, target);
    }
    else if (bKind == BEHAVIOR_REMOVE)
    {
        strAppendStr(deleted, backspaceToCursorLine(console->textArea));
        behavior->lineIndex = console->textArea->cursor->lineNumber;
        behavior->pos = console->textArea->cursor->linePos;
        behavior->bKind = BEHAVIOR_REMOVE;
        strCopy(behavior->behaviorString, deleted);
        strReverse(behavior->behaviorString);
    }
    clearHistoryStack(console->undo);
    return behavior;
}
void undoBehavior(FoConsole* console)
{
    Behavior* behavior = createBehaviorCopy(historyStackTop(console->history));
    if (behavior == NULL)
    {
        return;
    }
    FoLine* curLine = getLineWithFirstLine(behavior->lineIndex, console->textArea->textSource->firstLine);
    historyStackPop(console->history);
    if (behavior->bKind == BEHAVIOR_INSERT) //Undo - Delete Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos);
        for (int i = 0; i < strGetLength(behavior->behaviorString); i++)
        {
            backspaceToCursorLine(console->textArea);
        }
    }
    else if (behavior->bKind == BEHAVIOR_REMOVE) //Undo - Insert Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos);
        inputToCursorLine(console->textArea, behavior->behaviorString);
    }
    historyStackPush(console->undo, behavior);
}
void redoBehavior(FoConsole* console)
{
    Behavior* behavior = createBehaviorCopy(historyStackTop(console->undo));
    if (behavior == NULL)
    {
        return;
    }
    FoLine* curLine = getLineWithFirstLine(behavior->lineIndex, console->textArea->textSource->firstLine);
    historyStackPop(console->undo);
    if (behavior->bKind == BEHAVIOR_INSERT) //Redo - Insert Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos - strGetLength(behavior->behaviorString));
        inputToCursorLine(console->textArea, behavior->behaviorString);
    }
    else if (behavior->bKind == BEHAVIOR_REMOVE) //Redo - Delete Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos);
        int deletedLength = strGetLength(behavior->behaviorString);
        for (int i = 0; i < deletedLength; i++)
        {
            backspaceToCursorLine(console->textArea);
        }
    }
    historyStackPush(console->history, behavior);
}
