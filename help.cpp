#include "general.h"
#include "MDL.h"
#include <Shlwapi.h>
#include <fstream>
#include <regex>
#include <algorithm>
#include <memory> //for std::unique_ptr

class HelpDlgWindow{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;

  public:
    HWND hMe;
    HelpDlgWindow();
    bool Run();
    friend LRESULT CALLBACK HelpDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

char HelpDlgWindow::cClassName[] = "mdlhelpdlg";
bool HelpDlgWindow::bRegistered = false;
LRESULT CALLBACK HelpDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
std::unique_ptr<HelpDlgWindow> HelpDlgWindowInstance;

HelpDlgWindow::HelpDlgWindow(){
    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = GetModuleHandle(NULL); // Instance of the application
    WindowClass.lpfnWndProc = HelpDlgWindowProc; // Pointer to callback procedure

    // #2 Class styles
    WindowClass.style = CS_DBLCLKS; // Class styles

    // #3 Background
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //(HBRUSH) (COLOR_WINDOW); // Background brush

    // #4 Cursor
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW); // Class cursor

    // #5 Icon
    WindowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DLG_ICON)); //NULL; // Class Icon
    WindowClass.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DLG_ICON)); // Small icon for this class

    // #6 Menu
    WindowClass.lpszMenuName = NULL; // Menu Resource

    // #7 Other
    WindowClass.cbClsExtra = 0; // Extra bytes to allocate following the wndclassex structure
    WindowClass.cbWndExtra = 0; // Extra bytes to allocate following an instance of the structure
}

bool HelpDlgWindow::Run(){
    if(!bRegistered){
        if(!RegisterClassEx(&WindowClass)){
            std::cout << "Registering Window Class " << WindowClass.lpszClassName << " failed!\n";
            return false;
        }
        std::cout << "Class " << WindowClass.lpszClassName << " registered!\n";
        bRegistered = true;
    }
    //HMENU *has* to be NULL!!!!! Otherwise the function fails to create the window!
    hMe = CreateWindowEx(NULL, WindowClass.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
                         HWND_DESKTOP, NULL, GetModuleHandle(NULL), this);
    if(!hMe) return false;
    ShowWindow(hMe, true);
    return true;
}

std::string GetHelpData(const std::string & sTab);

LRESULT CALLBACK HelpDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HWND hTabs = GetDlgItem(hwnd, IDC_TABS);
    HWND hEdit = GetDlgItem(hwnd, IDDB_EDIT);
    HelpDlgWindow* helpdlg = nullptr;
    if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) helpdlg = (HelpDlgWindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* handle the messages */
    switch(message){
        case WM_CREATE:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*) lParam)->lpCreateParams);
            helpdlg = (HelpDlgWindow*) ((CREATESTRUCT*) lParam)->lpCreateParams;

            std::string sMonospace = "Consolas";
            if(!Font_IsInstalled(sMonospace)){
                std::cout << "Consolas font not installed! Switching to Courier New...\n";
                sMonospace = "Courier New";
                if(!Font_IsInstalled(sMonospace)){
                    std::cout << "Courier New font not installed! No further alternatives!\n";
                }
            }

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
                sMonospace.c_str() 	// pointer to typeface name string
            );
            HFONT hFont4 = CreateFont(
                12,  //Height
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
                "MS Shell Dlg" //"Segoe UI" //"MS Shell Dlg" // 	// pointer to typeface name string
            );

            GetClientRect(hwnd, &rcClient);
            hTabs = CreateWindowEx(NULL, WC_TABCONTROL, "", WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_MULTILINE,
                                   rcClient.left, rcClient.top, rcClient.right + 2, rcClient.bottom + 1,
                                   hwnd, (HMENU) IDC_TABS, GetModuleHandle(NULL), NULL);
            SendMessage(hTabs, WM_SETFONT, (WPARAM) hFont4, MAKELPARAM(TRUE, 0));
            hEdit= CreateWindowEx(NULL, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                                   rcClient.left + 5, rcClient.top + 18 * TabCtrl_GetRowCount(hTabs) + 6, rcClient.right - 8, rcClient.bottom - 18 * TabCtrl_GetRowCount(hTabs) - 9,
                                   hwnd, (HMENU) IDDB_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM) hFont4, MAKELPARAM(TRUE, 0));

            TabCtrl_AppendTab(hTabs, "Basics");
            TabCtrl_AppendTab(hTabs, "File Types");
            TabCtrl_AppendTab(hTabs, "Model Loading");
            TabCtrl_AppendTab(hTabs, "Model Saving");
            TabCtrl_AppendTab(hTabs, "Batch Processing");
            TabCtrl_AppendTab(hTabs, "Model Editing");
            TabCtrl_AppendTab(hTabs, "Hex View");

            SetWindowText(hwnd, "Help");
            SetWindowText(hEdit, GetHelpData(TabCtrl_GetCurSelName(hTabs)).c_str());
        }
        break;
        case WM_NOTIFY:
        {
            NMHDR * nmhdr = (NMHDR *) lParam;
            HWND hControl = nmhdr->hwndFrom;
            int nID = nmhdr->idFrom;
            int nNotification = nmhdr->code;
            switch(nID){
                case IDC_TABS:
                {
                    if(nNotification == TCN_SELCHANGE){
                        SetWindowText(hEdit, GetHelpData(TabCtrl_GetCurSelName(hTabs)).c_str());
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
                case IDDB_SAVE:
                {
                }
                break;
            }
        }
        break;
        case WM_SIZE:
        {
            SetWindowPos(hTabs, NULL, rcClient.left, rcClient.top, rcClient.right + 2, rcClient.bottom + 1, NULL);
            SetWindowPos(hEdit, NULL, rcClient.left + 5, rcClient.top + 18 * TabCtrl_GetRowCount(hTabs) + 6, rcClient.right - 8, rcClient.bottom - 18 * TabCtrl_GetRowCount(hTabs) - 9, NULL);
        }
        break;
        case WM_CTLCOLORSTATIC:
            {
                HDC hdcEdit = (HDC) wParam;
                HBRUSH hBackground = CreateSolidBrush(RGB(255, 255, 255));
                SetTextColor(hdcEdit, RGB(0, 0, 0));
                SetBkColor(hdcEdit, RGB(255, 255, 255));
                return (INT_PTR) hBackground;
            }
        break;
        case WM_DESTROY:
            HelpDlgWindowInstance.reset();
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        default:
        {
            return DefWindowProc (hwnd, message, wParam, lParam);
        }
    }
    return 0;
}

void OpenHelpDlg(){
    if(HelpDlgWindowInstance){
        HelpDlgWindow & helpdlg = *HelpDlgWindowInstance;
        SetWindowPos(helpdlg.hMe, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        return;
    }
    HelpDlgWindowInstance.reset(new HelpDlgWindow);
    HelpDlgWindow & helpdlg = *HelpDlgWindowInstance;
    if(!helpdlg.Run()){
        std::cout << "HelpDlgWindow creation failed!\n";
    }
}

#define nl "\r\n"
std::string GetHelpData(const std::string & sTab){
    std::stringstream ssHelp;
    //char nl [] = "\r\n"; /// New Line
    char sl [] = ""; /// Same Line
    if(sTab == "Basics"){
        ssHelp <<
           "MDLedit allows you to compile and decompile models for the games:"
        nl " - Star Wars Knights of the Old Republic"
        nl " - Star Wars Knights of the Old Republic II - the Sith Lords"
        nl "for both the PC and XBOX version of the games, as well as to directly edit them to a certain extent."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "File Types"){
        ssHelp <<
           "MDLedit handles the following types of binary files:"
        nl " * model files (.mdl): This is the file that contains most of the information about the model."
        nl " * model extension files (.mdx): This file contains information about the vertices of all meshes in the model."
        nl " * area walkmesh files (.wok): This file contains information about the area walkmesh."
        nl " * placeable walkmesh files (.pwk): This file contains information about the placeable walkmesh."
        nl " * door walkmesh files (.dwk): This file contains information about the door walkmesh. These files always come in triplets, because the "
           "engine supports or once supported swinging doors. The three .dwks represent the possible states: closed, open to one side, open to the other side."
        nl ""
        nl "MDLedit handles the following types of ascii files:"
        nl " * model files (.mdl or .mdl.ascii): This is the file that contains most of the information about the model, including vertex information."
        nl " * placeable walkmesh files (.pwk or .pwk.ascii): This file contains information about the placeable walkmesh."
        nl " * door walkmesh files (.dwk or .dwk.ascii): This file contains information about the door walkmesh. Unlike the binary file, this file contains "
           "the information about all three states, so there is only one ascii .dwk that goes with its .mdl."
        nl " * area walkmesh files (.wok or .wok.ascii): This file contains information about the area walkmesh. MDLedit can output this file, but it does "
           "not read it because the information it contains is already contained in the .mdl."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "Model Loading"){
        ssHelp <<
           "To load up a model, go to File -> Load and select your .mdl or .mdl.ascii file. MDLedit will automatically determine whether it is loading "
           "a binary or ascii .mdl file."
        nl ""
        nl "If the model file is in ascii format, then MDLedit will also search the same directory for files with the same name and a .pwk(.ascii) or "
           ".dwk(.ascii) extension. It will only load them if they are in the right format."
        nl "Note: MDLedit will not look for a .wok(.ascii) file. MDLedit assumes that any model with a binary .wok file also has an aabb node in the "
           ".mdl. The two meshes are always compatible, so MDLedit will use the aabb node data to construct the binary .wok file; hence, it does not "
           "need to load an ascii .wok(.ascii) file."
        nl ""
        nl "Ascii files are not specified for game (K1 vs K2) or platform (PC vs XBOX), so when they are loaded, the binary code is written according "
           "to the current game and platform setting. The binary code will be updated automatically when these settings are changed."
        nl ""
        nl "If the model file is in binary format, then MDLedit will also search the same directory for files with the same name and a .mdx, .pwk and "
           ".wok extension, as well as three .dwk files, each named the same as the .mdl file plus a digit (0, 1 or 2) representing the three states."
        nl ""
        nl "Binary files are specific to a game and platform, the settings will therefore change to reflect that."
        nl ""
        nl "Once all the files are loaded the model is read and processed. MDLedit will show a progress bar for the duration of the processing. When "
           "loading binary files, the processing only consists of the smoothing group calculation algorithm, but this algorithm may end up taking a LOT "
           "of time. Therefore, there is an option under Edit -> Settings that will let you turn this algorithm off. Of course, if you turn it off, the "
           "ascii file will not contain any smoothing information."
        nl "The processing is much more extensive in the case of .ascii files, but will never take a really long time."
        nl ""
        nl "After the model has been processed the data is loaded into the Tree View and the Hex View. You may now proceed to explore the model, edit it, "
           "or export it again."
        nl ""
        nl "If an error occurs during reading or processing, MDLedit will either let you know immediately (if it is a potentially fatal error) or just "
           "write it into its report, which you can view by clicking View -> Show Report. If you turn on automatic report writing under Edit -> Settings "
           "then the report will automatically be written as MODELNAME_report.txt to the model's directory."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "Model Saving"){
        ssHelp <<
           "To save a loaded model, go to File -> Save and select whether you want to export an ascii or binary file. Next, the standard saving dialog "
           "box will pop up. If you enter the name of an existing file, MDLedit will prompt you and ask whether you want to overwrite. It will only do "
           "this for the .mdl file itself! It will automatically overwrite .mdx, .pwk, .dwk, etc. files whose names happen to match."
        nl ""
        nl "There are several options related to saving ascii files under Edit -> Settings."
        nl "One is for whether you want to use the extra .ascii extension by default when exporting ascii files."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "Batch Processing"){
        ssHelp <<
           "Under File -> Batch it is possible to convert several files at once to either binary or ascii. With this option, all selected files are "
           "loaded whether they are binary or ascii, then they are saved according to the current settings as FILENAME-mdledit.mdl(.ascii). "
           "It also possible to convert only a single file, which is the quickest way of simply converting a file."
        nl ""
        nl "There is a shortcoming to this method - it is impossible for the program to determine whether it should make a head link or not. "
           "Therefore, when converting heads from ascii to binary, you need to load them up, manually enable the head link and then save them as binary."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "Model Editing"){
        ssHelp <<
           "It is possible to edit the model to some extent with MDLedit."
        nl ""
        nl "Under Edit -> Textures, MDLedit will list all the textures in the current model. By clicking on a selected texture name, you will be "
           "able to edit it and the change will be applied to all occurrences of that texture name. There is also a checkbox next to the texture "
           "names, which controls whether those textures are bumpmappable."
        nl ""
        nl "On some nodes in the Tree View, right-clicking will show the option 'Edit values'. When clicked, a new window will appear listing the "
           "values in an ascii-like format. The values that can be edited are generally the ones that cannot break the structure of the file itself, "
           "though they may possibly still crash the game if you enter some impossible values. It is also not possible to add data to the file this "
           "way, only change it. "
           "After you're done editing the values, click File -> Save and the new values will be saved. The binary code is changed only where new values "
           "are applied, so it is possible to use this method to only change specific values in the file and nothing else."
        nl ""
        nl "Currently it is not possible to edit all the values that could possibly be edited, though a lot of them will be added in the future. "
           "While it is possible to edit most controller values, it is currently not possible to edit compressed quaternions."
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl ""
        nl "";
    }
    else if(sTab == "Hex View"){
        ssHelp <<
           "The Hex View allows you to view the binary code of the model's files."
        nl ""
        nl "The binary code is color-coded per data type, here is the legend:"
        nl "1) Yellow - a counter, by definition an unsigned integer, size varies"
        nl "2) Green - an IEEE-754 4-byte floating point (float)"
        nl "3) Dark Gray - a null-terminated string of characters (string)"
        nl "4) Light Blue - a signed or unsigned 4-byte integer (int32)"
        nl "5) Purple - a signed or unsigned 2-byte integer (int16)"
        nl "6) Dark Blue - an offset/pointer, always a 4-byte unsigned integer (uint32)"
        nl "7) Brown - a signed or unsigned 1-byte integer (byte)"
        nl "8) Light Gray - invariant data across all models that either has runtime uses or is just padding"
        nl "9) Orange - function pointers, which come in pairs, are unsigned 4-byte integers (uint32)"
        nl "10) Red - data that represents something that we do not yet understand"
        nl "11) Light Olive-Gray - meaningless data that is mostly just padding or maybe space reserved for runtime uses"
        nl "12) Black - unparsed data"
        nl ""
        nl "By double-clicking the offset counters, you can toggle between decimal and hexadecimal display of the counters, but also the offsets that are "
           "displayed in the status bar."
        nl ""
        nl "Double-clicking the binary code will jump you to the corresponding node in the Tree View."
        nl ""
        nl "Highlighting bytes will show their int, uint and float values in the top right, where such values are representable by the highlighted bytes."
        nl ""
        nl ""
        nl ""
        nl "";
    }
    return ssHelp.str();
}
