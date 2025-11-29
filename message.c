#include "message.h"
FoMessage* createMessage(char* content, int32_t displayTime)
{
    FoMessage* newMessage = malloc(sizeof(FoMessage));
    if (newMessage == NULL)
    {
        return NULL;
    }
    newMessage->content = createAssignStr(content);
    newMessage->remainTime = displayTime;
    newMessage->timestamp = time(0);
    newMessage->isRendered = false;
    return newMessage;
}
FoMessage* createMessageStr(FoString* content, int32_t displayTime)
{
    if (content == NULL)
    {
        return NULL;
    }
    FoMessage* newMessage = malloc(sizeof(FoMessage));
    newMessage->content = createCopyStr(content);
    newMessage->remainTime = displayTime;
    newMessage->timestamp = time(0);
    newMessage->isRendered = false;
    return newMessage;
}
void freeMessage(FoMessage* message)
{
    if (message == NULL)
    {
        return;
    }
    freeStr(message->content);
    free(message);
}
void updateMessageTime(FoMessage* message)
{
    if (message == NULL)
    {
        return;
    }
    if (message->remainTime <= 0)
    {
        return;
    }

    time_t curTime = time(0);
    time_t elaspedTime = curTime - message->timestamp;
    if (elaspedTime >= message->remainTime)
    {
        message->remainTime = 0;
    }
    else
    {
        message->remainTime -= elaspedTime;
    }
}