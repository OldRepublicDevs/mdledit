#include "frame.h"


INT_PTR CALLBACK SettingsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

    switch(message){
        case WM_INITDIALOG:
        {

        }
        break;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case 0://IDOK:
                {

                }
                break;
                case 1://IDCANCEL:
                {

                }
            }
        break;
        case WM_CLOSE:
        {
            EndDialog(hwnd, wParam);
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}
