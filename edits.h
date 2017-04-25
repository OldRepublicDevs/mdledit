#ifndef EDITS_H_INCLUDED
#define EDITS_H_INCLUDED

#include "general.h"
#include "MDL.h"
#include <windowsx.h>
#include <commctrl.h>

extern MDL Model;

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
    std::string sSelected;
    std::vector<int> * nKnownArray = nullptr;
    std::vector<char> * sBuffer = nullptr;

  public:
    static HWND hIntEdit;
    static HWND hUIntEdit;
    static HWND hFloatEdit;

    Edits();
    bool Run(HWND hParent, UINT nID);
    friend LRESULT CALLBACK EditsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void Cleanup(){
        nKnownArray = nullptr;
        sBuffer = nullptr;
        ShowWindow(hScrollVert, false);
        UpdateEdit();
    }
    void LoadData(){
        int nSel = TabCtrl_GetCurSel(hTabs);
        if(nSel != -1){
            TCITEM tcitem;
            tcitem.mask = TCIF_TEXT;
            std::string sName(255, '\0');
            tcitem.pszText = &sName;
            tcitem.cchTextMax = 255;
            TabCtrl_GetItem(hTabs, nSel, &tcitem);
            sSelected = sName.c_str();
        }
        else sSelected = "";
        if(sSelected == "MDL" && !Model.empty()){
            nKnownArray = &Model.GetKnownData();
            sBuffer = &Model.GetBuffer();
        }
        else if(sSelected == "MDX" && Model.Mdx){
            nKnownArray = &Model.Mdx->GetKnownData();
            sBuffer = &Model.Mdx->GetBuffer();
        }
        else if(sSelected == "WOK" && Model.Wok){
            nKnownArray = &Model.Wok->GetKnownData();
            sBuffer = &Model.Wok->GetBuffer();
        }
        else if(sSelected == "PWK" && Model.Pwk){
            nKnownArray = &Model.Pwk->GetKnownData();
            sBuffer = &Model.Pwk->GetBuffer();
        }
        else if(sSelected == "DWK 0" && Model.Dwk0){
            nKnownArray = &Model.Dwk0->GetKnownData();
            sBuffer = &Model.Dwk0->GetBuffer();
        }
        else if(sSelected == "DWK 1" && Model.Dwk1){
            nKnownArray = &Model.Dwk1->GetKnownData();
            sBuffer = &Model.Dwk1->GetBuffer();
        }
        else if(sSelected == "DWK 2" && Model.Dwk2){
            nKnownArray = &Model.Dwk2->GetKnownData();
            sBuffer = &Model.Dwk2->GetBuffer();
        }
        else{
            nKnownArray = nullptr;
            sBuffer = nullptr;
        }

        if(!bShowHex) return;
        if(sBuffer != nullptr && nKnownArray != nullptr){
            if(sBuffer->empty()) ShowWindow(hScrollVert, false);
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
            yMaxScroll = ((sBuffer->size() - 1)/16 + 2) * ME_EDIT_NEXT_ROW;
            yCurrentScroll = 0;
        }
        else{
            SetClassLongPtr(hMe, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
            ShowWindow(hScrollVert, false);
        }
        UpdateEdit();
        UpdateStatusBar();
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) "");
        if(DEBUG_LEVEL > 80) std::cout<<"New MaxScroll: "<<yMaxScroll<<", new CurrentScroll: "<<yCurrentScroll<<"\n";
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
    void ShowHideEdit(){
        ShowWindow(hMe, bShowHex);
        if(bShowHex){
            LoadData();
        }
    }
    void UpdateEdit(){
        if(!bShowHex) return;
        UpdateClientRect();
        InvalidateRect(hMe, &rcClient, false);
        if(sBuffer == nullptr) return;
        if(!sBuffer->empty()){
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nMax = yMaxScroll;
            si.nMin = 0;
            si.nPos = yCurrentScroll;
            si.nPage = rcClient.bottom;
            SetScrollInfo(hScrollVert, SB_CTL, &si, true);
            if(DEBUG_LEVEL > 80) std::cout<<"GetScrollInfo(): max: "<<si.nMax<<", min: "<<si.nMin<<", page: n/a, current: "<<si.nPos<<"\n";
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
        nSelectEnd = std::min(nSelectEnd, (int) sBuffer->size() - 1);
        nSelectStart = std::min(nSelectStart, (int) sBuffer->size() - 1);
        if(DEBUG_LEVEL > 100) std::cout<<"Current selection from byte "<<nSelectStart<<" to byte "<<nSelectEnd<<".\n";
    }
    void UpdateStatusBar(bool bCheck = true){
        char cString1 [255];
        char cString2 [255];
        char cString3 [255];
        if(nSelectEnd == -1 || nSelectStart == -1 || !bCheck || sBuffer == nullptr){
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) "");
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) "");
            SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) "");
            SetWindowText(hIntEdit, "");
            SetWindowText(hUIntEdit, "");
            SetWindowText(hFloatEdit, "");
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
