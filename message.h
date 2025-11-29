#ifndef __FOLIC_MESSAGE__
#define __FOLIC_MESSAGE__

#include "folic.h"

typedef struct FoMessage
{
    FoString* content;
    bool isRendered;
    time_t timestamp; //What it was create
    int32_t remainTime; //Remained Presentation time, Unit: second
    //When it's 0. free the message.
    //When it's smaller than 0. It will not disappear automatically.
}FoMessage;

FoMessage* createMessage(char* content, int32_t displayTime);
FoMessage* createMessageStr(FoString* content, int32_t displayTime);
void freeMessage(FoMessage* message);
void updateMessageTime(FoMessage* message);

#endif