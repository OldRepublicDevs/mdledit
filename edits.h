#ifndef EDITS_H_INCLUDED
#define EDITS_H_INCLUDED

#include "general.h"
#include "MDL.h"
#include <windowsx.h>
#include <commctrl.h>

class Edits{
    //Control Creation
    WNDCLASSEX WindowClass;
    static char cClassName[];
    HWND hMe = NULL;
    HWND hScrollVert;
    RECT rcClient;

    char * cText;

    int yCurrentScroll;   /* current vertical scroll value   */
    int yMaxScroll;       /* max vertical scroll value       */
    POINT ptHover;
    POINT ptPrevious;
    POINT ptClick;
    POINT ptRelease;
    int nSelectStart;
    int nSelectEnd;
    bool bSelection;
    std::vector<int> nKnownArray;
    std::vector<char> sBuffer;
    int nBufferSize;

    public:
    static HWND hIntEdit;
    static HWND hUIntEdit;
    static HWND hFloatEdit;

    Edits();
    bool Run(HWND hParent, UINT nID);
    friend LRESULT CALLBACK EditsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void LoadData(){
        int nSel = TabCtrl_GetCurSel(hTabs);
        if(nSel == 0){
            nKnownArray = Model.GetKnownData();
            sBuffer = Model.GetBuffer();
            nBufferSize = Model.GetBufferLength();
        }
        else if(nSel == 1){
            nKnownArray = Mdx.GetKnownData();
            sBuffer = Mdx.GetBuffer();
            nBufferSize = Mdx.GetBufferLength();
        }
        else if(nSel == 2){
            nKnownArray = Walkmesh.GetKnownData();
            sBuffer = Walkmesh.GetBuffer();
            nBufferSize = Walkmesh.GetBufferLength();
        }
        else Error("Trying to select a tab that does not exist! (Don't ask me how that's possible)");

        if(nBufferSize == 0 || sBuffer.empty()) ShowWindow(hScrollVert, false);
        else ShowWindow(hScrollVert, true);

        SetClassLongPtr(hMe, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
        ptHover.x = -1;
        ptHover.y = -1;
        ptPrevious.x = -1;
        ptPrevious.y = -1;
        ptClick.x = -1;
        ptClick.y = -1;
        ptRelease.x = -1;
        ptRelease.y = -1;
        nSelectStart = -1;
        nSelectEnd = -1;
        bSelection = false;
        yMaxScroll = ((nBufferSize - 1)/16 + 2) * ME_EDIT_NEXT_ROW;
        yCurrentScroll = 0;
        UpdateEdit();
        UpdateStatusBar();
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) "");
        if(DEBUG_LEVEL > 80) std::cout<<string_format("New MaxScroll: %i, new CurrentScroll: %i \n", yMaxScroll, yCurrentScroll);
    }
    HWND GetWindowHandle(){
        return hMe;
    }
    void UpdateClientRect(){
        RECT rcParent;
        GetClientRect(GetParent(hMe), &rcParent);
        rcClient.top = 0;
        rcClient.left = ME_HEX_WIN_OFFSET_X;
        rcClient.bottom = rcParent.bottom - ME_STATUSBAR_Y - ME_TABS_OFFSET_Y_BOTTOM - ME_TABS_OFFSET_Y_TOP;
        rcClient.right = ME_HEX_WIN_SIZE_X;
    }
    void UpdateEdit(){
        UpdateClientRect();
        InvalidateRect(hMe, &rcClient, false);
        if(!sBuffer.empty()){
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nMax = yMaxScroll;
            si.nMin = 0;
            si.nPos = yCurrentScroll;
            si.nPage = rcClient.bottom;
            SetScrollInfo(hScrollVert, SB_CTL, &si, true);
            if(DEBUG_LEVEL > 80) std::cout<<string_format("GetScrollInfo(): max: %i, min: %i, page: n/a, current: %i \n", si.nMax, si.nMin, si.nPos);
        }
    }
    void Resize(){
        UpdateClientRect();
        SetWindowPos(hMe, NULL, rcClient.left, ME_TABS_OFFSET_Y_TOP, ME_HEX_WIN_SIZE_X, rcClient.bottom, NULL);
        SetWindowPos(hScrollVert, NULL, ME_HEX_WIN_OFFSET_X + ME_HEX_WIN_SIZE_X - ME_SCROLLBAR_X, 0, GetSystemMetrics(SM_CXHTHUMB), rcClient.bottom, NULL);
        UpdateEdit();
    }
    void DetermineSelection(){
        POINT ptLow;
        POINT ptHigh;
        if(ptClick.y > ptHover.y || (ptClick.y == ptHover.y && ptClick.x > ptHover.x)){
            ptHigh = ptClick;
            ptLow = ptHover;
        }
        else{
            ptLow = ptClick;
            ptHigh = ptHover;
        }

        nSelectStart = ptLow.y * 16 + (ptLow.x + 1) / 3;
        if(ptHigh.x <= 0) nSelectEnd = ptHigh.y * 16 - 1;
        else nSelectEnd = ptHigh.y * 16 + (ptHigh.x - 1) / 3;
        if(nSelectEnd < nSelectStart){
            nSelectStart = -1;
            nSelectEnd = -1;
        }
        nSelectEnd = std::min(nSelectEnd, nBufferSize - 1);
        if(DEBUG_LEVEL > 100) std::cout<<string_format("Current selection from byte %i to byte %i.\n", nSelectStart, nSelectEnd);
    }
    void UpdateStatusBar(bool bCheck = true){
        char cString1 [255];
        char cString2 [255];
        char cString3 [255];
        if(nSelectEnd == -1 || nSelectStart == -1 || !bCheck){
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) "");
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) "");
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) "");
        }
        else if(nSelectStart == nSelectEnd){
            sprintf(cString1, "Offset: %i", nSelectStart);
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) cString1);
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) "");
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) "");
        }
        else{
            sprintf(cString1, "Offset: %i", nSelectStart);
            sprintf(cString2, "Block: %i-%i", nSelectStart, nSelectEnd);
            sprintf(cString3, "Length: %i", nSelectEnd - nSelectStart + 1);
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) cString1);
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) cString2);
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) cString3);
        }
    }
    void UpdateStatusPositionMdx();
    void UpdateStatusPositionModel();
    void PrintValues(bool bCheck = true);
};

COLORREF DataColor(int nDataKnown, bool bHilite);

#endif // EDITS_H_INCLUDED
