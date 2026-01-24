#include "clipboard.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

struct FoClipboard
{
    FoString* content;            //Custom clipboard content
    bool useSystemClipboard;      //Whether system clipboard is available
};

static bool initSystemClipboard();
static bool systemClipboardSetText(const char* text);
static char* systemClipboardGetText();
static bool systemClipboardHasText();
static bool systemClipboardClear();

bool hasSystemClipboard()
{
    return initSystemClipboard();
}

FoClipboard* createClipboard()
{
    FoClipboard* newClipboard = malloc(sizeof(FoClipboard));
    if (newClipboard == NULL)
    {
        return NULL;
    }
    
    newClipboard->useSystemClipboard = initSystemClipboard();
    
    newClipboard->content = createStr();
    if (newClipboard->content == NULL)
    {
        free(newClipboard);
        return NULL;
    }
    
    return newClipboard;
}
void freeClipboard(FoClipboard* clipboard)
{
    if (clipboard == NULL)
    {
        return;
    }
    
    if (clipboard->content != NULL)
    {
        freeStr(clipboard->content);
    }
    
    free(clipboard);
}

void sendInClipboard(FoClipboard* clipboard, const char* str)
{
    if (clipboard == NULL || str == NULL)
    {
        return;
    }
    
    //Always update custom clipboard
    strAssign(clipboard->content, str);
    
    //Try to update system clipboard if available
    if (clipboard->useSystemClipboard)
    {
        systemClipboardSetText(str);
    }
}

void sendStrInClipboard(FoClipboard* clipboard, FoString* str)
{
    if (clipboard == NULL || str == NULL)
    {
        return;
    }
    
    strClear(clipboard->content);
    strCopy(clipboard->content, str);
    
    if (clipboard->useSystemClipboard)
    {
        const char* cstr = strCstr(str);
        if (cstr != NULL)
        {
            systemClipboardSetText(cstr);
        }
    }
}

const char* getFromClipboard(FoClipboard* clipboard)
{
    if (clipboard == NULL)
    {
        return NULL;
    }
    
    if (clipboard->useSystemClipboard && systemClipboardHasText())
    {
        char* systemText = systemClipboardGetText();
        if (systemText != NULL)
        {
            strAssign(clipboard->content, systemText);
            free(systemText);
        }
    }
    
    return strCstr(clipboard->content);
}
FoString* getStrFromClipboard(FoClipboard* clipboard)
{
    if (clipboard == NULL)
    {
        return NULL;
    }
    if (clipboard->useSystemClipboard && systemClipboardHasText())
    {
        char* systemText = systemClipboardGetText();
        if (systemText != NULL)
        {
            //Update custom clipboard with system content
            strAssign(clipboard->content, systemText);
            free(systemText);
        }
    }
    return clipboard->content;
}

void clearClipboard(FoClipboard* clipboard)
{
    if (clipboard == NULL)
    {
        return;
    }
    strClear(clipboard->content);
    if (clipboard->useSystemClipboard)
    {
        systemClipboardClear();
    }
}

#ifdef _WIN32

static bool initSystemClipboard()
{
    return true;
}

static bool systemClipboardSetText(const char* text)
{
    if (text == NULL)
    {
        return false;
    }
    
    if (!OpenClipboard(NULL))
    {
        return false;
    }
    
    EmptyClipboard();
    
    size_t len = strlen(text) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem)
    {
        CloseClipboard();
        return false;
    }
    
    memcpy(GlobalLock(hMem), text, len);
    GlobalUnlock(hMem);
    bool result = (SetClipboardData(CF_TEXT, hMem) != NULL);
    CloseClipboard();
    if (!result)
    {
        GlobalFree(hMem);
    }
    
    return result;
}

static char* systemClipboardGetText()
{
    if (!OpenClipboard(NULL))
    {
        return NULL;
    }
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL)
    {
        CloseClipboard();
        return NULL;
    }
    char* pszText = (char*)GlobalLock(hData);
    if (pszText == NULL)
    {
        CloseClipboard();
        return NULL;
    }
    char* text = strdup(pszText);
    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}

static bool systemClipboardHasText()
{
    if (!OpenClipboard(NULL))
    {
        return false;
    }
    bool hasText = IsClipboardFormatAvailable(CF_TEXT);
    CloseClipboard();
    return hasText;
}

static bool systemClipboardClear()
{
    if (!OpenClipboard(NULL))
    {
        return false;
    }
    bool result = EmptyClipboard();
    CloseClipboard();
    return result;
}

#elif defined(__APPLE__)

static bool initSystemClipboard()
{
    return true;
}

static bool systemClipboardSetText(const char* text)
{
    if (text == NULL)
    {
        return false;
    }
    
    CFStringRef cfText = CFStringCreateWithCString(
        kCFAllocatorDefault, 
        text, 
        kCFStringEncodingUTF8
    );
    
    if (!cfText)
    {
        return false;
    }
    
    CFStringRef pasteboardName = CFSTR("general");
    PasteboardRef pasteboard;
    OSStatus status = PasteboardCreate(pasteboardName, &pasteboard);
    
    if (status != noErr)
    {
        CFRelease(cfText);
        return false;
    }
    
    PasteboardClear(pasteboard);
    
    CFDataRef data = CFStringCreateExternalRepresentation(
        kCFAllocatorDefault, 
        cfText, 
        kCFStringEncodingUTF8, 
        0
    );
    
    bool result = false;
    if (data)
    {
        PasteboardItemID itemID = 1;
        result = (PasteboardPutItemFlavor(
            pasteboard, 
            itemID, 
            CFSTR("public.utf8-plain-text"), 
            data, 
            0
        ) == noErr);
        
        CFRelease(data);
    }
    
    CFRelease(cfText);
    CFRelease(pasteboard);
    
    return result;
}

static char* systemClipboardGetText()
{
    CFStringRef pasteboardName = CFSTR("general");
    PasteboardRef pasteboard;
    OSStatus status = PasteboardCreate(pasteboardName, &pasteboard);
    
    if (status != noErr)
    {
        return NULL;
    }
    
    PasteboardSynchronize(pasteboard);
    
    ItemCount itemCount;
    status = PasteboardGetItemCount(pasteboard, &itemCount);
    
    if (status != noErr || itemCount == 0)
    {
        CFRelease(pasteboard);
        return NULL;
    }
    
    char* result = NULL;
    PasteboardItemID itemID;
    
    if (PasteboardGetItemIdentifier(pasteboard, 1, &itemID) == noErr)
    {
        CFDataRef data;
        if (PasteboardCopyItemFlavorData(
            pasteboard, 
            itemID, 
            CFSTR("public.utf8-plain-text"), 
            &data
        ) == noErr)
        {
            CFIndex length = CFDataGetLength(data);
            if (length > 0)
            {
                result = malloc(length + 1);
                if (result)
                {
                    CFDataGetBytes(data, CFRangeMake(0, length), (UInt8 *)result);
                    result[length] = '\0';
                }
            }
            CFRelease(data);
        }
    }
    
    CFRelease(pasteboard);
    return result;
}

static bool systemClipboardHasText()
{
    CFStringRef pasteboardName = CFSTR("general");
    PasteboardRef pasteboard;
    OSStatus status = PasteboardCreate(pasteboardName, &pasteboard);
    
    if (status != noErr)
    {
        return false;
    }
    
    PasteboardSynchronize(pasteboard);
    ItemCount itemCount;
    status = PasteboardGetItemCount(pasteboard, &itemCount);
    CFRelease(pasteboard);
    return (status == noErr && itemCount > 0);
}

static bool systemClipboardClear()
{
    CFStringRef pasteboardName = CFSTR("general");
    PasteboardRef pasteboard;
    OSStatus status = PasteboardCreate(pasteboardName, &pasteboard);
    
    if (status != noErr)
    {
        return false;
    }
    status = PasteboardClear(pasteboard);
    CFRelease(pasteboard);
    return (status == noErr);
}

#else

static bool initSystemClipboard()
{
    return false;
}

static bool systemClipboardSetText(const char* text)
{
    (void)text;
    return false;
}

static char* systemClipboardGetText()
{
    return NULL;
}

static bool systemClipboardHasText()
{
    return false;
}

static bool systemClipboardClear()
{
    return false;
}

#endif