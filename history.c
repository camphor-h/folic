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
        historys->behaviors[historys->curIndex] = NULL;
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
    if (historys == NULL || historys->curSize == 0)
    {
        return NULL;
    }
    
    int index;
    if (historys->curIndex > 0)
    {
        index = historys->curIndex - 1;
    }
    else
    {
        index = HISTORY_STACK_MAX - 1;
    }
    
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
    static int lastBehaviorTime = 0;

    if (deleted == NULL)
    {
        deleted = createStr();
    }
    else
    {
        strClear(deleted);
    }

    behaviorKind bKind = strAt(target, 0)->data[0] == 8 ? BEHAVIOR_REMOVE : BEHAVIOR_INSERT; //ASCII 8 == BS

    int currentTime = (int)((long long)clock() * 1000 / CLOCKS_PER_SEC);
    bool shouldMerge = false;

    //Check if we should merge with last behavior (within 500ms)
    if (console->history->curSize > 0)
    {
        Behavior* lastBehavior = historyStackTop(console->history);
        if (lastBehavior && lastBehavior->bKind == bKind &&
            currentTime - lastBehaviorTime < 500)
        {
            shouldMerge = true;
        }
    }

    if (bKind == BEHAVIOR_INSERT)
    {
        int prevLineNum = console->textArea->cursor->lineNumber;
        keyboardInput(console->textArea, target);

        Behavior* lastBehavior = historyStackTop(console->history);
        if (shouldMerge && lastBehavior && prevLineNum == lastBehavior->lineIndex)
        {
            strAppendStr(lastBehavior->behaviorString, target);
            lastBehavior->pos = console->textArea->cursor->linePos;
            lastBehaviorTime = currentTime;
            return NULL; //Don't push new behavior
        }

        Behavior* behavior = createBehavior();
        behavior->lineIndex = prevLineNum;
        behavior->pos = console->textArea->cursor->linePos;
        behavior->bKind = BEHAVIOR_INSERT;
        strCopy(behavior->behaviorString, target);
        lastBehaviorTime = currentTime;
        return behavior;
    }
    else if (bKind == BEHAVIOR_REMOVE)
    {
        int prevLineNum = console->textArea->cursor->lineNumber;
        int prevPos = console->textArea->cursor->linePos;
        int deleteCount = strGetLength(target);
        deleted = rawDelete(console->textArea, deleteCount);
        //Store in reverse order for undo (as they were deleted)
        strReverse(deleted);

        Behavior* lastBehavior = historyStackTop(console->history);
        if (shouldMerge && lastBehavior && prevLineNum == lastBehavior->lineIndex)
        {
            //Prepend newly deleted content
            FoString* temp = createStr();
            strAppendStr(temp, deleted);
            strAppendStr(temp, lastBehavior->behaviorString);
            strClear(lastBehavior->behaviorString);
            strCopy(lastBehavior->behaviorString, temp);
            freeStr(temp);
            //pos should be the start of all deleted content
            lastBehavior->pos = prevPos;
            lastBehaviorTime = currentTime;
            return NULL;
        }

        Behavior* behavior = createBehavior();
        behavior->lineIndex = prevLineNum;
        behavior->pos = prevPos;
        behavior->bKind = BEHAVIOR_REMOVE;
        strCopy(behavior->behaviorString, deleted);
        lastBehaviorTime = currentTime;
        return behavior;
    }

    if (console->textArea->textSource->isModified == false)
    {
        console->textArea->textSource->isModified = true;
    }
    clearHistoryStack(console->undo);
    return NULL;
}
void undoBehavior(FoConsole* console)
{
    Behavior* behavior = createBehaviorCopy(historyStackTop(console->history));
    if (behavior == NULL)
    {
        return;
    }
    FoLine* curLine = getLineWithFirstLine(behavior->lineIndex, console->textArea->textSource->firstLine);
    if (curLine == NULL)
    {
        freeBehavior(behavior);
        return;
    }
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
        int32_t targetPos = behavior->pos;
        int32_t lineLen = strGetLength(curLine->lineString);
        if (targetPos > lineLen) targetPos = lineLen;
        
        cursorMoveToLine(console->textArea, curLine, targetPos);
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
    if (curLine == NULL)
    {
        freeBehavior(behavior);
        return;
    }
    historyStackPop(console->undo);
    if (behavior->bKind == BEHAVIOR_INSERT) //Redo - Insert Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos - strGetLength(behavior->behaviorString));
        inputToCursorLine(console->textArea, behavior->behaviorString);
    }
    else if (behavior->bKind == BEHAVIOR_REMOVE) //Redo - Delete Mode
    {
        cursorMoveToLine(console->textArea, curLine, behavior->pos + strGetLength(behavior->behaviorString));
        int deletedLength = strGetLength(behavior->behaviorString);
        for (int i = 0; i < deletedLength; i++)
        {
            backspaceToCursorLine(console->textArea);
        }
    }
    historyStackPush(console->history, behavior);
}
