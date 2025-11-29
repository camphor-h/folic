#include "folic.h"

#ifdef DOC_PATH
static const char* WELCOME_DOC_PATH = DOC_PATH "/welcome.txt";
#else
static const char* WELCOME_DOC_PATH = "./doc/welcome.txt";
#endif

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    noecho();
    raw();
    start_color();
    initColorPairs();
    keypad(stdscr, TRUE);
    meta(stdscr, FALSE); //if you set it as FALSE, the alt key will be returned as ESC key
    FoTextFile* loadFile = NULL;
    if (argc > 1)
    {
        FILE* fileTryer = fopen(argv[1], "r");
        if (fileTryer != NULL)
        {
            fclose(fileTryer);
            loadFile = createTextFile(argv[1]);
        }
    }
    else
    {
        loadFile = createTextFile(WELCOME_DOC_PATH);
    }
    FoConsole* mainConsole = createConsole(loadFile);
    consoleMainLoop(mainConsole);
    freeConsole(mainConsole);
    endwin();
    return 0;
}