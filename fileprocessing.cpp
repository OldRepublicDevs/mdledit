#include "frame.h"
#include "edits.h"
#include <Shlwapi.h>
#include <fstream>
#include "MDL.h"

extern Edits Edit1;
HANDLE hThread;
HWND hProgress;
HWND hProgressMass;
char * lpStrFiles = nullptr;
DWORD WINAPI ThreadReprocess(LPVOID lpParam);
DWORD WINAPI ThreadProcessAscii(LPVOID lpParam);
DWORD WINAPI ThreadProcessBinary(LPVOID lpParam);
DWORD WINAPI ThreadMassAscii(LPVOID lpParam);
DWORD WINAPI ThreadMassBinary(LPVOID lpParam);

bool FileEditor(HWND hwnd, int nID, std::string & cFile){
    OPENFILENAME ofn;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    std::string cExt;
    bool bReturn = false;

    cFile.resize(MAX_PATH);
    if(nID == IDM_ASCII_SAVE){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "ASCII MDL Format (*.mdl)\0*.mdl\0";
        ofn.lpstrDefExt = "mdl";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT && strlen(cFile.c_str()) + 5 > MAX_PATH) {
                Error("The specified file is not an .mdl file! Unable to save!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1);
            cFile = sFileNoExt + std::string(".mdl");

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

            //Save Pwk
            if(Model.Pwk){
                std::string cPwk = sFileNoExt + ".pwk";

                file.open(cPwk, std::fstream::out);

                sAsciiExport.clear();
                Model.ExportPwkAscii(sAsciiExport);

                //Write and close file
                file<<sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }

            //Save Dwk
            if(Model.Dwk0 || Model.Dwk1 || Model.Dwk2){
                std::string cDwk = sFileNoExt + ".dwk";

                file.open(cDwk, std::fstream::out);

                sAsciiExport.clear();
                Model.ExportDwkAscii(sAsciiExport);

                //Write and close file
                file<<sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }

            //Save Wok
            if(Model.Wok && Model.bExportWok){
                std::string cWok = sFileNoExt + ".wok";

                file.open(cWok, std::fstream::out);

                sAsciiExport.clear();
                Model.ExportWokAscii(sAsciiExport);

                //Write and close file
                file<<sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }


            bReturn = true;        }
        else std::cout<<"Selecting file failed. :( \n";
    }
    else if(nID == IDM_BIN_SAVE){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "Binary MDL Format (*.mdl)\0*.mdl\0";
        ofn.lpstrDefExt = "mdl";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT && strlen(cFile.c_str()) + 5 > MAX_PATH) {
                Error("The specified file is not an .mdl file! Unable to save!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1);
            cFile = sFileNoExt + std::string(".mdl");

            //Create file
            std::ofstream file(cFile, std::ios::binary | std::fstream::out);
            //std::ofstream file(cFile);

            if(!file.is_open()){
                std::cout<<"File creation failed. Aborting.\n";
                return false;
            }

            //Put data into a string
            std::string sBinaryExport;
            Model.Export(sBinaryExport);

            //Write and close file
            file<<sBinaryExport;
            file.close();

            //Save mdx
            if(Model.Mdx){
                std::string cMdx = sFileNoExt + ".mdx";

                file.open(cMdx, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Mdx->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Wok
            if(Model.Wok){
                std::string cWok = sFileNoExt + ".wok";

                file.open(cWok, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Wok->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Pwk
            if(Model.Pwk){
                std::string cPwk = sFileNoExt + ".pwk";

                file.open(cPwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Pwk->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Dwk
            if(Model.Dwk0){
                std::string cDwk = sFileNoExt + "0.dwk";

                file.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Dwk0->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
            if(Model.Dwk1){
                std::string cDwk = sFileNoExt + "1.dwk";

                file.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Dwk1->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
            if(Model.Dwk2){
                std::string cDwk = sFileNoExt + "2.dwk";

                file.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                Model.Dwk2->Export(sBinaryExport);

                //Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            bReturn = true;        }
        else std::cout<<"Selecting file failed. :( \n";
    }
    else if(nID == IDM_MDL_OPEN){        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "MDL Format (*.mdl)\0*.mdl\0";
        ofn.lpstrDefExt = "mdl";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;        if(GetOpenFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT) {
                Error("The specified file is not an .mdl file!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1).c_str();
            cFile = sFileNoExt + std::string(".mdl");

            //Create file
            std::ifstream file(cFile, std::ios::binary);


            if(!file.is_open()){
                std::cout<<"File creation/opening failed. Aborting.\n";
                return false;
            }

            //If everything checks out, we may begin reading
            bool bAscii = false;
            file.seekg(0, std::ios::end);
            std::streampos filelength = file.tellg();
            if(filelength < 10){
                std::cout<<"File too short. Aborting.\n";
                return false;
            }
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
                TabCtrl_DeleteAllItems(hTabs);
                TreeView_DeleteAllItems(hTree);
                Model.FlushData();
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
                AppendTab(hTabs, "MDL");
                AppendTab(hTabs, "MDX");

                //Open and process .pwk if it exists
                std::string cPwk = sFileNoExt + ".pwk";
                if(PathFileExists(cPwk.c_str())){
                    file.open(cPwk, std::ifstream::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (pwk). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.PwkAscii.reset(new ASCII());
                            std::vector<char> & sBufferRef = Model.PwkAscii->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.PwkAscii->SetFilePath(cPwk);
                            AppendTab(hTabs, "PWK");
                        }
                    }
                }

                //Open and process .dwk if it exists
                std::string cDwk = sFileNoExt + ".dwk";
                if(PathFileExists(cDwk.c_str())){
                    file.open(cDwk);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (pwk). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.DwkAscii.reset(new ASCII());
                            std::vector<char> & sBufferRef = Model.DwkAscii->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.DwkAscii->SetFilePath(cDwk);
                            AppendTab(hTabs, "DWK 0");
                            AppendTab(hTabs, "DWK 1");
                            AppendTab(hTabs, "DWK 2");
                        }
                    }
                }

                //Process the data
                Model.SetFilePath(cFile);
                if(DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 1)){
                    bReturn = true;
                }
                else{
                    //We failed reading the ascii, so we need to clean up
                    TabCtrl_DeleteAllItems(hTabs);
                    TreeView_DeleteAllItems(hTree);
                    Model.FlushData();
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
                AppendTab(hTabs, "MDL");

                //Open and process .mdx if it exists
                std::string cMdx = sFileNoExt + ".mdx";
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
                        AppendTab(hTabs, "MDX");
                    }
                }
                else{                    PathStripPath(&cMdx.front());
                    Warning("Could not find "+std::string(cMdx.c_str())+" in the same directory. Will load without the MDX data.");
                }

                //Open and process .wok if it exists
                std::string cWok = sFileNoExt + ".wok";
                if(PathFileExists(cWok.c_str())){
                    file.open(cWok, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (wok). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(!bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.Wok.reset(new WOK());
                            std::vector<char> & sBufferRef = Model.Wok->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.Wok->SetFilePath(cWok);
                            AppendTab(hTabs, "WOK");
                        }
                    }
                }

                //Open and process .pwk if it exists
                std::string cPwk = sFileNoExt + ".pwk";
                if(PathFileExists(cPwk.c_str())){
                    file.open(cPwk, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (wok). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(!bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.Pwk.reset(new PWK());
                            std::vector<char> & sBufferRef = Model.Pwk->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.Pwk->SetFilePath(cPwk);
                            AppendTab(hTabs, "PWK");
                        }
                    }
                }

                //Open and process .dwk if it exists
                std::string cDwk = sFileNoExt + "0.dwk";
                if(PathFileExists(cDwk.c_str())){
                    file.open(cDwk, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (dwk0). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(!bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.Dwk0.reset(new DWK());
                            std::vector<char> & sBufferRef = Model.Dwk0->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.Dwk0->SetFilePath(cDwk);
                            AppendTab(hTabs, "DWK 0");
                        }
                    }
                }
                cDwk = sFileNoExt + "1.dwk";
                if(PathFileExists(cDwk.c_str())){
                    file.open(cDwk, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (dwk1). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(!bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.Dwk1.reset(new DWK());
                            std::vector<char> & sBufferRef = Model.Dwk1->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.Dwk1->SetFilePath(cDwk);
                            AppendTab(hTabs, "DWK 1");
                        }
                    }
                }
                cDwk = sFileNoExt + "2.dwk";
                if(PathFileExists(cDwk.c_str())){
                    file.open(cDwk, std::ios::binary);
                    if(!file.is_open()){
                        std::cout<<"File creation/opening failed (dwk2). Aborting.\n";
                    }
                    else{
                        bAscii = false;
                        file.seekg(0,std::ios::beg);
                        char cBinary [8];
                        file.read(cBinary, 8);
                        //First check whether it's an ascii or a binary
                        if(std::string(cBinary, 8) != "BWM V1.0"){
                            bAscii = true;
                        }
                        if(!bAscii){
                            //We may begin reading
                            file.seekg(0, std::ios::end);
                            std::streampos length = file.tellg();
                            file.seekg(0,std::ios::beg);
                            Model.Dwk2.reset(new DWK());
                            std::vector<char> & sBufferRef = Model.Dwk2->CreateBuffer(length);
                            file.read(&sBufferRef[0], length);
                            file.close();
                            Model.Dwk2->SetFilePath(cDwk);
                            AppendTab(hTabs, "DWK 2");
                        }
                    }
                }

                //Process the data
                if(DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 2)){
                    bReturn = true;
                }
                else{
                    //Something failed, cleanup
                    Edit1.LoadData();
                    ProcessTreeAction(NULL, ACTION_UPDATE_DISPLAY, nullptr);
                    bReturn = false;
                }
            }        }
        else std::cout<<"Selecting file failed. :( \n";    }
    else if(nID == IDM_MASS_TO_ASCII){
        int nOffsetToFirst = 0;        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "MDL Format (*.mdl)\0*.mdl\0";
        ofn.lpstrDefExt = "mdl";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileName(&ofn)){
            lpStrFiles = &cFile[0] + ofn.nFileOffset;
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 1);
            bReturn = true;        }
        else std::cout<<"Selecting file failed. :( \n";    }
    else if(nID == IDM_MASS_TO_BIN){
        int nOffsetToFirst = 0;        ZeroMemory(&ofn, sizeof(ofn));        ofn.lStructSize = sizeof(ofn);        ofn.hwndOwner = hwnd;        ofn.lpstrFile = &cFile; //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH;        ofn.lpstrFilter = "MDL Format (*.mdl)\0*.mdl\0";
        ofn.lpstrDefExt = "mdl";        ofn.nFilterIndex = 1;        ofn.lpstrFileTitle = NULL;        ofn.nMaxFileTitle = 0;        ofn.lpstrInitialDir = NULL;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileName(&ofn)){
            lpStrFiles = &cFile[0] + ofn.nFileOffset;
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 2);
            bReturn = true;        }
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
                                            10, 25, rcStatus.right - 20, 18,
                                            hwnd, (HMENU) IDC_STATUSBAR_PROGRESS, NULL, NULL);
            SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);

            Model.PtrReport = Report;
            Model.PtrProgressSize = ProgressSize;
            Model.PtrProgressPos = ProgressPos;

            if(lParam == 1) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessAscii, hwnd, 0, NULL);
            else if(lParam == 2) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessBinary, hwnd, 0, NULL);
            else if(lParam == 3) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadReprocess, hwnd, 0, NULL);
            else EndDialog(hwnd, NULL);
        }
        break;
        case 69:
        {
            CloseHandle(hThread);
            if(wParam == 1){
                EndDialog(hwnd, false);
            }
            else EndDialog(hwnd, true);
        }
        break;
        case WM_CLOSE:
            PostQuitMessage(0);
        break;
        default:
            return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK ProgressMassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch(message){
        case WM_INITDIALOG:
        {
            RECT rcStatus;
            GetClientRect(hwnd, &rcStatus);
            hProgress = CreateWindowEx(NULL, PROGRESS_CLASS, "", WS_VISIBLE | WS_CHILD,
                                            10, 25, rcStatus.right - 20, 18,
                                            hwnd, (HMENU) IDC_STATUSBAR_PROGRESS, NULL, NULL);
            hProgressMass = CreateWindowEx(NULL, PROGRESS_CLASS, "", WS_VISIBLE | WS_CHILD,
                                            10, 45, rcStatus.right - 20, 18,
                                            hwnd, (HMENU) IDC_STATUSBAR_PROGRESSMASS, NULL, NULL);
            SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);
            SendMessage(hProgressMass, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);

            if(lParam == 1) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMassAscii, hwnd, 0, NULL);
            else if(lParam == 2) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMassBinary, hwnd, 0, NULL);
            else EndDialog(hwnd, NULL);
        }
        break;
        case 69:
        {
            hProgress = NULL;
            hProgressMass = NULL;
            CloseHandle(hThread);
            if(wParam == 1) EndDialog(hwnd, false);
            else EndDialog(hwnd, true);
        }
        break;
        case WM_CLOSE:
            PostQuitMessage(0);
        break;
        default:
            return FALSE;
    }
    return TRUE;
}

void Report(std::string sMessage){
    SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sMessage.c_str());
}
void ProgressSize(int nMin, int nMax){
    SendMessage(hProgress, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(nMin, nMax));
}
void ProgressPos(int nPos){
    SendMessage(hProgress, PBM_SETPOS, (WPARAM) nPos, (LPARAM) NULL);
}

DWORD WINAPI ThreadReprocess(LPVOID lpParam){
    Model.Compile();
    //This should bring us to a state where all data is ready,
    //even the binary-file-specific data

    //Load the data
    Report("Loading data...");
    SetWindowText(hDisplayEdit, "");
    Edit1.LoadData(); //Loads up the binary file onto the screen
    BuildTree(Model); //Fills the TreeView control
    std::cout<<"Data loaded!\n";

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadProcessAscii(LPVOID lpParam){
    bool bReadWell = Model.ReadAscii();
    //Just read it.
    if(!bReadWell){
        SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
        return 0;
    }

    /// Now we know whether we have WOK data
    if(Model.Wok) AppendTab(hTabs, "WOK");

    Model.Compile();
    //This should bring us to a state where all data is ready,
    //even the binary-file-specific data

    //Load the data
    Report("Loading data...");
    SetWindowText(hDisplayEdit, "");
    Edit1.LoadData(); //Loads up the binary file onto the screen
    BuildTree(Model); //Fills the TreeView control
    if(Model.Pwk) BuildTree(*Model.Pwk);
    if(Model.Dwk0) BuildTree(*Model.Dwk0);
    if(Model.Dwk1) BuildTree(*Model.Dwk1);
    if(Model.Dwk2) BuildTree(*Model.Dwk2);
    if(Model.Wok){
        std::string sWok = ".wok"; //Model.GetFullPath();
        //char * cExt2 = PathFindExtension(sWok.c_str());
        //sprintf(cExt2, ".wok");
        Model.Wok->SetFilePath(sWok);
        std::cout<< "About to build wok tree.\n";
        BuildTree(*Model.Wok);
    }
    std::cout<<"Data loaded!\n";

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadProcessBinary(LPVOID lpParam){
    Model.DecompileModel();
    Report("Processing walkmesh...");
    if(Model.Wok){
        Model.Wok->ProcessBWM();
        Model.GetLytPositionFromWok();
    }
    if(Model.Pwk) Model.Pwk->ProcessBWM();
    if(Model.Dwk0) Model.Dwk0->ProcessBWM();
    if(Model.Dwk1) Model.Dwk1->ProcessBWM();
    if(Model.Dwk2) Model.Dwk2->ProcessBWM();

    //Load the data
    Report("Loading data...");
    BuildTree(Model);
    if(Model.Wok) BuildTree(*Model.Wok);
    if(Model.Pwk) BuildTree(*Model.Pwk);
    if(Model.Dwk0) BuildTree(*Model.Dwk0);
    if(Model.Dwk1) BuildTree(*Model.Dwk1);
    if(Model.Dwk2) BuildTree(*Model.Dwk2);
    SetWindowText(hDisplayEdit, "");
    Edit1.LoadData();
    std::cout<<"Data loaded!\n";

    Model.CheckPeculiarities(); //Finally, check for peculiarities

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadMassAscii(LPVOID lpParam){
    int nMax = 0;
    char * lpstrCounter = lpStrFiles;
    while(*lpstrCounter != 0){
        lpstrCounter = lpstrCounter + (strlen(lpstrCounter) + 1);
        nMax++;
    }

    SendMessage(hProgressMass, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, nMax));

    int nCounter = 0;
    while(*lpStrFiles != 0){
        std::string sCurrentFile = lpStrFiles;
        lpStrFiles = lpStrFiles + (strlen(lpStrFiles) + 1);

        std::string sFileNoExt = sCurrentFile.c_str();
        if(safesubstr(sCurrentFile, sCurrentFile.size() - 4, 4) == ".mdl") sFileNoExt = safesubstr(sCurrentFile, 0, sCurrentFile.size() - 4);

        std::string sStatic = "Processing " + std::string(PathFindFileName(sFileNoExt.c_str())) + "...";
        SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

        //Create file
        std::ifstream file(sCurrentFile, std::ios::binary);

        if(!file.is_open()){
            std::cout<<"File creation/opening failed for "<<PathFindFileName(sFileNoExt.c_str())<<". Aborting.\n";
            continue;
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

        if(bAscii){
            std::cout<<"File not binary, skipping...\n";
            continue;
        }
        else{
            /// Set tempModel and its settings
            MDL tempModel;
            tempModel.PtrReport = nullptr;
            tempModel.PtrProgressSize = ProgressSize;
            tempModel.PtrProgressPos = ProgressPos;
            tempModel.bDetermineSmoothing = Model.bDetermineSmoothing;
            tempModel.bLightsaberToTrimesh = Model.bLightsaberToTrimesh;
            tempModel.bSkinToTrimesh = Model.bSkinToTrimesh;
            tempModel.bSmoothAngleWeighting = Model.bSmoothAngleWeighting;
            tempModel.bSmoothAreaWeighting = Model.bSmoothAreaWeighting;
            tempModel.bWriteAnimations = Model.bWriteAnimations;

            file.seekg(0, std::ios::end);
            std::streampos length = file.tellg();
            file.seekg(0,std::ios::beg);
            std::vector<char> & sBufferRef = tempModel.CreateBuffer(length);
            file.read(&sBufferRef[0], length);
            file.close();
            tempModel.SetFilePath(sCurrentFile);

            //Open and process .mdx if it exists
            std::string cMdx = sFileNoExt + ".mdx";
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
                    tempModel.Mdx.reset(new MDX());
                    std::vector<char> & sBufferRef = tempModel.Mdx->CreateBuffer(length);
                    file.read(&sBufferRef[0], length);
                    file.close();
                    tempModel.Mdx->SetFilePath(cMdx);
                }
            }
            else{                //PathStripPath(&cMdx.front());
                //Warning("Could not find "+std::string(cMdx.c_str())+" in the same directory. Will load without the MDX data.");
            }

            //Open and process .wok if it exists
            std::string cWok = sFileNoExt + ".wok";
            if(PathFileExists(cWok.c_str())){
                file.open(cWok, std::ios::binary);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (wok). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(!bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.Wok.reset(new WOK());
                        std::vector<char> & sBufferRef = tempModel.Wok->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.Wok->SetFilePath(cWok);
                    }
                }
            }

            //Open and process .pwk if it exists
            std::string cPwk = sFileNoExt + ".pwk";
            if(PathFileExists(cPwk.c_str())){
                file.open(cPwk, std::ios::binary);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (wok). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(!bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.Pwk.reset(new PWK());
                        std::vector<char> & sBufferRef = tempModel.Pwk->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.Pwk->SetFilePath(cPwk);
                    }
                }
            }

            //Open and process .dwk if it exists
            std::string cDwk = sFileNoExt + "0.dwk";
            if(PathFileExists(cDwk.c_str())){
                file.open(cDwk, std::ios::binary);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (dwk0). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(!bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.Dwk0.reset(new DWK());
                        std::vector<char> & sBufferRef = tempModel.Dwk0->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.Dwk0->SetFilePath(cDwk);
                    }
                }
            }
            cDwk = sFileNoExt + "1.dwk";
            if(PathFileExists(cDwk.c_str())){
                file.open(cDwk, std::ios::binary);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (dwk1). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(!bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.Dwk1.reset(new DWK());
                        std::vector<char> & sBufferRef = tempModel.Dwk1->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.Dwk1->SetFilePath(cDwk);
                    }
                }
            }
            cDwk = sFileNoExt + "2.dwk";
            if(PathFileExists(cDwk.c_str())){
                file.open(cDwk, std::ios::binary);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (dwk2). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(!bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.Dwk2.reset(new DWK());
                        std::vector<char> & sBufferRef = tempModel.Dwk2->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.Dwk2->SetFilePath(cDwk);
                    }
                }
            }

            tempModel.DecompileModel();
            if(tempModel.Wok){
                tempModel.Wok->ProcessBWM();
                tempModel.GetLytPositionFromWok();
            }
            if(tempModel.Pwk) tempModel.Pwk->ProcessBWM();
            if(tempModel.Dwk0) tempModel.Dwk0->ProcessBWM();
            if(tempModel.Dwk1) tempModel.Dwk1->ProcessBWM();
            if(tempModel.Dwk2) tempModel.Dwk2->ProcessBWM();

            sFileNoExt += "-mdledit-ascii";
            sCurrentFile = sFileNoExt + std::string(".mdl");

            //Create file
            std::ofstream fileout(sCurrentFile, std::fstream::out);

            if(!fileout.is_open()){
                std::cout<<"File creation failed. Aborting.\n";
                continue;
            }

            //Convert the data and put it into a string
            //std::cout<<"Converting to ascii.\n";
            std::string sAsciiExport;
            tempModel.ExportAscii(sAsciiExport);

            //Write and close file
            //std::cout<<"Writing ascii.\n";
            fileout<<sAsciiExport;
            fileout.close();

            //std::cout<<"Cleanup.\n";
            sAsciiExport.clear();
            sAsciiExport.shrink_to_fit();

            //Save Pwk
            if(tempModel.Pwk){
                std::string cPwk = sFileNoExt + ".pwk";

                fileout.open(cPwk, std::fstream::out);

                sAsciiExport.clear();
                tempModel.ExportPwkAscii(sAsciiExport);

                //Write and close file
                fileout<<sAsciiExport;
                fileout.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }

            //Save Dwk
            if(tempModel.Dwk0 || tempModel.Dwk1 || tempModel.Dwk2){
                std::string cDwk = sFileNoExt + ".dwk";

                fileout.open(cDwk, std::fstream::out);

                sAsciiExport.clear();
                tempModel.ExportDwkAscii(sAsciiExport);

                //Write and close file
                fileout<<sAsciiExport;
                fileout.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }

            //Save Wok
            if(tempModel.Wok && tempModel.bExportWok){
                std::string cWok = sFileNoExt + ".wok";

                fileout.open(cWok, std::fstream::out);

                sAsciiExport.clear();
                tempModel.ExportWokAscii(sAsciiExport);

                //Write and close file
                fileout<<sAsciiExport;
                fileout.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }

        nCounter++;
        SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
    }

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadMassBinary(LPVOID lpParam){
    int nMax = 0;
    char * lpstrCounter = lpStrFiles;
    while(*lpstrCounter != 0){
        lpstrCounter = lpstrCounter + (strlen(lpstrCounter) + 1);
        nMax++;
    }

    SendMessage(hProgressMass, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, nMax));

    int nCounter = 0;
    while(*lpStrFiles != 0){
        std::string sCurrentFile = lpStrFiles;
        lpStrFiles = lpStrFiles + (strlen(lpStrFiles) + 1);

        std::string sFileNoExt = sCurrentFile.c_str();
        if(safesubstr(sCurrentFile, sCurrentFile.size() - 4, 4) == ".mdl") sFileNoExt = safesubstr(sCurrentFile, 0, sCurrentFile.size() - 4);

        std::string sStatic = "Processing " + std::string(PathFindFileName(sFileNoExt.c_str())) + "...";
        SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

        //Create file
        std::ifstream file(sCurrentFile, std::ios::binary);

        if(!file.is_open()){
            std::cout<<"File creation/opening failed for "<<PathFindFileName(sFileNoExt.c_str())<<". Aborting.\n";
            continue;
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

        if(!bAscii){
            std::cout<<"Not ascii, skipping... \n";
            continue;
        }
        else{
            /// Set tempModel and its settings
            MDL tempModel;
            tempModel.PtrReport = nullptr;
            tempModel.PtrProgressSize = ProgressSize;
            tempModel.PtrProgressPos = ProgressPos;
            tempModel.bExportWok = Model.bExportWok;
            tempModel.bDetermineSmoothing = Model.bDetermineSmoothing;
            tempModel.bLightsaberToTrimesh = Model.bLightsaberToTrimesh;
            tempModel.bSkinToTrimesh = Model.bSkinToTrimesh;
            tempModel.bSmoothAngleWeighting = Model.bSmoothAngleWeighting;
            tempModel.bSmoothAreaWeighting = Model.bSmoothAreaWeighting;
            tempModel.bWriteAnimations = Model.bWriteAnimations;
            tempModel.bK2 = Model.bK2;

            file.seekg(0, std::ios::end);
            std::streampos length = file.tellg();
            file.seekg(0,std::ios::beg);
            tempModel.Ascii.reset(new ASCII());
            std::vector<char> & sBufferRef = tempModel.CreateAsciiBuffer(length);
            file.read(&sBufferRef[0], length);
            file.close();

            //Open and process .pwk if it exists
            std::string cPwk = sFileNoExt + ".pwk";
            if(PathFileExists(cPwk.c_str())){
                file.open(cPwk);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (pwk). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.PwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferRef = tempModel.PwkAscii->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.PwkAscii->SetFilePath(cPwk);
                    }
                }
            }

            //Open and process .dwk if it exists
            std::string cDwk = sFileNoExt + ".dwk";
            if(PathFileExists(cDwk.c_str())){
                file.open(cDwk);
                if(!file.is_open()){
                    std::cout<<"File creation/opening failed (pwk). Aborting.\n";
                }
                else{
                    bAscii = false;
                    file.seekg(0,std::ios::beg);
                    char cBinary [8];
                    file.read(cBinary, 8);
                    //First check whether it's an ascii or a binary
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        bAscii = true;
                    }
                    if(bAscii){
                        //We may begin reading
                        file.seekg(0, std::ios::end);
                        std::streampos length = file.tellg();
                        file.seekg(0,std::ios::beg);
                        tempModel.DwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferRef = tempModel.DwkAscii->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        tempModel.DwkAscii->SetFilePath(cDwk);
                    }
                }
            }

            //Process the data
            tempModel.SetFilePath(sCurrentFile);
            if(!tempModel.ReadAscii()) continue;
            tempModel.Compile();

            sFileNoExt += "-mdledit-bin";
            sCurrentFile = sFileNoExt + std::string(".mdl");

            //Create file
            std::ofstream fileout (sCurrentFile, std::ios::binary | std::fstream::out);

            if(!fileout.is_open()){
                std::cout<<"File creation failed. Aborting.\n";
                continue;
            }

            //Put data into a string
            std::string sBinaryExport;
            tempModel.Export(sBinaryExport);

            //Write and close file
            fileout<<sBinaryExport;
            fileout.close();

            //Save mdx
            if(tempModel.Mdx){
                std::string cMdx = sFileNoExt + ".mdx";

                fileout.open(cMdx, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Mdx->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Wok
            if(tempModel.Wok){
                std::string cWok = sFileNoExt + ".wok";

                fileout.open(cWok, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Wok->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Pwk
            if(tempModel.Pwk){
                std::string cPwk = sFileNoExt + ".pwk";

                fileout.open(cPwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Pwk->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }

            //Save Dwk
            if(tempModel.Dwk0){
                std::string cDwk = sFileNoExt + "0.dwk";

                fileout.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Dwk0->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
            if(tempModel.Dwk1){
                std::string cDwk = sFileNoExt + "1.dwk";

                fileout.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Dwk1->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
            if(tempModel.Dwk2){
                std::string cDwk = sFileNoExt + "2.dwk";

                fileout.open(cDwk, std::ios::binary | std::fstream::out);

                sBinaryExport.clear();
                tempModel.Dwk2->Export(sBinaryExport);

                //Write and close file
                fileout<<sBinaryExport;
                fileout.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        nCounter++;
        SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
    }

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}
