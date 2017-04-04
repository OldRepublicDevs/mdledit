#include "frame.h"
#include "edits.h"
#include <Shlwapi.h>
#include <fstream>
#include "MDL.h"

char Frame::cClassName[] = "mdleditframe";
HINSTANCE Frame::hInstance = NULL;

HWND hFrame;
Edits Edit1;
HWND hTree;
HWND hStatusBar;
HWND hDisplayEdit;
HWND hTabs;
HWND hProgress;
MDL Model;
//MDX Mdx;
WOK Walkmesh;
HANDLE hThread;
bool FileEditor(HWND hwnd, int nID, std::string & cFile);
DWORD WINAPI ThreadProcessAscii(LPVOID lpParam);
DWORD WINAPI ThreadProcessBinary(LPVOID lpParam);

Frame::Frame(HINSTANCE hInstanceCreate){
    hInstance = hInstanceCreate; // Save Instance handle

    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = hInstance; // Instance of the application
    WindowClass.lpfnWndProc = FrameProc; // Pointer to callback procedure

    // #2 Class styles
    WindowClass.style = CS_DBLCLKS; // Class styles

    // #3 Background
    WindowClass.hbrBackground = CreateSolidBrush(RGB(250, 250, 250)); //(HBRUSH) (COLOR_WINDOW); // Background brush

    // #4 Cursor
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW); // Class cursor

    // #5 Icon
    WindowClass.hIcon = LoadIcon (NULL, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)); // Class Icon
    WindowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)); // Small icon for this class

    // #6 Menu
    WindowClass.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN); //MAKEINTRESOURCE(IDM_MAINMENU); // Menu Resource

    // #7 Other
    WindowClass.cbClsExtra = 0; // Extra bytes to allocate following the wndclassex structure
    WindowClass.cbWndExtra = 0; // Extra bytes to allocate following an instance of the structure
}

bool Frame::Run(int nCmdShow){
    if(!RegisterClassEx(&WindowClass)) return false;

    // Initialize Common controls
    /*
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES; // ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
    /**/

    // Ensure common control DLL is loaded
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    //icx.dwICC = ICC_BAR_CLASSES; // Specify BAR classes
    InitCommonControlsEx(&icx); // Load the common control DLL

    hFrame = CreateWindowEx(NULL, cClassName, "MDLedit", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT, CW_USEDEFAULT, 980, 610,
                        HWND_DESKTOP, NULL, hInstance, NULL);
    hMe = hFrame;
    if(!hMe) return false;
    ShowWindow(hMe, nCmdShow);
    return true;
}

LRESULT CALLBACK Frame::FrameProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    static RECT rcClient;
    //static char cFile[MAX_PATH];
    static std::string sFile;
    if(DEBUG_LEVEL > 500) std::cout<<string_format("FrameProc(): %i\n", (int) message);
    /* handle the messages */

    static HWND hIntLabel;
    static HWND hUIntLabel;
    static HWND hFloatLabel;
    switch(message){
        case WM_CREATE:
        {
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

            int nLabelOffsetX = ME_HEX_WIN_OFFSET_X + ME_HEX_WIN_SIZE_X + ME_DATA_LABEL_OFFSET_X;
            int nEditOffsetX = ME_HEX_WIN_OFFSET_X + ME_HEX_WIN_SIZE_X + ME_DATA_LABEL_OFFSET_X + ME_DATA_LABEL_SIZE_X + ME_DATA_EDIT_OFFSET_X;
            int nDataOffsetY [3];
            for(int n = 0; n < 3; n++){ nDataOffsetY[n] = ME_BASIC_OFFSET_Y + n * ME_DATA_NEXT_ROW;}
            hIntLabel = CreateWindowEx(NULL, "STATIC", "Int:", WS_VISIBLE | WS_CHILD | SS_RIGHT,
                                        nLabelOffsetX, nDataOffsetY[0] + ME_DATA_LABEL_ROW_OFFSET_Y, ME_DATA_LABEL_SIZE_X, ME_DATA_LABEL_SIZE_Y,
                                        hwnd, (HMENU) IDC_LBL_INT, GetModuleHandle(NULL), NULL);
            hUIntLabel = CreateWindowEx(NULL, "STATIC", "uInt:", WS_VISIBLE | WS_CHILD | SS_RIGHT,
                                        nLabelOffsetX, nDataOffsetY[1] + ME_DATA_LABEL_ROW_OFFSET_Y, ME_DATA_LABEL_SIZE_X, ME_DATA_LABEL_SIZE_Y,
                                        hwnd, (HMENU) IDC_LBL_UINT, GetModuleHandle(NULL), NULL);
            hFloatLabel = CreateWindowEx(NULL, "STATIC", "Float:", WS_VISIBLE | WS_CHILD | SS_RIGHT,
                                        nLabelOffsetX, nDataOffsetY[2] + ME_DATA_LABEL_ROW_OFFSET_Y, ME_DATA_LABEL_SIZE_X, ME_DATA_LABEL_SIZE_Y,
                                        hwnd, (HMENU) IDC_LBL_FLOAT, GetModuleHandle(NULL), NULL);
            SendMessage(hIntLabel, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));
            SendMessage(hUIntLabel, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));
            SendMessage(hFloatLabel, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));

            Edits::hIntEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY, // | ES_RIGHT,
                                        nEditOffsetX, nDataOffsetY[0], ME_DATA_EDIT_SIZE_X, ME_DATA_EDIT_SIZE_Y,
                                        hwnd, (HMENU) IDC_EDIT_INT, GetModuleHandle(NULL), NULL);
            Edits::hUIntEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY, // | ES_RIGHT,
                                        nEditOffsetX, nDataOffsetY[1], ME_DATA_EDIT_SIZE_X, ME_DATA_EDIT_SIZE_Y,
                                        hwnd, (HMENU) IDC_EDIT_UINT, GetModuleHandle(NULL), NULL);
            Edits::hFloatEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY, // | ES_RIGHT,
                                        nEditOffsetX, nDataOffsetY[2], ME_DATA_EDIT_SIZE_X, ME_DATA_EDIT_SIZE_Y,
                                        hwnd, (HMENU) IDC_EDIT_FLOAT, GetModuleHandle(NULL), NULL);
            SendMessage(Edits::hIntEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));
            SendMessage(Edits::hUIntEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));
            SendMessage(Edits::hFloatEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));

            hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwnd, IDC_STATUSBAR);
            int nBorders [4];
            nBorders[0] = ME_STATUSBAR_PART_X;
            nBorders[1] = 2 * ME_STATUSBAR_PART_X;
            nBorders[2] = 3 * ME_STATUSBAR_PART_X;
            nBorders[3] = -1;
            SendMessage(hStatusBar, SB_SETPARTS, (WPARAM) 4, (LPARAM) nBorders);

            GetClientRect(hwnd, &rcClient);
            hTree = CreateWindowEx(WS_EX_TOPMOST, WC_TREEVIEW, "Structure", WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
                           ME_TREE_OFFSET_X, ME_TREE_OFFSET_Y, ME_TREE_SIZE_X, rcClient.bottom - ME_TREE_OFFSET_Y - ME_STATUSBAR_Y - ME_TREE_SIZE_DIFF_Y,
                           hwnd, (HMENU) IDC_TREEVIEW, GetModuleHandle(NULL), NULL);
            hDisplayEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL,
                                        ME_TREE_OFFSET_X, ME_DISPLAY_OFFSET_Y, ME_TREE_SIZE_X, ME_DISPLAY_SIZE_Y,
                                        hwnd, (HMENU) IDC_EDIT_DISPLAY, GetModuleHandle(NULL), NULL);
            SendMessage(hDisplayEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));

            hTabs = CreateWindowEx(NULL, WC_TABCONTROL, "", WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_FIXEDWIDTH,
                                   ME_HEX_WIN_OFFSET_X, 0, ME_HEX_WIN_OFFSET_X + ME_TABS_SIZE_X, rcClient.bottom - ME_STATUSBAR_Y,
                                   hwnd, (HMENU) IDC_TABS, GetModuleHandle(NULL), NULL);
            SendMessage(hTabs, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));

            TCITEM tcAdd;
            std::string sAdd;
            tcAdd.mask = TCIF_TEXT;
            sAdd = "MDL";
            tcAdd.pszText = &sAdd[0];
            tcAdd.cchTextMax = 255;
            TabCtrl_InsertItem(hTabs, 0, &tcAdd);
            sAdd = "MDX";
            TabCtrl_InsertItem(hTabs, 1, &tcAdd);
            sAdd = "WOK";
            TabCtrl_InsertItem(hTabs, 2, &tcAdd);

            if(!Edit1.Run(hwnd, IDC_MAIN_EDIT)){
                std::cout<<"Major error, creation of Edit1 window failed.\n";
            }

            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;
            mii.fState = MFS_DISABLED;
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), IDM_LINK_HEAD, false, &mii);
            mii.fState = MFS_UNCHECKED;
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
            mii.fState = MFS_CHECKED;
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
        }
        break;
        case WM_NOTIFY:
        {
            NMHDR * nmhdr = (NMHDR *) lParam;
            HWND hControl = nmhdr->hwndFrom;
            int nID = nmhdr->idFrom;
            int nNotification = nmhdr->code;
            switch(nID){
                case IDC_TREEVIEW:
                {
                    if(nNotification == TVN_SELCHANGED){
                        NMTREEVIEW * nmtv = (NMTREEVIEW *) nmhdr;
                        ProcessTreeAction(nmtv->itemNew.hItem, ACTION_UPDATE_DISPLAY);
                    }
                    else if(nNotification == NM_RCLICK){
                        POINT pt;
                        GetCursorPos(&pt);
                        RECT rcItem;
                        HTREEITEM hSel = NULL;
                        HTREEITEM hSearch = TreeView_GetFirstVisible(hTree);
                        while(hSearch != NULL){
                            TreeView_GetItemRect(hTree, hSearch, &rcItem, true);
                            MapWindowPoints(hTree, HWND_DESKTOP, (LPPOINT) &rcItem, 2);
                            //std::cout<<string_format("Point: (%i, %i), Rect: (%i, %i, %i, %i)\n", pt.x, pt.y, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
                            if(PtInRect(&rcItem, pt)){
                                hSel = hSearch;
                                hSearch = NULL;
                            }
                            else{
                                hSearch = TreeView_GetNextVisible(hTree, hSearch);
                            }
                        }

                        if(hSel != NULL){
                            TreeView_SelectItem(hTree, hSel);

                            MenuLineAdder mla;
                                mla.hMenu = CreatePopupMenu();
                                mla.nIndex = 0;

                            ProcessTreeAction(hSel, ACTION_ADD_MENU_LINES, (void*) &mla);

                            bool bExpanded = (TreeView_GetItemState(hTree, hSel, TVIS_EXPANDED) & TVIS_EXPANDED);
                            bool bHasChild = (TreeView_GetChild(hTree, hSel) != NULL);
                            bool bExpandedChild = false;
                            bool bChildHasChild = false;
                            HTREEITEM hChild = TreeView_GetChild(hTree, hSel);
                            while(hChild != NULL && !bExpandedChild && bExpanded && bHasChild){
                                if(TreeView_GetChild(hTree, hChild) != NULL){
                                    bChildHasChild = true;
                                }
                                if(TreeView_GetItemState(hTree, hChild, TVIS_EXPANDED) & TVIS_EXPANDED){
                                    bExpandedChild = true;
                                }
                                else hChild = TreeView_GetNextSibling(hTree, hChild);
                            }

                            if(bHasChild && !bExpanded){
                                InsertMenu(mla.hMenu, mla.nIndex, MF_BYPOSITION | MF_STRING, IDPM_TV_EXPAND, "Expand");
                                mla.nIndex++;
                            }
                            else if(bHasChild){
                                InsertMenu(mla.hMenu, mla.nIndex, MF_BYPOSITION | MF_STRING, IDPM_TV_FOLD, "Fold");
                                mla.nIndex++;
                            }
                            else if(mla.nIndex == 0){
                                InsertMenu(mla.hMenu, mla.nIndex, MF_BYPOSITION | MF_STRING, NULL, "Nothing");
                                mla.nIndex++;
                            }

                            if(bExpandedChild && bExpanded){
                                InsertMenu(mla.hMenu, mla.nIndex, MF_BYPOSITION | MF_STRING, IDPM_TV_FOLD_CHILDREN, "Fold Children");
                                mla.nIndex++;
                            }
                            else if(bChildHasChild && bExpanded){
                                InsertMenu(mla.hMenu, mla.nIndex, MF_BYPOSITION | MF_STRING, IDPM_TV_EXPAND_CHILDREN, "Expand Children");
                                mla.nIndex++;
                            }

                            TrackPopupMenu(mla.hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                        }
                        else{
                            //It seems like I'm not on an item at all.
                        }
                    }
                }
                break;
                case IDC_TABS:
                {
                    if(nNotification == TCN_SELCHANGE){
                        Edit1.LoadData();
                    }
                }
                break;
            }
        }
        break;
        case WM_COMMAND:
        {
            int nNotification = HIWORD(wParam);
            int nID = LOWORD(wParam);
            HWND hControl = (HWND) lParam;
            switch(nID){
                case IDM_FILE_EXIT:
                {
                    SendMessage(hwnd, WM_DESTROY, NULL, NULL);
                }
                break;
                case IDM_MDL_OPEN:
                {
                    bool bSuccess = FileEditor(hwnd, nID, sFile);
                    if(bSuccess){
                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        bool bLinkHead = Model.HeadLinked();
                        if(!Model.NodeExists("neck_g")) mii.fState = MFS_DISABLED;
                        else if(bLinkHead) mii.fState = MFS_CHECKED;
                        else mii.fState = MFS_UNCHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), IDM_LINK_HEAD, false, &mii);
                        if(Model.bK2) mii.fState = MFS_UNCHECKED;
                        else mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
                        if(!Model.bK2) mii.fState = MFS_UNCHECKED;
                        else mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
                    }
                }
                break;
                case IDM_BIN_SAVE:
                {
                    int nReturn = IDOK;
                    if(Model.GetBuffer().empty()) nReturn = IDCANCEL;
                    if(nReturn == IDOK) FileEditor(hwnd, nID, sFile);
                }
                break;
                case IDM_ASCII_SAVE:
                {
                    int nReturn = IDOK;
                    if(!Model.GetFileData()) nReturn = IDCANCEL;
                    if(!Model.Mdx && nReturn == IDOK) nReturn = MessageBox(hwnd, "Warning! No MDX is loaded! MDLedit can still export without the MDX data, but this means exporting without weights, UVs and smoothing groups. Mesh geometry may also be affected.", "Warning!", MB_OKCANCEL | MB_ICONWARNING);
                    if(nReturn == IDOK) FileEditor(hwnd, nID, sFile);
                }
                break;
                case IDM_LINK_HEAD:
                {
                    if(!Model.empty()){
                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        GetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), IDM_LINK_HEAD, false, &mii);
                        bool bLinkHead = !(mii.fState & MFS_CHECKED); //Revert it, because user just clicked it so we need to turn it off/on
                        if(Model.LinkHead(bLinkHead)){
                            if(bLinkHead) mii.fState = MFS_CHECKED;
                            else mii.fState = MFS_UNCHECKED;
                            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), IDM_LINK_HEAD, false, &mii);
                            Edit1.UpdateEdit();
                            HTREEITEM hSel = TreeView_GetSelection(hTree);
                            if(hSel != NULL) ProcessTreeAction(hSel, ACTION_UPDATE_DISPLAY);
                        }
                        else std::cout<<"neck_g was not found!\n";
                    }
                }
                break;
                case IDM_GAME_K1:
                {
                    if(Model.bK2){
                        if(Model.GetFileData()){
                            int nResult = WarningCancel("Changing the game will cause recompilation of the open model.");
                            if(nResult == MB_OK){
                                Model.bK2 = false;
                            }
                        }
                        else{
                            Model.bK2 = false;
                        }
                        if(!Model.bK2){
                            MENUITEMINFO mii;
                            mii.cbSize = sizeof(MENUITEMINFO);
                            mii.fMask = MIIM_STATE;
                            mii.fState = MFS_CHECKED;
                            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
                            mii.fState = MFS_UNCHECKED;
                            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
                        }
                    }
                }
                break;
                case IDM_GAME_K2:
                {
                    if(!Model.bK2){
                        if(Model.GetFileData()){
                            int nResult = WarningCancel("Changing the game will cause recompilation of the open model.");
                            if(nResult == MB_OK){
                                Model.bK2 = true;
                            }
                        }
                        else{
                            Model.bK2 = true;
                        }
                        if(Model.bK2){
                            MENUITEMINFO mii;
                            mii.cbSize = sizeof(MENUITEMINFO);
                            mii.fMask = MIIM_STATE;
                            mii.fState = MFS_UNCHECKED;
                            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
                            mii.fState = MFS_CHECKED;
                            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
                        }
                    }
                }
                break;
                case IDD_EDITOR_DLG:
                {
                    Edit1.UpdateEdit();
                    HTREEITEM hSel = TreeView_GetSelection(hTree);
                    if(hSel != NULL) ProcessTreeAction(hSel, ACTION_UPDATE_DISPLAY);
                }
                break;
                case IDM_SETTINGS:
                {
                    DialogBoxParam(NULL, MAKEINTRESOURCE(DLG_SETTINGS), hwnd, SettingsProc, (LPARAM) &Model);
                }
                break;
                case IDPM_TV_FOLD:
                case IDPM_TV_EXPAND:
                {
                    HTREEITEM hSel = TreeView_GetSelection(hTree);
                    TreeView_Expand(hTree, hSel, TVE_TOGGLE);
                }
                break;
                case IDPM_TV_FOLD_CHILDREN:
                {
                    HTREEITEM hSel = TreeView_GetSelection(hTree);
                    HTREEITEM hChild = TreeView_GetChild(hTree, hSel);
                    while(hChild != NULL){
                        TreeView_Expand(hTree, hChild, TVE_COLLAPSE);
                        hChild = TreeView_GetNextSibling(hTree, hChild);
                    }
                }
                break;
                case IDPM_TV_EXPAND_CHILDREN:
                {
                    HTREEITEM hSel = TreeView_GetSelection(hTree);
                    HTREEITEM hChild = TreeView_GetChild(hTree, hSel);
                    while(hChild != NULL){
                        TreeView_Expand(hTree, hChild, TVE_EXPAND);
                        hChild = TreeView_GetNextSibling(hTree, hChild);
                    }
                }
                break;
                case IDPM_OPEN_CONTROLLER_DATA:
                {
                    ProcessTreeAction(TreeView_GetSelection(hTree), ACTION_OPEN_VIEWER, NULL);
                }
                break;
                case IDPM_OPEN_GEO_VIEWER:
                {
                    ProcessTreeAction(TreeView_GetSelection(hTree), ACTION_OPEN_GEO_VIEWER, NULL);
                }
                break;
                case IDPM_OPEN_EDITOR:
                {
                    ProcessTreeAction(TreeView_GetSelection(hTree), ACTION_OPEN_EDITOR, NULL);
                }
                break;
            }
        }
        break;
        case WM_SIZE:
        {
            GetClientRect(hwnd, &rcClient);

            SetWindowPos(hTree, NULL, ME_TREE_OFFSET_X, ME_TREE_OFFSET_Y, ME_TREE_SIZE_X, rcClient.bottom - ME_TREE_OFFSET_Y - ME_STATUSBAR_Y - ME_TREE_SIZE_DIFF_Y, NULL);
            SetWindowPos(hTabs, NULL, ME_HEX_WIN_OFFSET_X, 0, ME_HEX_WIN_OFFSET_X + ME_TABS_SIZE_X, rcClient.bottom - ME_STATUSBAR_Y, NULL);
            Edit1.Resize();

            // Resize the statusbar;
            SendMessage(hStatusBar,message,wParam,lParam);
        }
        break;
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC) wParam;
            HWND hControl = (HWND) lParam;

            if(hControl == Edits::hIntEdit ||
               hControl == Edits::hUIntEdit ||
               hControl == Edits::hFloatEdit ||
               hControl == hDisplayEdit ){
                SetBkColor(hdcStatic, RGB(255, 255, 255));
                return (INT_PTR) CreateSolidBrush(RGB(255, 255, 255));
            }
            else{
                SetBkColor(hdcStatic, RGB(250, 250, 250));
                return (INT_PTR) CreateSolidBrush(RGB(250, 250, 250));
            }
        }
        case WM_DESTROY:
        {
            if(!Model.empty()){
                TreeView_DeleteAllItems(hTree);
                Model.FlushAll();
            }
            PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
        }
        break;
        /* for messages that we don't deal with */
        default:
        {
            return DefWindowProc (hwnd, message, wParam, lParam);
        }
    }

    return 0;
}

bool FileEditor(HWND hwnd, int nID, std::string & cFile){
    OPENFILENAME ofn;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    std::string cExt;
    bool bReturn = false;

    cFile.resize(MAX_PATH);
    if(nID == IDM_ASCII_SAVE){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "ASCII MDL Format (*.mdl)\0*.mdl\0";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            cExt = PathFindExtension(cFile.c_str());
            if(cExt != ".mdl"){
                if(strlen(cFile.c_str()) + 5 < MAX_PATH){
                    cFile += ".mdl";
                }
                else{
                    Error("The file is not an .mdl file! Unable to save!");
                    return false;
                }
            }

            //Create file
            std::ofstream file(cFile, std::fstream::out);

            if(!file.is_open()){
                std::cout<<"File creation failed. Aborting.\n";
                return false;
            }

            //Convert the data and put it into a string
            std::string sAsciiExport;
            Model.ExportAscii(sAsciiExport);

            //Write and close file
            file<<sAsciiExport;
            file.close();

            sAsciiExport.clear();
            sAsciiExport.shrink_to_fit();
            bReturn = true;        }
        else std::cout<<"Selecting file failed. :( \n";
    }
    else if(nID == IDM_BIN_SAVE){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "Binary MDL Format (*.mdl)\0*.mdl\0";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            cExt = PathFindExtension(cFile.c_str());
            if(cExt != ".mdl"){
                if(strlen(cFile.c_str()) + 5 < MAX_PATH){
                    cFile += ".mdl";
                }
                else{
                    Error("The file is not an .mdl file! Unable to save!");
                    return false;
                }
            }

            //Create file
            std::ofstream file(cFile, std::ios::binary | std::fstream::out);
            //std::ofstream file(cFile);

            if(!file.is_open()){
                std::cout<<"File creation failed. Aborting.\n";
                return false;
            }

            //Put data into a strings
            std::string sBinaryExport;
            Model.Export(sBinaryExport);

            //Write and close file
            file<<sBinaryExport;
            file.close();

            //Save mdx
            std::string cMdx;
            cMdx = cFile;
            char * cExt2 = PathFindExtension(cMdx.c_str());
            sprintf(cExt2, ".mdx");

            file.open(cMdx, std::ios::binary | std::fstream::out);

            sBinaryExport.clear();
            Model.Mdx->Export(sBinaryExport);

            //Write and close file
            file<<sBinaryExport;
            file.close();

            sBinaryExport.clear();
            sBinaryExport.shrink_to_fit();

            bReturn = true;        }
        else std::cout<<"Selecting file failed. :( \n";
    }
    else if(nID == IDM_MDL_OPEN){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "MDL Format (*.mdl)\0*.mdl\0";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;        if(GetOpenFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            cExt = PathFindExtension(cFile.c_str());
            if(cExt != ".mdl"){
                Error("The file is not an .mdl file!");
                return false;
            }

            //Create file
            std::ifstream file(cFile, std::ios::binary);


            if(!file.is_open()){
                std::cout<<"File creation/opening failed. Aborting.\n";
                return false;
            }

            //If everything checks out, we may begin reading
            bool bAscii = false;
            file.seekg(0,std::ios::beg);
            char cBinary [4];
            file.read(cBinary, 4);
            //First check whether it's an ascii or a binary
            if(cBinary[0]!='\0' || cBinary[1]!='\0' || cBinary[2]!='\0' || cBinary[3]!='\0'){
                bAscii = true;
            }

            //Now we need to check our current data
            if(Model.GetFileData()){
                //A model is already open. Flush it to make room for the new one.
                TreeView_DeleteAllItems(hTree);
                Model.FlushData();
                Walkmesh.FlushAll();
            }

            if(bAscii){
                std::cout<<"Reading ascii...\n";
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0,std::ios::beg);
                Model.Ascii.reset(new ASCII());
                std::vector<char> & sBufferRef = Model.CreateAsciiBuffer(length);
                file.read(&sBufferRef[0], length);
                file.close();

                //Process the data
                Model.SetFilePath(cFile);
                if(Model.ReadAscii()){
                    DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 1);

                    bReturn = true;
                }
                else{
                    //We failed reading the ascii, so we need to clean up
                    Edit1.LoadData();
                    ProcessTreeAction(NULL, ACTION_UPDATE_DISPLAY, nullptr);
                    bReturn = false;
                }
            }
            else{
                std::cout<<"Reading binary...\n";
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0,std::ios::beg);
                std::vector<char> & sBufferRef = Model.CreateBuffer(length);
                file.read(&sBufferRef[0], length);
                file.close();
                Model.SetFilePath(cFile);

                //Open and process .mdx if it exists
                std::string cMdx;
                cMdx = cFile;
                char * cExt2 = PathFindExtension(cMdx.c_str());
                sprintf(cExt2, ".mdx");
                if(PathFileExists(cMdx.c_str())){
                    file.open(cMdx, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (mdx). Aborting.\n";
                    }
                    else{
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        Model.Mdx.reset(new MDX());
                        std::vector<char> & sBufferRef = Model.Mdx->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        Model.Mdx->SetFilePath(cMdx);
                    }
                }
                else{                    PathStripPath(&cMdx.front());
                    Warning("Could not find "+cMdx+" in the same directory. Will load the without the MDX data.");
                }

                //Open and process .wok if it exists
                std::string cWok;
                cWok = cFile;
                cExt2 = PathFindExtension(cWok.c_str());
                sprintf(cExt2, ".wok");
                if(PathFileExists(cWok.c_str())){
                    file.open(cWok, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (wok). Aborting.\n";
                    }
                    else{
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        std::vector<char> & sBufferRef = Walkmesh.CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        Walkmesh.SetFilePath(cWok);
                    }
                }

                //Process the data
                DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 2);

                bReturn = true;
            }        }
        else std::cout<<"Selecting file failed. :( \n";    }
    return bReturn;
}

INT_PTR CALLBACK ProgressProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch(message){
        case WM_INITDIALOG:
        {
            RECT rcStatus;
            GetClientRect(hwnd, &rcStatus);
            hProgress = CreateWindowEx(NULL, PROGRESS_CLASS, "", WS_VISIBLE | WS_CHILD,
                                            10, 20, rcStatus.right - 20, 18,
                                            hwnd, (HMENU) IDC_STATUSBAR_PROGRESS, NULL, NULL);
            SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);

            if(lParam == 1) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessAscii, hwnd, 0, NULL);
            else if(lParam == 2) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessBinary, hwnd, 0, NULL);
            else EndDialog(hwnd, NULL);
        }
        break;
        case 69:
        {
            CloseHandle(hThread);
            EndDialog(hwnd, NULL);
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}

DWORD WINAPI ThreadProcessAscii(LPVOID lpParam){
    Model.AsciiPostProcess();
    //This should bring us to a state where all the practical data is ready,
    //but not the binary file-specific data, such as offsets, etc..

    Model.Compile();
    //This should bring us to a state where all data is ready,
    //even the binary file-specific data

    SetWindowText(hDisplayEdit, "");
    Edit1.LoadData(); //Loads up the binary file onto the screen
    BuildTree(Model); //Fills the TreeView control

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadProcessBinary(LPVOID lpParam){
    Model.DecompileModel();
    if(!Walkmesh.empty()) Walkmesh.ProcessWalkmesh();

    //Load the data
    SetWindowText(hDisplayEdit, "");
    Edit1.LoadData();
    BuildTree(Model);
    if(!Walkmesh.empty()) BuildTree(Walkmesh);
    std::cout<<string_format("Data loaded!\n");

    Model.CheckPeculiarities(); //Finally, check for peculiarities

    //Create filedebug
    //std::string sSuperDebug = Model.GetFullPath().c_str();
    //sSuperDebug += "_super.txt";
    //std::stringstream filedebug;
    /*
    std::vector<MDL> Supermodels;
    LoadSupermodel(Model, Supermodels);
    for(int m = Supermodels.size() - 1; m >= 0; m--){
        FileHeader & Data = *Supermodels.at(m).GetFileData();
        //filedebug<<Data.MH.GH.sName.c_str()<<"\r\n";
        for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
            Node & node = Data.MH.ArrayOfNodes.at(n);
            //filedebug<<Data.MH.Names.at(node.Head.nNameIndex).sName<<" "<<node.Head.nNameIndex<<" "<<node.Head.nID1;
            //if(node.Head.nType & NODE_HAS_MESH) filedebug<<" "<<node.Mesh.nMeshInvertedCounter;
            //filedebug<<"\r\n";
        }
    }
    FileHeader & Data = *Model.GetFileData();
    filedebug<<Data.MH.GH.sName<<"\r\n";
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);
        filedebug<<Data.MH.Names.at(node.Head.nNameIndex).sName<<" "<<node.Head.nNameIndex<<" "<<node.Head.nID1;
        if(node.Head.nType & NODE_HAS_MESH) filedebug<<" "<<node.Mesh.nMeshInvertedCounter;
        filedebug<<"\r\n";
    }
    Supermodels.clear();
    Supermodels.shrink_to_fit();
    std::ofstream filedebugwrite(sSuperDebug, std::fstream::out);
    if(filedebugwrite.is_open()){
        filedebugwrite<<filedebug.str();
    }
    filedebugwrite.close();
    */

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

void ProcessTreeAction(HTREEITEM hItem, const int & nAction, void * Pointer){
    std::vector<std::string> cItem(15);
    LPARAM lParam;

    //Get our selected item
    TVITEM tvNewSelect;
    tvNewSelect.mask = TVIF_TEXT | TVIF_PARAM;
    tvNewSelect.cchTextMax = 255;
    char cGet [255];
    tvNewSelect.hItem = hItem;
    tvNewSelect.pszText = cGet;
    TreeView_GetItem(hTree, &tvNewSelect);

    //Get pointer to data and name
    cItem[0] = cGet;
    lParam = tvNewSelect.lParam;

    //Fill cItem with all the ancestors of our item
    bool bStop = false;
    if(hItem == NULL) bStop = true;
    int n = 1;
    while(!bStop && !(cItem[0] == Model.GetFilename()) && !(cItem[0] == Walkmesh.GetFilename())){
        tvNewSelect.hItem = TreeView_GetParent(hTree, tvNewSelect.hItem);
        tvNewSelect.pszText = cGet;
        TreeView_GetItem(hTree, &tvNewSelect);
        cItem[n] = cGet;
        if((cItem[n] == Model.GetFilename()) || (cItem[n] == Walkmesh.GetFilename())) bStop = true;
        else n++;
    }

    //Perform desired action
    if(nAction == ACTION_UPDATE_DISPLAY){
        std::stringstream sPrint;
        //Determine cPrint that is to be shown
        if(hItem != NULL) DetermineDisplayText(cItem, sPrint, lParam);

        //Update DisplayEdit
        SetWindowText(hDisplayEdit, sPrint.str().c_str());
    }
    else if(nAction == ACTION_ADD_MENU_LINES){
        AddMenuLines(cItem, lParam, (MenuLineAdder*) Pointer);
    }
    else if(nAction == ACTION_OPEN_VIEWER){
        OpenViewer(Model, cItem, lParam);
    }
    else if(nAction == ACTION_OPEN_GEO_VIEWER){
        OpenGeoViewer(Model, cItem, lParam);
    }
    else if(nAction == ACTION_OPEN_EDITOR){
        OpenEditorDlg(Model, cItem, lParam);
    }
}
