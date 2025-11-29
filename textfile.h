#ifndef __FOLIC_TEXTFILE__
#define __FOLIC_TEXTFILE__
#include "folic.h"
#include "types.h"
typedef struct FoTextFile
{
    char* fileName;
    char* filePath;
    FoLine* firstLine;
    FILE* fileP;
}FoTextFile;

FoTextFile* createTextFile(const char* filePath);
void setTextFilePath(FoTextFile* textFile, const char* filePath);
void freeTextFile(FoTextFile* textFile);
void writeFile(FoTextFile* textFile);
void readFile(FoTextFile* textFile);
#endif