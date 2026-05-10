#include "syntax.h"
#include <regex.h>
#include <dirent.h>
#include <strings.h>

#ifndef SYNTAX_PATH
#define SYNTAX_PATH "./syntax"
#endif

static int colorNameToPair(const char* name)
{
    if (strcasecmp(name, "RED") == 0) return BUFFERCELL_COLOR_RED;
    if (strcasecmp(name, "GREEN") == 0) return BUFFERCELL_COLOR_GREEN;
    if (strcasecmp(name, "YELLOW") == 0) return BUFFERCELL_COLOR_YELLOW;
    if (strcasecmp(name, "BLUE") == 0) return BUFFERCELL_COLOR_BLUE;
    if (strcasecmp(name, "MAGENTA") == 0) return BUFFERCELL_COLOR_MAGENTA;
    if (strcasecmp(name, "CYAN") == 0) return BUFFERCELL_COLOR_CYAN;
    if (strcasecmp(name, "WHITE") == 0) return BUFFERCELL_COLOR_WHITE;
    if (strcasecmp(name, "BRIGHT_RED") == 0) return BUFFERCELL_COLOR_BRIGHT_RED;
    if (strcasecmp(name, "BRIGHT_GREEN") == 0) return BUFFERCELL_COLOR_BRIGHT_GREEN;
    if (strcasecmp(name, "BRIGHT_YELLOW") == 0) return BUFFERCELL_COLOR_BRIGHT_YELLOW;
    if (strcasecmp(name, "BRIGHT_BLUE") == 0) return BUFFERCELL_COLOR_BRIGHT_BLUE;
    if (strcasecmp(name, "BRIGHT_MAGENTA") == 0) return BUFFERCELL_COLOR_BRIGHT_MAGENTA;
    if (strcasecmp(name, "BRIGHT_CYAN") == 0) return BUFFERCELL_COLOR_BRIGHT_CYAN;
    if (strcasecmp(name, "BRIGHT_WHITE") == 0) return BUFFERCELL_COLOR_BRIGHT_WHITE;
    if (strcasecmp(name, "BRIGHT_BLACK") == 0) return BUFFERCELL_COLOR_BRIGHT_BLACK;
    return 0;
}

static char* trimWhitespace(char* str)
{
    while (*str == ' ' || *str == '\t') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) *end-- = '\0';
    return str;
}

SyntaxDef* syntaxParseFile(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    SyntaxDef* def = calloc(1, sizeof(SyntaxDef));
    def->hasUsing = false;

    //Extract file extension from filename
    //For syntax files, the name before .syn represents the target extension
    //e.g., c.syn -> fileExtension = "c"
    const char* filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;
#ifdef _WIN32
    const char* winFilename = strrchr(filepath, '\\');
    if (winFilename) filename = winFilename + 1;
#endif
    const char* dot = strrchr(filename, '.');
    if (dot)
    {
        //Copy the part before .syn as the file extension
        int extLen = dot - filename;
        if (extLen > 0 && extLen < (int)sizeof(def->fileExtension) - 1)
        {
            strncpy(def->fileExtension, filename, extLen);
            def->fileExtension[extLen] = '\0';
        }
    }

    char line[512];
    bool firstLine = true;
    
    while (fgets(line, sizeof(line), file))
    {
        char* trimmed = trimWhitespace(line);
        if (trimmed[0] == '#' || trimmed[0] == '\0') continue; //Skip comments and empty lines
        
        //Check for "using" directive
        if (firstLine && strncmp(trimmed, "using", 5) == 0)
        {
            char* quoteStart = strchr(trimmed, '"');
            char* quoteEnd = strchr(quoteStart + 1, '"');
            if (quoteStart && quoteEnd)
            {
                *quoteEnd = '\0';
                strncpy(def->usingFile, quoteStart + 1, sizeof(def->usingFile) - 1);
                def->hasUsing = true;
                fclose(file);
                return def;
            }
        }
        firstLine = false;
        
        //Parse rule: "pattern" = COLOR
        char* equalsPos = strchr(trimmed, '=');
        if (equalsPos)
        {
            *equalsPos = '\0';
            char* pattern = trimWhitespace(trimmed);
            char* colorName = trimWhitespace(equalsPos + 1);

            //Remove quotes from pattern
            if (pattern[0] == '"') pattern++;
            char* endQuote = strrchr(pattern, '"');
            if (endQuote) *endQuote = '\0';

            //Check if it's a simple keyword (alphanumeric only) or a regex pattern
            bool isKeyword = true;
            for (int i = 0; pattern[i]; i++)
            {
                if (!isalnum(pattern[i]) && pattern[i] != '_')
                {
                    isKeyword = false;
                    break;
                }
            }

            if (isKeyword && def->keywordCount < SYNTAX_MAX_KEYWORDS)
            {
                strncpy(def->keywords[def->keywordCount], pattern, sizeof(def->keywords[0]) - 1);
                def->keywordCount++;
                //Store keyword color in rules for reference
                if (def->ruleCount < SYNTAX_MAX_RULES)
                {
                    SyntaxRule* rule = &def->rules[def->ruleCount++];
                    strncpy(rule->pattern, pattern, sizeof(rule->pattern) - 1);
                    rule->color = colorNameToPair(colorName);
                    rule->type = SYNTAX_RULE_KEYWORD;
                    rule->regex = NULL;
                }
            }
            else if (!isKeyword && def->ruleCount < SYNTAX_MAX_RULES)
            {
                SyntaxRule* rule = &def->rules[def->ruleCount++];
                strncpy(rule->pattern, pattern, sizeof(rule->pattern) - 1);
                rule->color = colorNameToPair(colorName);
                rule->type = SYNTAX_RULE_PATTERN;

                //Compile regex
                rule->regex = malloc(sizeof(regex_t));
                if (regcomp((regex_t*)rule->regex, pattern, REG_EXTENDED | REG_NEWLINE) != 0)
                {
                    free(rule->regex);
                    rule->regex = NULL;
                }
            }
        }
    }
    
    fclose(file);
    return def;
}

SyntaxHighlighter* createSyntaxHighlighter(const char* syntaxDir)
{
    SyntaxHighlighter* highlighter = malloc(sizeof(SyntaxHighlighter));
    highlighter->currentSyntax = NULL;
    highlighter->syntaxes = NULL;
    highlighter->syntaxCount = 0;
    highlighter->initialized = false;

    const char* dir = syntaxDir ? syntaxDir : SYNTAX_PATH;
    strncpy(highlighter->syntaxDir, dir, sizeof(highlighter->syntaxDir) - 1);

    syntaxLoadAll(highlighter);
    highlighter->initialized = true;

    return highlighter;
}

void freeSyntaxHighlighter(SyntaxHighlighter* highlighter)
{
    if (highlighter == NULL)
    {
        return;
    }

    for (int i = 0; i < highlighter->syntaxCount; i++)
    {
        for (int j = 0; j < highlighter->syntaxes[i].ruleCount; j++)
        {
            if (highlighter->syntaxes[i].rules[j].regex)
            {
                regfree((regex_t*)highlighter->syntaxes[i].rules[j].regex);
                free(highlighter->syntaxes[i].rules[j].regex);
            }
        }
    }
    free(highlighter->syntaxes);
    free(highlighter);
}

SyntaxDef* syntaxLoadFromFile(SyntaxHighlighter* highlighter, const char* filename)
{
    char filepath[SYNTAX_MAX_PATH_LEN + 256];
    snprintf(filepath, sizeof(filepath), "%s/%s", highlighter->syntaxDir, filename);
    
    SyntaxDef* def = syntaxParseFile(filepath);
    if (!def) return NULL;
    
    //Handle "using" directive
    if (def->hasUsing)
    {
        SyntaxDef* referenced = syntaxLoadFromFile(highlighter, def->usingFile);
        free(def);
        return referenced;
    }
    
    //Add to syntaxes array
    highlighter->syntaxes = realloc(highlighter->syntaxes, (highlighter->syntaxCount + 1) * sizeof(SyntaxDef));
    memcpy(&highlighter->syntaxes[highlighter->syntaxCount], def, sizeof(SyntaxDef));
    highlighter->syntaxCount++;
    
    free(def);
    return &highlighter->syntaxes[highlighter->syntaxCount - 1];
}

void syntaxLoadAll(SyntaxHighlighter* highlighter)
{
    DIR* dir = opendir(highlighter->syntaxDir);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, ".syn"))
        {
            syntaxLoadFromFile(highlighter, entry->d_name);
        }
    }
    closedir(dir);
}

SyntaxDef* syntaxGetForFileExtension(SyntaxHighlighter* highlighter, const char* fileExtension)
{
    for (int i = 0; i < highlighter->syntaxCount; i++)
    {
        if (strcmp(highlighter->syntaxes[i].fileExtension, fileExtension) == 0)
        {
            return &highlighter->syntaxes[i];
        }
    }
    return NULL;
}

void syntaxApplyToLine(SyntaxHighlighter* highlighter, FoString* lineStr, Buffer* buffer, int y, int scrollOffset, int startX)
{
    if (!highlighter->currentSyntax || !lineStr || !buffer) return;

    SyntaxDef* def = highlighter->currentSyntax;

    //Convert FoString to C string for regex matching
    const char* lineCStr = strCstr(lineStr);
    if (!lineCStr) return;

    int lineLen = strlen(lineCStr);
    
    //Apply pattern rules
    for (int r = 0; r < def->ruleCount; r++)
    {
        SyntaxRule* rule = &def->rules[r];
        if (!rule->regex) continue;
        
        regmatch_t pmatch[1];
        int offset = 0;
        const char* searchStr = lineCStr;
        
        while (regexec((regex_t*)rule->regex, searchStr, 1, pmatch, 0) == 0)
        {
            int matchStart = offset + pmatch[0].rm_so;
            int matchEnd = offset + pmatch[0].rm_eo;
            
            //Apply color to matched region
            for (int i = matchStart; i < matchEnd && i < lineLen; i++)
            {
                //Calculate display position
                int displayPos = i - scrollOffset;
                int targetX = startX + displayPos;
                if (targetX >= 0 && targetX < buffer->w)
                {
                    buffer->bufferArea[y][targetX].colorPair = rule->color;
                }
            }
            
            offset = matchEnd;
            searchStr = lineCStr + offset;
            if (offset >= lineLen) break;
        }
    }
    
    //Apply keyword rules (simple word boundary matching)
    for (int k = 0; k < def->keywordCount; k++)
    {
        const char* keyword = def->keywords[k];
        int keywordLen = strlen(keyword);
        short keywordColor = 0;
        //Find the color for this keyword
        for (int r = 0; r < def->ruleCount; r++)
        {
            if (def->rules[r].type == SYNTAX_RULE_KEYWORD && strcmp(def->rules[r].pattern, keyword) == 0)
            {
                keywordColor = def->rules[r].color;
                break;
            }
        }
        if (keywordColor == 0) continue;

        const char* pos = lineCStr;

        while ((pos = strstr(pos, keyword)) != NULL)
        {
            int keywordStart = pos - lineCStr;
            int keywordEnd = keywordStart + keywordLen;

            //Check word boundaries
            bool isStartWord = (keywordStart == 0 || !isalnum(lineCStr[keywordStart - 1]));
            bool isEndWord = (keywordEnd >= lineLen || !isalnum(lineCStr[keywordEnd]));

            if (isStartWord && isEndWord)
            {
                for (int i = keywordStart; i < keywordEnd && i < lineLen; i++)
                {
                    int displayPos = i - scrollOffset;
                    int targetX = startX + displayPos;
                    if (targetX >= 0 && targetX < buffer->w)
                    {
                        buffer->bufferArea[y][targetX].colorPair = keywordColor;
                    }
                }
            }
            pos += keywordLen;
        }
    }
}

typedef struct BracketInfo
{
    char type;
    int fileLineIndex; //Actual line number in file
    int charIndex;     //Character index in line
    int screenY;       //Screen position (-1 if off-screen)
    int screenX;
} BracketInfo;

static void checkBracketMatching(FoTextArea* textArea)
{
    if (!textArea || !textArea->topLine || !textArea->textSource->firstLine) return;

    //Phase 1: Extract all bracket/quote symbols from file start to end of visible area
    BracketInfo symbols[2048];
    int symbolCount = 0;

    FoLine* curLine = textArea->textSource->firstLine;
    int fileLineIdx = 0;
    int visibleStartLine = 0;
    //Find the file line index of topLine
    FoLine* temp = textArea->textSource->firstLine;
    while (temp && temp != textArea->topLine)
    {
        visibleStartLine++;
        temp = temp->next;
    }

    int visibleEndLine = visibleStartLine + textArea->window->h;

    while (curLine && fileLineIdx < visibleEndLine)
    {
        const char* lineCStr = strCstr(curLine->lineString);
        int lineLen = strlen(lineCStr);
        bool isVisible = (fileLineIdx >= visibleStartLine);
        int screenY = isVisible ? (fileLineIdx - visibleStartLine) : -1;

        for (int i = 0; i < lineLen; i++)
        {
            char c = lineCStr[i];

            //Check for string literals
            if (c == '"')
            {
                //Double quotes are always treated as string delimiters
                if (isVisible)
                {
                    int displayPos = i - textArea->scrollOffset;
                    if (displayPos >= 0 && displayPos < textArea->window->buffer->w)
                    {
                        textArea->window->buffer->bufferArea[screenY][displayPos].colorPair = BUFFERCELL_COLOR_GREEN;
                    }
                }
                i++;
                while (i < lineLen)
                {
                    char sc = lineCStr[i];
                    if (isVisible)
                    {
                        int strDisplayPos = i - textArea->scrollOffset;
                        if (strDisplayPos >= 0 && strDisplayPos < textArea->window->buffer->w)
                        {
                            textArea->window->buffer->bufferArea[screenY][strDisplayPos].colorPair = BUFFERCELL_COLOR_GREEN;
                        }
                    }
                    if (sc == '\\')
                    {
                        i++;
                        continue;
                    }
                    if (sc == c)
                    {
                        if (isVisible)
                        {
                            int closeDisplayPos = i - textArea->scrollOffset;
                            if (closeDisplayPos >= 0 && closeDisplayPos < textArea->window->buffer->w)
                            {
                                textArea->window->buffer->bufferArea[screenY][closeDisplayPos].colorPair = BUFFERCELL_COLOR_GREEN;
                            }
                        }
                        i++;
                        break;
                    }
                    i++;
                }
                continue;
            }
            else if (c == '\'')
            {
                //Single quotes: only treat as string if there's a matching closing quote
                int quoteStart = i;
                int j = i + 1;
                bool foundClose = false;
                while (j < lineLen)
                {
                    char sc = lineCStr[j];
                    if (sc == '\\')
                    {
                        j++;
                        continue;
                    }
                    if (sc == '\'')
                    {
                        foundClose = true;
                        break;
                    }
                    j++;
                }

                if (foundClose)
                {
                    //Mark the whole string green
                    if (isVisible)
                    {
                        for (int k = quoteStart; k <= j && k < lineLen; k++)
                        {
                            int displayPos = k - textArea->scrollOffset;
                            if (displayPos >= 0 && displayPos < textArea->window->buffer->w)
                            {
                                textArea->window->buffer->bufferArea[screenY][displayPos].colorPair = BUFFERCELL_COLOR_GREEN;
                            }
                        }
                    }
                    i = j;
                }
                //If not found close, don't mark as string - just continue normally
                continue;
            }

            //Extract bracket symbols
            if (c == '{' || c == '}' || c == '[' || c == ']' || c == '(' || c == ')')
            {
                if (symbolCount < 2048)
                {
                    symbols[symbolCount].type = c;
                    symbols[symbolCount].fileLineIndex = fileLineIdx;
                    symbols[symbolCount].charIndex = i;
                    symbols[symbolCount].screenY = screenY;
                    symbols[symbolCount].screenX = isVisible ? (i - textArea->scrollOffset) : -1;
                    symbolCount++;
                }
            }
        }
        curLine = curLine->next;
        fileLineIdx++;
    }

    //Phase 2: Match brackets from extracted symbols
    int braceStack[1024];
    int bracketStack[1024];
    int parenStack[1024];
    int braceTop = 0, bracketTop = 0, parenTop = 0;

    for (int i = 0; i < symbolCount; i++)
    {
        char c = symbols[i].type;

        if (c == '{')
        {
            if (braceTop < 1024) braceStack[braceTop++] = i;
        }
        else if (c == '[')
        {
            if (bracketTop < 1024) bracketStack[bracketTop++] = i;
        }
        else if (c == '(')
        {
            if (parenTop < 1024) parenStack[parenTop++] = i;
        }
        else if (c == '}')
        {
            if (braceTop > 0)
            {
                int openIdx = braceStack[--braceTop];
                //Color both visible
                if (symbols[openIdx].screenY >= 0 && symbols[openIdx].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[openIdx].screenY][symbols[openIdx].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
                if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
            }
            else if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
            {
                textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_RED;
            }
        }
        else if (c == ']')
        {
            if (bracketTop > 0)
            {
                int openIdx = bracketStack[--bracketTop];
                if (symbols[openIdx].screenY >= 0 && symbols[openIdx].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[openIdx].screenY][symbols[openIdx].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
                if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
            }
            else if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
            {
                textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_RED;
            }
        }
        else if (c == ')')
        {
            if (parenTop > 0)
            {
                int openIdx = parenStack[--parenTop];
                if (symbols[openIdx].screenY >= 0 && symbols[openIdx].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[openIdx].screenY][symbols[openIdx].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
                if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
                    textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_CYAN;
            }
            else if (symbols[i].screenY >= 0 && symbols[i].screenX >= 0)
            {
                textArea->window->buffer->bufferArea[symbols[i].screenY][symbols[i].screenX].colorPair = BUFFERCELL_COLOR_RED;
            }
        }
    }

    //Mark remaining unclosed open brackets as red (only if visible)
    for (int i = 0; i < braceTop; i++)
    {
        int idx = braceStack[i];
        if (symbols[idx].screenY >= 0 && symbols[idx].screenX >= 0)
            textArea->window->buffer->bufferArea[symbols[idx].screenY][symbols[idx].screenX].colorPair = BUFFERCELL_COLOR_RED;
    }
    for (int i = 0; i < bracketTop; i++)
    {
        int idx = bracketStack[i];
        if (symbols[idx].screenY >= 0 && symbols[idx].screenX >= 0)
            textArea->window->buffer->bufferArea[symbols[idx].screenY][symbols[idx].screenX].colorPair = BUFFERCELL_COLOR_RED;
    }
    for (int i = 0; i < parenTop; i++)
    {
        int idx = parenStack[i];
        if (symbols[idx].screenY >= 0 && symbols[idx].screenX >= 0)
            textArea->window->buffer->bufferArea[symbols[idx].screenY][symbols[idx].screenX].colorPair = BUFFERCELL_COLOR_RED;
    }
}

void syntaxHighlightTextArea(SyntaxHighlighter* highlighter, FoTextArea* textArea)
{
    if (!textArea) return;

    //Update current syntax based on file extension
    highlighter->currentSyntax = NULL;
    if (textArea->textSource && textArea->textSource->fileName)
    {
        char* dot = strrchr(textArea->textSource->fileName, '.');
        if (dot)
        {
            highlighter->currentSyntax = syntaxGetForFileExtension(highlighter, dot + 1);
        }
    }

    //Apply highlighting to visible lines
    FoLine* curLine = textArea->topLine;
    int y = 0;
    while (curLine && y < textArea->window->h)
    {
        if (highlighter->currentSyntax)
        {
            syntaxApplyToLine(highlighter, curLine->lineString, textArea->window->buffer, y, textArea->scrollOffset, 0);
        }
        curLine = curLine->next;
        y++;
    }

    //Check bracket matching for all file types
    checkBracketMatching(textArea);
}
