#include "edits.h"

char Edits::cClassName[] = "mdledithexcontrol";
LRESULT CALLBACK EditsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND Edits::hIntEdit;
HWND Edits::hUIntEdit;
HWND Edits::hFloatEdit;

Edits::Edits(){
    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = GetModuleHandle(NULL); // Instance of the application
    WindowClass.lpfnWndProc = EditsProc; // Pointer to callback procedure

    // #2 Class styles
    WindowClass.style = CS_DBLCLKS; // Class styles

    // #3 Background
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //(HBRUSH) (COLOR_WINDOW); // Background brush

    // #4 Cursor
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW); // Class cursor

    // #5 Icon
    WindowClass.hIcon = NULL; // Class Icon
    WindowClass.hIconSm = NULL; // Small icon for this class

    // #6 Menu
    WindowClass.lpszMenuName = NULL; // Menu Resource

    // #7 Other
    WindowClass.cbClsExtra = 0; // Extra bytes to allocate following the wndclassex structure
    WindowClass.cbWndExtra = 0; // Extra bytes to allocate following an instance of the structure

    sBuffer = nullptr;
    nKnownArray = nullptr;
}

bool Edits::Run(HWND hParent, UINT nID){
    if(!RegisterClassEx(&WindowClass)) return false;

    RECT rcParent;
    GetClientRect(hParent, &rcParent);

    hMe = CreateWindowEx(WS_EX_TOPMOST, cClassName, "", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                        ME_HEX_WIN_OFFSET_X, ME_TABS_OFFSET_Y_TOP, ME_HEX_WIN_SIZE_X, rcParent.bottom - ME_TABS_OFFSET_Y_BOTTOM - ME_TABS_OFFSET_Y_TOP,
                        hParent, (HMENU) nID, GetModuleHandle(NULL), NULL);
    if(!hMe) return false;
    return true;
}

extern Edits Edit1;

LRESULT CALLBACK EditsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    Edits * Edit;
    Edit = NULL;
    if(GetDlgCtrlID(hwnd) == IDC_MAIN_EDIT) Edit = &Edit1;
    if(Edit == NULL){
        std::cout<<string_format("MAJORMAJORMAJOR ERROR!! Running the EditsProc() for an unexisting Edit :S \n");
        return DefWindowProc (hwnd, message, wParam, lParam);
    }
    //After here, Edit is definitely not NULL
    if(DEBUG_LEVEL > 500) std::cout<<string_format("EditsProc(): %i\n", (int) message);

	PAINTSTRUCT ps;
	HDC hdc;
    SCROLLINFO si;

    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    static bool bDrag;
    static bool bDblClick;
    static bool bClick;
    static int nArea;
    static int nClickArea;

    /* handle the messages */
    switch(message){
        case WM_CREATE:
        {
                if(DEBUG_LEVEL > 80) std::cout<<"Should be creating the scrollbar now.\n";

                bDrag = false;
                bDblClick = false;
                bClick = false;
                nArea = 0;
                nClickArea = 0;

                //Initialize points
                Edit->ptClick.x = -1;
                Edit->ptClick.y = -1;
                Edit->ptRelease.x = -1;
                Edit->ptRelease.y = -1;
                Edit->ptHover.x = -1;
                Edit->ptHover.y = -1;

                //The extreme values are then defined as everything that is initially visible
                Edit->UpdateClientRect();
                Edit->yMaxScroll = Edit->rcClient.bottom;
                Edit->yCurrentScroll = 0;

                Edit->hScrollVert = CreateWindowEx(WS_EX_TOPMOST, "SCROLLBAR", "",
                                                    WS_CHILD | WS_VISIBLE | SBS_VERT | SBS_RIGHTALIGN		,
                                                    ME_HEX_WIN_OFFSET_X + ME_HEX_WIN_SIZE_X - ME_SCROLLBAR_X, 0, GetSystemMetrics(SM_CXHTHUMB), Edit->rcClient.bottom,
                                                    hwnd, (HMENU) IDC_SBS_SBV, GetModuleHandle(NULL), NULL);
                ShowWindow(Edit->hScrollVert, false);
                si.cbSize = sizeof(SCROLLINFO);
                si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
                si.nMax = Edit->yMaxScroll;
                si.nMin = 0;
                si.nPage = Edit->rcClient.bottom;
                si.nPos = 0;
                SetScrollInfo(Edit->hScrollVert, SB_CTL, &si, true);
                if(DEBUG_LEVEL > 80) std::cout<<string_format("GetScrollInfo(): max: %i, min: %i, page: %i, current: %i \n", si.nMax, si.nMin, si.nPage, si.nPos);
        }
        break;
        case WM_COMMAND:
        {
            int nNotification = HIWORD(wParam);
            int nID = LOWORD(wParam);
            HWND hControl = (HWND) lParam;
            switch(nID){
            }
        }
        break;
        case WM_ERASEBKGND:
        {
            return 1;
        }
        case WM_MOUSEMOVE:
        {
            if(Edit->sBuffer == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);
            if(Edit->sBuffer->empty()) return DefWindowProc(hwnd, message, wParam, lParam);

            //Determine drag

            if(!bDrag && (bClick || bDblClick)) bDrag = true;

            //Determine area
            if(!bDrag){
                if(xPos > 65 && xPos < 450) nArea = 1;
                else if(xPos > 454 && xPos < 587) nArea = 2;
                else nArea = 0;
            }

            if(nArea == 0){
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
            }
            else if(nArea == 1){
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_IBEAM));
            }
            else if(nArea == 2){
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_IBEAM));
            }

            bool bThreshold = false;
            if(nArea == 1){
                int nCurrentY = (yPos + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW;
                nCurrentY = std::max((0 + Edit->yCurrentScroll - 1 + 10) / ME_EDIT_NEXT_ROW, nCurrentY);
                nCurrentY = std::min( (int) (Edit->rcClient.bottom + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW - 1, nCurrentY);
                if(Edit->ptHover.y != nCurrentY){
                    Edit->ptHover.y = nCurrentY;
                    if(DEBUG_LEVEL > 100) std::cout<<string_format("Update current row hover: %i\n", Edit->ptHover.y);
                    bThreshold = true;
                }
                int nCurrentX = (xPos + 2 - ME_EDIT_ROWNUM_OFFSET) / ME_EDIT_CHAR_SIZE_X;
                nCurrentX = std::max(nCurrentX, 0);
                nCurrentX = std::min(nCurrentX, 47);
                if(Edit->ptHover.x != nCurrentX){
                    Edit->ptHover.x = nCurrentX;
                    if(DEBUG_LEVEL > 49) std::cout<<string_format("Update current char hover: %i\n", Edit->ptHover.x);
                    bThreshold = true;
                }
            }
            else if(nArea == 2){
                int nCurrentY = (yPos + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW;
                nCurrentY = std::max((0 + Edit->yCurrentScroll - 1 + 10) / ME_EDIT_NEXT_ROW, nCurrentY);
                nCurrentY = std::min((int)(Edit->rcClient.bottom + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW - 1, nCurrentY);
                if(Edit->ptHover.y != nCurrentY){
                    Edit->ptHover.y = nCurrentY;
                    if(DEBUG_LEVEL > 100) std::cout<<string_format("Update current row hover: %i\n", Edit->ptHover.y);
                    bThreshold = true;
                }
                int nCurrentX = (xPos + 2 - ME_EDIT_CHARSET_OFFSET) / ME_EDIT_CHAR_SIZE_X * 3;
                nCurrentX = std::max(nCurrentX, 0);
                nCurrentX = std::min(nCurrentX, 47);
                if(Edit->ptHover.x != nCurrentX){
                    Edit->ptHover.x = nCurrentX;
                    if(DEBUG_LEVEL > 49) std::cout<<string_format("Update current char hover: %i\n", Edit->ptHover.x);
                    bThreshold = true;
                }
            }

            char cGet [255];
            SendMessage(hStatusBar, SB_GETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cGet);
            if(!BVstrcmp(cGet, "") && (nArea == 0 || TabCtrl_GetCurSel(hTabs) > 1)){
                sprintf(cGet, "");
                SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cGet);
            }

            if(bThreshold){
                //Update position, clear if area equals 0
                if(nArea > 0 && TabCtrl_GetCurSel(hTabs) == 0) Edit->UpdateStatusPositionModel();
                else if(nArea > 0 && TabCtrl_GetCurSel(hTabs) == 1) Edit->UpdateStatusPositionMdx();

                if(bDrag){
                    Edit->DetermineSelection();
                    Edit->UpdateStatusBar();
                    InvalidateRect(hwnd, &Edit->rcClient, false);
                    Edit->ptPrevious = Edit->ptHover;
                    Edit->PrintValues();
                }
            }
        }
        break;
        case WM_LBUTTONDBLCLK:
        {
            if(Edit->sBuffer == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);
            if(Edit->sBuffer->empty()) return DefWindowProc(hwnd, message, wParam, lParam);

            if(nArea == 1){ //We are in the byte view region
                nClickArea = nArea;
                bDblClick = true;
                Edit->DetermineSelection();
                Edit->bSelection = true;
                Edit->UpdateStatusBar();
                InvalidateRect(hwnd, &Edit->rcClient, false);
                Edit->ptPrevious = Edit->ptHover;
                Edit->PrintValues();
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            if(Edit->sBuffer == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);
            if(Edit->sBuffer->empty()) return DefWindowProc(hwnd, message, wParam, lParam);

            if(nArea == 1 || nArea == 2){ //We are in the byte or chAR view region
                nClickArea = nArea;
                SetCapture(hwnd); //Capture the mouse even if it leaves the window
                if(Edit->bSelection || bDrag){
                    Edit->bSelection = false;
                    bDrag = false;
                    InvalidateRect(hwnd, &Edit->rcClient, false);
                    Edit->UpdateStatusBar(false);
                }
                bClick = true;
                Edit->ptPrevious = Edit->ptHover;
                if(!(wParam & MK_SHIFT) || Edit->ptClick.x == -1 || Edit->ptClick.y == -1) Edit->ptClick = Edit->ptHover;
                Edit->PrintValues(bDrag || Edit->bSelection);
                if(DEBUG_LEVEL > 20) std::cout<<string_format("WM_LBUTTONDOWN: bSelection: %i bDrag: %i\n", Edit->bSelection, bDrag);
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if(Edit->sBuffer == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);
            if(Edit->sBuffer->empty()) return DefWindowProc(hwnd, message, wParam, lParam);
            ReleaseCapture(); //Release mouse
            if(bDrag){
                Edit->ptRelease = Edit->ptHover;
                if((Edit->ptRelease.x != Edit->ptClick.x || Edit->ptRelease.y != Edit->ptClick.y) && Edit->ptClick.x != -1 && Edit->ptClick.y != -1) Edit->bSelection = true;
                else if(bDblClick) Edit->bSelection = true;
                else Edit->bSelection = false;
            }
            bDrag = false;
            bDblClick = false;
            bClick = false;
            InvalidateRect(hwnd, &Edit->rcClient, false);
            if(DEBUG_LEVEL > 20) std::cout<<string_format("WM_LBUTTONUP: bSelection: %i bDrag: %i\n", Edit->bSelection, bDrag);
            Edit->PrintValues(bDrag || Edit->bSelection);
        }
        break;
        case WM_PAINT:
        {
            if(DEBUG_LEVEL > 50) std::cout<<string_format("Edits: WM_PAINT. Start.\n");

            HDC hdcReal = BeginPaint(hwnd, &ps);

            hdc = CreateCompatibleDC(hdcReal);

            HBITMAP memBM = CreateCompatibleBitmap(hdcReal, ME_HEX_WIN_SIZE_X, Edit->rcClient.bottom);
            HBITMAP hBMold = (HBITMAP) SelectObject(hdc, memBM);

            HFONT hFont1 = CreateFont(
                14, //Size
                0,  //??
                0,  //??
                0,  //??
                FW_REGULAR, // font weight
                FALSE,	    // italic attribute flag
                FALSE,	    // underline attribute flag
                FALSE,	    // strikeout attribute flag
                DEFAULT_CHARSET,	    // character set identifier
                OUT_DEFAULT_PRECIS	,	// output precision
                CLIP_DEFAULT_PRECIS	,	// clipping precision
                DEFAULT_QUALITY	,	    // output quality
                DEFAULT_PITCH | FF_DONTCARE	,	// pitch and family
                "Consolas" 	// pointer to typeface name string
            );

            HBRUSH hFill = CreateSolidBrush(RGB(255, 255, 255));
            Edit->UpdateClientRect();
            RECT rcFile = Edit->rcClient;
            rcFile.left = 0;
            rcFile.top = 0;
            //rcFile.bottom = Edit->yMaxScroll;
            FillRect(hdc, &rcFile, hFill);
            DeleteObject(hFill);

            /**/
        if(!(Edit->sBuffer == nullptr)){
        if(!Edit->sBuffer->empty()){

            HPEN hPenLine = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
            SelectObject(hdc, hPenLine);
            MoveToEx(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_OFFSET, 0, NULL);
            LineTo(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_OFFSET, Edit->rcClient.bottom);
            MoveToEx(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_2_OFFSET, 0, NULL);
            LineTo(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_2_OFFSET, Edit->rcClient.bottom);
            DeleteObject(hPenLine);

            SetTextColor(hdc, RGB(50, 50, 50));
            //SetBkColor(hdc, GetSysColor(COLOR_MENU));
            SelectObject(hdc, hFont1);

            int nRowMin;
            int nRowMax;
            int nCharMin;
            int nCharMax;
            bool bHilite = false;
            POINT ptEnd;
            if(bDrag) ptEnd = Edit->ptHover;
            else ptEnd = Edit->ptRelease;
            if(Edit->bSelection || bDrag){
                    bHilite = true;
                    //std::cout<<string_format("bSelection: %i bDrag: %i\n", Edit->bSelection, bDrag);
            }
            if(ptEnd.y < Edit->ptClick.y || (ptEnd.y == Edit->ptClick.y && ptEnd.x < Edit->ptClick.x)){
                nRowMin = ptEnd.y;
                nCharMin = ptEnd.x;
                nRowMax = Edit->ptClick.y;
                nCharMax = Edit->ptClick.x;
            }
            else{
                nRowMin = Edit->ptClick.y;
                nCharMin = Edit->ptClick.x;
                nRowMax = ptEnd.y;
                nCharMax = ptEnd.x;
            }

            int n = 0;
            n = Edit->yCurrentScroll / ME_EDIT_NEXT_ROW;
            int nMaxRowsInScreen = (Edit->rcClient.bottom) / ME_EDIT_NEXT_ROW + 2;
            int nMax = n + nMaxRowsInScreen; //Edit->Model->GetHexRowCount();
            nMax = std::min(nMax, Edit->nBufferSize/16 + 1);
            int nStrlen;
            char cIntPrint [255];
            int i = 0;
            int nDataKnown;
            if(DEBUG_LEVEL > 50) std::cout<<string_format("Edits: WM_PAINT. Beginning data while. Starting position: %i. Max: %i.\n", n, nMax);
            char cHexText [50];
            while(n < nMax){
                CharsToHex(cHexText, *Edit->sBuffer, n * 16, 16);
                nStrlen = std::min((Edit->nBufferSize - (n * 16))*3, (int) strlen(cHexText));
                i = 0;
                while(i < nStrlen){
                    SetTextColor(hdc, RGB(50, 50, 50));
                    SetBkColor(hdc, RGB(255, 255, 255));
                    sprintf(cIntPrint, "%i", n*16);
                    AddSignificantZeroes(cIntPrint, 8);
                    ExtTextOut(hdc, ME_EDIT_PADDING_LEFT, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, cIntPrint, strlen(cIntPrint), NULL);
                    if(Edit->nKnownArray->size() > n*16 + i / 3) nDataKnown = Edit->nKnownArray->at(n*16 + i / 3);
                    else nDataKnown = 0;
                    if(bHilite && (
                       ((Edit->nSelectStart / 16) < n && (Edit->nSelectEnd / 16) > n)
                       || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) == n && (Edit->nSelectStart % 16 * 3) <= i && (Edit->nSelectEnd % 16 * 3) >= i - 1)
                       || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) != n && (Edit->nSelectStart % 16 * 3) <= i)
                       || ((Edit->nSelectEnd / 16) == n && (Edit->nSelectStart / 16) != n && (Edit->nSelectEnd % 16 * 3) >= i - 1))){
                        SetTextColor(hdc, RGB(255, 255, 255));
                        if(nClickArea == 1) SetBkColor(hdc, RGB(135, 135, 255));
                        else SetBkColor(hdc, RGB(185, 185, 255));
                    }
                    else{
                        SetTextColor(hdc, DataColor(nDataKnown, false));
                        SetBkColor(hdc, RGB(255, 255, 255));
                    }
                    ExtTextOut(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_ROWNUM_OFFSET + i * ME_EDIT_CHAR_SIZE_X, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, cHexText + i, 1, NULL);

                    //Do a completely separate draw for the charset.
                    if(i <= nStrlen / 3){
                        if(Edit->nKnownArray->size() > n*16 + i) nDataKnown = Edit->nKnownArray->at(n*16 + i);
                        else nDataKnown = 0;
                        if(bHilite && (
                        ((Edit->nSelectStart / 16) < n && (Edit->nSelectEnd / 16) > n)
                        || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) == n && (Edit->nSelectStart % 16) <= i && (Edit->nSelectEnd % 16) >= i)
                        || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) != n && (Edit->nSelectStart % 16) <= i)
                        || ((Edit->nSelectEnd / 16) == n && (Edit->nSelectStart / 16) != n && (Edit->nSelectEnd % 16) >= i))){
                            SetTextColor(hdc, RGB(255, 255, 255));
                            if(nClickArea == 2) SetBkColor(hdc, RGB(135, 135, 255));
                            else SetBkColor(hdc, RGB(185, 185, 255));
                        }
                        else{
                            if(nDataKnown == 3) SetTextColor(hdc, DataColor(nDataKnown, false));
                            else SetTextColor(hdc, RGB(220, 220, 220));
                            SetBkColor(hdc, RGB(255, 255, 255));
                        }
                        char cToWrite;
                        if(Edit->sBuffer->size() > n*16 + i){
                            cToWrite = Edit->sBuffer->at(n * 16 + i);
                            PrepareCharForDisplay(&cToWrite);
                            ExtTextOut(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_CHARSET_OFFSET + i * ME_EDIT_CHAR_SIZE_X, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, &cToWrite, 1, NULL);
                        }
                    }
                    i++;
                }
                n++;
            }
            if(DEBUG_LEVEL > 50) std::cout<<string_format("Edits: WM_PAINT. Ending data while. Ending position: %i\n", n);
        }
        }
            /**/

            BitBlt(hdcReal, 0, 0, ME_HEX_WIN_SIZE_X, Edit->rcClient.bottom, hdc, 0, 0, SRCCOPY);

            SelectObject(hdc, hBMold);
            DeleteObject(hFont1);
            DeleteObject(memBM);
            DeleteDC(hdc);

            EndPaint(hwnd, &ps);
            if(DEBUG_LEVEL > 50) std::cout<<string_format("Edits: WM_PAINT. End.\n");
        }
        break;
        case WM_VSCROLL:
        {
            int yNewPos;    /* new position */

            si.cbSize = sizeof(si);
            si.fMask  = SIF_DISABLENOSCROLL | SIF_POS | SIF_TRACKPOS | SIF_PAGE;
            GetScrollInfo(Edit->hScrollVert, SB_CTL, &si);
            if(DEBUG_LEVEL > 80) std::cout<<string_format("GetScrollInfo(): max: n/a, min: n/a, page: %i, current: %i \n", si.nPage, si.nPos);

            switch(LOWORD(wParam)){
                case SB_TOP:
                    yNewPos = 0;
                break;
                case SB_BOTTOM:
                    yNewPos = Edit->yMaxScroll - (signed int) si.nPage; // - nPage ?
                break;
                /* User clicked the shaft above the scroll box. */
                case SB_PAGEUP:
                    yNewPos = Edit->yCurrentScroll - (signed int) si.nPage;
                    //yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
                    //yNewPos = std::max(0, yNewPos);
                break;
                /* User clicked the shaft below the scroll box. */
                case SB_PAGEDOWN:
                    yNewPos = Edit->yCurrentScroll + (signed int) si.nPage;
                    //yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
                    //yNewPos = std::max(0, yNewPos);
                break;
                /* User clicked the top arrow. */
                case SB_LINEUP:
                    yNewPos = Edit->yCurrentScroll - ME_EDIT_NEXT_ROW;
                    //yNewPos = std::max(0, yNewPos);
                break;
                /* User clicked the bottom arrow. */
                case SB_LINEDOWN:
                    yNewPos = Edit->yCurrentScroll + ME_EDIT_NEXT_ROW;
                    ////yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
                break;
                /* User dragged the scroll box. */
                case SB_THUMBPOSITION:
                    yNewPos = (signed int) si.nPos;
                    //std::cout<<string_format("WM_HSCROLL: Thumb: %i \n", yNewPos);
                    //yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
                    //std::cout<<string_format("WM_HSCROLL: Thumb corrected: %i \n", yNewPos);
                break;
                /* User is dragging the scroll box. */
                case SB_THUMBTRACK:
                    yNewPos = (signed int) si.nTrackPos;
                    //std::cout<<string_format("WM_HSCROLL: Thumbtrack: %i \n", yNewPos);
                    //std::cout<<string_format("WM_HSCROLL: Thumbtrack corrected: %i (compared to %i - %i)\n", yNewPos, Edit->yMaxScroll, (signed int) si.nPage);
                break;
                default:
                    yNewPos = Edit->yCurrentScroll;
            }
            //yNewPos = min(Edit->yMaxScroll, yNewPos);
            //yNewPos = max(0, yNewPos);
            if(DEBUG_LEVEL > 80) std::cout<<string_format("WM_VSCROLL: max: %i, page: %i, current: %i, new: %i \n", Edit->yMaxScroll, si.nPage, Edit->yCurrentScroll, yNewPos);

            /* If the current position does not change, do not scroll.*/
            if (yNewPos == Edit->yCurrentScroll) break;

            //CUSTOM ADDITION, make it so that we can only scroll for discrete amounts
            /* Reset the current scroll position. */
            yNewPos = (yNewPos / ME_EDIT_NEXT_ROW) * ME_EDIT_NEXT_ROW;
            yNewPos = std::max(0, yNewPos);
            yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
            Edit->yCurrentScroll = yNewPos;


            /* Reset the current scroll position. */
            //Edit->yCurrentScroll = yNewPos;

           /*
            * DONT SCROLL THE WINDOW, ITS THE SOURCE OF ALL EVIL
            * JUST REPAINT NORMALLY
            */
            InvalidateRect(hwnd, &Edit->rcClient, false);
            UpdateWindow(hwnd);

            /* Reset the scroll bar. */
            si.cbSize = sizeof(si);
            si.fMask  = SIF_DISABLENOSCROLL | SIF_POS | SIF_RANGE;
            si.nPos   = Edit->yCurrentScroll;
            si.nMax   = Edit->yMaxScroll;
            si.nMin   = 0;
            SetScrollInfo(Edit->hScrollVert, SB_CTL, &si, true);

            //std::cout<<string_format("Zoom DEBUG: Zooming. \nnMinP=%i, nMaxP=%i, xCurrentScroll=%i, \nxCurrentScroll+rcClient.right/2=%i\n0=%i, Edit->yMaxScroll=%i, Edit->yCurrentScroll=%i, \nyCurrentScroll+(rcClient.bottom-nNonScreenY)/2=%i\n", nMinP, nMaxP, xCurrentScroll, xCurrentScroll+rcClient.right/2, 0, Edit->yMaxScroll, Edit->yCurrentScroll, Edit->yCurrentScroll+(rcClient.bottom-nNonScreenY)/2);

            return 0;
        }
        break;
        /* for messages that we don't deal with */
        case WM_DESTROY:
            //Close
        default:
        {
            return DefWindowProc (hwnd, message, wParam, lParam);
        }
    }

    return 0;
}

void Edits::PrintValues(bool bCheck){
    if(nSelectStart < 0 ||
       nSelectEnd < 0 ||
       !bCheck){
        SetWindowText(hIntEdit, "");
        SetWindowText(hUIntEdit, "");
        SetWindowText(hFloatEdit, "");
        return;
    }

    int nSize = nSelectEnd - nSelectStart + 1;
    if(DEBUG_LEVEL > 70) std::cout<<"Selected bytes: "<<nSize<<".\n";
    std::string sString;
    //char * cString = new char [nSize+1];
    char cInt [255];
    char cUInt [255];
    char cFloat [255];
    int n = 0;
    while(nSelectStart + n <= nSelectEnd){
        if(DEBUG_LEVEL > 70) std::cout<<"Extract byte #"<<nSelectStart + n<<".\n";
        sString += sBuffer->at(nSelectStart+n);
        //if(nSelectStart + n == nSelectEnd) cString[n+1] = '\0';
        n++;
    }
    char cBlock2 [2];
    char cBlock4 [4];
    char cBlock8 [8];

    n = 0;
    if(nSize == 1){
        sprintf(cInt, "%hhi", (signed int) sString.at(0));
        sprintf(cUInt, "%hhu", (unsigned int) sString.at(0));
        sprintf(cFloat, "");
    }
    else if(nSize == 2){
        while(n < 2){
            ByteBlock2.bytes[n] = sString[n];
            n++;
        }
        sprintf(cInt, "%hi", ByteBlock2.i);
        sprintf(cUInt, "%hu", ByteBlock2.ui);
        sprintf(cFloat, "");

    }
    else if(nSize == 4){
        while(n < 4){
            ByteBlock4.bytes[n] = sString[n];
            n++;
        }
        sprintf(cInt, "%i", ByteBlock4.i);
        sprintf(cUInt, "%u", ByteBlock4.ui);
        sprintf(cFloat, "%.16f", ByteBlock4.f);
    }
    else if(nSize == 8){
        while(n < 8){
            ByteBlock8.bytes[n] = sString[n];
            n++;
        }
        sprintf(cInt, "%li", ByteBlock8.i);
        sprintf(cUInt, "%lu", ByteBlock8.ui);
        sprintf(cFloat, "%.16g", ByteBlock8.d);
    }
    else{
        sprintf(cInt, "");
        sprintf(cUInt, "");
        sprintf(cFloat, "");
    }
    TruncateDec(cFloat);
    SetWindowText(hIntEdit, cInt);
    SetWindowText(hUIntEdit, cUInt);
    SetWindowText(hFloatEdit, cFloat);
}

COLORREF DataColor(int nDataKnown, bool bHilite){
    if(bHilite) return RGB(255, 255, 255);
    switch(nDataKnown){
        case 10: //known unknowns, mark red
            return RGB(255, 0, 0);
        case 1: //anything that counts something, unsigned by definition. (uint32 or uint16)
            return RGB(235, 220, 0);
        case 2: //float
            return RGB(76, 206, 20);
        case 3: //string
            return RGB(110, 110, 110);
        case 4: //int ((u)int32)
            return RGB(0, 200, 200);
        case 5: //short ((u)int16)
            return RGB(200, 0, 200);
        case 6: //offset/pointer (uint32)
            return RGB(100, 100, 200);
        case 7: //byte
            return RGB(165, 103, 16);
        case 8: //Possibly constant/meaningless/padding
            return RGB(232, 232, 232);
        case 9: //function pointer - it can make the beginning certain structures stand out more
            return RGB(255, 160, 80);
    }
    return RGB(0, 0, 0);
}

void Edits::UpdateStatusPositionMdx(){
    if(DEBUG_LEVEL > 80) std::cout<<"Begin updating status position for the MDX.\n";
    char cPrint [255];
    FileHeader * FH = (Model.GetFileData());
    int nPos = ptHover.y * 16 + (ptHover.x) / 3;
    int nMin;

    //Find position
    Node * NODE = nullptr;
    //NodeRecursionMdx(&Model.GetNodeByNameIndex(0), NODE, nPos;
    for(int b = 0; b < FH->MH.ArrayOfNodes.size(); b++){
        Node & node = FH->MH.ArrayOfNodes[b];
        if(node.Head.nType & NODE_HAS_MESH &&
           node.Mesh.nMdxDataSize > 0 &&
           (nPos >= node.Mesh.nOffsetIntoMdx &&  nPos < (node.Mesh.nOffsetIntoMdx + (node.Mesh.nNumberOfVerts+1) * node.Mesh.nMdxDataSize))){
               NODE = &node;
           }
    }
    if(nPos >= Model.GetBufferLength()){
        sprintf(cPrint, "");
    }
    else if(NODE != nullptr){
        nMin = NODE->Mesh.nOffsetIntoMdx;
        //std::cout<<string_format("Calculatin (%i - %i) / %i = %i \n", nPos, nMin, NODE->Mesh.nMdxDataSize, (nPos - nMin) / (NODE->Mesh.nMdxDataSize));
        if((nPos - nMin) / (NODE->Mesh.nMdxDataSize) == NODE->Mesh.nNumberOfVerts){
            sprintf(cPrint, "MDX > %s > Extra Data", FH->MH.Names[NODE->Head.nNameIndex].cName.c_str());
        }
        else sprintf(cPrint, "MDX > %s > Vertex %i", FH->MH.Names[NODE->Head.nNameIndex].cName.c_str(), (nPos - nMin) / (NODE->Mesh.nMdxDataSize));
    }
    else{
        sprintf(cPrint, "MDX > Unknown");
    }

    //Change text
    char cGet [255];
    SendMessage(hStatusBar, SB_GETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cGet);
    if(!BVstrcmp(cGet, cPrint)){
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cPrint);
    }
}

void Edits::UpdateStatusPositionModel(){
    if(DEBUG_LEVEL > 80) std::cout<<"Begin updating status position for the MDL.\n";
    char cPrint [255];
    FileHeader * FH = (Model.GetFileData());
    if(FH == NULL){
        if(DEBUG_LEVEL > 80) std::cout<<"No data. Stop updating status position for MDL.\n";
        return;
    }
    int nPos = ptHover.y * 16 + (ptHover.x) / 3;
    int nMin;

    if(nPos >= Model.GetBufferLength()){
        sprintf(cPrint, "");
    }
    else if(nPos < 208){
        //We are in Header
        sprintf(cPrint, "Header");
    }
    else if(nPos < FH->MH.AnimationArray.nOffset + 12){
        //We are in Names
        if(nPos < FH->MH.Names[0].nOffset + 12){
            nMin = 208;
            sprintf(cPrint, "Name Array > Pointers > Pointer %i", (nPos - nMin)/4);
        }
        else{
            int n = 1;
            bool bFound = false;
            while(!bFound){
                if(nPos < FH->MH.Names[n].nOffset + 12) bFound = true;
                else if(n + 1 == FH->MH.NameArray.nCount2){
                    bFound = true;
                    n++;
                }
                else if(n == FH->MH.NameArray.nCount2){
                    bFound = true;
                }
                else n++;
            }
            int nName = n - 1;
            sprintf(cPrint, "Name Array > Strings > \"%s\"", FH->MH.Names[nName].cName.c_str());
        }
    }
    else if(nPos < FH->MH.GH.nOffsetToRootNode + 12){
        //We are in Animations
        if(nPos < FH->MH.Animations[0].nOffset + 12){
            nMin = FH->MH.AnimationArray.nOffset + 12;
            sprintf(cPrint, "Animations > Pointers > Pointer %i", (nPos - nMin)/4);
        }
        else{
            int n = 1;
            bool bFound = false;
            while(!bFound){
                if(nPos < FH->MH.Animations[n].nOffset + 12) bFound = true;
                else if(n + 1 == FH->MH.AnimationArray.nCount2){
                    bFound = true;
                    n++;
                }
                else if(n == FH->MH.AnimationArray.nCount2){
                    bFound = true;
                }
                else n++;
            }
            int nAnimation = n - 1;
            nMin = FH->MH.Animations[nAnimation].nOffset + 12;
            if(nPos < nMin + FH->MH.Animations[nAnimation].nLength){
                sprintf(cPrint, "Animations > %s > Header", FH->MH.Animations[nAnimation].cName.c_str());
            }
            else{
                Node * NODE = nullptr;
                for(int b = 0; b < FH->MH.Animations[nAnimation].ArrayOfNodes.size() && NODE==nullptr; b++){
                    Node & node = FH->MH.Animations[nAnimation].ArrayOfNodes[b];
                    if(nPos >= node.nOffset+12 &&
                       nPos < node.Head.ChildrenArray.nOffset+12 + 4*node.Head.ChildrenArray.GetCount()){
                        NODE = &node;
                    }
                    else if(node.Head.ControllerArray.GetCount() > 0 &&
                            nPos >= node.Head.ControllerArray.nOffset+12 &&
                            nPos < node.Head.ControllerArray.nOffset+12 + 16*node.Head.ControllerArray.GetCount()){
                        NODE = &node;
                    }
                    else if(node.Head.ControllerDataArray.GetCount() > 0 &&
                            nPos >= node.Head.ControllerDataArray.nOffset+12 &&
                            nPos < node.Head.ControllerDataArray.nOffset+12 + 4*node.Head.ControllerDataArray.GetCount()){
                        NODE = &node;
                    }
                }
                if(NODE == nullptr){
                    sprintf(cPrint, "Animations > %s > Unknown", FH->MH.Animations[nAnimation].cName.c_str());
                }
                else{
                    int nNode = NODE->Head.nNameIndex;
                    nMin += FH->MH.Animations[nAnimation].nLength;
                    if(nPos < NODE->Head.ChildrenArray.nOffset + 12 + 4 * NODE->Head.ChildrenArray.nCount){
                        if(nPos < NODE->nOffset + 12 + NODE_SIZE_HEADER){
                            sprintf(cPrint, "Animations > %s > %s > Header", FH->MH.Animations[nAnimation].cName.c_str(), FH->MH.Names[nNode].cName.c_str());
                        }
                        else{
                            nMin = NODE->nOffset + 12 + NODE_SIZE_HEADER;
                            sprintf(cPrint, "Animations > %s > %s > Child Pointers > Pointer %i", FH->MH.Animations[nAnimation].cName.c_str(), FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/4);
                        }
                    }
                    else if(nPos >= NODE->Head.ControllerDataArray.nOffset + 12 && NODE->Head.ControllerDataArray.nOffset > 0){
                        nMin = NODE->Head.ControllerDataArray.nOffset + 12;
                        sprintf(cPrint, "Animations > %s > %s > Controller Data > Float %i", FH->MH.Animations[nAnimation].cName.c_str(), FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/4);
                    }
                    else if(nPos >= NODE->Head.ControllerArray.nOffset + 12 && NODE->Head.ControllerArray.nOffset > 0){
                        nMin = NODE->Head.ControllerArray.nOffset + 12;
                        sprintf(cPrint, "Animations > %s > %s > Controllers > Controller %i", FH->MH.Animations[nAnimation].cName.c_str(), FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/16);
                    }
                    else{
                        sprintf(cPrint, "Animations > %s > %s > Unknown", FH->MH.Animations[nAnimation].cName.c_str(), FH->MH.Names[nNode].cName.c_str());
                    }
                }
            }
        }
    }
    else{
        //We are in Geometry
        Node * NODE = nullptr;
        for(int b = 0; b < FH->MH.ArrayOfNodes.size() && NODE==nullptr; b++){
            Node & node = FH->MH.ArrayOfNodes[b];
            if(nPos >= node.nOffset+12 &&
               nPos < node.Head.ChildrenArray.nOffset+12 + 4*node.Head.ChildrenArray.GetCount()){
                NODE = &node;
            }
            else if(node.Head.ControllerArray.GetCount() > 0 &&
                    nPos >= node.Head.ControllerArray.nOffset+12 &&
                    nPos < node.Head.ControllerArray.nOffset+12 + 16*node.Head.ControllerArray.GetCount()){
                NODE = &node;
            }
            else if(node.Head.ControllerDataArray.GetCount() > 0 &&
                    nPos >= node.Head.ControllerDataArray.nOffset+12 &&
                    nPos < node.Head.ControllerDataArray.nOffset+12 + 4*node.Head.ControllerDataArray.GetCount()){
                NODE = &node;
            }
        }

        if(NODE == NULL){
            sprintf(cPrint, "Geometry > Unknown");
        }
        else{
            int nNode = NODE->Head.nNameIndex;
            int nType = NODE->Head.nType;
            if(nPos < NODE->Head.ChildrenArray.nOffset + 12 + 4 * NODE->Head.ChildrenArray.nCount){
                int nHeaderSize = 12;
                bool bFound = false;
                if(nType & NODE_HAS_HEADER && !bFound){
                    nHeaderSize += NODE_SIZE_HEADER;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Basic", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_LIGHT && !bFound){
                    nHeaderSize += NODE_SIZE_LIGHT;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Light", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_EMITTER && !bFound){
                    nHeaderSize += NODE_SIZE_EMITTER;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Emitter", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_MESH && !bFound){
                    nHeaderSize += NODE_SIZE_MESH;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Mesh", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_SKIN && !bFound){
                    nHeaderSize += NODE_SIZE_SKIN;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Skin", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_DANGLY && !bFound){
                    nHeaderSize += NODE_SIZE_DANGLY;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Danglymesh", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_AABB && !bFound){
                    nHeaderSize += NODE_SIZE_AABB;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Walkmesh", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_SABER && !bFound){
                    nHeaderSize += NODE_SIZE_SABER;
                    if(nPos < NODE->nOffset + nHeaderSize){
                        sprintf(cPrint, "Geometry > %s > Header > Saber", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_HEADER && !bFound){
                    if(nPos >= NODE->Head.ChildrenArray.nOffset + 12){
                        nMin = NODE->Head.ChildrenArray.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Child Array > Pointer %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/4);
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_LIGHT && !bFound){
                }
                if(nType & NODE_HAS_EMITTER && !bFound){
                }
                if(nType & NODE_HAS_MESH && !bFound){
                    if(nPos >= NODE->Mesh.nVertIndicesLocation + 12 && NODE->Mesh.IndexLocationArray.nCount > 0){
                        nMin = NODE->Mesh.nVertIndicesLocation + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Vert Indexes > Face %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/6);
                        bFound = true;
                    }
                    else if(nPos >= NODE->Mesh.MeshInvertedCounterArray.nOffset + 12){
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Inverted Counter", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Mesh.IndexLocationArray.nOffset + 12){
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Pointer to Vert Indexes", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Mesh.nOffsetToVertArray + 12){
                        nMin = NODE->Mesh.nOffsetToVertArray + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Vert Coordinates > Vert %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/12);
                        bFound = true;
                    }
                    else if(nPos >= NODE->Mesh.IndexCounterArray.nOffset + 12){
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Pointer to Vert Number", FH->MH.Names[nNode].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Mesh.FaceArray.nOffset + 12){
                        nMin = NODE->Mesh.FaceArray.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Mesh > Faces > Face %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/32);
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_SKIN && !bFound){
                    if(nPos >= NODE->Skin.Array8Array.nOffset + 12){
                        nMin = NODE->Skin.Array8Array.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Skin > Array8 > %i", FH->MH.Names[nNode].cName.c_str(), FH->MH.Names[(nPos - nMin)/4].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Skin.TBoneArray.nOffset + 12){
                        nMin = NODE->Skin.TBoneArray.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Skin > T-Bones > %i", FH->MH.Names[nNode].cName.c_str(), FH->MH.Names[(nPos - nMin)/4].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Skin.QBoneArray.nOffset + 12){
                        nMin = NODE->Skin.QBoneArray.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Skin > Q-Bones > %i", FH->MH.Names[nNode].cName.c_str(), FH->MH.Names[(nPos - nMin)/4].cName.c_str());
                        bFound = true;
                    }
                    else if(nPos >= NODE->Skin.nOffsetToBonemap + 12){
                        nMin = NODE->Skin.nOffsetToBonemap + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Skin > Bonemap > %i", FH->MH.Names[nNode].cName.c_str(), FH->MH.Names[(nPos - nMin)/4].cName.c_str());
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_DANGLY && !bFound){
                    if(nPos >= NODE->Dangly.nOffsetToData2 + 12){
                        nMin = NODE->Dangly.nOffsetToData2 + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Danglymesh > Data2 > Member %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/12);
                        bFound = true;
                    }
                    else if(nPos >= NODE->Dangly.ConstraintArray.nOffset + 12){
                        nMin = NODE->Dangly.ConstraintArray.nOffset + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Danglymesh > Constraints > Float %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/4);
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_AABB && !bFound){
                    if(nPos >= NODE->Walkmesh.nOffsetToAabb + 12){
                        nMin = NODE->Walkmesh.nOffsetToAabb + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Walkmesh > AABB > Member %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/40);
                        bFound = true;
                    }
                }
                if(nType & NODE_HAS_SABER && !bFound){
                    if(nPos >= NODE->Saber.nOffsetToSaberData2 + 12){
                        nMin = NODE->Saber.nOffsetToSaberData2 + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Saber > Data2 > Member %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/8);
                        bFound = true;
                    }
                    else if(nPos >= NODE->Saber.nOffsetToSaberData3 + 12){
                        nMin = NODE->Saber.nOffsetToSaberData3 + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Saber > Data3 > Member %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/12);
                        bFound = true;
                    }
                    else if(nPos >= NODE->Saber.nOffsetToSaberData1 + 12){
                        nMin = NODE->Saber.nOffsetToSaberData1 + 12;
                        sprintf(cPrint, "Geometry > %s > Data > Saber > Data1 > Member %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/12);
                        bFound = true;
                    }
                }
                if(!bFound){
                        sprintf(cPrint, "Geometry > %s > Data > Unknown", FH->MH.Names[nNode].cName.c_str());
                }
            }
            else if(nPos >= NODE->Head.ControllerDataArray.nOffset + 12 && NODE->Head.ControllerDataArray.nOffset > 0){
                nMin = NODE->Head.ControllerDataArray.nOffset + 12;
                sprintf(cPrint, "Geometry > %s > Data > Controller Data > Float %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/4);
            }
            else if(nPos >= NODE->Head.ControllerArray.nOffset + 12 && NODE->Head.ControllerArray.nOffset > 0){
                nMin = NODE->Head.ControllerArray.nOffset + 12;
                sprintf(cPrint, "Geometry > %s > Data > Controllers > Controller %i", FH->MH.Names[nNode].cName.c_str(), (nPos - nMin)/16);
            }
            else{
                sprintf(cPrint, "Geometry > %s > Unknown", FH->MH.Names[nNode].cName.c_str());
            }
        }
    }
    char cGet [255];
    SendMessage(hStatusBar, SB_GETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cGet);
    if(!BVstrcmp(cGet, cPrint)){
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) cPrint);
    }
}
