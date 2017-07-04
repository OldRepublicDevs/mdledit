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
MDL Model;
bool bShowHex;

bool AppendTab(HWND hTabControl, std::string sName){
    int nTabs = TabCtrl_GetItemCount(hTabControl);
    TCITEM tcAdd;
    tcAdd.mask = TCIF_TEXT;
    tcAdd.pszText = &sName[0];
    tcAdd.cchTextMax = strlen(sName.c_str());
    return (TabCtrl_InsertItem(hTabControl, nTabs, &tcAdd) != -1);
}

void FixHead(){
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    GetMenuItemInfo(GetSubMenu(GetMenu(hFrame), 1), IDM_LINK_HEAD, false, &mii);
    bool bLinkHead = mii.fState & MFS_CHECKED;
    if(bLinkHead){
        if(Model.LinkHead(bLinkHead)){
            Edit1.UpdateEdit();
            HTREEITEM hSel = TreeView_GetSelection(hTree);
            if(hSel != NULL) ProcessTreeAction(hSel, ACTION_UPDATE_DISPLAY);
        }
        else std::cout<<"neck_g was not found!\n";
    }
}

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
    /** /

    // Ensure common control DLL is loaded
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icx.dwICC = ICC_BAR_CLASSES; // Specify BAR classes
    InitCommonControlsEx(&icx); // Load the common control DLL
    /**/

    hFrame = CreateWindowEx(NULL, cClassName, "MDLedit", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT, CW_USEDEFAULT, 368, 610,
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
    if(DEBUG_LEVEL > 500) std::cout<<"FrameProc(): "<<(int) message<<"\n";
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
            HFONT hFont2 = CreateFont(
                14,  //Height
                0,  //Width
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
                "MS Shell Dlg" //"Segoe UI" 	// pointer to typeface name string
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
            SendMessage(hIntLabel, WM_SETFONT, (WPARAM) hFont2, MAKELPARAM(TRUE, 0));
            SendMessage(hUIntLabel, WM_SETFONT, (WPARAM) hFont2, MAKELPARAM(TRUE, 0));
            SendMessage(hFloatLabel, WM_SETFONT, (WPARAM) hFont2, MAKELPARAM(TRUE, 0));
            ShowWindow(hIntLabel, false);
            ShowWindow(hUIntLabel, false);
            ShowWindow(hFloatLabel, false);

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
            ShowWindow(Edits::hIntEdit, false);
            ShowWindow(Edits::hUIntEdit, false);
            ShowWindow(Edits::hFloatEdit, false);

            hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwnd, IDC_STATUSBAR);
            int nBorders [4];
            nBorders[0] = ME_STATUSBAR_PART_X;
            nBorders[1] = 2 * ME_STATUSBAR_PART_X;
            nBorders[2] = 3 * ME_STATUSBAR_PART_X;
            nBorders[3] = -1;
            SendMessage(hStatusBar, SB_SETPARTS, (WPARAM) 4, (LPARAM) nBorders);
            ShowWindow(hStatusBar, false);

            GetClientRect(hwnd, &rcClient);
            hTree = CreateWindowEx(WS_EX_TOPMOST, WC_TREEVIEW, "Structure", WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
                           ME_TREE_OFFSET_X, ME_TREE_OFFSET_Y, ME_TREE_SIZE_X, rcClient.bottom - ME_TREE_OFFSET_Y - ME_STATUSBAR_Y - ME_TREE_SIZE_DIFF_Y,
                           hwnd, (HMENU) IDC_TREEVIEW, GetModuleHandle(NULL), NULL);
            hDisplayEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL,
                                        ME_TREE_OFFSET_X, ME_DISPLAY_OFFSET_Y, ME_TREE_SIZE_X, ME_DISPLAY_SIZE_Y,
                                        hwnd, (HMENU) IDC_EDIT_DISPLAY, GetModuleHandle(NULL), NULL);
            SendMessage(hDisplayEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));

            hTabs = CreateWindowEx(NULL, WC_TABCONTROL, "", WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_FIXEDWIDTH,
                                   ME_HEX_WIN_OFFSET_X, 0, ME_HEX_WIN_OFFSET_X + ME_TABS_SIZE_X, rcClient.bottom - ME_STATUSBAR_Y,
                                   hwnd, (HMENU) IDC_TABS, GetModuleHandle(NULL), NULL);
            SendMessage(hTabs, WM_SETFONT, (WPARAM) hFont2, MAKELPARAM(TRUE, 0));
            ShowWindow(hTabs, false);

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
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_XBOX, false, &mii);
            mii.fState = MFS_CHECKED;
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_PC, false, &mii);
            mii.fState = MFS_GRAYED;
            SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 0), 1, true, &mii);
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
                case IDM_MASS_TO_ASCII:
                case IDM_MASS_TO_BIN:
                {
                    FileEditor(hwnd, nID, sFile);
                }
                break;
                case IDM_MDL_OPEN:
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_STATE;
                    bool bSuccess = FileEditor(hwnd, nID, sFile);
                    if(bSuccess){
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
                        if(Model.bXbox) mii.fState = MFS_UNCHECKED;
                        else mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_PC, false, &mii);
                        if(!Model.bXbox) mii.fState = MFS_UNCHECKED;
                        else mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_XBOX, false, &mii);
                        if(!Model.empty()) mii.fState = MFS_ENABLED;
                        else mii.fState = MFS_GRAYED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 0), 1, true, &mii);
                    }
                    std::string sNewName = "MDLedit";
                    if(bSuccess) sNewName += " (" + Model.GetFilename() + ")";
                    SetWindowText(hwnd, sNewName.c_str());
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
                        Model.bK2 = false;
                        if(Model.GetFileData()){
                            Model.Compile();
                            FixHead();
                            Edit1.LoadData();
                        }

                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
                        mii.fState = MFS_UNCHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
                    }
                }
                break;
                case IDM_GAME_K2:
                {
                    if(!Model.bK2){
                        Model.bK2 = true;
                        if(Model.GetFileData()){
                            Model.Compile();
                            FixHead();
                            Edit1.LoadData();
                        }

                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        mii.fState = MFS_UNCHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K1, false, &mii);
                        mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 2), IDM_GAME_K2, false, &mii);
                    }
                }
                break;
                case IDM_PLATFORM_PC:
                {
                    if(Model.bXbox){
                        Model.bXbox = false;
                        if(Model.GetFileData()){
                            Model.Compile();
                            FixHead();
                            Edit1.LoadData();
                        }

                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_PC, false, &mii);
                        mii.fState = MFS_UNCHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_XBOX, false, &mii);
                    }
                }
                break;
                case IDM_PLATFORM_XBOX:
                {
                    if(!Model.bXbox){
                        Model.bXbox = true;
                        if(Model.GetFileData()){
                            Model.Compile();
                            FixHead();
                            Edit1.LoadData();
                        }

                        MENUITEMINFO mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        mii.fState = MFS_UNCHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_PC, false, &mii);
                        mii.fState = MFS_CHECKED;
                        SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 3), IDM_PLATFORM_XBOX, false, &mii);
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
                case IDM_EDIT_TEXTURES:
                {
                    if(2 == DialogBoxParam(NULL, MAKEINTRESOURCE(DLG_EDIT_TEXTURES), hwnd, TexturesProc, (LPARAM) &Model)){
                        std::cout<<"Cause model reprocessing!\n";
                        //TreeView_DeleteAllItems(hTree);
                        //DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 3);
                        Model.Compile();
                        FixHead();
                    }
                    Edit1.UpdateEdit();
                    HTREEITEM hSel = TreeView_GetSelection(hTree);
                    if(hSel != NULL) ProcessTreeAction(hSel, ACTION_UPDATE_DISPLAY);
                }
                break;
                case IDM_VIEW_HEX:
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_STATE;
                    GetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 4), IDM_VIEW_HEX, false, &mii);
                    bShowHex = !(mii.fState & MFS_CHECKED); //Revert it, because user just clicked it so we need to turn it off/on
                    if(bShowHex) mii.fState = MFS_CHECKED;
                    else mii.fState = MFS_UNCHECKED;
                    SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 4), IDM_VIEW_HEX, false, &mii);
                    Edit1.ShowHideEdit();
                    if(bShowHex){
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_INT), true);
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_UINT), true);
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_FLOAT), true);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_INT), true);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_UINT), true);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_FLOAT), true);
                        ShowWindow(hTabs, true);
                        ShowWindow(hStatusBar, true);
                        Edit1.UpdateEdit();
                        SetWindowPos(hwnd, NULL, 0, 0, 980, 610, SWP_NOMOVE | SWP_NOZORDER);
                    }
                    else{
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_INT), false);
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_UINT), false);
                        ShowWindow(GetDlgItem(hwnd, IDC_LBL_FLOAT), false);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_INT), false);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_UINT), false);
                        ShowWindow(GetDlgItem(hwnd, IDC_EDIT_FLOAT), false);
                        ShowWindow(hTabs, false);
                        ShowWindow(hStatusBar, false);
                        SetWindowPos(hwnd, NULL, 0, 0, 368, 610, SWP_NOMOVE | SWP_NOZORDER);
                    }
                }
                break;
                case IDM_HELP_ABOUT:
                {
                    DialogBox(NULL, MAKEINTRESOURCE(DLG_ABOUT), hwnd, AboutProc);
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
                case IDPM_VIEW_ASCII:
                {
                    ProcessTreeAction(TreeView_GetSelection(hTree), ACTION_OPEN_VIEWER, NULL);
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
            const int nCompactOffsetTop = 1;
            const int nCompactOffsetBottom = 1;
            const int nCompactOffsetLeft = 1;
            const int nCompactOffsetRight = 1;

            if(bShowHex) SetWindowPos(hDisplayEdit, NULL,
                                      ME_TREE_OFFSET_X,
                                      ME_DISPLAY_OFFSET_Y,
                                      (rcClient.right - rcClient.left) - ME_HEX_WIN_OFFSET_X - ME_TREE_OFFSET_X - 5,
                                      ME_DISPLAY_SIZE_Y, NULL);
            else SetWindowPos(hDisplayEdit, NULL,
                              nCompactOffsetLeft,
                              nCompactOffsetTop,
                              (rcClient.right - rcClient.left) - nCompactOffsetLeft - nCompactOffsetRight,
                              ME_DISPLAY_SIZE_Y, NULL);

            if(bShowHex) SetWindowPos(hTree, NULL,
                                      ME_TREE_OFFSET_X,
                                      ME_DISPLAY_OFFSET_Y + ME_DISPLAY_SIZE_Y + 2, //ME_TREE_OFFSET_Y,
                                      (rcClient.right - rcClient.left) - ME_HEX_WIN_OFFSET_X - ME_TREE_OFFSET_X - 5,
                                      rcClient.bottom - (ME_DISPLAY_OFFSET_Y + ME_DISPLAY_SIZE_Y + 2) - ME_STATUSBAR_Y - ME_TREE_SIZE_DIFF_Y + 5, NULL);
            else SetWindowPos(hTree, NULL,
                              nCompactOffsetLeft,
                              nCompactOffsetTop*2 + ME_DISPLAY_SIZE_Y,
                              (rcClient.right - rcClient.left) - nCompactOffsetLeft - nCompactOffsetRight,
                              rcClient.bottom - (nCompactOffsetTop*2 + ME_DISPLAY_SIZE_Y + nCompactOffsetBottom), NULL);

            if(bShowHex) SetWindowPos(hTabs, NULL, ME_HEX_WIN_OFFSET_X, 0, ME_HEX_WIN_OFFSET_X + ME_TABS_SIZE_X, rcClient.bottom - ME_STATUSBAR_Y, NULL);
            if(bShowHex) Edit1.Resize();

            // Resize the statusbar;
            RECT rcStatus;
            GetClientRect(hStatusBar, &rcStatus);
            SendMessage(hStatusBar,message,wParam,lParam);
            InvalidateRect(hStatusBar, &rcStatus, false);
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
    std::string FilenameModel = Model.GetFilename();
    std::string FilenameWalkmesh = (Model.Wok ? Model.Wok->GetFilename() : "");
    std::string FilenamePwk = (Model.Pwk ? Model.Pwk->GetFilename() : "");
    std::string FilenameDwk0 = (Model.Dwk0 ? Model.Dwk0->GetFilename() : "");
    std::string FilenameDwk1 = (Model.Dwk1 ? Model.Dwk1->GetFilename() : "");
    std::string FilenameDwk2 = (Model.Dwk2 ? Model.Dwk2->GetFilename() : "");
    int nFile = -1;
    while(!bStop && !(cItem[0] == FilenameModel) && !(cItem[0] == FilenameWalkmesh) && !(cItem[0] == FilenamePwk) && !(cItem[0] == FilenameDwk0) && !(cItem[0] == FilenameDwk1) && !(cItem[0] == FilenameDwk2)){
        tvNewSelect.hItem = TreeView_GetParent(hTree, tvNewSelect.hItem);
        tvNewSelect.pszText = cGet;
        TreeView_GetItem(hTree, &tvNewSelect);
        cItem[n] = cGet;

        if(cItem[n] == FilenameModel) nFile = 0;
        else if(cItem[n] == FilenameWalkmesh) nFile = 1;
        else if(cItem[n] == FilenamePwk) nFile = 2;
        else if(cItem[n] == FilenameDwk0) nFile = 3;
        else if(cItem[n] == FilenameDwk1) nFile = 4;
        else if(cItem[n] == FilenameDwk2) nFile = 5;
        else{
            n++;
            continue;
        }
        break;
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
        AddMenuLines(cItem, lParam, (MenuLineAdder*) Pointer, nFile);
    }
    else if(nAction == ACTION_OPEN_VIEWER){
        OpenViewer(Model, cItem, lParam);
    }
    else if(nAction == ACTION_OPEN_EDITOR){
        OpenEditorDlg(Model, cItem, lParam, nFile);
    }
}

INT_PTR CALLBACK AboutProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch(message){
        case WM_INITDIALOG:
        {
            std::string sText;
            sText = "MDLedit version 1.0\nDeveloped by bead-v.";
            sText += "\n\nThis application was made with the knowledge from other MDL-related tools and the discoveries talked about on deadlystream.com forums. ";
            sText += "A big thank you goes out to all the people who contributed to this knowledge, including:";
            sText += "\nCChargin, Magnusll, JdNoa, ndix UR, VarsityPuppet, FairStrides, DarthSapiens";
            sText += "\n\nI would also like to thank the kind people who were very helpful in answering the bunch of questions I had while making this application:";
            sText += "\nDarthParametric, JCarter42, Quanon, FairStrides";
            sText += "\n\nA very special thanks goes to ndix UR, both for sharing his very complete knowledge of the format and his advice and support while making this application.";
            SetWindowText(GetDlgItem(hwnd, DLG_ID_STATIC), sText.c_str());
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
