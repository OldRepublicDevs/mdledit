#include "frame.h"

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
    ShowWindow(hMe, false);
    return true;
}

extern Edits Edit1;

LRESULT CALLBACK EditsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    Edits * Edit = nullptr;
    if(GetDlgCtrlID(hwnd) == IDC_MAIN_EDIT) Edit = &Edit1;
    if(Edit == nullptr){
        std::cout << "MAJORMAJORMAJOR ERROR!! Running the EditsProc() for an unexisting Edit :S \n";
        return DefWindowProc (hwnd, message, wParam, lParam);
    }
    //After here, Edit is definitely not NULL
    if(DEBUG_LEVEL > 500) std::cout << "EditsProc(): " << (int) message << "\n";

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
                if(DEBUG_LEVEL > 80) std::cout << "Should be creating the scrollbar now.\n";

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
                if(DEBUG_LEVEL > 80) std::cout << "GetScrollInfo(): max: " << si.nMax << ", min: " << si.nMin << ", page: " << si.nPage << ", current: " << si.nPos << "\n";
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
                if(xPos < 64) nArea = -1;
                else if(xPos > 65 && xPos < 450) nArea = 1;
                else if(xPos > 454 && xPos < 587) nArea = 2;
                else nArea = 0;
            }

            if(nArea == 1){
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_IBEAM));
            }
            else if(nArea == 2){
                SetCursor(LoadCursor(NULL, IDC_IBEAM));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_IBEAM));
            }
            else{
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
            }

            bool bThreshold = false;
            if(nArea == 1){
                int nCurrentY = (yPos + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW;
                nCurrentY = std::max((0 + Edit->yCurrentScroll - 1 + 10) / ME_EDIT_NEXT_ROW, nCurrentY);
                nCurrentY = std::min( (int) (Edit->rcClient.bottom + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW - 1, nCurrentY);
                if(Edit->ptHover.y != nCurrentY){
                    Edit->ptHover.y = nCurrentY;
                    if(DEBUG_LEVEL > 100) std::cout << "Update current row hover: " << Edit->ptHover.y << "\n";
                    bThreshold = true;
                }
                int nCurrentX = (xPos + 2 - ME_EDIT_ROWNUM_OFFSET) / ME_EDIT_CHAR_SIZE_X;
                nCurrentX = std::max(nCurrentX, 0);
                nCurrentX = std::min(nCurrentX, 47);
                if(Edit->ptHover.x != nCurrentX){
                    Edit->ptHover.x = nCurrentX;
                    if(DEBUG_LEVEL > 49) std::cout << "Update current char hover: " << Edit->ptHover.x << "\n";
                    bThreshold = true;
                }
            }
            else if(nArea == 2){
                int nCurrentY = (yPos + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW;
                nCurrentY = std::max((0 + Edit->yCurrentScroll - 1 + 10) / ME_EDIT_NEXT_ROW, nCurrentY);
                nCurrentY = std::min((int)(Edit->rcClient.bottom + Edit->yCurrentScroll - 1) / ME_EDIT_NEXT_ROW - 1, nCurrentY);
                if(Edit->ptHover.y != nCurrentY){
                    Edit->ptHover.y = nCurrentY;
                    if(DEBUG_LEVEL > 100) std::cout << "Update current row hover: " << Edit->ptHover.y << "\n";
                    bThreshold = true;
                }
                int nCurrentX = (xPos + 2 - ME_EDIT_CHARSET_OFFSET) / ME_EDIT_CHAR_SIZE_X * 3;
                nCurrentX = std::max(nCurrentX, 0);
                nCurrentX = std::min(nCurrentX, 47);
                if(Edit->ptHover.x != nCurrentX){
                    Edit->ptHover.x = nCurrentX;
                    if(DEBUG_LEVEL > 49) std::cout << "Update current char hover: " << Edit->ptHover.x << "\n";
                    bThreshold = true;
                }
            }

            int nLength = HIWORD(SendMessage(hStatusBar, SB_GETTEXTLENGTH, 3, 0));
            if(nLength > 0){
                std::string sGet;
                sGet.resize(nLength);
                SendMessage(hStatusBar, SB_GETTEXT, 3, (LPARAM) sGet.c_str());
                if(sGet.size() > 0 && sGet.at(0) != 0 && nArea == 0){
                    sGet = "";
                    SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) sGet.c_str());
                }
            }

            if(bThreshold){
                if(nArea > 0) Edit->UpdateStatusPosition();

                if(bDrag){
                    Edit->bSelection = true; /// If we are dragging, we are also selecting.
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

                Edit->ptPrevious = Edit->ptHover;
                Edit->bSelection = true; /// true because we select by double clicking
                Edit->DetermineSelection();
                Edit->UpdateStatusBar();
                InvalidateRect(hwnd, &Edit->rcClient, false);
                Edit->PrintValues();

                if(Edit->htHoverItem != NULL) TreeView_Select(hTree, Edit->htHoverItem, TVGN_CARET);
            }
            else if(nArea == -1){
                bHexLocation = !bHexLocation;
                InvalidateRect(hwnd, &Edit->rcClient, false);
                Edit->UpdateStatusBar();
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

                /// User pressed shift. Do not take this as a new click and a cancellation of the previous selection,
                /// but as if by doing this, we are actually proceeding with a dragging operation.
                /// The user must have selected something beforehand.
                if(wParam & MK_SHIFT && Edit->bSelection){
                    bDrag = true; /// Make it as if we were dragging

                    /// I forgot what this does - it probably doesn't matter because if click was undefined, bSelection wouldn't be on anyway,
                    /// we would skip this if anyway.
                    if(Edit->ptClick.x == -1 || Edit->ptClick.y == -1)
                        Edit->ptClick = Edit->ptHover;
                }
                else{
                    if(Edit->bSelection){ /// If there was a previous selection (or we were dragging)
                        Edit->bSelection = false; /// by clicking we clear the selection
                        bDrag = false; /// we stop dragging
                    }

                    bClick = true;

                    Edit->ptPrevious = Edit->ptHover;
                    Edit->ptClick = Edit->ptHover;
                }

                Edit->DetermineSelection();
                InvalidateRect(hwnd, &Edit->rcClient, false);
                Edit->UpdateStatusBar();
                Edit->PrintValues();

                if(DEBUG_LEVEL > 20) std::cout << "WM_LBUTTONDOWN: bSelection: " << Edit->bSelection << " bDrag: " << bDrag << "\n";
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if(Edit->sBuffer == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);
            if(Edit->sBuffer->empty()) return DefWindowProc(hwnd, message, wParam, lParam);

            ReleaseCapture(); //Release mouse

            if(bDrag){ /// If we are dragging
                Edit->ptRelease = Edit->ptHover; /// The current hover point is our release point
                if((Edit->ptRelease.x != Edit->ptClick.x || Edit->ptRelease.y != Edit->ptClick.y) && Edit->ptClick.x != -1 && Edit->ptClick.y != -1)
                    Edit->bSelection = true; /// If the drag selection is valid, selection is true
                else if(bDblClick) Edit->bSelection = true; /// If we just double-clicked, selection is true
                else Edit->bSelection = false; /// If we simply clicked and released, selection is false
            }

            bDrag = false;
            bDblClick = false;
            bClick = false;

            //Edit->DetermineSelection();
            InvalidateRect(hwnd, &Edit->rcClient, false);
            Edit->UpdateStatusBar();
            Edit->PrintValues();

            if(DEBUG_LEVEL > 20) std::cout << "WM_LBUTTONUP: bSelection: " << Edit->bSelection << " bDrag: " << bDrag << "\n";
        }
        break;
        case WM_PAINT:
        {
            if(DEBUG_LEVEL > 50) std::cout << "Edits: WM_PAINT. Start.\n";

            HDC hdcReal = BeginPaint(hwnd, &ps);

            hdc = CreateCompatibleDC(hdcReal);

            HBITMAP memBM = CreateCompatibleBitmap(hdcReal, ME_HEX_WIN_SIZE_X, Edit->rcClient.bottom);
            HBITMAP hBMold = (HBITMAP) SelectObject(hdc, memBM);

            HBRUSH hFill = CreateSolidBrush(RGB(255, 255, 255));
            Edit->UpdateClientRect();
            RECT rcFile = Edit->rcClient;
            rcFile.left = 0;
            rcFile.top = 0;
            //rcFile.bottom = Edit->yMaxScroll;
            FillRect(hdc, &rcFile, hFill);
            DeleteObject(hFill);

            /**/
            if(Edit->sBuffer && !Edit->sBuffer->empty()){

                /// Draw the two separator lines
                HPEN hPenLine = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
                SelectObject(hdc, hPenLine);
                MoveToEx(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_OFFSET, 0, NULL);
                LineTo(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_OFFSET, Edit->rcClient.bottom);
                MoveToEx(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_2_OFFSET, 0, NULL);
                LineTo(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_SEPARATOR_2_OFFSET, Edit->rcClient.bottom);
                DeleteObject(hPenLine);

                SetTextColor(hdc, RGB(50, 50, 50));
                //SetBkColor(hdc, GetSysColor(COLOR_MENU));
                SelectObject(hdc, hMonospace);

                int nRowMin;
                int nRowMax;
                int nCharMin;
                int nCharMax;

                /// Determine whether we are hiliting.
                bool bHilite = false;
                POINT ptEnd;
                if(bDrag) ptEnd = Edit->ptHover;
                else ptEnd = Edit->ptRelease;
                if(Edit->bSelection || bDrag){
                        bHilite = true;
                        //std::cout << string_format("bSelection: %i bDrag: %i\n", Edit->bSelection, bDrag);
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

                int n = Edit->yCurrentScroll / ME_EDIT_NEXT_ROW; /// n is the number of rows that are scrolled away, it will be ++ed later
                int nMaxRowsInScreen = (Edit->rcClient.bottom) / ME_EDIT_NEXT_ROW + 2; /// Max number of rows the screen may display
                int nMax = n + nMaxRowsInScreen; /// Max number of rows possible by the current scroll
                nMax = std::min(nMax, (int) Edit->sBuffer->size()/16 + 1); /// Cap the max number of rows by the actual number of rows of the data

                int nDataKnown;
                if(DEBUG_LEVEL > 50) std::cout << "Edits: WM_PAINT. Beginning data while. Starting position: " << n << ". Max: " << nMax << ".\n";
                std::string sHexText, sHexCompareText;
                sHexText.reserve(50);
                sHexCompareText.reserve(50);
                for(; n < nMax; n++){
                    SetTextColor(hdc, RGB(50, 50, 50));
                    SetBkColor(hdc, RGB(255, 255, 255));
                    std::stringstream ssIntPrint;
                    ssIntPrint << std::uppercase << (bHexLocation ? std::hex : std::dec) << n * 16;
                    //if(bHexLocation) sprintf(cIntPrint, "%X", n*16);
                    //else sprintf(cIntPrint, "%i", n*16);
                    //AddSignificantZeroes(cIntPrint, 8);
                    std::string sCounter = ssIntPrint.str();
                    AddSignificantZeroes(sCounter, 8);
                    ExtTextOut(hdc, ME_EDIT_PADDING_LEFT, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, sCounter.c_str(), sCounter.size(), NULL);

                    CharsToHex(sHexText, *Edit->sBuffer, n * 16, 16);
                    CharsToHex(sHexCompareText, *Edit->sCompareBuffer, n * 16, 16);
                    for(int i = 0; i < sHexText.size(); i++){
                        if(Edit->nKnownArray->size() > n*16 + i / 3) nDataKnown = Edit->nKnownArray->at(n*16 + i / 3) & 0xFFFF;
                        else nDataKnown = 0;
                        COLORREF rgbUnderline = DataColor(nDataKnown, false);// RGB(0,0,0);
                        COLORREF rgbText = DataColor(nDataKnown, false);// RGB(0,0,0);
                        COLORREF rgbBackground = RGB(255,255,255);
                        bool bHighlited = false, bDifferent = false, bOutOfRange = false;
                        if(bHilite && (
                           ((Edit->nSelectStart / 16) < n && (Edit->nSelectEnd / 16) > n)
                           || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) == n && (Edit->nSelectStart % 16 * 3) <= i && (Edit->nSelectEnd % 16 * 3) >= i - 1)
                           || ((Edit->nSelectStart / 16) == n && (Edit->nSelectEnd / 16) != n && (Edit->nSelectStart % 16 * 3) <= i)
                           || ((Edit->nSelectEnd / 16) == n && (Edit->nSelectStart / 16) != n && (Edit->nSelectEnd % 16 * 3) >= i - 1))){
                            bHighlited = true;
                        }

                        if(bShowDiff && Edit->Compare(n*16 + i / 3) > 0){
                            if(i % 3 < 2 || // ignore the space
                               Edit->Compare(n*16 + i / 3) == Edit->Compare(n*16 + i / 3 + 1) && Edit->Compare(n*16 + i / 3) == 2 || // color the space if out of range
                               Edit->Compare(n*16 + i / 3) == Edit->Compare(n*16 + i / 3 + 1) && (Edit->nKnownArray->at(n*16 + i / 3) & 0xFFFF) == (Edit->nKnownArray->at(n*16 + i / 3 + 1) & 0xFFFF)) //color the space if same known value
                            {
                                if(Edit->Compare(n*16 + i / 3) == 1){ // Different
                                    bDifferent = true;
                                }
                                if(Edit->Compare(n*16 + i / 3) == 2){
                                    bOutOfRange = true;
                                }
                            }
                        }

                        if(bHighlited){
                            if(bDifferent){
                                if(nClickArea == 1){
                                    rgbText = RGB(255, 170, 70);
                                    rgbUnderline = RGB(255, 170, 70);
                                }
                                else{
                                    rgbText = RGB(255, 210, 170);
                                    rgbUnderline = RGB(255, 210, 170);
                                }
                            }
                            else{
                                rgbText = RGB(255, 255, 255);
                                rgbUnderline = RGB(255,255,255);
                            }
                            if(nClickArea == 1) rgbBackground = RGB(135, 135, 255);
                            else rgbBackground = RGB(185, 185, 255);
                        }
                        else if(bDifferent){
                            rgbText = RGB(255, 255, 255);
                            rgbUnderline = RGB(255,255,255);
                            rgbBackground = DataColor(nDataKnown, false);
                        }
                        else if(bOutOfRange){
                            rgbBackground = RGB(230, 180, 230); // Out of range
                        }

                        SetTextColor(hdc, rgbText);
                        SetBkColor(hdc, rgbBackground);
                        if(sHexText.at(i) != ' ' || rgbBackground != RGB(255,255,255)){
                            ExtTextOut(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_ROWNUM_OFFSET + i * ME_EDIT_CHAR_SIZE_X, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, (bHighlited && bDifferent ? &sHexCompareText.at(i) : &sHexText.at(i)), 1, NULL);
                        }

                        int nVertical = ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll + 12;
                        int nHorizontal = ME_EDIT_PADDING_LEFT + ME_EDIT_ROWNUM_OFFSET + i * ME_EDIT_CHAR_SIZE_X;
                        int nExtra = -1;
                        if(sHexText.size() >= i+1 && !(Edit->nKnownArray->at(n*16 + i / 3) & 0x00010000)) nExtra = 0;

                        if(bShowGroup && (i % 3 == 0 && (n*16 + i / 3 - 1 < 0 || Edit->nKnownArray->at(n*16 + i / 3 - 1) & 0x00010000))){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, rgbUnderline);
                            SelectObject(hdc, hPenUnderline);
                            MoveToEx(hdc, nHorizontal - 1,  nVertical, NULL);
                            LineTo(hdc, nHorizontal - 1,  nVertical - 3);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }
                        if(bShowDataStruct && (i % 3 == 0 && (n*16 + i / 3 - 1 < 0 || Edit->nKnownArray->at(n*16 + i / 3 - 1) & 0x00020000))){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, RGB(0,0,0));
                            SelectObject(hdc, hPenUnderline);
                            MoveToEx(hdc, nHorizontal - 3,  nVertical - 3, NULL);
                            LineTo(hdc, nHorizontal - 3,  nVertical - 12);
                            LineTo(hdc, nHorizontal + 5,  nVertical - 12);
                            //MoveToEx(hdc, nHorizontal - 3,  nVertical - 5, NULL);
                            //LineTo(hdc, nHorizontal - 3,  nVertical + 2);
                            //LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra + 1,  nVertical + 2);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }

                        if(bShowGroup && (sHexText.at(i) != ' ' || !(Edit->nKnownArray->at(n*16 + i / 3) & 0x00010000))){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, rgbUnderline);
                            SelectObject(hdc, hPenUnderline);
                            MoveToEx(hdc, nHorizontal - 1,  nVertical, NULL);
                            LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra, nVertical);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }

                        if(bShowGroup && (i % 3 == 1 && Edit->nKnownArray->at(n*16 + i / 3) & 0x00010000)){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, rgbUnderline);
                            SelectObject(hdc, hPenUnderline);
                            MoveToEx(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra,  nVertical, NULL);
                            LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra,  nVertical - 3);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }
                        else if(bShowGroup && (i % 3 == 2 && (sHexText.at(i) != ' ' || rgbBackground != RGB(255,255,255)) && Edit->nKnownArray->at(n*16 + i / 3) & 0x00010000)){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, rgbUnderline);
                            SelectObject(hdc, hPenUnderline);
                            MoveToEx(hdc, nHorizontal + nExtra,  nVertical, NULL);
                            LineTo(hdc, nHorizontal + nExtra,  nVertical - 3);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }
                        /*
                        if(i % 3 == 1 && Edit->nKnownArray->at(n*16 + i / 3) & 0x00020000){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, RGB(0,0,0));
                            SelectObject(hdc, hPenUnderline);
                            //MoveToEx(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra + 1,  nVertical - 8, NULL);
                            //LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra + 1,  nVertical - 13);
                            //LineTo(hdc, nHorizontal - 2,  nVertical - 13);
                            MoveToEx(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra + 4,  nVertical - 12, NULL);
                            LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra + 4,  nVertical);
                            //LineTo(hdc, nHorizontal + ME_EDIT_CHAR_SIZE_X + nExtra,  nVertical);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }
                        else if(i % 3 == 2 && Edit->nKnownArray->at(n*16 + i / 3) & 0x00020000){
                            HPEN hPenUnderline = CreatePen(PS_SOLID, 1, RGB(0,0,0));
                            SelectObject(hdc, hPenUnderline);
                            //MoveToEx(hdc, nHorizontal + nExtra + 1,  nVertical - 8, NULL);
                            //LineTo(hdc, nHorizontal + nExtra + 1,  nVertical - 13);
                            //LineTo(hdc, nHorizontal - ME_EDIT_CHAR_SIZE_X - 2,  nVertical - 13);
                            MoveToEx(hdc, nHorizontal + nExtra + 4,  nVertical - 12, NULL);
                            LineTo(hdc, nHorizontal + nExtra + 4,  nVertical);
                            //LineTo(hdc, nHorizontal + nExtra,  nVertical);
                            DeleteObject(hPenUnderline);
                            SelectObject(hdc, hMonospace);
                        }
                        */


                        /// Do a completely separate draw for the charset.
                        if(i % 3 == 0){
                            SetTextColor(hdc, RGB(220, 220, 220));
                            SetBkColor(hdc, RGB(255, 255, 255));

                            if(bHighlited){
                                if(bDifferent){
                                    if(nClickArea == 2){
                                        SetTextColor(hdc, RGB(255, 170, 70));
                                    }
                                    else{
                                        SetTextColor(hdc, RGB(255, 210, 170));
                                    }
                                }
                                else{
                                    SetTextColor(hdc, RGB(255, 255, 255));
                                }
                                if(nClickArea == 2) SetBkColor(hdc, RGB(135, 135, 255));
                                else SetBkColor(hdc, RGB(185, 185, 255));
                            }
                            else if(nDataKnown == 3){
                                SetTextColor(hdc, DataColor(nDataKnown, false));
                            }
                            if(Edit->sBuffer->size() > n*16 + (i/3)){
                                char cToWrite = (bHighlited && bDifferent ? Edit->sCompareBuffer->at(n*16 + (i/3)) : Edit->sBuffer->at(n * 16 + (i/3)));
                                PrepareCharForDisplay(&cToWrite);
                                ExtTextOut(hdc, ME_EDIT_PADDING_LEFT + ME_EDIT_CHARSET_OFFSET + (i/3) * ME_EDIT_CHAR_SIZE_X, ME_EDIT_PADDING_TOP + n * ME_EDIT_NEXT_ROW - Edit->yCurrentScroll, NULL, NULL, &cToWrite, 1, NULL);
                            }
                        }
                    }
                }
                if(DEBUG_LEVEL > 50) std::cout << "Edits: WM_PAINT. Ending data while. Ending position: " << n << "\n";
            }
            /**/

            BitBlt(hdcReal, 0, 0, ME_HEX_WIN_SIZE_X, Edit->rcClient.bottom, hdc, 0, 0, SRCCOPY);

            SelectObject(hdc, hBMold);
            DeleteObject(memBM);
            DeleteDC(hdc);

            EndPaint(hwnd, &ps);
            if(DEBUG_LEVEL > 50) std::cout << "Edits: WM_PAINT. End.\n";
        }
        break;
        case WM_VSCROLL:
        {
            int yNewPos;    /* new position */

            si.cbSize = sizeof(si);
            si.fMask  = SIF_DISABLENOSCROLL | SIF_POS | SIF_TRACKPOS | SIF_PAGE;
            GetScrollInfo(Edit->hScrollVert, SB_CTL, &si);
            if(DEBUG_LEVEL > 80) std::cout << "GetScrollInfo(): max: n/a, min: n/a, page: " << si.nPage << ", current: " << si.nPos << " \n";

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
                    //std::cout << string_format("WM_HSCROLL: Thumb: %i \n", yNewPos);
                    //yNewPos = std::min(Edit->yMaxScroll - (signed int) si.nPage, yNewPos);
                    //std::cout << string_format("WM_HSCROLL: Thumb corrected: %i \n", yNewPos);
                break;
                /* User is dragging the scroll box. */
                case SB_THUMBTRACK:
                    yNewPos = (signed int) si.nTrackPos;
                    //std::cout << string_format("WM_HSCROLL: Thumbtrack: %i \n", yNewPos);
                    //std::cout << string_format("WM_HSCROLL: Thumbtrack corrected: %i (compared to %i - %i)\n", yNewPos, Edit->yMaxScroll, (signed int) si.nPage);
                break;
                default:
                    yNewPos = Edit->yCurrentScroll;
            }
            //yNewPos = min(Edit->yMaxScroll, yNewPos);
            //yNewPos = max(0, yNewPos);
            if(DEBUG_LEVEL > 80) std::cout << "WM_VSCROLL: max: " << Edit->yMaxScroll << ", page: " << si.nPage << ", current: " << Edit->yCurrentScroll << ", new: " << yNewPos << "\n";

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

            //std::cout << string_format("Zoom DEBUG: Zooming. \nnMinP=%i, nMaxP=%i, xCurrentScroll=%i, \nxCurrentScroll+rcClient.right/2=%i\n0=%i, Edit->yMaxScroll=%i, Edit->yCurrentScroll=%i, \nyCurrentScroll+(rcClient.bottom-nNonScreenY)/2=%i\n", nMinP, nMaxP, xCurrentScroll, xCurrentScroll+rcClient.right/2, 0, Edit->yMaxScroll, Edit->yCurrentScroll, Edit->yCurrentScroll+(rcClient.bottom-nNonScreenY)/2);

            return 0;
        }
        break;
        case WM_MOUSEWHEEL:
        {
            si.cbSize = sizeof(si);
            si.fMask  = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE;
            GetScrollInfo(Edit->hScrollVert, SB_CTL, &si);

            signed short nHiword = HIWORD(wParam);
            //std::cout << "WM_MOUSEWHEEL: Hiword=" << nHiword << ", Page=" << si.nPage << ", scrollDelta=" << (signed) si.nPage / 3 * (nHiword/120) << ", CurrentScroll=" << Edit->yCurrentScroll << ", NewScroll=" << Edit->yCurrentScroll - (signed) si.nPage / 3 * (nHiword/120) << "\n";
            Edit->yCurrentScroll -= (signed) (si.nPage / 6 * (nHiword/120));
            Edit->yCurrentScroll = std::max(si.nMin, std::min(Edit->yCurrentScroll, (signed) (si.nMax - si.nPage)));
            Edit->yCurrentScroll = (Edit->yCurrentScroll / ME_EDIT_NEXT_ROW) * ME_EDIT_NEXT_ROW;
            InvalidateRect(hwnd, &Edit->rcClient, false);
            UpdateWindow(hwnd);

            si.fMask  = SIF_DISABLENOSCROLL | SIF_POS;
            si.nPos   = Edit->yCurrentScroll;
            SetScrollInfo(Edit->hScrollVert, SB_CTL, &si, true);
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
    if(DEBUG_LEVEL > 70) std::cout << "Selected bytes: " << nSize << ".\n";
    std::string sString;
    char cInt [255];
    char cUInt [255];
    char cFloat [255];
    for(int n = 0; nSelectStart + n <= nSelectEnd; n++){
        if(DEBUG_LEVEL > 70) std::cout << "Extract byte #" << nSelectStart + n << ".\n";
        sString += (bShowDiff && Compare(nSelectStart+n) == 1 ? sCompareBuffer->at(nSelectStart+n) : sBuffer->at(nSelectStart+n));
    }

    if(nSize == 1){
        sprintf(cInt, "%hhi", (signed int) sString.front());
        sprintf(cUInt, "%hhu", (unsigned int) sString.front());
        sprintf(cFloat, "");
    }
    else if(nSize == 2){
        sprintf(cInt, "%hi", * (signed short*) &sString.front());
        sprintf(cUInt, "%hu", * (unsigned short*) &sString.front());
        sprintf(cFloat, "");

    }
    else if(nSize == 4){
        sprintf(cInt, "%i", * (signed int*) &sString.front());
        sprintf(cUInt, "%u", * (unsigned int*) &sString.front());
        sprintf(cFloat, "%.16f", * (float*) &sString.front());
    }
    else if(nSize == 8){
        sprintf(cInt, "%li", * (signed long*) &sString.front());
        sprintf(cUInt, "%lu", * (unsigned long*) &sString.front());
        sprintf(cFloat, "%.16g", * (double*) &sString.front());
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

int Edits::Compare(unsigned nPos){
    if(sCompareBuffer == nullptr || sBuffer == nullptr || sCompareBuffer->size() == 0 || sBuffer->size() == 0 || nPos >= sBuffer->size()) return -1; // Don't mark
    if(nPos >= sCompareBuffer->size()) return 2; // Out of range
    if(sBuffer->at(nPos) != sCompareBuffer->at(nPos)) return 1; // Different
    return 0; // Not different
}

void Edits::Cleanup(){
    nKnownArray = nullptr;
    sBuffer = nullptr;
    ShowWindow(hScrollVert, false);
    UpdateEdit();
}

void Edits::LoadData(){
    sSelected = TabCtrl_GetCurSelName(hTabs);
    if(sSelected == "MDL" && !Model.empty()){
        nKnownArray = &Model.GetKnownData();
        sCompareBuffer = &Model.GetCompareData();
        sBuffer = &Model.GetBuffer();
    }
    else if(sSelected == "MDX" && Model.Mdx){
        nKnownArray = &Model.Mdx->GetKnownData();
        sCompareBuffer = &Model.Mdx->GetCompareData();
        sBuffer = &Model.Mdx->GetBuffer();
    }
    else if(sSelected == "WOK" && Model.Wok){
        nKnownArray = &Model.Wok->GetKnownData();
        sCompareBuffer = &Model.Wok->GetCompareData();
        sBuffer = &Model.Wok->GetBuffer();
    }
    else if(sSelected == "PWK" && Model.Pwk){
        nKnownArray = &Model.Pwk->GetKnownData();
        sCompareBuffer = &Model.Pwk->GetCompareData();
        sBuffer = &Model.Pwk->GetBuffer();
    }
    else if(sSelected == "DWK 0" && Model.Dwk0){
        nKnownArray = &Model.Dwk0->GetKnownData();
        sCompareBuffer = &Model.Dwk0->GetCompareData();
        sBuffer = &Model.Dwk0->GetBuffer();
    }
    else if(sSelected == "DWK 1" && Model.Dwk1){
        nKnownArray = &Model.Dwk1->GetKnownData();
        sCompareBuffer = &Model.Dwk1->GetCompareData();
        sBuffer = &Model.Dwk1->GetBuffer();
    }
    else if(sSelected == "DWK 2" && Model.Dwk2){
        nKnownArray = &Model.Dwk2->GetKnownData();
        sCompareBuffer = &Model.Dwk2->GetCompareData();
        sBuffer = &Model.Dwk2->GetBuffer();
    }
    else{
        nKnownArray = nullptr;
        sCompareBuffer = nullptr;
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
    if(DEBUG_LEVEL > 80) std::cout << "New MaxScroll: " << yMaxScroll << ", new CurrentScroll: " << yCurrentScroll << "\n";
}

HWND Edits::GetWindowHandle(){
    return hMe;
}

void Edits::UpdateClientRect(){
    RECT rcParent;
    GetClientRect(GetParent(hMe), &rcParent);
    rcClient.top = 0;
    rcClient.left = ME_HEX_WIN_OFFSET_X;
    rcClient.bottom = rcParent.bottom - ME_STATUSBAR_Y - ME_TABS_OFFSET_Y_BOTTOM - ME_TABS_OFFSET_Y_TOP;
    rcClient.right = ME_HEX_WIN_SIZE_X;
}

void Edits::ShowHideEdit(){
    ShowWindow(hMe, bShowHex);
    if(bShowHex){
        LoadData();
    }
}

void Edits::UpdateEdit(){
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
        if(DEBUG_LEVEL > 80) std::cout << "GetScrollInfo(): max: " << si.nMax << ", min: " << si.nMin << ", page: n/a, current: " << si.nPos << "\n";
    }
}

void Edits::Resize(){
    UpdateClientRect();
    SetWindowPos(hMe, NULL, rcClient.left, ME_TABS_OFFSET_Y_TOP, ME_HEX_WIN_SIZE_X, rcClient.bottom, NULL);
    SetWindowPos(hScrollVert, NULL, ME_HEX_WIN_OFFSET_X + ME_HEX_WIN_SIZE_X - ME_SCROLLBAR_X, 0, GetSystemMetrics(SM_CXHTHUMB), rcClient.bottom, NULL);
    UpdateEdit();
}

void Edits::DetermineSelection(){
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
    if(nSelectEnd < nSelectStart || !bSelection || (nSelectStart > (int) sBuffer->size() - 1 && nSelectEnd > (int) sBuffer->size() - 1)){
        nSelectStart = -1;
        nSelectEnd = -1;
    }
    nSelectEnd = std::min(nSelectEnd, (int) sBuffer->size() - 1);
    nSelectStart = std::min(nSelectStart, (int) sBuffer->size() - 1);
    if(DEBUG_LEVEL > 100) std::cout << "Current selection from byte " << nSelectStart << " to byte " << nSelectEnd << ".\n";
}

void Edits::UpdateStatusBar(bool bCheck){
    char cString1 [255];
    char cString2 [255];
    char cString3 [255];
    if(nSelectEnd == -1 || nSelectStart == -1 || !bCheck || sBuffer == nullptr || !bSelection){
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) "");
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) "");
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) "");
        SetWindowText(hIntEdit, "");
        SetWindowText(hUIntEdit, "");
        SetWindowText(hFloatEdit, "");
    }
    else{
        if(bHexLocation){
            sprintf(cString1, "Offset: %X", nSelectStart);
            sprintf(cString2, "Block: %X-%X", nSelectStart, nSelectEnd);
            sprintf(cString3, "Length: %X", nSelectEnd - nSelectStart + 1);
        }
        else{
            sprintf(cString1, "Offset: %i", nSelectStart);
            sprintf(cString2, "Block: %i-%i", nSelectStart, nSelectEnd);
            sprintf(cString3, "Length: %i", nSelectEnd - nSelectStart + 1);
        }
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(0, 0), NULL), (LPARAM) cString1);
        if(nSelectStart == nSelectEnd) SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) "");
        else SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(1, 0), NULL), (LPARAM) cString2);
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(2, 0), NULL), (LPARAM) cString3);
    }
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
        case 8: //Possibly constant/meaningless/padding, but not random data
            return RGB(232, 232, 232);
        case 9: //function pointer - it can make the beginning certain structures stand out more
            return RGB(255, 160, 80);
        case 11: //Padding/unused portions filled with random data
            return RGB(162, 177, 90);
    }
    return RGB(0, 0, 0);
}

void Edits::UpdateStatusPosition(){
    if(!Model.GetFileData()) return;

    std::string sType = TabCtrl_GetCurSelName(hTabs);
    if(DEBUG_LEVEL > 80)
        std::cout << "Begin updating status position for " << sType << ".\n";

    BinaryFile * ptr_binfile = nullptr;
    if(sType == "MDL"){
        ptr_binfile = &Model;
    }
    else if(sType == "MDX"){
        ptr_binfile = &*Model.Mdx;
    }
    else if(sType == "WOK"){
        ptr_binfile = &*Model.Wok;
    }
    else if(sType == "PWK"){
        ptr_binfile = &*Model.Pwk;
    }
    else if(sType == "DWK 0"){
        ptr_binfile = &*Model.Dwk0;
    }
    else if(sType == "DWK 1"){
        ptr_binfile = &*Model.Dwk1;
    }
    else if(sType == "DWK 2"){
        ptr_binfile = &*Model.Dwk2;
    }
    else{
        //std::cout << "No appropriate type for data output...\n";
        return;
    }

    int nPos = ptHover.y * 16 + (ptHover.x) / 3;

    std::string sCaption;
    if(nPos < ptr_binfile->GetBuffer().size()) sCaption = ptr_binfile->GetPosition(nPos);

    /// Change text
    std::string sGet ((size_t) 255, '\0');
    SendMessage(hStatusBar, SB_GETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) &sGet.front());
    if(std::string(sGet.c_str()) != sCaption){
        SendMessage(hStatusBar, SB_SETTEXT, MAKEWPARAM(MAKEWORD(3, 0), NULL), (LPARAM) sCaption.c_str());
    }
}

void ScrollToData(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam, int nFile){
    if(cItem.at(0) == "") return;

    /// Geometry Node
    else if((cItem.at(1) == "Geometry") || ((cItem.at(3) == "Geometry") && ((cItem.at(1) == "Children") || (safesubstr(cItem.at(0), 0, 7) == "Parent:")))){
        Node & node = * (Node*) lParam;
        TabCtrl_SetCurSel(hTabs, TabCtrl_GetTabIndexByText(hTabs, "MDL"));
        Edit1.LoadData();
        int nTargetRow = ((int) node.nOffset + 12) / 16;
        Edit1.yCurrentScroll = std::min(Edit1.yMaxScroll, nTargetRow * ME_EDIT_NEXT_ROW);
        Edit1.UpdateEdit();
    }
    /// Animation
    else if(cItem.at(1) == "Animations"){
        Animation & anim = * (Animation*) lParam;
        TabCtrl_SetCurSel(hTabs, TabCtrl_GetTabIndexByText(hTabs, "MDL"));
        Edit1.LoadData();
        int nTargetRow = ((int) anim.nOffset + 12) / 16;
        Edit1.yCurrentScroll = std::min(Edit1.yMaxScroll, nTargetRow * ME_EDIT_NEXT_ROW);
        Edit1.UpdateEdit();
    }
    /// Animation Node
    else if((cItem.at(2) == "Animations") || ((cItem.at(4) == "Animations") && ((cItem.at(1) == "Children") || (safesubstr(cItem.at(0), 0, 7) == "Parent:")))){
        Node & node = * (Node*) lParam;
        TabCtrl_SetCurSel(hTabs, TabCtrl_GetTabIndexByText(hTabs, "MDL"));
        Edit1.LoadData();
        int nTargetRow = ((int) node.nOffset + 12) / 16;
        Edit1.yCurrentScroll = std::min(Edit1.yMaxScroll, nTargetRow * ME_EDIT_NEXT_ROW);
        Edit1.UpdateEdit();
    }
    else if(cItem.at(0) == "Vertices" && nFile == 0){
        Node & node = * (Node*) lParam;
        TabCtrl_SetCurSel(hTabs, TabCtrl_GetTabIndexByText(hTabs, "MDX"));
        Edit1.LoadData();
        int nTargetRow = ((int) node.Mesh.nOffsetIntoMdx) / 16;
        Edit1.yCurrentScroll = std::min(Edit1.yMaxScroll, nTargetRow * ME_EDIT_NEXT_ROW);
        Edit1.UpdateEdit();
    }
}
