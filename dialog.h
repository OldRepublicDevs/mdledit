#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

#include "general.h"

class DialogWindow{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;

  public:
    HWND hMe;
    DialogWindow();
    bool Run();
    friend LRESULT CALLBACK DialogWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // DIALOG_H_INCLUDED
