#ifndef __FOLIC_HISTORY__
#define __FOLIC_HISTORY__
#include "folic.h"
#include "types.h"
typedef enum
{
    BEHAVIOR_INSERT,
    BEHAVIOR_REMOVE
}behaviorKind;

typedef struct Behavior
{
    behaviorKind bKind;

    //All pos is the cursor pos that insert/remove end.
    
    int lineIndex;
    int32_t pos;

    FoString* behaviorString;
}Behavior;

#define HISTORY_STACK_MAX 50
typedef struct HistoryStack
{
    Behavior* behaviors[HISTORY_STACK_MAX];
    int curIndex;
    int curSize;
}HistoryStack;

HistoryStack* createHistoryStack();
void freeHistoryStack(HistoryStack* historys); //I think that there is no need to implement it. Because, there will only have two Historty Stack
void historyStackPush(HistoryStack* historys, Behavior* behavior);
void historyStackPop(HistoryStack* historys);
int historyStackGetNextIndex(HistoryStack* HistoryStack);
int historyStackGetLastIndex(HistoryStack* HistoryStack);
void clearHistoryStack(HistoryStack* historys);

Behavior* createBehavior();
Behavior* createBehaviorCopy(Behavior* behavior);
void freeBehavior(Behavior* behavior);

Behavior* behaviorManage(FoString* target, FoConsole* console);
void undoBehavior(FoConsole* console);
void redoBehavior(FoConsole* console);

static inline int getNextIndex(int curIndex, int minIndex, int maxIndex)
{
    return (curIndex == maxIndex) ? minIndex : curIndex + 1;
}
static inline int getLastIndex(int curIndex, int minIndex, int maxIndex)
{
    return (curIndex == minIndex) ? maxIndex : curIndex - 1;
}

#endif