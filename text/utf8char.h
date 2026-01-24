#ifndef __FOUTF8CHAR__
#define __FOUTF8CHAR__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef utf8firstByte
#define utf8firstByte(x) (((uint8_t*)((x).data))[0])
#endif

//I think you may say "Why this looks so strange? Couldn't you just write a more programmer-friendly utf8char struct?"
//I'm sorry to my decision. Yes, I admit, to write string based on this utf8char is not a easy job.
//However, this is the best way that can balance the memory space cost and logic.
typedef struct utf8char
{
    char* data;
}utf8char;
utf8char* createUtf8char(uint8_t* firstByte);
void freeUtf8char(utf8char* utf8char_);
int32_t getUtf8Length(uint8_t first_byte);
static inline int32_t getUtf8charLength(utf8char utf8char_)
{
    return getUtf8Length(utf8firstByte(utf8char_));
}
void utf8charToChar(utf8char utf8char_, char* dest, int size);
void utf8charAssign(utf8char* dest, const char* source);
void utf8charCopy(utf8char* dest, const utf8char* source);
int32_t utf8charCodepoint(utf8char* utf8char_);
int32_t codepointDisplayWidth(uint32_t codepoint);
static inline int32_t getUtf8charDisplayWidth(utf8char* utf8char_)
{
    return codepointDisplayWidth(utf8charCodepoint(utf8char_));
}

//Here is the experimental content.
#define UTF8CHAR_MEMORYPOOL_SIZE 16
typedef struct Utf8charMemoryPool
{
    char* memory;
    int32_t size;
    int32_t usedSize;
}Utf8charMemoryPool;
Utf8charMemoryPool* initUtf8charMemoryPool(int size);
void freeUtf8MemoryPool(Utf8charMemoryPool* memPool);
utf8char* createUtf8CharFromPool(uint8_t* firstByte, Utf8charMemoryPool* memPool);
#endif