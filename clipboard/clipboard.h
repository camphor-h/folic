#ifndef __FOLIC_CLIPBOARD__
#define __FOLIC_CLIPBOARD__

#include <stdbool.h>
#include "../text/string.h"

typedef struct FoClipboard FoClipboard;

FoClipboard* createClipboard();
void freeClipboard(FoClipboard* clipboard);

void sendInClipboard(FoClipboard* clipboard, const char* str);
void sendStrInClipboard(FoClipboard* clipboard, FoString* str);

const char* getFromClipboard(FoClipboard* clipboard);
FoString* getStrFromClipboard(FoClipboard* clipboard);

bool hasSystemClipboard();
void clearClipboard(FoClipboard* clipboard);

#endif