#include "string.h"
FoString* createStr()
{
    FoString* str = (FoString*)malloc(sizeof(FoString));
    if (str == NULL)
    {
        return NULL;
    }
    str->data = (char*)malloc(sizeof(char) * INIT_STRING_MEMORY_SIZE);
    if (str->data == NULL)
    {
        free(str);
        return NULL;
    }
    str->size = 0;
    str->capacity = INIT_STRING_MEMORY_SIZE;
    str->length = 0;
    str->mapTable = createVec(sizeof(int32_t));
    uint8_t temp = 'P';
    str->atTemp = createUtf8char(&temp);
    return str;
}
FoString* createAssignStr(const char* cStr)
{
    FoString* str = createStr();
    strAssign(str, cStr);
    return str;
}
FoString* createCopyStr(FoString* source)
{
    FoString* str = createStr();
    strCopy(str, source);
    return str;
}
void freeStr(FoString* str)
{
    if (str == NULL)
    {
        return;
    }
    if (str->data != NULL)
    {
        free(str->data);
    }
    if (str->mapTable != NULL)
    {
        freeVec(str->mapTable);
    }
    if (str->atTemp != NULL)
    {
        freeUtf8char(str->atTemp);
    }
    free(str);
}

void strPushBack(FoString* str, utf8char character)
{
    if (str == NULL || character.data == NULL)
    {
        return;
    }
    
    int length = getUtf8charLength(character);
    if (length == 0)
    {
        return;
    }
    int index = str->size;
    if (strReachLimit(str, length + 1)) //+1 for '\0'
    {
        strDoubleCapacity(str);
    }
    char* handleP = str->data;
    handleP += index;
    memcpy(handleP, character.data, length);
    int32_t temp = str->size;
    vecPushBack(str->mapTable, &temp); //map the offset of the first character's address to the mapTable
    str->size += length;
    str->length++;
    str->data[str->size] = '\0'; //add '\0' in the end
}
void strPushBackAscii(FoString* str, char ASCIIChar)
{
    //So you may ask, why we didn't use a static utf8char pointer like what we do in keyboard.c or elsewhere?
    //There question is. Other structure like FoString and FoVector has a simple clear() function. And the utf8char doesn't
    //I know you may ask me why not to implement it. To be honest, it's not a easy job.
    //You think, if I want to write a clear() for utf8char. What will the clear() do for the utf8char?
    //Free it? Oh no! You couldn't write a clear() do free()'s job!
    static uint8_t temp[4];
    memset(temp, '\0', 4);
    temp[0] = (uint8_t)ASCIIChar;
    utf8char* tempChar = createUtf8char(temp);
    strPushBack(str, *tempChar);
    freeUtf8char(tempChar);
}
void strPopBack(FoString* str)
{
    if (str == NULL || str->length == 0)
    {
        return;
    }
    int32_t* lastOffsetPtr = (int32_t*)vecAt(str->mapTable, str->length - 1);
    if (lastOffsetPtr == NULL)
    {
        return;
    }
    int32_t lastCharOffset = *lastOffsetPtr;
    str->size = lastCharOffset;
    str->length--;
    str->data[str->size] = '\0';
    vecPopBack(str->mapTable);
}

void strInsert(FoString* str, int32_t pos, char* source)
{
    if (str == NULL || source == NULL)
    {
        return;
    }
    if (pos < str->length)
    {
        FoString* temp = createAssignStr(source);
        if (temp == NULL)
        {
            return;
        }
        strInsertStr(str, pos, temp);
        freeStr(temp);
    }
    else if (pos == str->length)
    {
        strAppend(str, source);
    }
}
void strInsertStr(FoString* str, int32_t pos, FoString* source)
{
    if (str == NULL || source == NULL)
    {
        return;
    }
    if (str->length == 0)
    {
        strCopy(str, source);
        return;
    }
    int32_t sourceSize = source->size;
    //int32_t prevStrSize = str->size;
    //int32_t prevStrLength = str->length;
    int32_t posOffset = *((int32_t*)vecAt(str->mapTable, pos));
    int32_t moveByte = str->size - posOffset;
    if (strReachLimit(str, source->size + 1) == true)
    {
        strReserve(str, str->size + source->size + 1);
    }
    if (pos < str->length)
    {
        //resize map table
        int32_t filler = 0;
        vecResize(str->mapTable, str->length + source->length, &filler);
        //Firstly, shift the existing data that follows the insertion index.
        memmove(str->data + posOffset + sourceSize, str->data + posOffset, moveByte);
        memcpy(str->data + posOffset, source->data, sourceSize);
        //migrate the original mapping table section that follows the insertion index.
        for (int i = str->length - 1; i >= pos; i--)
        {
            int32_t newOffset = *(int32_t*)vecAt(str->mapTable, i) + sourceSize;
            vecAtAssign(str->mapTable, i + source->length, &newOffset);
        }
        //Writo to the mapping table
        for (int i = 0; i < source->length; i++)
        {
            int32_t offset = *(int32_t*)vecAt(source->mapTable, i) + posOffset;
            vecAtAssign(str->mapTable, pos + i, &offset);
        }
    }
    else if (pos == str->length) //when insert at the end of string. Use the append mode.
    {
        strAppendStr(str, source);
    }
    str->size += source->size;
    str->length += source->length;
}

int32_t strComputeOffset(const FoString* str, int32_t begin, int32_t end, int32_t* beginOffset, int32_t* endOffset)
{
    if (str == NULL || begin > end || end >= str->length)
    {
        return 0;
    }
    *beginOffset = *(int32_t*)vecAt(str->mapTable, begin);
    if (end == str->length - 1) //if it's the last character
    {
        *endOffset = str->size;
    }
    else
    {
        *endOffset = *(int32_t*)vecAt(str->mapTable, end + 1);
    }
    return *endOffset - *beginOffset;
}
void strRemove(FoString* str, int32_t begin, int32_t end)
{
    if (str == NULL || begin > end || end >= str->length)
    {
        return;
    }
    int32_t beginOffset;
    int32_t endOffset;
    int32_t bytesToRemove = strComputeOffset(str, begin, end, &beginOffset, &endOffset);
    memmove(str->data + beginOffset, str->data + endOffset, str->size + 1 - endOffset);
    for (int i = end + 1; i < str->length; i++)
    {
        int32_t newOffset = *(int32_t*)vecAt(str->mapTable, i) - bytesToRemove;
        vecAtAssign(str->mapTable, i - end + begin - 1, &newOffset);
    }
    str->size -= bytesToRemove;
    str->length -= end - begin + 1;
    vecResize(str->mapTable, str->length, NULL);
}

bool strReachLimit(const FoString* str, int32_t expectedOffset)
{
    return (((str->size + expectedOffset)>= str->capacity) ? true : false);
}
void strDoubleCapacity(FoString* str)
{
    str->data = (char*)realloc(str->data, str->capacity * 2);
    str->capacity *= 2;
}
void strReserve(FoString* str, int32_t new_capacity)
{
    if (str == NULL || new_capacity <= str->capacity)
    {
        return;
    }
    str->data = realloc(str->data, new_capacity);
    if (str->data == NULL)
    {
        return;
    }
    str->capacity = new_capacity;
}
utf8char* strAt(const FoString* str, int index)
{
    if (str == NULL || index < 0 || index >= str->length)
    {
        return NULL;
    }
    int32_t* offsetPtr = (int32_t*)vecAt(str->mapTable, index);
    if (offsetPtr == NULL)
    {
        return NULL;
    }
    int32_t offset = *offsetPtr;
    utf8charAssign(str->atTemp, (str->data + offset));
    //uint8_t firstByte = (uint8_t)str->data[offset];
    //uint32_t length = getUtf8_length(firstByte);
    return str->atTemp;
}

void strClear(FoString* str)
{
    str->size = 0;
    str->length = 0;
    vecClear(str->mapTable);
    str->data[0] = '\0';
}

void strAssign(FoString* str, const char* cStr)
{
    strClear(str);
    uint32_t offset = 0;
    while(cStr[offset] != '\0')
    {
        int charLength = getUtf8Length((uint8_t)cStr[offset]);
        utf8char* aimChar = createUtf8char((uint8_t*)&cStr[offset]);
        strPushBack(str, *aimChar);
        freeUtf8char(aimChar);
        offset += charLength;
    }
}

void strCopy(FoString* dest, const FoString* source)
{
    if (dest == NULL || source == NULL)
    {
        return;
    }
    dest->data = realloc(dest->data, source->capacity);
    if (dest->data == NULL)
    {
        return;
    }
    memcpy(dest->data, source->data, source->size + 1); //+1 for '\0'
    dest->capacity = source->capacity;
    dest->size = source->size;
    dest->length = source->length;
    vecCopy(dest->mapTable, source->mapTable);
}

void strAppend(FoString* str, char* cStr)
{
    if (str == NULL || cStr == NULL)
    {
        return;
    }
    uint32_t offset = 0;
    while(cStr[offset] != '\0')
    {
        int charLength = getUtf8Length((uint8_t)cStr[offset]);
        utf8char* aimChar = createUtf8char((uint8_t*)&cStr[offset]);
        strPushBack(str, *aimChar);
        freeUtf8char(aimChar);
        offset += charLength;
    }
}
void strAppendStr(FoString* str1, FoString* str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        return;
    }
    if (strReachLimit(str1, str2->size + 1))
    {
        strReserve(str1, str1->size + str2->size + 1);
    }
    memcpy(str1->data + str1->size, str2->data, str2->size);
    for (int i = 0; i < str2->length; i++)
    {
        int32_t offset = *(int32_t*)vecAt(str2->mapTable, i) + str1->size;
        vecPushBack(str1->mapTable, &offset);
    }
    str1->size += str2->size;
    str1->length += str2->length;
    str1->data[str1->size] = '\0';
}

FoString* strSplit(FoString* str, int32_t begin, int32_t end)
{
    FoString* newStr = strSubStr(str, begin, end);
    strRemove(str, begin, end);
    return newStr;
}
FoString* strSubStr(const FoString* str, int32_t begin, int32_t end)
{
    if (str == NULL || begin > end || end >= str->length)
    {
        return NULL;
    }
    int32_t beginOffset;
    int32_t endOffset;
    int32_t offsets = strComputeOffset(str, begin, end, &beginOffset, &endOffset);
    char* tempStr = malloc(offsets + 1);
    memcpy(tempStr, str->data + beginOffset, offsets);
    tempStr[offsets] = '\0';
    FoString* newStr = createAssignStr(tempStr);
    free(tempStr);
    return newStr;
}

int32_t strFindChar(const FoString* str, utf8char target)
{
    char temp[5];
    memset(temp, '\0', sizeof(temp));
    utf8charToChar(target, temp, sizeof(temp));
    char* aim = strstr(strCstr(str), temp);
    if (aim == NULL)
    {
        return STRING_NPOS;
    }
    int32_t byteOffset = aim - str->data;
    for (int i = 0; i < str->length; i++)
    {
        if (*(int32_t*)vecAt(str->mapTable, i) == byteOffset)
        {
            return i;
        }
    }
    return STRING_NPOS;
}
int32_t strRfindChar(const FoString* str, utf8char target)
{
    char temp[5];
    memset(temp, '\0', sizeof(temp));
    utf8charToChar(target, temp, sizeof(temp));
    int32_t lastIndex = STRING_NPOS;
    char* pos = str->data;
    while ((pos = strstr(pos, temp)) != NULL)
    {
        int32_t byteOffset = pos - str->data;
        for (int i = 0; i < str->length; i++)
        {
            if (*(int32_t*)vecAt(str->mapTable, i) == byteOffset)
            {
                lastIndex = i;
                break;
            }
        }
        pos++;
    }
    return lastIndex;
}
int32_t strFind(const FoString* str, const char* target)
{
    if (str == NULL || target == NULL)
    {
        return STRING_NPOS;
    }
    char* aim = strstr(strCstr(str), target);
    if (aim == NULL)
    {
        return STRING_NPOS;
    }
    int32_t offsets = aim - strCstr(str);
    for (int i = 0; i < str->length; i++)
    {
        if (*(int32_t*)vecAt(str->mapTable, i) == offsets)
        {
            return i;
        }
    }
    return STRING_NPOS;
}
int32_t strFindWithStartIndex(const FoString* str, const char* target, int32_t startIndex)
{
    if (str == NULL || target == NULL || startIndex >= strGetLength(str))
    {
        return STRING_NPOS;
    }
    const char* startOffset = strCstr(str) + *(int32_t*)vecAt(str->mapTable, startIndex);
    char* aim = strstr(startOffset, target);
    if (aim == NULL)
    {
        return STRING_NPOS;
    }
    int32_t offsets = aim - strCstr(str);
    for (int i = startIndex; i < str->length; i++)
    {
        if (*(int32_t*)vecAt(str->mapTable, i) == offsets)
        {
            return i;
        }
    }
    return STRING_NPOS;
}
int32_t strRFind(const FoString* str, const char* target)
{
    if (str == NULL || target == NULL)
    {
        return STRING_NPOS;
    }
    int32_t lastIndex = STRING_NPOS;
    char* searchStart = str->data;
    char* found;
    while ((found = strstr(searchStart, target)) != NULL)
    {
        int32_t byteOffset = found - str->data;
        for (int i = 0; i < str->length; i++)
        {
            if (*(int32_t*)vecAt(str->mapTable, i) == byteOffset)
            {
                lastIndex = i;
                break;
            }
        }
        searchStart = found + 1;
    }
    
    return lastIndex;
}

bool strMatch(const FoString* str, int32_t begin, int32_t end, const char* cStr)
{
    if (str == NULL || begin > end || end >= str->length)
    {
        return false;
    }
    int32_t cStrLength = strlen(cStr);
    int32_t beginOffset;
    int32_t endOffset;
    int32_t offsets = strComputeOffset(str, begin, end, &beginOffset, &endOffset);
    if (offsets != cStrLength)
    {
        return false;
    }
    return (memcmp(str->data + beginOffset, cStr, offsets) == 0 ? true : false);
}
bool strMatchStr(const FoString* str1, int32_t begin, int32_t end, const FoString* str2)
{
    if (str1 == NULL || str2 == NULL || begin > end || end >= str1->length)
    {
        return false;
    }
    int32_t beginOffset;
    int32_t endOffset;
    int32_t offsets = strComputeOffset(str1, begin, end, &beginOffset, &endOffset);
    return (memcmp(str1->data + beginOffset, str2->data, offsets) == 0 ? true : false);
}

void strReverse(FoString* target)
{
    int strLength = strGetLength(target);
    if (strLength <= 1)
    {
        return;
    }

    static FoString* temp = NULL;

    if (temp == NULL)
    {
        temp = createStr();
    }
    else
    {
        strClear(temp);
    }

    for (int i = strLength - 1; i >= 0; i--)
    {
        strPushBack(temp, *strAt(target, i));
    }
    strClear(target);
    strAppendStr(target, temp);
}