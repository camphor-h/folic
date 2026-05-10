#include "textfile.h"

#ifndef _WIN32
#include <errno.h>
#include <unistd.h>
#endif

#define MAX3(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

static char* foStrdup(const char* target)
{
    char* newStr = malloc(strlen(target) + 1);
    strcpy(newStr, target);
    return newStr;
}

char* expandPath(const char* path)
{
    if (path == NULL || path[0] == '\0')
    {
        return NULL;
    }

    if (path[0] == '~')
    {
#ifdef _WIN32
        char temp[32768];
        DWORD len = GetEnvironmentVariableA("USERPROFILE", temp, sizeof(temp));
        if (len == 0)
        {
            len = GetEnvironmentVariableA("HOME", temp, sizeof(temp));
        }
        if (len == 0)
        {
            temp[0] = '\0';
        }
#else
        const char* temp = getenv("HOME");
        if (temp == NULL)
        {
            temp = getenv("USERPROFILE");
        }
        if (temp == NULL)
        {
            return NULL;
        }
#endif
        size_t homeLen = strlen(temp);
        size_t pathLen = strlen(path);
        char* expanded = malloc(homeLen + pathLen);
        strcpy(expanded, temp);
        strcpy(expanded + homeLen, path + 1);
        return expanded;
    }

    char* dollar = strchr(path, '$');
    if (dollar != NULL)
    {
#ifdef _WIN32
        char temp[32768];
        if (GetEnvironmentVariableA("HOME", temp, sizeof(temp)) == 0 &&
            GetEnvironmentVariableA("USERPROFILE", temp, sizeof(temp)) == 0)
        {
            temp[0] = '\0';
        }
#else
        const char* temp = getenv("HOME");
        if (temp == NULL)
        {
            temp = "";
        }
#endif
        char* expanded = malloc(strlen(path) + strlen(temp) + 1);
        expanded[0] = '\0';

        const char* src = path;
        char* dst = expanded;

        while (*src)
        {
            if (*src == '$' && *(src + 1) == 'H' && *(src + 2) == 'O' && *(src + 3) == 'M' && *(src + 4) == 'E')
            {
                strcpy(dst, temp);
                dst += strlen(temp);
                src += 5;
            }
            else
            {
                *dst++ = *src++;
            }
        }
        *dst = '\0';
        return expanded;
    }

    return foStrdup(path);
}

int checkFilePermission(const char* filePath)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(filePath);
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return 1;
    }
    if (attr & FILE_ATTRIBUTE_READONLY)
    {
        return 0;
    }
    return 1;
#else
    if (access(filePath, W_OK) == 0)
    {
        return 1;
    }
    if (errno == ENOENT)
    {
        return 1;
    }
    return 0;
#endif
}

int checkFileReadable(const char* filePath)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(filePath);
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }
    return 1;
#else
    if (access(filePath, R_OK) == 0)
    {
        return 1;
    }
    return 0;
#endif
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
        char* expandedPath = expandPath(filePath);
        textFile->filePath = foStrdup(expandedPath ? expandedPath : filePath);
        textFile->fileName = foStrdup(getFileNameStartIndex(textFile->filePath) + textFile->filePath);
        FILE* testFile = fopen(textFile->filePath, "rb");
        if (testFile == NULL)
        {
            textFile->isModified = true;
            textFile->newlineType = DEFAULT_NEWLINE;
        }
        else
        {
            fclose(testFile);
            readFile(textFile);
            textFile->isModified = false;
        }
        if (expandedPath != NULL)
        {
            free(expandedPath);
        }
    }
    else
    {
        textFile->fileName = NULL;
        textFile->filePath = NULL;
        textFile->newlineType = DEFAULT_NEWLINE;
        textFile->isModified = false;
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
    char* expandedPath = expandPath(filePath);
    const char* finalPath = expandedPath ? expandedPath : filePath;
    if (textFile->filePath != NULL)
    {
        free(textFile->filePath);
    }
    if (textFile->fileName != NULL)
    {
        free(textFile->fileName);
    }
    textFile->filePath = foStrdup(finalPath);
    textFile->fileName = foStrdup(getFileNameStartIndex(finalPath) + finalPath);
    if (expandedPath != NULL)
    {
        free(expandedPath);
    }
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
    if (textFile->fileP == NULL)
    {
        //File does not exist, keep empty content
        //isModified remains false, will create file on first save
        return;
    }
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
    int countCR = 0;
    int countLF = 0;
    int countCRLF = 0;
    int countMax = 0;
    while ((tryer = fgetc(textFile->fileP)) != EOF)
    {
        //Firstly, we should handle about newline
        if (tryer == '\r') //CRLF?
        {
            int nextChar = fgetc(textFile->fileP);
            if (nextChar != '\n' && nextChar != EOF) //CR? (Nobody know why Classic MacOS use such strange newline)
            {
                ungetc(nextChar, textFile->fileP); //if not LF, put it back
                countCR++;
            }
            else if (nextChar == '\n')
            {
                countCRLF++;
            }
            curLine = lineAppend(curLine);
        }
        else if (tryer == '\n') //LF?
        {
            curLine = lineAppend(curLine);
            countLF++;
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

    countMax = MAX3(countCR, countLF, countCRLF);
    if (countMax == countLF) //Default, if all counts equally
    {
        textFile->newlineType = NEWLINE_LF;
    }
    else if (countMax == countCRLF)
    {
        textFile->newlineType = NEWLINE_CRLF;
    }
    else
    {
        textFile->newlineType = NEWLINE_CR;
    }
}

static void writeCR(FILE* file)
{
    fputc('\r', file);
}
static void writeLF(FILE* file)
{
    fputc('\n', file);
}
static void writeCRLF(FILE* file)
{
    fputc('\r', file);
    fputc('\n', file);
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

    void (*writeNewline)(FILE*) = writeLF;
    switch (textFile->newlineType)
    {
        case NEWLINE_LF:
            writeNewline = writeLF;
            break;
        case NEWLINE_CRLF:
            writeNewline = writeCRLF;
            break;
        case NEWLINE_CR:
            writeNewline = writeCR;
            break;
        case NEWLINE_MIXED:
            writeNewline = writeLF;
            break;
    }

    textFile->fileP = fopen(textFile->filePath, "wb");
    FoLine* curLine = textFile->firstLine;
    while (curLine != NULL)
    {
        fwrite(curLine->lineString->data, 1, strGetSize(curLine->lineString), textFile->fileP);
        if (curLine->next != NULL)
        {
            writeNewline(textFile->fileP);
        }
        curLine = curLine->next;
    }
    fclose(textFile->fileP);
    textFile->fileP = NULL;
    textFile->isModified = false;
}
