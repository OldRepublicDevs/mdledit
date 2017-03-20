#include "dialog.h"

char DialogWindow::cClassName[] = "mdleditdialog";
LRESULT CALLBACK DialogWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool DialogWindow::bRegistered = false;

DialogWindow::DialogWindow(){
    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = GetModuleHandle(NULL); // Instance of the application
    WindowClass.lpfnWndProc = DialogWindowProc; // Pointer to callback procedure

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
}

bool DialogWindow::Run(){
    if(!bRegistered){
        if(!RegisterClassEx(&WindowClass)){
            std::cout<<string_format("Registering Window Class %s failed!\n", WindowClass.lpszClassName);
            return false;
        }
        std::cout<<string_format("Class %s registered!\n", WindowClass.lpszClassName);
        bRegistered = true;
    }
    //HMENU HAS to be NULL!!!!! Otherwise the function fails to create the window!
    hMe = CreateWindowEx(NULL, WindowClass.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         CW_USEDEFAULT, CW_USEDEFAULT, 600, 300,
                         HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
    if(!hMe) return false;
    ShowWindow(hMe, true);
    return true;
}

LRESULT CALLBACK DialogWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HWND hEdit = GetDlgItem(hwnd, IDDB_EDIT);

    /* handle the messages */
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
            GetClientRect(hwnd, &rcClient);
            hEdit = CreateWindowEx(NULL, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL,
                           rcClient.left+3, rcClient.top, rcClient.right-3, rcClient.bottom,
                           hwnd, (HMENU) IDDB_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM) hFont1, MAKELPARAM(TRUE, 0));
        }
        break;
        case WM_SIZE:
        {
            SetWindowPos(hEdit, NULL, rcClient.left+3, rcClient.top, rcClient.right-3, rcClient.bottom, NULL);
            //InvalidateRect(hwnd, &rcClient, false);
        }
        break;
        case WM_DESTROY:
            SetWindowText(hEdit, "");
            DestroyWindow(hwnd);
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
