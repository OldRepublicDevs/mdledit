#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED

#include "general.h"
#include <fstream>
#include <commctrl.h>
#include <Shlwapi.h>

class Frame{
    //Main Window Creation
    WNDCLASSEX WindowClass;
    static HINSTANCE hInstance;
    static char cClassName[];
    HWND hMe;

    public:
    //Main Window Creation
    static LRESULT CALLBACK FrameProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    Frame(HINSTANCE hInstanceCreate);
    bool Run(int nCmdShow);
};

#define ACTION_UPDATE_DISPLAY      0
#define ACTION_ADD_MENU_LINES      1
#define ACTION_OPEN_VIEWER         2
#define ACTION_OPEN_GEO_VIEWER     3

void ProcessTreeAction(HTREEITEM hItem, const int & nAction, void * Pointer = NULL);
INT_PTR CALLBACK SettingsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif // FRAME_H_INCLUDED
