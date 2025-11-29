#include "buffer.h"

void initColorPairs()
{
    //The first pair is default black and white
    init_pair(BUFFERCELL_COLOR_CYAN_AND_WHITE, COLOR_WHITE, COLOR_CYAN);
}
static void clearBufferCell(BufferCell* cell)
{
    memset(cell->data, '\0', BUFFERCELL_DATA_LENGTH);
    cell->isUnderlined = false;
    cell->isInvertColor = false;
    cell->isConnectedLast = false;
    cell->isConnectedNext = false;
    cell->colorPair = 0;
}
void clearBuffer(Buffer* buffer)
{
    for (int i = 0; i < buffer->h; i++)
    {
        clearBufferLine(buffer, i);
    }
}

Buffer* createBuffer(int w, int h)
{
    Buffer* buffer = malloc(sizeof(Buffer));
    buffer->w = w;
    buffer->h = h;
    buffer->bufferArea = malloc(sizeof(BufferCell*) * h);
    for (int i = 0; i < h; i++)
    {
        buffer->bufferArea[i] = malloc(sizeof(BufferCell) * w);
        for (int j = 0; j < w; j++)
        {
            clearBufferCell(&(buffer->bufferArea[i][j]));
        }
    }
    return buffer;
}
void freeBuffer(Buffer* buffer)
{
    for (int i = 0; i < buffer->h; i++)
    {
        free(buffer->bufferArea[i]);
    }
    free(buffer->bufferArea);
    free(buffer);
}

Buffer* copyBuffer(Buffer* source, int w, int h)
{
    Buffer* newBuffer = createBuffer(w, h);
    int smallerW = (source->w < w) ? source->w : w;
    int smallerH = (source->h < h) ? source->h : h;
    for (int i = 0; i < smallerH; i++)
    {
        for (int j = 0; j < smallerW; j++)
        {
            memcpy(&(newBuffer->bufferArea[i][j]), &(source->bufferArea[i][j]), sizeof(BufferCell));
        }
    }
    return newBuffer;
}

void clearBufferLine(Buffer* buffer, int line)
{
    if (line < 0 || buffer == NULL || line >= buffer->h)
    {
        return;
    }
    for (int x = 0; x < buffer->w; x++)
    {
        clearBufferCell(&(buffer->bufferArea[line][x]));
    }
}
void setBufferLineSpace(Buffer* buffer, int line)
{
    if (line < 0 || buffer == NULL || line >= buffer->h)
    {
        return;
    }
    for (int x = 0; x < buffer->w; x++)
    {
        (buffer->bufferArea[line][x]).data[0] = ' ';
    }
}
void setBufferSpace(Buffer* buffer)
{
    for (int i = 0; i < buffer->h; i++)
    {
        setBufferLineSpace(buffer, i);
    }
}

bool isBufferCellEqual(BufferCell* bufferCell1, BufferCell* bufferCell2)
{
    if (memcmp(bufferCell1->data, bufferCell2->data, 4) == 0 && bufferCell1->colorPair == bufferCell2->colorPair &&
        bufferCell1->isConnectedLast == bufferCell2->isConnectedLast && bufferCell1->isConnectedNext == bufferCell2->isConnectedNext &&
        bufferCell1->isInvertColor == bufferCell2->isInvertColor && bufferCell1->isUnderlined == bufferCell2->isUnderlined)
    {
        return true;
    }
    return false;
}

void renderCharToBuffer(Buffer* buffer, int x, int y, char* char_, int length)
{
    if (length > BUFFERCELL_DATA_LENGTH)
    {
        length = BUFFERCELL_DATA_LENGTH;
    }
    memcpy(buffer->bufferArea[y][x].data, char_, length);
    if (length < BUFFERCELL_DATA_LENGTH)
    {
        memset(buffer->bufferArea[y][x].data + length, '\0', BUFFERCELL_DATA_LENGTH - length);
    }
}
void renderCStrToBuffer(Buffer* buffer, int x, int y, char* cStr)
{
    int length = strlen(cStr);
    for (int i = x; i < buffer->w && i < x + length; i++) //when reach the window edge or string length, break
    {
        renderCharToBuffer(buffer, i, y, &(cStr[i - x]), 1);
    }
}
void renderStrToBuffer(Buffer* buffer, int x, int y, FoString* str)
{
    if (str == NULL)
    {
        return;
    }
    int length = strGetLength(str);
    for (int i = x; i < buffer->w && i < x + length; i++) //same as CStr
    {
        int displayLength = getUtf8charDisplayWidth((utf8char*)strAt(str, i - x));
        if (displayLength == 1) //when it's not a wide char
        {
            renderCharToBuffer(buffer, i, y, ((utf8char*)strAt(str, i - x))->data, getUtf8charLength(*(utf8char*)strAt(str, i - x)));
        }
        else //process wide char
        //now I only think about wide char that need two display-unit(like Chinese and Japanese). because as far as I know. Most of the wide character only need two display-unit.
        //Do you really think that you need to process some text with display-unit more that 2 in console? Well, if you think so, you win.
        {
            renderCharToBuffer(buffer, i, y, ((utf8char*)strAt(str, i - x))->data, getUtf8charLength(*(utf8char*)strAt(str, i - x)));
            buffer->bufferArea[y][i].isConnectedLast = false;
            buffer->bufferArea[y][i].isConnectedNext = true;
            if (i != buffer->w - 1)
            {
                i++;
                renderCharToBuffer(buffer, i, y, ((utf8char*)strAt(str, i - x))->data, getUtf8charLength(*(utf8char*)strAt(str, i - x)));
                buffer->bufferArea[y][i].isConnectedNext = false;
                buffer->bufferArea[y][i].isConnectedLast = true;
            }
        }
    }
}
int32_t scrollOffsetToStrIndex(FoString* str, int scrollOffset)
{
    if (str == NULL)
    {
        return 0;
    }
    if (str->mapTable == NULL)
    {
        return 0;
    }

    int vecSize = vecGetSize(str->mapTable);
    if (vecSize == 0)
    {
        return 0;
    }
    
    for (int i = 0; i < vecSize; i++)
    {
        if (*((int32_t*)vecAt(str->mapTable, i)) >= scrollOffset)
        {
            return i;
        }
    }
    return STRING_NPOS;
}
int32_t scrollOffsetToDisplayStrIndex(FoString* str, int scrollOffset)
{
    if (str == NULL || str->mapTable == NULL)
    {
        return 0;
    }
    int displayPos = 0;
    for (int i = 0; i < vecGetSize(str->mapTable); i++)
    {
        utf8char* ch = strAt(str, i);
        if (ch == NULL)
        {
            continue;
        }
        int charWidth = getUtf8charDisplayWidth(ch);
        if (displayPos + charWidth > scrollOffset)
        {
            return i;
        }
        displayPos += charWidth;
    }
    return 0;
}
int strIndexToBufferCellPosX(FoString* str, int index, int scrollOffset)
{
    int bufferCellPosX = 0;
    int indexStart = scrollOffsetToStrIndex(str, scrollOffset);
    for (int i = indexStart; i < index && i < strGetLength(str); i++)
    {
        bufferCellPosX += getUtf8charDisplayWidth(strAt(str, i));
    }
    return bufferCellPosX;
}