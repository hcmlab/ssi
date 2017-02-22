//BABYGRID code is copyrighted (C) 20002 by David Hillard
//
//This code must retain this copyright message
//
//Printed BABYGRID message reference and tutorial available.
//email: mudcat@mis.net for more information.

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "windows.h"

#define BGN_LBUTTONDOWN 0x0001
#define BGN_MOUSEMOVE   0x0002
#define BGN_OUTOFRANGE  0x0003
#define BGN_OWNERDRAW   0x0004
#define BGN_SELCHANGE   0x0005
#define BGN_ROWCHANGED  0x0006
#define BGN_COLCHANGED  0x0007
#define BGN_EDITBEGIN   0x0008
#define BGN_DELETECELL  0x0009
#define BGN_EDITEND     0x000A
#define BGN_F1          0x000B
#define BGN_F2          0x000C
#define BGN_F3          0x000D
#define BGN_F4          0x000E
#define BGN_F5          0x000F
#define BGN_F6          0x0010
#define BGN_F7          0x0011
#define BGN_F8          0x0012
#define BGN_F9          0x0013
#define BGN_F10         0x0014
#define BGN_F11         0x0015
#define BGN_F12         0x0016
#define BGN_GOTFOCUS    0x0017
#define BGN_LOSTFOCUS   0x0018
#define BGN_CELLCLICKED 0x0019

#define BGM_PROTECTCELL WM_USER + 1
#define BGM_SETPROTECT  WM_USER + 2
#define BGM_SETCELLDATA WM_USER + 3
#define BGM_GETCELLDATA WM_USER + 4
#define BGM_CLEARGRID   WM_USER + 5
#define BGM_SETGRIDDIM  WM_USER + 6
#define BGM_DELETECELL  WM_USER + 7
#define BGM_SETCURSORPOS WM_USER + 8
#define BGM_AUTOROW     WM_USER + 9
#define BGM_GETOWNERDRAWITEM WM_USER + 10
#define BGM_SETCOLWIDTH WM_USER + 11
#define BGM_SETHEADERROWHEIGHT WM_USER + 12
#define BGM_GETTYPE     WM_USER + 13
#define BGM_GETPROTECTION WM_USER + 14
#define BGM_DRAWCURSOR  WM_USER + 15
#define BGM_SETROWHEIGHT WM_USER + 16
#define BGM_SETCURSORCOLOR WM_USER + 17
#define BGM_SETPROTECTCOLOR WM_USER + 18
#define BGM_SETUNPROTECTCOLOR WM_USER + 19
#define BGM_SETROWSNUMBERED WM_USER + 20
#define BGM_SETCOLSNUMBERED WM_USER + 21 
#define BGM_SHOWHILIGHT WM_USER + 22
#define BGM_GETROWS WM_USER + 23
#define BGM_GETCOLS WM_USER + 24
#define BGM_NOTIFYROWCHANGED WM_USER + 25
#define BGM_NOTIFYCOLCHANGED WM_USER + 26
#define BGM_GETROW WM_USER + 27
#define BGM_GETCOL WM_USER + 28
#define BGM_PAINTGRID WM_USER + 29
#define BGM_GETCOLWIDTH WM_USER + 30
#define BGM_GETROWHEIGHT WM_USER + 31
#define BGM_GETHEADERROWHEIGHT WM_USER + 32
#define BGM_SETTITLEHEIGHT WM_USER + 33

#define BGM_SETHILIGHTCOLOR WM_USER + 34
#define BGM_SETHILIGHTTEXTCOLOR WM_USER + 35
#define BGM_SETEDITABLE WM_USER + 36
#define BGM_SETGRIDLINECOLOR WM_USER + 37
#define BGM_EXTENDLASTCOLUMN WM_USER + 38
#define BGM_SHOWINTEGRALROWS WM_USER + 39
#define BGM_SETELLIPSIS WM_USER + 40
#define BGM_SETCOLAUTOWIDTH WM_USER + 41
#define BGM_SETALLOWCOLRESIZE WM_USER + 42
#define BGM_SETTITLEFONT WM_USER + 43
#define BGM_SETHEADINGFONT WM_USER + 44

struct _BGCELL {
    int row;
    int col;
};

//function forward declarations
ATOM RegisterGridClass(HINSTANCE);
LRESULT CALLBACK GridProc(HWND, UINT, WPARAM, LPARAM);
void SetCell(_BGCELL *cell,int row, int col);

#endif

