
#ifndef CRAYONIZER_COMMON_H
#define CRAYONIZER_COMMON_H

#define _GNU_SOURCE 
#include "libUseful-2.1/libUseful.h"
#include <sys/types.h>
#include <glob.h>
#include <regex.h>
#include <signal.h>


//these go in highbytes of 'Crayon->Action->Attribs'
#define FLAG_HIDE			65536
#define FLAG_BOLD			131072
#define FLAG_FAINT		262144
#define FLAG_UNDER		524288
#define FLAG_BLINK		1048576
#define FLAG_INVERSE	2097152
#define FLAG_CAPS			4194304
#define FLAG_LOWERCASE 8388608
#define FLAG_CLR2EOL 16777216

#define FLAG_RESTORE_XTITLE 2
#define FLAG_DONTCRAYON 4
#define FLAG_DOING_PREPENDS 8
#define FLAG_DOING_APPENDS 16
#define FLAG_STRIP_ANSI 32
#define FLAG_EXPECT_LINES 64
#define FLAG_CURSOR_HOME 128
#define FLAG_STATUSBAR_INPUT 256

typedef enum {NONE, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET, RESET2, DARKGREY, LIGHTRED, LIGHTGREEN, LIGHTYELLOW, LIGHTBLUE, LIGHTMAGENTA, LIGHTCYAN, LIGHTGREY} EColors;

//these go in 'Crayon->Action->ActType'
typedef enum {ACTION_NONE, ACTION_ECHO, ACTION_SETSTR, ACTION_SETENV, ACTION_BASENAME, ACTION_SEND, ACTION_PLAYSOUND, ACTION_EXEC,  ACTION_PASSTO, ACTION_PASSINPUT, ACTION_SET_XTITLE, ACTION_RESTORE_XTITLE, ACTION_ALTSCREEN, ACTION_NORMSCREEN, ACTION_ARGS, ACTION_XTERM_BGCOLOR, ACTION_XTERM_FGCOLOR, ACTION_RXVT_BGCOLOR, ACTION_RXVT_FGCOLOR, ACTION_BGCOLOR, ACTION_FGCOLOR, ACTION_DONTCRAYON, ACTION_CLEARSCREEN, ACTION_REDRAW, ACTION_BELL, ACTION_FONT_UP, ACTION_FONT_DOWN, ACTION_EDIT, ACTION_REPLACE, ACTION_QUERYBAR, ACTION_SELECTBAR} ECrayonActions;

#define GOT_SIGTERM 524288
#define GOT_SIGINT 1048576
#define GOT_SIGWINCH 2097152
#define HAS_STATUSBAR 4194304


typedef enum {CRAYON_ACTION, CRAYON_ARGS, CRAYON_LINE, CRAYON_STRING, CRAYON_SECTION, CRAYON_LINENO, CRAYON_VALUE, CRAYON_MAPTO, CRAYON_LINEMAPTO, CRAYON_APPEND, CRAYON_PREPEND, CRAYON_CMDLINE, CRAYON_PASSINPUT, CONFIG_INCLUDE, CRAYON_KEYPRESS, CRAYON_IF, CRAYON_STATUSBAR, CRAYON_COMPARATOR} ECrayonTypes;

typedef struct t_crayon TCrayon;

//This structure holds all the information needed to crayon a line
struct t_crayon
{
uint8_t Type;
uint8_t ActType;
char *Match;
char Op;
float Value;
uint8_t Start, Len;
char *String;
int Attribs;
int ActionCount;
TCrayon *Actions;
}; 


extern char *Version;
extern int GlobalFlags;
extern TCrayon *KeyPresses;
extern int NoOfKeyPresses;
extern ListNode *Vars;
extern int ScreenRows;
extern int LineNo;
extern int cmdline_argc;
extern char **cmdline_argv;
extern STREAM *StdIn, *CommandPipe;
extern ListNode *ColorMatches, *Streams;

void CrayonizerProcessInputs();

#endif
