#ifndef __FOSTRING_T__
#define __FOSTRING_T__

#define INIT_STRING_MEMORY_SIZE 32
#include "vector.h"
#include "utf8char.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STRING_NPOS -1

typedef struct FoString
{
    char* data; //Warning: The ->data is UTF-8 encoded. Do not reat it as a simple character array.
    //the concept of size differs from FoVector!
    int32_t size; //the used memory size of ->data
    int32_t capacity; //the allocated memory size of ->data
    int32_t length; //the "real" character amount
    FoVector* mapTable; //Mapping table, maps the byte of each UTF-8 character's starting address to its corresponding index value. Using index type: unsigned short (int32_t)
    utf8char* atTemp; //For strAt(), though I know that it's not a good idea.
}FoString;

FoString* createStr();
FoString* createAssignStr(const char* cStr);
FoString* createCopyStr(FoString* source);
void freeStr(FoString* str);

void strPushBack(FoString* str, utf8char character);
void strPushBackAscii(FoString* str, char ASCIIChar);
void strPopBack(FoString* str);

void strInsert(FoString* str, int32_t pos, char* source);
void strInsertStr(FoString* str, int32_t pos, FoString* source);


int32_t strComputeOffset(const FoString* str, int32_t begin, int32_t end, int32_t* beginOffset, int32_t* endOffset); //writes to 'beginOffset' and 'endOffset', returns 'endOffset - beginOffset'
void strRemove(FoString* str, int32_t begin, int32_t end);

bool strReachLimit(const FoString* str, int32_t expected_offset);
//expected_offset is a value to be added (to the current size) for comparison against capacity. Pass 0 to only check current size.
void strDoubleCapacity(FoString* str);
void strReserve(FoString* str, int32_t new_capacity);
utf8char* strAt(const FoString* str, int index);

void strClear(FoString* str);

void strAssign(FoString* str, const char* cStr);

static inline const char* strCstr(const FoString* str)
{
    return str->data;
}

void strCopy(FoString* dest, const FoString* source);

void strAppend(FoString* str, char* cStr);
void strAppendStr(FoString* str1, FoString* str2);

FoString* strSplit(FoString* str, int32_t begin, int32_t end);
FoString* strSubStr(const FoString* str, int32_t begin, int32_t end);

//these functions return the index of target character
int32_t strFindChar(const FoString* str, utf8char target);
int32_t strRfindChar(const FoString* str, utf8char target);
int32_t strFind(const FoString* str, const char* target);
int32_t strFindWithStartIndex(const FoString* str, const char* target, int32_t startIndex);
int32_t strRFind(const FoString* str, const char* target);
static inline int32_t strFindStr(const FoString* str, const FoString* target)
{
    return strFind(str, strCstr(target));
}
static inline int32_t strFindStrWithStartIndex(const FoString* str, const FoString* target, int32_t startIndex)
{
    return strFindWithStartIndex(str, strCstr(target), startIndex);
}
static inline int32_t strRfindStr(const FoString* str, const FoString* target)
{
    return strRFind(str, strCstr(target));
}

bool strMatch(const FoString* str, int32_t begin, int32_t end, const char* cStr);
bool strMatchStr(const FoString* str1, int32_t begin, int32_t end, const FoString* str2);

void strReverse(FoString* target);

static inline bool strEqual(const FoString* str, const char* cStr)
{
    return strcmp(str->data, cStr) == 0 ? true : false;
}
static inline bool strEqualStr(const FoString* str1, const FoString* str2)
{
    return strcmp(str1->data, str2->data) == 0 ? true : false;
}
static inline int32_t strGetLength(const FoString* str)
{
    return str->length;
}
static inline int32_t strGetSize(const FoString* str)
{
    return str->size;
}
#endif