#include "MDL.h"
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
            std::cout<<"Registering Window Class "<<WindowClass.lpszClassName<<" failed!\n";
            return false;
        }
        std::cout<<"Class "<<WindowClass.lpszClassName<<" registered!\n";
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

void OpenViewer(MDL & Mdl, std::vector<std::string>cItem, LPARAM lParam){
    std::stringstream sName;
    std::stringstream sPrint;

    if((cItem[0] == "")) return;

    /// Animation ///
    else if((cItem[1] == "Animations")){
        Animation * anim = (Animation*) lParam;

        sName<<"animation "<<anim->sName;
        Mdl.ConvertToAscii(CONVERT_ANIMATION, sPrint, (void*) lParam);
    }
    /// Anim Node ///
    else if((cItem[1] == "Animated Nodes") || ((cItem[3] == "Animated Nodes") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node * node = (Node*) lParam;

        sName<<"animated node "<<Mdl.GetFileData()->MH.Names[node->Head.nNodeNumber].sName;
        Mdl.ConvertToAscii(CONVERT_ANIMATION_NODE, sPrint, (void*) lParam);
        Mdl.ConvertToAscii(CONVERT_ENDNODE, sPrint, (void*) lParam);
    }
    /// Geo Node ///
    else if((cItem[1] == "Geometry") || ((cItem[3] == "Geometry") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node * node = (Node*) lParam;

        sName<<"node "<<Mdl.GetFileData()->MH.Names[node->Head.nNodeNumber].sName;
        sPrint<<"";
        //sPrint = "";
        Mdl.ConvertToAscii(CONVERT_HEADER, sPrint, (void*) lParam);
        if(node->Head.nType & NODE_HAS_AABB){
            Mdl.ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            Mdl.ConvertToAscii(CONVERT_AABB, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_SABER){
            Mdl.ConvertToAscii(CONVERT_SABER, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_DANGLY){
            Mdl.ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            Mdl.ConvertToAscii(CONVERT_DANGLY, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_SKIN){
            Mdl.ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            Mdl.ConvertToAscii(CONVERT_SKIN, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_MESH){
            Mdl.ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_EMITTER){
            Mdl.ConvertToAscii(CONVERT_EMITTER, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_LIGHT){
            Mdl.ConvertToAscii(CONVERT_LIGHT, sPrint, (void*) lParam);
        }
        Mdl.ConvertToAscii(CONVERT_ENDNODE, sPrint, (void*) lParam);
    }
    /// Controller ///
    else if((cItem[1] == "Controllers")){
        Controller * ctrl = (Controller*) lParam;

        std::string sLocation;
        if(ctrl->nAnimation == -1){
            sLocation = "geometry";
        }
        else{
            sLocation = "animation '" + std::string(Mdl.GetFileData()->MH.Animations[ctrl->nAnimation].sName.c_str()) + "'";
        }
        std::string sController = ReturnControllerName(ctrl->nControllerType, Mdl.GetNodeByNameIndex(ctrl->nNodeNumber).Head.nType);
        if(ctrl->nColumnCount & 16) sController+="bezierkey";
        else if(ctrl->nAnimation != -1) sController+="key";
        sName<<sLocation<<" > node '"<<Mdl.GetFileData()->MH.Names[ctrl->nNodeNumber].sName<<"' > controller '"<<sController<<"'";
        //sName<<"controller '"<<sController<<"' in node '"<<Mdl.GetFileData()->MH.Names[ctrl->nNodeNumber].sName<<"' in "<<sLocation;
        if(!(cItem[3] == "Geometry")){
            Mdl.ConvertToAscii(CONVERT_CONTROLLER_KEYED, sPrint, (void*) lParam);
        }
        else{
            Mdl.ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sPrint, (void*) lParam);
        }
    }
    else return;

    //Fix newlines
    std::string line;
    std::string text;
    while(std::getline(sPrint, line)){
        line += "\r\n";
        text += line;
    }

    //Create window
    DialogWindow ctrldata;
    if(!ctrldata.Run()){
        std::cout<<"DialogWindow creation failed!\n";
    }
    SetWindowText(ctrldata.hMe, sName.str().c_str());
    SetWindowText(GetDlgItem(ctrldata.hMe, IDDB_EDIT), text.c_str());
}
