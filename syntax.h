#ifndef __FOLIC_SYNTAX__
#define __FOLIC_SYNTAX__

#include "folic.h"
#include "window.h"
#include "textfile.h"
#include "buffer.h"

#define SYNTAX_MAX_COLORS 16
#define SYNTAX_MAX_RULES 64
#define SYNTAX_MAX_PATTERN_LEN 256
#define SYNTAX_MAX_KEYWORDS 128
#define SYNTAX_MAX_PATH_LEN 512

typedef enum SyntaxColor
{
    SYNTAX_COLOR_DEFAULT = 0,
    SYNTAX_COLOR_RED,
    SYNTAX_COLOR_GREEN,
    SYNTAX_COLOR_YELLOW,
    SYNTAX_COLOR_BLUE,
    SYNTAX_COLOR_MAGENTA,
    SYNTAX_COLOR_CYAN,
    SYNTAX_COLOR_WHITE,
    SYNTAX_COLOR_BRIGHT_RED,
    SYNTAX_COLOR_BRIGHT_GREEN,
    SYNTAX_COLOR_BRIGHT_YELLOW,
    SYNTAX_COLOR_BRIGHT_BLUE,
    SYNTAX_COLOR_BRIGHT_MAGENTA,
    SYNTAX_COLOR_BRIGHT_CYAN,
    SYNTAX_COLOR_BRIGHT_WHITE,
    SYNTAX_COLOR_BRIGHT_BLACK
}SyntaxColor;

typedef enum SyntaxRuleType
{
    SYNTAX_RULE_KEYWORD,
    SYNTAX_RULE_PATTERN
}SyntaxRuleType;

typedef struct SyntaxRule
{
    SyntaxRuleType type;
    char pattern[SYNTAX_MAX_PATTERN_LEN];
    void* regex; //regex_t* from regex.h
    SyntaxColor color;
}SyntaxRule;

typedef struct SyntaxDef
{
    char name[64];
    char fileExtension[32];
    char usingFile[SYNTAX_MAX_PATH_LEN]; //for "using" directive
    bool hasUsing;
    
    char keywords[SYNTAX_MAX_KEYWORDS][64];
    int keywordCount;
    
    SyntaxRule rules[SYNTAX_MAX_RULES];
    int ruleCount;
    
    short colorPairs[SYNTAX_MAX_COLORS + 1]; //ncurses color pair IDs
}SyntaxDef;

typedef struct SyntaxHighlighter
{
    SyntaxDef* currentSyntax;
    SyntaxDef* syntaxes;
    int syntaxCount;
    char syntaxDir[SYNTAX_MAX_PATH_LEN];
    bool initialized;
}SyntaxHighlighter;

//Create and free
SyntaxHighlighter* createSyntaxHighlighter(const char* syntaxDir);
void freeSyntaxHighlighter(SyntaxHighlighter* highlighter);

//Load and manage syntax definitions
SyntaxDef* syntaxLoadFromFile(SyntaxHighlighter* highlighter, const char* filename);
SyntaxDef* syntaxGetForFileExtension(SyntaxHighlighter* highlighter, const char* fileExtension);
void syntaxLoadAll(SyntaxHighlighter* highlighter);

//Apply syntax highlighting to a line
void syntaxApplyToLine(SyntaxHighlighter* highlighter, FoString* lineStr, Buffer* buffer, int y, int scrollOffset, int startX);

//Highlight entire visible area
void syntaxHighlightTextArea(SyntaxHighlighter* highlighter, FoTextArea* textArea);

//Parse .syn file
SyntaxDef* syntaxParseFile(const char* filepath);

#endif
