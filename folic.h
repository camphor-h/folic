#ifndef __FOLIC__
#define __FOLIC__

#ifdef _WIN32
#include <ncursesw/ncurses.h>
#include <ncursesw/menu.h>
#else
#include <ncurses.h>
#include <menu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <locale.h>

#include "text/type.h"
#include "history.h"
#include "line.h"
#include "message.h"
#include "window.h"
#include "console.h"
#include "buffer.h"
#include "textfile.h"
#include "keyboard.h"

#define TEST_READY mvwprintw(stdscr, 0, 0, "Ready! File:%s Row:%d!\n", __FILE__, __LINE__ - 1)
#define TEST_PAUSE mvwprintw(stdscr, 0, 0, "Ready! File:%s Row:%d!\n", __FILE__, __LINE__ - 1);getch()

#endif