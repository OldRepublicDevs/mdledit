#include "frame.h"
#include <windowsx.h>

INT_PTR CALLBACK TexturesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    static bool bChange = false;
    static bool bReady = false;
    MDL * Mdl = nullptr;
    if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) Mdl = (MDL*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch(message){
        case WM_INITDIALOG:
        {
            bChange = false;
            bReady = false;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);
            Mdl = (MDL*) lParam;

            //Create ListViews
            HWND hList1 = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_EDITLABELS | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL,
                                         5, 25 + 5, 230, 180,
                                         hwnd, (HMENU) IDC_TEXTURE_LISTVIEW1, NULL, NULL);
            HWND hList2 = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_EDITLABELS | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL,
                                         240, 25 + 5, 230, 180,
                                         hwnd, (HMENU) IDC_TEXTURE_LISTVIEW2, NULL, NULL);
            HWND hList3 = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_EDITLABELS | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL,
                                         5, 25 + 190, 230, 60,
                                         hwnd, (HMENU) IDC_TEXTURE_LISTVIEW3, NULL, NULL);
            HWND hList4 = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_EDITLABELS | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL,
                                         240, 25 + 190, 230, 60,
                                         hwnd, (HMENU) IDC_TEXTURE_LISTVIEW4, NULL, NULL);
            ListView_SetExtendedListViewStyle(hList1, LVS_EX_DOUBLEBUFFER /*| LVS_EX_AUTOSIZECOLUMNS */ | LVS_EX_CHECKBOXES);
            ListView_SetExtendedListViewStyle(hList2, LVS_EX_DOUBLEBUFFER /*| LVS_EX_AUTOSIZECOLUMNS */ | LVS_EX_CHECKBOXES);
            ListView_SetExtendedListViewStyle(hList3, LVS_EX_DOUBLEBUFFER /*| LVS_EX_AUTOSIZECOLUMNS */ | LVS_EX_CHECKBOXES);
            ListView_SetExtendedListViewStyle(hList4, LVS_EX_DOUBLEBUFFER /*| LVS_EX_AUTOSIZECOLUMNS */ | LVS_EX_CHECKBOXES);
            LVCOLUMN col;
            col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
            /*col.fmt = LVCFMT_FIXED_WIDTH; */
            col.iSubItem = 0;
            col.cx = 120;
            ListView_InsertColumn(hList1, 0, &col);
            ListView_InsertColumn(hList2, 0, &col);
            ListView_InsertColumn(hList3, 0, &col);
            ListView_InsertColumn(hList4, 0, &col);

            //Fill ListViews
            if(Mdl != nullptr){
                //std::cout<<"Mdl is valid pointer\n";
                if(Mdl->GetFileData()){
                    //std::cout<<"Mdl has data\n";
                    FileHeader & Data = *Mdl->GetFileData();
                    int nCount1 = 0, nCount2 = 0, nCount3 = 0, nCount4 = 0;
                    LVITEM item;
                    item.mask = LVIF_TEXT | LVIF_STATE;
                    item.stateMask = 0;
                    item.iSubItem  = 0;
                    item.state     = 0;
                    LVFINDINFO find;
                    find.flags = LVFI_STRING;
                    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
                        //std::cout<<"Checking node\n";
                        Node & node = Data.MH.ArrayOfNodes.at(n);
                        if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_AABB)){
                            if(std::string(node.Mesh.cTexture1.c_str()) != "" && std::string(node.Mesh.cTexture1.c_str()) != "NULL"){
                                find.psz = &node.Mesh.cTexture1;
                                if(ListView_FindItem(hList1, -1, &find) == -1){
                                    item.pszText = &node.Mesh.cTexture1;
                                    nCount1 = ListView_InsertItem(hList1, &item);
                                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1) ListView_SetCheckState(hList1, nCount1, true);
                                    nCount1++;
                                }
                            }
                            if(node.Mesh.cTexture2.c_str() != std::string("") && node.Mesh.cTexture2.c_str() != std::string("NULL")){
                                find.psz = &node.Mesh.cTexture2;
                                if(ListView_FindItem(hList2, -1, &find) == -1){
                                    item.pszText = &node.Mesh.cTexture2;
                                    nCount2 = ListView_InsertItem(hList2, &item);
                                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2) ListView_SetCheckState(hList2, nCount2, true);
                                    nCount2++;
                                }
                            }
                            if(node.Mesh.cTexture3.c_str() != std::string("") && node.Mesh.cTexture3.c_str() != std::string("NULL")){
                                find.psz = &node.Mesh.cTexture3;
                                if(ListView_FindItem(hList3, -1, &find) == -1){
                                    item.pszText = &node.Mesh.cTexture3;
                                    nCount3 = ListView_InsertItem(hList3, &item);
                                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3) ListView_SetCheckState(hList3, nCount3, true);
                                    nCount3++;
                                }
                            }
                            if(node.Mesh.cTexture4.c_str() != std::string("") && node.Mesh.cTexture4.c_str() != std::string("NULL")){
                                find.psz = &node.Mesh.cTexture4;
                                if(ListView_FindItem(hList4, -1, &find) == -1){
                                    item.pszText = &node.Mesh.cTexture4;
                                    nCount4 = ListView_InsertItem(hList4, &item);
                                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4) ListView_SetCheckState(hList4, nCount4, true);
                                    nCount4++;
                                }
                            }
                        }
                    }
                    bReady = true;
                }
            }
        }
        break;
        case WM_NOTIFY:
        {
            NMHDR* hdr = (NMHDR*) lParam;
            int nNotification = hdr->code;
            int nID = hdr->idFrom;
            HWND hControl = hdr->hwndFrom;
            switch(nID){
                case IDC_TEXTURE_LISTVIEW1:
                case IDC_TEXTURE_LISTVIEW2:
                case IDC_TEXTURE_LISTVIEW3:
                case IDC_TEXTURE_LISTVIEW4:
                {
                    if(nNotification == LVN_ITEMCHANGED){
                        if(Mdl != nullptr && bReady){
                            if(Mdl->GetFileData()){
                                NMLISTVIEW * ia = (NMLISTVIEW*) lParam;
                                bool bChecked = ListView_GetCheckState(hControl, ia->iItem);
                                //std::cout<<"Model is good, bChecked="<<bChecked<<"\n";
                                FileHeader & Data = *Mdl->GetFileData();
                                std::string sOldTex;
                                sOldTex.resize(33);
                                ListView_GetItemText(hControl, ia->iItem, 0, &sOldTex, 33);
                                for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
                                    Node & node = Data.MH.ArrayOfNodes.at(n);
                                    if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_AABB)){
                                        if(nID == IDC_TEXTURE_LISTVIEW1 && std::string(sOldTex.c_str()) == std::string(node.Mesh.cTexture1.c_str()) && (!bChecked != !(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1))){
                                            //std::cout<<"Found difference. ("<<Data.MH.Names.at(node.Head.nNameIndex).sName<<")\n";
                                            bChange = true;
                                            node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap ^ MDX_FLAG_HAS_TANGENT1;
                                        }
                                        else if(nID == IDC_TEXTURE_LISTVIEW2 && std::string(sOldTex.c_str()) == std::string(node.Mesh.cTexture2.c_str()) && (!bChecked != !(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2))){
                                            bChange = true;
                                            node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap ^ MDX_FLAG_HAS_TANGENT2;
                                        }
                                        else if(nID == IDC_TEXTURE_LISTVIEW3 && std::string(sOldTex.c_str()) == std::string(node.Mesh.cTexture3.c_str()) && (!bChecked != !(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3))){
                                            bChange = true;
                                            node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap ^ MDX_FLAG_HAS_TANGENT3;
                                        }
                                        else if(nID == IDC_TEXTURE_LISTVIEW4 && std::string(sOldTex.c_str()) == std::string(node.Mesh.cTexture4.c_str()) && (!bChecked != !(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4))){
                                            bChange = true;
                                            node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap ^ MDX_FLAG_HAS_TANGENT4;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if(nNotification == LVN_BEGINLABELEDIT){
                        NMLVDISPINFO * info = (NMLVDISPINFO *) lParam;
                        HWND hEdit = ListView_GetEditControl(hControl);
                        if(nID == IDC_TEXTURE_LISTVIEW3 || nID == IDC_TEXTURE_LISTVIEW4) SendMessage(hEdit, EM_LIMITTEXT, (WPARAM) 12, NULL);
                        else SendMessage(hEdit, EM_LIMITTEXT, (WPARAM) 32, NULL);
                        return true; /// Allow the user to edit the label. Returning false would prevent it.
                    }
                    else if(nNotification == LVN_ENDLABELEDIT){
                        NMLVDISPINFO * info = (NMLVDISPINFO *) lParam;
                        HWND hEdit = ListView_GetEditControl(hControl);
                        if(info->item.pszText != nullptr && Mdl !=nullptr){
                            std::string sNewTex = info->item.pszText;
                            std::string sOldTex;
                            sOldTex.resize(33);
                            ListView_GetItemText(hControl, info->item.iItem, 0, &sOldTex, 33);

                            LVFINDINFO find;
                            find.flags = LVFI_STRING;
                            find.psz = &sNewTex;
                            bool bRemove = false;
                            bool bCancel = false;
                            if(ListView_FindItem(hControl, -1, &find) != -1){
                                if(MessageBox(hwnd, "This texture is already used by at least one mesh. The two textures will be merged in the list. Do you want to continue anyway?", "Warning", MB_ICONWARNING | MB_YESNO) == IDYES){
                                    bRemove = true;
                                }
                                else bCancel = true;
                            }
                            if(sNewTex != sOldTex && Mdl->GetFileData() && !bCancel){
                                FileHeader & Data = *Mdl->GetFileData();
                                if(sNewTex.size() > 16) MessageBox(hwnd, "Texture name is longer than 16 characters. This may or my not cause problems in the game.", "Warning", MB_ICONWARNING | MB_OK);
                                for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
                                    Node & node = Data.MH.ArrayOfNodes.at(n);
                                    if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_AABB)){
                                        if(nID == IDC_TEXTURE_LISTVIEW1 && std::string(node.Mesh.cTexture1.c_str()) == std::string(sOldTex.c_str())) Mdl->UpdateTexture(node, sNewTex, 1);
                                        else if(nID == IDC_TEXTURE_LISTVIEW2 && std::string(node.Mesh.cTexture2.c_str()) == std::string(sOldTex.c_str())) Mdl->UpdateTexture(node, sNewTex, 2);
                                        else if(nID == IDC_TEXTURE_LISTVIEW3 && std::string(node.Mesh.cTexture3.c_str()) == std::string(sOldTex.c_str())) Mdl->UpdateTexture(node, sNewTex, 3);
                                        else if(nID == IDC_TEXTURE_LISTVIEW4 && std::string(node.Mesh.cTexture4.c_str()) == std::string(sOldTex.c_str())) Mdl->UpdateTexture(node, sNewTex, 4);
                                    }
                                }
                                if(bRemove) ListView_DeleteItem(hControl, info->item.iItem);
                                else ListView_SetItemText(hControl, info->item.iItem, 0, &sNewTex);
                            }
                        }
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
            if(Mdl != nullptr){
                switch(nID){

                }
            }
        }
        break;
        case WM_CLOSE:
        {
            if(Mdl != nullptr){
                if(bChange && Mdl->GetFileData()){
                    //MessageBox(hwnd, "Bumpmapping has changed. The program will now reprocess the model to apply the changes.", "Note", MB_OK | MB_ICONINFORMATION);
                    EndDialog(hwnd, 2);
                    return TRUE;
                }
            }
            EndDialog(hwnd, 1);
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}


INT_PTR CALLBACK SettingsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    static bool bChange = false;
    switch(message){
        case WM_INITDIALOG:
        {
            bChange = false;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);
            MDL* Mdl = (MDL*) lParam;
            if(Mdl->bSmoothAngleWeighting) Button_SetCheck(GetDlgItem(hwnd, DLG_ID_ANGLE_WEIGHT), BST_CHECKED);
            if(Mdl->bSmoothAreaWeighting) Button_SetCheck(GetDlgItem(hwnd, DLG_ID_AREA_WEIGHT), BST_CHECKED);
            if(Mdl->bDetermineSmoothing) Button_SetCheck(GetDlgItem(hwnd, DLG_ID_CALC_SMOOTHING), BST_CHECKED);
        }
        break;
        case WM_COMMAND:
        {
            int nNotification = HIWORD(wParam);
            int nID = LOWORD(wParam);
            HWND hControl = (HWND) lParam;
            MDL * Mdl = nullptr;
            if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) Mdl = (MDL*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(Mdl != nullptr){
                bChange = true;
                switch(nID){
                    case DLG_ID_AREA_WEIGHT:
                    {
                        if(Button_GetCheck(hControl) == BST_CHECKED) Mdl->bSmoothAreaWeighting = true;
                        else Mdl->bSmoothAreaWeighting = false;
                    }
                    break;
                    case DLG_ID_ANGLE_WEIGHT:
                    {
                        if(Button_GetCheck(hControl) == BST_CHECKED) Mdl->bSmoothAngleWeighting = true;
                        else Mdl->bSmoothAreaWeighting = false;
                    }
                    break;
                    case DLG_ID_CALC_SMOOTHING:
                    {
                        if(Button_GetCheck(hControl) == BST_CHECKED) Mdl->bDetermineSmoothing = true;
                        else Mdl->bDetermineSmoothing = false;
                    }
                    break;
                }
            }
        }
        break;
        case WM_CLOSE:
        {
            MDL * Mdl = nullptr;
            if(GetWindowLongPtr(hwnd, GWLP_USERDATA) != 0) Mdl = (MDL*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(Mdl != nullptr){
                if(bChange && Mdl->GetFileData()) MessageBox(hwnd, "You need to reload the current model for the changes to take effect.", "Note", MB_OK | MB_ICONINFORMATION);
            }
            EndDialog(hwnd, wParam);
        }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}
