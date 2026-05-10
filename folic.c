#include "folic.h"
#include "console.h"
#include "mouse/mouse.h"

#ifdef DOC_PATH
static const char* WELCOME_DOC_PATH = DOC_PATH "/welcome.txt";
#else
static const char* WELCOME_DOC_PATH = "./doc/welcome.txt";
#endif

#define CURRENT_VERSION "0.15.0"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    raw();
    start_color();
    initColorPairs();
    keypad(stdscr, TRUE);
    meta(stdscr, FALSE); //if you set it as FALSE, the alt key will be returned as ESC key
    mousemask(BUTTON1_PRESSED | BUTTON3_PRESSED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL); //Enable mouse wheel support
    mouseBackendInit();
    FoTextFile* loadFile = NULL;
    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            printf("Folic Editor\n");
            printf("Version %s\n", CURRENT_VERSION);
            printf("Usage: folic\n");
            printf("       folic [File Name]\n");
            printf("Read the about and update log to get more information.\n");
            return 0;
        }
        else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
        {
            printf("Version %s\n", CURRENT_VERSION);
            return 0;
        }
        else
        {
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
    mouseBackendFree();
    endwin();
    return 0;
}