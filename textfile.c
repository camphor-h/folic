#include "textfile.h"


static char* foStrdup(const char* target)
{
    //In some place, it's not easy to use foStrdup(), because it's not in C standard. So I wrote this.
    char* newStr = malloc(sizeof(target) + 1);
    strcpy(newStr, target);
    return newStr;
}

static uint8_t getFileNameStartIndex(const char* filePath)
{
    char* lastLeftSlash = strrchr(filePath, '\\');
    char* lastRightSlash = strrchr(filePath, '/');
    char* lastSeparator = NULL;
    if (lastLeftSlash && lastRightSlash)
    {
        lastSeparator = ((lastLeftSlash > lastRightSlash) ? lastLeftSlash : lastRightSlash);
    }
    else if (lastLeftSlash)
    {
        lastSeparator = lastLeftSlash;
    }
    else if (lastRightSlash)
    {
        lastSeparator = lastRightSlash;
    }
    if (lastSeparator)
    {
        return (uint8_t)(lastSeparator - filePath + 1);
    }
    else
    {
        return 0;
    }
}
FoTextFile* createTextFile(const char* filePath)
{
    FoTextFile* textFile = malloc(sizeof(FoTextFile));
    textFile->firstLine = lineAppend(NULL);
    if (filePath != NULL)
    {
        textFile->filePath = foStrdup(filePath);
        textFile->fileName = foStrdup(getFileNameStartIndex(filePath) + filePath);
        readFile(textFile);
    }
    else
    {
        textFile->fileName = NULL;
        textFile->filePath = NULL;
    }
    textFile->fileP = NULL;
    return textFile;
}
void setTextFilePath(FoTextFile* textFile, const char* filePath)
{
    if (textFile == NULL || filePath == NULL)
    {
        return;
    }
    textFile->filePath = foStrdup(filePath);
    textFile->fileName = foStrdup(getFileNameStartIndex(filePath) + filePath);
}
void freeTextFile(FoTextFile* textFile)
{
    if (textFile == NULL)
    {
        return;
    }

    if (textFile->fileName != NULL)
    {
        free(textFile->fileName);
    }
    
    if (textFile->filePath != NULL)
    {
        free(textFile->filePath);
    }

    freeAllLineWithFirstLine(textFile->firstLine);
    free(textFile);
}

void readFile(FoTextFile* textFile)
{
    if (textFile == NULL)
    {
        return;
    }
    if (textFile->filePath == NULL || textFile->fileName == NULL)
    {
        return;
    }
    textFile->fileP = fopen(textFile->filePath, "rb");
    FoLine* curLine = lineAppend(NULL);
    if (textFile->firstLine != NULL)
    {
        freeAllLineWithFirstLine(textFile->firstLine);
    }
    textFile->firstLine = curLine;
    utf8char* tempUtf8char = NULL;
    uint8_t tempUtf8charBuffer[4];
    memset(tempUtf8charBuffer, '\0', 4);
    char tryer;
    while ((tryer = fgetc(textFile->fileP)) != EOF)
    {
        //Firstly, we should handle about newline
        if (tryer == '\r') //CRLF?
        {
            int nextChar = fgetc(textFile->fileP);
            if (nextChar != '\n' && nextChar != EOF) //CR? (Nobody know why Classic MacOS use such strange newline)
            {
                ungetc(nextChar, textFile->fileP); //if not LF, put it back
            }
            curLine = lineAppend(curLine);
        }
        else if (tryer == '\n') //LF?
        {
            curLine = lineAppend(curLine);
        }
        //then we handle about normal character
        else 
        {
            tempUtf8charBuffer[0] = (uint8_t)tryer;
            int length = getUtf8Length(tryer);
            for (int i = 1; i < length; i++)
            {
                tryer = fgetc(textFile->fileP);
                if (tryer == ERR) //if it gets wrong, throw the data it has received
                {
                    memset(tempUtf8charBuffer, '\0', 4);
                    break;
                }
                tempUtf8charBuffer[i] = (uint8_t)tryer;
            }
            tempUtf8char = createUtf8char(tempUtf8charBuffer);
            strPushBack(curLine->lineString, *tempUtf8char);
            freeUtf8char(tempUtf8char);
        }
    }
    fclose(textFile->fileP);
    textFile->fileP = NULL;
}
void writeFile(FoTextFile* textFile)
{
    if (textFile == NULL)
    {
        return;
    }
    if (textFile->fileName == NULL || textFile->filePath == NULL)
    {
        return;
    }
    textFile->fileP = fopen(textFile->filePath, "wb");
    FoLine* curLine = textFile->firstLine;
    while (curLine != NULL)
    {
        fwrite(curLine->lineString->data, 1, strGetSize(curLine->lineString), textFile->fileP);
        if (curLine->next != NULL)
        {
            fputc('\n', textFile->fileP);
        }
        curLine = curLine->next;
    }
    fclose(textFile->fileP);
    textFile->fileP = NULL;
}
