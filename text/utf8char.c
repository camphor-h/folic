#include "utf8char.h"

utf8char* createUtf8char(uint8_t* firstByte)
{
    int32_t length = getUtf8Length(*firstByte);
    utf8char* utf8char_ = (utf8char*)malloc(sizeof(utf8char));
    if (!utf8char_)
    {
        return NULL;
    }
    utf8char_->data = (char*)malloc(length);
    if (!utf8char_->data)
    {
        free(utf8char_);
        return NULL;
    }
    /*
    for (int i = 0; i < length; i++)
    {
        utf8char_->data[i] = firstByte[i];
    }
    */
    memcpy(utf8char_->data, firstByte, length);
    return utf8char_;
}
void freeUtf8char(utf8char* utf8char_)
{
    if (utf8char_)
    {
        free(utf8char_->data);
        free(utf8char_);
    }
}

int32_t getUtf8Length(uint8_t firstByte)
{
    if ((firstByte & 0x80) == 0x00)
    {
        return 1;
    }
    else if ((firstByte & 0xE0) == 0xC0)
    {
        return 2;
    }
    else if ((firstByte & 0xF0) == 0xE0)
    {
        return 3;
    }
    else if ((firstByte & 0xF8) == 0xF0)
    {
        return 4;
    }
    return 0;
}
void utf8charToChar(utf8char utf8char_, char* dest, int size)
{
    if (size <= 4)
    {
        return;
    }
    memset(dest, '\0', size);
    int u8charLength = getUtf8charLength(utf8char_);
    memcpy(dest, utf8char_.data, u8charLength);
}
void utf8charAssign(utf8char* dest, const char* source)
{
    int32_t sourceLength = getUtf8Length(*source);
    dest->data = realloc(dest->data, sourceLength);
    memcpy(dest->data, source, sourceLength);
}
void utf8charCopy(utf8char* dest, const utf8char* source)
{
    int32_t sourceLength = getUtf8charLength(*source);
    dest->data = realloc(dest->data, sourceLength);
    if (dest->data == NULL)
    {
        return;
    }
    memcpy(dest->data, source->data, sourceLength);
}

int32_t utf8charCodepoint(utf8char* utf8char_)
{
    if (utf8char_ == NULL)
    {
        return 0;
    }

    int charLength = getUtf8charLength(*utf8char_);
    if (charLength == 1)
    {
        return utf8char_->data[0];
    }
    else if (charLength == 2)
    {
        return ((utf8char_->data[0] & 0x1F) << 6) | (utf8char_->data[1] & 0x3F);
    }
    else if (charLength == 3)
    {
        return ((utf8char_->data[0] & 0x0F) << 12) | ((utf8char_->data[1] & 0x3F) << 6) | (utf8char_->data[2] & 0x3F);
    }
    else if (charLength == 4)
    {
        return ((utf8char_->data[0] & 0x07) << 18) | ((utf8char_->data[1] & 0x3F) << 12) | ((utf8char_->data[2] & 0x3F) << 6) | (utf8char_->data[3] & 0x3F);
    }
    return utf8char_->data[0];
}
int32_t codepointDisplayWidth(uint32_t codepoint)
{
    if ((codepoint >= 0x4E00 && codepoint <= 0x9FFF) || //CJK Unified Ideographs
        (codepoint >= 0x3400 && codepoint <= 0x4DBF) || //CJK Extended A
        (codepoint >= 0xF900 && codepoint <= 0xFAFF) || //CJK Compatibility Ideographs
        (codepoint >= 0xAC00 && codepoint <= 0xD7AF) || //Korean
        (codepoint >= 0x3040 && codepoint <= 0x309F) || //hiragana
        (codepoint >= 0x30A0 && codepoint <= 0x30FF) || //katakana
        (codepoint >= 0xFF00 && codepoint <= 0xFFEF)) //Fullwidths
    {
        return 2;
    }
    return 1;
}

Utf8charMemoryPool* initUtf8charMemoryPool(int size)
{
    if (size <= 0)
    {
        return NULL;
    }
    Utf8charMemoryPool* memPool = malloc(sizeof(Utf8charMemoryPool));
    memPool->size = size;
    memPool->usedSize = 0;
    memPool->memory = calloc(size, sizeof(char));
    return memPool;
}
void freeUtf8MemoryPool(Utf8charMemoryPool* memPool)
{
    free(memPool->memory);
    free(memPool);
}