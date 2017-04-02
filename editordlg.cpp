#include "editordlg.h"
#include "MDL.h"

char EditorDlgWindow::cClassName[] = "mdleditordlg";
LRESULT CALLBACK EditorDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool EditorDlgWindow::bRegistered = false;

EditorDlgWindow::EditorDlgWindow(){
    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = GetModuleHandle(NULL); // Instance of the application
    WindowClass.lpfnWndProc = EditorDlgWindowProc; // Pointer to callback procedure

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
    WindowClass.lpszMenuName = MAKEINTRESOURCE(IDM_EDITOR_DLG); // Menu Resource

    // #7 Other
    WindowClass.cbClsExtra = 0; // Extra bytes to allocate following the wndclassex structure
    WindowClass.cbWndExtra = 0; // Extra bytes to allocate following an instance of the structure
}

bool EditorDlgWindow::Run(){
    if(!bRegistered){
        if(!RegisterClassEx(&WindowClass)){
            std::cout<<string_format("Registering Window Class %s failed!\n", WindowClass.lpszClassName);
            return false;
        }
        std::cout<<string_format("Class %s registered!\n", WindowClass.lpszClassName);
        bRegistered = true;
    }
    //HMENU *has* to be NULL!!!!! Otherwise the function fails to create the window!
    hMe = CreateWindowEx(NULL, WindowClass.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         CW_USEDEFAULT, CW_USEDEFAULT, 600, 300,
                         HWND_DESKTOP, NULL, GetModuleHandle(NULL), this);
    if(!hMe) return false;
    ShowWindow(hMe, true);
    return true;
}

std::vector<EditorDlgWindow> EditDlgs;

LRESULT CALLBACK EditorDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HWND hEdit = GetDlgItem(hwnd, IDDB_EDIT);
    EditorDlgWindow* editdlg = nullptr;
    if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) editdlg = (EditorDlgWindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* handle the messages */
    switch(message){
        case WM_CREATE:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*) lParam)->lpCreateParams);
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
        case WM_COMMAND:
        {
            int nNotification = HIWORD(wParam);
            int nID = LOWORD(wParam);
            HWND hControl = (HWND) lParam;
            switch(nID){
                case IDDB_SAVE:
                {
                    if(editdlg == nullptr) Error("Internal error. Cannot get window data.");
                    else{
                        int nTextLength = GetWindowTextLength(hEdit);
                        std::vector<char> & sBuffer = editdlg->sBuffer;
                        sBuffer.resize(nTextLength, 0);
                        if(GetWindowText(hEdit, &sBuffer.front(), sBuffer.size()) != 0){
                            ///Passed all the error checks, time to get to business
                            if(editdlg->SaveData()){
                                SendMessage(hFrame, WM_COMMAND, MAKEWPARAM(IDD_EDITOR_DLG, IDP_DISPLAY_UPDATE), (LPARAM) hwnd);
                                DestroyWindow(hwnd);
                            }
                            SendMessage(hFrame, WM_COMMAND, MAKEWPARAM(IDD_EDITOR_DLG, IDP_DISPLAY_UPDATE), (LPARAM) hwnd);
                        }
                        else Error("An unknown error occurred! Could not save data!");
                    }
                }
                break;
            }
        }
        break;
        case WM_SIZE:
        {
            SetWindowPos(hEdit, NULL, rcClient.left+3, rcClient.top, rcClient.right-3, rcClient.bottom, NULL);
        }
        break;
        case WM_DESTROY:
            SetWindowText(hEdit, "");
            for(int i = 0; i < EditDlgs.size(); i++){
                if(&EditDlgs.at(i) == editdlg) EditDlgs.erase(EditDlgs.begin() + i);
            }
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

void OpenEditorDlg(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam){
    bool bVertex = false;
    if(cItem[0].length() > 6){
        if(cItem[0].substr(0, 6) == "Vertex") bVertex = true;
    }

    EditDlgs.reserve(10);
    if(EditDlgs.size() >= 10) return; ///Don't open a new window if we have 10 already

    std::stringstream sName;
    std::stringstream sPrint;

    sPrint.precision(7);
    sPrint<<std::fixed;

    if((cItem[0] == "")) return;

    /// Saber Vertex ///
    else if(cItem[1] == "Saber Data" && bVertex){
        SaberDataStruct * saberdata = (SaberDataStruct*) lParam;
        sName<<"Editing Saber Data for "<<cItem[0];

        sPrint<<"vert_x "<< saberdata->vVertex.fX << "\r\n";
        sPrint<<"vert_y "<< saberdata->vVertex.fY << "\r\n";
        sPrint<<"vert_z "<< saberdata->vVertex.fZ << "\r\n";
        sPrint << "\r\n";
        sPrint<<"UV_x "<< saberdata->vUV.fX << "\r\n";
        sPrint<<"UV_y "<< saberdata->vUV.fY << "\r\n";
        sPrint << "\r\n";
        sPrint<<"normal_x "<< saberdata->vNormal.fX << "\r\n";
        sPrint<<"normal_y "<< saberdata->vNormal.fY << "\r\n";
        sPrint<<"normal_z "<< saberdata->vNormal.fZ << "\r\n";

        EditDlgs.push_back(EditorDlgWindow());
        EditorDlgWindow & editdlg = EditDlgs.back();
        if(!editdlg.Run()){
            std::cout<<"EditorDlgWindow creation failed!\n";
        }
        else{
            editdlg.SetData(EDITOR_DLG_SABER_DATA, (LPVOID) lParam, Mdl);
            SetWindowText(editdlg.hMe, sName.str().c_str());
            SetWindowText(GetDlgItem(editdlg.hMe, IDDB_EDIT), sPrint.str().c_str());
        }
    }
    /// Mesh Vertex ///
    else if(cItem[1] == "Vertices" && bVertex){
        Vertex * vert = (Vertex*) lParam;
        sName<<"Editing "<<cItem[0];

        sPrint<<"vert_x "<< vert->fX << "\r\n";
        sPrint<<"vert_y "<< vert->fY << "\r\n";
        sPrint<<"vert_z "<< vert->fZ << "\r\n";

        EditDlgs.push_back(EditorDlgWindow());
        EditorDlgWindow & editdlg = EditDlgs.back();
        if(!editdlg.Run()){
            std::cout<<"EditorDlgWindow creation failed!\n";
        }
        else{
            editdlg.SetData(EDITOR_DLG_VERTEX, (LPVOID) lParam, Mdl);
            SetWindowText(editdlg.hMe, sName.str().c_str());
            SetWindowText(GetDlgItem(editdlg.hMe, IDDB_EDIT), sPrint.str().c_str());
        }
    }
    else if(cItem[0] == "Trimesh Flags"){
        Node * node = (Node*) lParam;
        sName<<"Editing "<<cItem[0];

        sPrint<<"lightmapped "<< (int) node->Mesh.nHasLightmap << "\r\n";
        sPrint<<"rotatetex "<< (int) node->Mesh.nRotateTexture << "\r\n";
        sPrint<<"backgroundgeometry "<< (int) node->Mesh.nBackgroundGeometry << "\r\n";
        sPrint<<"shadow "<< (int) node->Mesh.nShadow << "\r\n";
        sPrint<<"beaming "<< (int) node->Mesh.nBeaming << "\r\n";
        sPrint<<"render "<< (int) node->Mesh.nRender << "\r\n";
        if(Mdl.bK2) sPrint<<"dirtenabled "<< (int) node->Mesh.nDirtEnabled << "\r\n";
        if(Mdl.bK2) sPrint<<"hideinholograms "<< (int) node->Mesh.nHideInHolograms << "\r\n";

        EditDlgs.push_back(EditorDlgWindow());
        EditorDlgWindow & editdlg = EditDlgs.back();
        if(!editdlg.Run()){
            std::cout<<"EditorDlgWindow creation failed!\n";
        }
        else{
            editdlg.SetData(EDITOR_DLG_TRIMESH_FLAGS, (LPVOID) lParam, Mdl);
            SetWindowText(editdlg.hMe, sName.str().c_str());
            SetWindowText(GetDlgItem(editdlg.hMe, IDDB_EDIT), sPrint.str().c_str());
        }
    }
    else return;
}

bool EditorDlgWindow::SaveData(){
    MDL & Mdl = *MdlPtr;
    std::string sID;
    std::string sGet;
    std::stringstream ssCompare;
    ssCompare.precision(7);
    ssCompare<<std::fixed;

    nPosition = 0;
    bool bError = false;
    double fConvert = 0.0;
    int nConvert = 0;
    switch(nDataType){
        case EDITOR_DLG_TRIMESH_FLAGS:
        {
            Node * node = (Node*) lpData;
            while(nPosition < sBuffer.size()){
                if(EmptyRow()) SkipLine();
                else{
                    bool bFound = ReadUntilText(sID);
                    if(!bFound) SkipLine();
                    else if(sID == "lightmapped"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nHasLightmap = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 0);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "rotatetex"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nRotateTexture = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 1);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "backgroundgeometry"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nBackgroundGeometry = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 2);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "shadow"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nShadow = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 3);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "beaming"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nBeaming = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 4);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "render"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nRender = nConvert;
                            Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 5);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "dirtenabled"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nDirtEnabled = nConvert;
                            if(Mdl.bK2) Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 6);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "hideinholograms"){
                        if(ReadInt(nConvert)){
                            node->Mesh.nHideInHolograms = nConvert;
                            if(Mdl.bK2) Mdl.WriteByteToPlaceholder(nConvert, node->nOffset + 400 + 12);
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else{
                        SkipLine();
                    }
                }
            }
        }
        break;
        case EDITOR_DLG_SABER_DATA:
        {
            SaberDataStruct * saberdata = (SaberDataStruct*) lpData;
            while(nPosition < sBuffer.size()){
                ssCompare.str(std::string());
                if(EmptyRow()) SkipLine();
                else{
                    bool bFound = ReadUntilText(sID, false);
                    if(!bFound) SkipLine();
                    else if(sID == "vert_x"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vVertex.fX;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vVertex.fX = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetVertex);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "vert_y"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vVertex.fY;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vVertex.fY = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetVertex + 4);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "vert_z"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vVertex.fZ;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vVertex.fZ = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetVertex + 8);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "normal_x"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vNormal.fX;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vNormal.fX = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetNormal);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "normal_y"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vNormal.fY;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vNormal.fY = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetNormal + 4);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "normal_z"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vNormal.fZ;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vNormal.fZ = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetNormal + 8);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "UV_x"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vUV.fX;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vUV.fX = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetUV);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "UV_y"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << saberdata->vUV.fY;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                saberdata->vUV.fY = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, saberdata->nOffsetUV + 4);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else{
                        SkipLine();
                    }
                }
            }
        }
        break;
        case EDITOR_DLG_VERTEX:
        {
            Vertex * vert = (Vertex*) lpData;
            while(nPosition < sBuffer.size()){
                ssCompare.str(std::string());
                if(EmptyRow()) SkipLine();
                else{
                    bool bFound = ReadUntilText(sID);
                    if(!bFound) SkipLine();
                    else if(sID == "vert_x"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << vert->fX;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                vert->fX = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, vert->nOffset);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "vert_y"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << vert->fY;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                vert->fY = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, vert->nOffset + 4);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else if(sID == "vert_z"){
                        if(ReadFloat(fConvert, sGet)){
                            ssCompare << vert->fZ;
                            if(ssCompare.str() != sGet){
                                std::cout<<sID<<" "<<sGet<<" does not equal "<<ssCompare.str()<<"\n";
                                vert->fZ = fConvert;
                                ByteBlock4.f = fConvert;
                                Mdl.WriteUintToPlaceholder(ByteBlock4.ui, vert->nOffset + 8);
                            }
                            else std::cout<<sID<<" "<<sGet<<" equals "<<ssCompare.str()<<"\n";
                        }
                        else bError = true;
                        SkipLine();
                    }
                    else{
                        SkipLine();
                    }
                }
            }
        }
        break;
    }
    if(bError) Error("An error occurred while saving the data. At least one value was not saved properly!");
    else return true;
    return false;
}
/*
bool EditorDlgWindow::ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"EditorDlgWindow::ReadFloat(): Reading: "<<sCheck<<". ";

    try{
        fNew = std::stof(sCheck, (size_t*) NULL);
        sGetFloat = sCheck;
    }
    catch(std::invalid_argument){
        std::cout<<"EditorDlgWindow::ReadFloat(): There was an error converting the string: "<<sCheck<<". Printing 0.0. \n";
        fNew = 0.0;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<fNew<<".\n";
    return true;
}

bool EditorDlgWindow::ReadInt(int & nNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"EditorDlgWindow::ReadInt(): Reading: "<<sCheck<<". ";

    try{
        nNew = stoi(sCheck,(size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout<<"EditorDlgWindow::ReadInt(): There was an error converting the string: "<<sCheck<<". Printing 0xFFFFFFFF. \n";
        nNew = 0xFFFFFFFF;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<nNew<<".\n";
    return true;
}

void EditorDlgWindow::SkipLine(){
    if(sBuffer[nPosition] == 0x0A){
        nPosition+=1;
        return;
    }
    else if(sBuffer[nPosition] == 0x0D && sBuffer[nPosition+1] == 0x0A){
        nPosition+=2;
        return;
    }
    bool bStop = false;
    while(nPosition + 1 < sBuffer.length() && !bStop){
        if(sBuffer[nPosition] != 0x0D && sBuffer[nPosition+1] != 0x0A) nPosition++;
        else bStop = true;
    }
    nPosition+=2;
}

bool EditorDlgWindow::EmptyRow(){
    int n = nPosition; //Do not use the iterator
    while(sBuffer[n] != 0x0D &&
        sBuffer[n+1] != 0x0A &&
                n+1 < sBuffer.length())
    {
        if(sBuffer[n] != 0x20 || (sBuffer[n+1] != 0x20 && sBuffer[n+1] != 0x0D)) return false;
        n++;
    }
    return true;
}

bool EditorDlgWindow::ReadUntilText(std::string & sHandle, bool bToLowercase, bool bStrictNoNewLine){
    sHandle = ""; //Make sure the handle is cleared
    while(nPosition < sBuffer.length()){
        //std::cout<<"Looping in ReadUntilText main while(), nPosition="<<nPosition<<".\n";
        if(sBuffer[nPosition] == 0x20){
            //Skip space
            nPosition++;
            if(nPosition >= sBuffer.length()) return false;
        }
        else if(sBuffer[nPosition] == 0x0A){
            if(bStrictNoNewLine) return false;
            nPosition++;
            if(nPosition >= sBuffer.length()) return false;
        }
        else if(sBuffer[nPosition] == '#'){
            //Return because end of line and nothing was found
            return false;
        }
        else{
            if(nPosition + 1 < sBuffer.length()){
                if(sBuffer[nPosition] == 0x0D &&
                 sBuffer[nPosition+1] == 0x0A)
                {
                    //Return because end of line and nothing was found
                    return false;
                }
            }
            //Now it gets interesting - we may actually have relevant text now
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
            do{
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
                sHandle.push_back(sBuffer[nPosition]);
                nPosition++;
            }
            while(sBuffer[nPosition] != 0x20 &&
                  sBuffer[nPosition] != '#' &&
                  sBuffer[nPosition] != 0x0D &&
                  sBuffer[nPosition] != 0x0A &&
                  nPosition < sBuffer.length());

            //Report
            //if(sHandle != "") std::cout<<"ReadUntilText() found the following string: "<<sHandle<<".\n";

            //convert to lowercase
            //if(bToLowercase) std::transform(sHandle.begin(), sHandle.end(), sHandle.begin(), ::tolower);

            //Go back and tell them you've found something
            return true;
        }
    }
    //Go back and tell them you're done
    return false;
}
*/
