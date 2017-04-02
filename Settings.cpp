#include <windowsx.h>
#include "frame.h"

INT_PTR CALLBACK SettingsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    static bool bChange = false;
    switch(message){
        case WM_INITDIALOG:
        {
            bChange = false;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);
            MDL* Mdl = (MDL*) lParam;
            if(Mdl->bSmoothAngleWeighting) Button_SetCheck(GetDlgItem(hwnd, DLG_ID_ANGLE_WEIGHT), BST_CHECKED);
            if(Mdl->bSmoothAreaWeighting) Button_SetCheck(GetDlgItem(hwnd, DLG_ID_AREA_WEIGHT), BST_CHECKED);
        }
        break;
        case WM_COMMAND:
        {
            int nNotification = HIWORD(wParam);
            int nID = LOWORD(wParam);
            HWND hControl = (HWND) lParam;
            MDL * Mdl = nullptr;
            if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) Mdl = (MDL*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(Mdl != nullptr){
                bChange = true;
                switch(nID){
                    case DLG_ID_AREA_WEIGHT:
                    {
                        if(Button_GetCheck(hControl) == BST_CHECKED) Mdl->bSmoothAreaWeighting = true;
                        else Mdl->bSmoothAreaWeighting = false;
                    }
                    break;
                    case DLG_ID_ANGLE_WEIGHT:
                    {
                        if(Button_GetCheck(hControl) == BST_CHECKED) Mdl->bSmoothAngleWeighting = true;
                        else Mdl->bSmoothAreaWeighting = false;
                    }
                }
            }
        }
        break;
        case WM_CLOSE:
        {
            MDL * Mdl = nullptr;
            if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) Mdl = (MDL*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(Mdl != nullptr){
                if(bChange && Mdl->GetFileData()) MessageBox(hwnd, "You need to reload the current model for the changes to take effect.", "Note", MB_OK | MB_ICONINFORMATION);
            }
            EndDialog(hwnd, wParam);
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}
