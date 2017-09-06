#ifndef REPORTDLG_H_INCLUDED
#define REPORTDLG_H_INCLUDED

#include "general.h"
#include "MDL.h"

class ReportDlgWindow{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;
    MDL * MdlPtr = nullptr;

  public:
    HWND hMe;
    ReportDlgWindow(MDL & Mdl);
    bool Run();
    void SetData(MDL & Mdl){
        MdlPtr = &Mdl;
    }
    friend LRESULT CALLBACK ReportDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // REPORTDLG_H_INCLUDED
