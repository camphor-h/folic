#ifndef __FOLIC_TEXTFILE__
#define __FOLIC_TEXTFILE__

#include <stdbool.h>
#include "text/string.h"
#include "line.h"

typedef enum FoTextFileNewlineType
{
    NEWLINE_CR = 0,
    NEWLINE_LF = 1,
    NEWLINE_CRLF = 2,
    NEWLINE_MIXED = 3
}FoTextFileNewlineType;

#define DEFAULT_NEWLINE NEWLINE_LF

typedef struct FoTextFile
{
    char* fileName;
    char* filePath;
    FoLine* firstLine;
    FILE* fileP;

    FoTextFileNewlineType newlineType;
    bool isModified;
}FoTextFile;

FoTextFile* createTextFile(const char* filePath);
void setTextFilePath(FoTextFile* textFile, const char* filePath);
void freeTextFile(FoTextFile* textFile);
void writeFile(FoTextFile* textFile);
void readFile(FoTextFile* textFile);
int checkFileReadable(const char* filePath);
char* expandPath(const char* path);

int checkFilePermission(const char* filePath);

#endif