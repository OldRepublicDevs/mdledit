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
std::string sFolder;
DWORD WINAPI ThreadReprocess(LPVOID lpParam);
DWORD WINAPI ThreadProcessAscii(LPVOID lpParam);
DWORD WINAPI ThreadProcessBinary(LPVOID lpParam);
DWORD WINAPI ThreadMassAscii(LPVOID lpParam);
DWORD WINAPI ThreadMassBinary(LPVOID lpParam);
DWORD WINAPI ThreadMassAnalyze(LPVOID lpParam);
bool bCancelMass = false;

void ReportGetOpenFileNameError(){
    //std::cout<<"GetOpenFileName() failed. :( \n";
    unsigned int nError = CommDlgExtendedError();
    if(nError == CDERR_DIALOGFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_DIALOGFAILURE\n";
    else if(nError == CDERR_FINDRESFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_FINDRESFAILURE\n";
    else if(nError == CDERR_INITIALIZATION) std::cout << "GetOpenFileName() failed. Error: CDERR_INITIALIZATION\n";
    else if(nError == CDERR_LOADRESFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_LOADRESFAILURE\n";
    else if(nError == CDERR_LOADSTRFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_LOADSTRFAILURE\n";
    else if(nError == CDERR_LOCKRESFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_LOCKRESFAILURE\n";
    else if(nError == CDERR_MEMALLOCFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_MEMALLOCFAILURE\n";
    else if(nError == CDERR_MEMLOCKFAILURE) std::cout << "GetOpenFileName() failed. Error: CDERR_MEMLOCKFAILURE\n";
    else if(nError == CDERR_NOHINSTANCE) std::cout << "GetOpenFileName() failed. Error: CDERR_NOHINSTANCE\n";
    else if(nError == CDERR_NOHOOK) std::cout << "GetOpenFileName() failed. Error: CDERR_NOHOOK\n";
    else if(nError == CDERR_NOTEMPLATE) std::cout << "GetOpenFileName() failed. Error: CDERR_NOTEMPLATE\n";
    else if(nError == CDERR_STRUCTSIZE) std::cout << "GetOpenFileName() failed. Error: CDERR_STRUCTSIZE\n";
    else if(nError == FNERR_BUFFERTOOSMALL){
        std::cout << "GetOpenFileName() failed. Error: FNERR_BUFFERTOOSMALL\n";
        Error("Too many files selected. Try dividing your files into smaller groups and process them separately.");
    }
    else if(nError == FNERR_INVALIDFILENAME) std::cout << "GetOpenFileName() failed. Error: FNERR_INVALIDFILENAME\n";
    else if(nError == FNERR_SUBCLASSFAILURE) std::cout << "GetOpenFileName() failed. Error: FNERR_SUBCLASSFAILURE\n";
    //else std::cout << "Error: unknown\n";
}

bool LoadFiles(MDL & mdl, const std::string & sFileNoExt, bool & bAscii){
    bool bDisplay = false;
    if(&mdl == &Model) bDisplay = true;
    /// Create file

    std::string sMdl = sFileNoExt + ".mdl";
    std::ifstream file(sMdl, std::ios::binary);

    if(!file.is_open()){
        std::cout << ("File creation/opening failed for " + sMdl + ". Aborting.\n");
        return false;
    }

    /// If everything checks out, we may begin reading
    file.seekg(0, std::ios::end);
    std::streampos filelength = file.tellg();
    if(filelength < 12){
        std::cout << "File too short. Aborting.\n";
        return false;
    }

    /// First check whether it's an ascii or a binary
    file.seekg(0,std::ios::beg);
    char cBinary [4];
    file.read(cBinary, 4);
    if(cBinary[0] != '\0' || cBinary[1] != '\0' || cBinary[2] != '\0' || cBinary[3] != '\0') bAscii = true;
    else bAscii = false;

    /// Now we need to check our current data
    if(mdl.GetFileData()){
        /// A model is already open. Flush it to make room for the new one.
        if(bDisplay){
            TabCtrl_DeleteAllItems(hTabs);
            TreeView_DeleteAllItems(hTree);
        }
        mdl.FlushData();
    }

    if(bAscii){
        std::cout << "Reading ascii...\n";
        file.seekg(0, std::ios::beg);
        mdl.Ascii.reset(new ASCII());
        std::vector<char> & sBufferRef = mdl.CreateAsciiBuffer(filelength);
        file.read(&sBufferRef[0], filelength);
        file.close();
        mdl.SetFilePath(sMdl);
        if(bDisplay){
            AppendTab(hTabs, "MDL");
            AppendTab(hTabs, "MDX");
        }

        /// Open and process .pwk if it exists
        std::string cPwk = sFileNoExt + ".pwk";
        if(PathFileExists(cPwk.c_str())){
            file.open(cPwk, std::ifstream::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed (pwk) for " << cPwk << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cPwk << " too short. Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        /// We may begin reading
                        mdl.PwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferRef = mdl.PwkAscii->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        mdl.PwkAscii->SetFilePath(cPwk);
                        if(bDisplay) AppendTab(hTabs, "PWK");
                    }
                }
                file.close();
            }
        }

        //Open and process .dwk if it exists
        std::string cDwk = sFileNoExt + ".dwk";
        if(PathFileExists(cDwk.c_str())){
            file.open(cDwk);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cDwk << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cDwk << " too short. Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) != "BWM V1.0"){
                        /// We may begin reading
                        mdl.DwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferRef = mdl.DwkAscii->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.DwkAscii->SetFilePath(cDwk);
                        if(bDisplay){
                            AppendTab(hTabs, "DWK 0");
                            AppendTab(hTabs, "DWK 1");
                            AppendTab(hTabs, "DWK 2");
                        }
                    }
                }
                file.close();
            }
        }
    }
    else{
        std::cout << "Reading binary...\n";

        file.seekg(0,std::ios::beg);
        std::vector<char> & sBufferRef = mdl.CreateBuffer(filelength);
        file.read(&sBufferRef[0], filelength);
        file.close();
        mdl.SetFilePath(sMdl);
        if(bDisplay) AppendTab(hTabs, "MDL");

        /// Open and process .mdx if it exists
        std::string cMdx = sFileNoExt + ".mdx";
        if(PathFileExists(cMdx.c_str())){
            file.open(cMdx, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cMdx << ". Aborting.\n";
            }
            else{
                //We may begin reading
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0,std::ios::beg);
                mdl.Mdx.reset(new MDX());
                std::vector<char> & sBufferRef = mdl.Mdx->CreateBuffer(length);
                file.read(&sBufferRef[0], length);
                file.close();
                mdl.Mdx->SetFilePath(cMdx);
                if(bDisplay) AppendTab(hTabs, "MDX");
            }
        }
        else if(bDisplay){            PathStripPath(&cMdx.front());
            Warning("Could not find " + std::string(cMdx.c_str()) + " in the same directory. Will load without the MDX data.");
        }

        /// Open and process .wok if it exists
        std::string cWok = sFileNoExt + ".wok";
        if(PathFileExists(cWok.c_str())){
            file.open(cWok, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cWok << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cWok << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) == "BWM V1.0"){
                        /// We may begin reading
                        mdl.Wok.reset(new WOK());
                        std::vector<char> & sBufferRef = mdl.Wok->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.Wok->SetFilePath(cWok);
                        if(bDisplay) AppendTab(hTabs, "WOK");
                    }
                }
                file.close();
            }
        }

        //Open and process .pwk if it exists
        std::string cPwk = sFileNoExt + ".pwk";
        if(PathFileExists(cPwk.c_str())){
            file.open(cPwk, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cPwk << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cPwk << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) == "BWM V1.0"){
                        /// We may begin reading
                        mdl.Pwk.reset(new PWK());
                        std::vector<char> & sBufferRef = mdl.Pwk->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.Pwk->SetFilePath(cPwk);
                        if(bDisplay) AppendTab(hTabs, "PWK");
                    }
                }
                file.close();
            }
        }

        //Open and process .dwk if it exists
        std::string cDwk = sFileNoExt + "0.dwk";
        if(PathFileExists(cDwk.c_str())){
            file.open(cDwk, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cDwk << ". Aborting.\n";
            }
            else{
                bAscii = false;
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cDwk << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) == "BWM V1.0"){
                        /// We may begin reading
                        mdl.Dwk0.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk0->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.Dwk0->SetFilePath(cDwk);
                        if(bDisplay) AppendTab(hTabs, "DWK 0");
                    }
                }
                file.close();
            }
        }
        cDwk = sFileNoExt + "1.dwk";
        if(PathFileExists(cDwk.c_str())){
            file.open(cDwk, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cDwk << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cDwk << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) == "BWM V1.0"){
                        /// We may begin reading
                        mdl.Dwk1.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk1->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.Dwk1->SetFilePath(cDwk);
                        if(bDisplay) AppendTab(hTabs, "DWK 1");
                    }
                }
                file.close();
            }
        }
        cDwk = sFileNoExt + "2.dwk";
        if(PathFileExists(cDwk.c_str())){
            file.open(cDwk, std::ios::binary);
            if(!file.is_open()){
                std::cout<<"File creation/opening failed for " << cDwk << ". Aborting.\n";
            }
            else{
                file.seekg(0, std::ios::end);
                std::streampos length = file.tellg();
                file.seekg(0, std::ios::beg);
                if(length < 12) std::cout << "File " << cDwk << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    char cBinary [8];
                    file.read(cBinary, 8);
                    if(std::string(cBinary, 8) == "BWM V1.0"){
                        /// We may begin reading
                        mdl.Dwk2.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk2->CreateBuffer(length);
                        file.read(&sBufferRef[0], length);
                        file.close();
                        mdl.Dwk2->SetFilePath(cDwk);
                        if(bDisplay) AppendTab(hTabs, "DWK 2");
                    }
                }
                file.close();
            }
        }
    }

    return true;
}

bool SaveFiles(MDL & mdl, const std::string & sFileNoExt, int nID){
    std::string sMdl = sFileNoExt + ".mdl";
    if(nID == IDM_ASCII_SAVE){
        /// Convert the data and put it into a string
        std::string sAsciiExport;
        mdl.ExportAscii(sAsciiExport);

        /// Create file
        std::ofstream file(sMdl, std::fstream::out);

        if(!file.is_open()){
            std::cout<<"File creation failed for " << sMdl << ". Aborting.\n";
            return false;
        }

        /// Write and close file
        file << sAsciiExport;
        file.close();

        sAsciiExport.clear();
        sAsciiExport.shrink_to_fit();

        /// Save Pwk
        if(mdl.Pwk){
            std::string cPwk = sFileNoExt + ".pwk";

            sAsciiExport.clear();
            mdl.ExportPwkAscii(sAsciiExport);

            file.open(cPwk, std::fstream::out);

            if(!file.is_open()) std::cout << "File creation failed for " << cPwk << ". Aborting.\n";
            else{
                /// Write and close file
                file << sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }

        /// Save Dwk
        if(mdl.Dwk0 || mdl.Dwk1 || mdl.Dwk2){
            std::string cDwk = sFileNoExt + ".dwk";

            sAsciiExport.clear();
            mdl.ExportDwkAscii(sAsciiExport);

            file.open(cDwk, std::fstream::out);

            if(!file.is_open()) std::cout << "File creation failed for " << cDwk << ". Aborting.\n";
            else{
                /// Write and close file
                file << sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }

        //Save Wok
        if(mdl.Wok && mdl.bExportWok){
            std::string cWok = sFileNoExt + ".wok";

            sAsciiExport.clear();
            mdl.ExportWokAscii(sAsciiExport);

            file.open(cWok, std::fstream::out);

            if(!file.is_open()) std::cout << "File creation failed for " << cWok << ". Aborting.\n";
            else{
                /// Write and close file
                file << sAsciiExport;
                file.close();

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }
    }
    else if(IDM_BIN_SAVE){
        /// Put data into a string
        std::string sBinaryExport;
        mdl.Export(sBinaryExport);

        /// Create file
        std::ofstream file(sMdl, std::ios::binary | std::fstream::out);

        if(!file.is_open()){
            std::cout<<"File creation failed for " << sMdl << ". Aborting.\n";
            return false;
        }

        /// Write and close file
        file << sBinaryExport;
        file.close();

        /// Save mdx
        if(mdl.Mdx){
            std::string cMdx = sFileNoExt + ".mdx";

            sBinaryExport.clear();
            mdl.Mdx->Export(sBinaryExport);

            file.open(cMdx, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cMdx << ". Aborting.\n";
            else{
                /// Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Wok
        if(mdl.Wok){
            std::string cWok = sFileNoExt + ".wok";

            sBinaryExport.clear();
            mdl.Wok->Export(sBinaryExport);

            file.open(cWok, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cWok << ". Aborting.\n";
            else{
                /// Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Pwk
        if(mdl.Pwk){
            std::string cPwk = sFileNoExt + ".pwk";

            sBinaryExport.clear();
            mdl.Pwk->Export(sBinaryExport);

            file.open(cPwk, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cPwk << ". Aborting.\n";
            else{
                /// Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Dwk
        if(mdl.Dwk0){
            std::string cDwk = sFileNoExt + "0.dwk";

            sBinaryExport.clear();
            mdl.Dwk0->Export(sBinaryExport);

            file.open(cDwk, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cDwk << ". Aborting.\n";
            else{
                /// Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
        if(mdl.Dwk1){
            std::string cDwk = sFileNoExt + "1.dwk";

            sBinaryExport.clear();
            mdl.Dwk1->Export(sBinaryExport);

            file.open(cDwk, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cDwk << ". Aborting.\n";
            else{
                /// Write and close file
                file<<sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
        if(mdl.Dwk2){
            std::string cDwk = sFileNoExt + "2.dwk";

            sBinaryExport.clear();
            mdl.Dwk2->Export(sBinaryExport);

            file.open(cDwk, std::ios::binary | std::fstream::out);

            if(!file.is_open()) std::cout<<"File creation failed for " << cDwk << ". Aborting.\n";
            else{
                /// Write and close file
                file << sBinaryExport;
                file.close();

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
    }
    else return false;
    return true;
}

bool FileEditor(HWND hwnd, int nID, std::string & cFile){
    OPENFILENAME ofn;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    std::string cExt;
    bool bReturn = false;
    const unsigned int MAX_PATH_MASS = 40000;
    ZeroMemory(&ofn, sizeof(ofn));    ofn.lStructSize = sizeof(ofn);    ofn.hwndOwner = hwnd;
    ofn.lpstrDefExt = "mdl";
    ofn.lpstrFile = nullptr;    ofn.nMaxFile = MAX_PATH;    ofn.lpstrFilter = "MDL Format (*.mdl)\0*.mdl\0";    ofn.nFilterIndex = 1;    ofn.lpstrFileTitle = NULL;    ofn.nMaxFileTitle = 0;    ofn.lpstrInitialDir = NULL;

    cFile.resize(MAX_PATH);
    if(nID == IDM_ASCII_SAVE){        ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path        ofn.lpstrFilter = "ASCII MDL Format (*.mdl)\0*.mdl\0";        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout << "\nSelected File:\n" << cFile.c_str() << "\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT && strlen(cFile.c_str()) + 5 > MAX_PATH) {
                Error("The specified file is not an .mdl file! Unable to save!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1);
            cFile = sFileNoExt + std::string(".mdl");

            bReturn = SaveFiles(Model, sFileNoExt, IDM_ASCII_SAVE);
            //if(bReturn) Model.SetFilePath(cFile);        }
        else ReportGetOpenFileNameError();
    }
    else if(nID == IDM_BIN_SAVE){        ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path        ofn.lpstrFilter = "Binary MDL Format (*.mdl)\0*.mdl\0";        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;        if(GetSaveFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT && strlen(cFile.c_str()) + 5 > MAX_PATH) {
                Error("The specified file is not an .mdl file! Unable to save!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1);
            cFile = sFileNoExt + std::string(".mdl");

            bReturn = SaveFiles(Model, sFileNoExt, IDM_BIN_SAVE);
            //if(bReturn) Model.SetFilePath(cFile);        }
        else ReportGetOpenFileNameError();
    }
    else if(nID == IDM_MDL_OPEN){        ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path
        std::cout << "Default file: " << cFile << "\n";        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;        if(GetOpenFileName(&ofn)){            std::cout<<"\nSelected File:\n"<<cFile.c_str()<<"\n";

            //First figure out if we're opening a .mdl.
            if (ofn.Flags & OFN_EXTENSIONDIFFERENT) {
                Error("The specified file is not an .mdl file!");
                return false;
            }
            std::string sFileNoExt = cFile.c_str();
            if (ofn.nFileExtension != 0) sFileNoExt = safesubstr(cFile, 0, ofn.nFileExtension - 1).c_str();
            cFile = sFileNoExt + std::string(".mdl");

            bool bAscii = false;
            bReturn = LoadFiles(Model, sFileNoExt, bAscii);

            if(bAscii){
                /// Process the data
                Model.SetFilePath(cFile);
                if(DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 1)){
                    bReturn = true;
                }
                else{
                    /// We failed reading the ascii, so we need to clean up
                    TabCtrl_DeleteAllItems(hTabs);
                    TreeView_DeleteAllItems(hTree);
                    Model.FlushData();
                    Edit1.LoadData();
                    ProcessTreeAction(NULL, ACTION_UPDATE_DISPLAY, nullptr);
                    bReturn = false;
                }
            }
            else{
                /// Process the data
                if(DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, 2)){
                    bReturn = true;
                }
                else{
                    /// Something failed, cleanup
                    Model.FlushData();
                    Edit1.LoadData();
                    ProcessTreeAction(NULL, ACTION_UPDATE_DISPLAY, nullptr);
                    bReturn = false;
                }
            }        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_MASS_TO_ASCII){
        std::string sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front(); //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileName(&ofn)){
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += "\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::string(lpStrFiles);
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 1);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nConverted files in: "<< sFolder <<"\n";
            else std::cout << "\nConverted file: "<< cFile <<"\n";        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_MASS_TO_BIN){
        std::string sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front();        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileName(&ofn)){
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += "\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::string(lpStrFiles);
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 2);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nConverted files in: "<< sFolder <<"\n";
            else std::cout << "\nConverted file: "<< cFile <<"\n";        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_MASS_ANALYZE){
        std::string sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front(); //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileName(&ofn)){
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += "\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::string(lpStrFiles);
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 3);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nAnalyzed files in: "<< sFolder <<"\n";
            else std::cout << "\nAnalyzed file: "<< cFile <<"\n";        }
        else ReportGetOpenFileNameError();    }
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

            bCancelMass = false;

            if(lParam == 1) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMassAscii, hwnd, 0, NULL);
            else if(lParam == 2) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMassBinary, hwnd, 0, NULL);
            else if(lParam == 3) hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMassAnalyze, hwnd, 0, NULL);
            else EndDialog(hwnd, NULL);
        }
        break;
        case WM_COMMAND:
        {
            if(wParam == IDCANCEL){
                //std::cout << "Escape pressed! Canceling the processing!\n";
                bCancelMass = true;
            }
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
    UpdateWindow(hProgress);
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
        std::cout << "About to build wok tree.\n";
        BuildTree(*Model.Wok);
    }
    std::cout<<"Data loaded!\n";

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadProcessBinary(LPVOID lpParam){
    try{
        Model.DecompileModel();
    }
    catch(const std::exception & e){
        std::cout << "Model decompilation for " << Model.GetFilename() << " failed with the following exception:\n" << e.what() << "\n";
        throw e;
    }
    catch(...){
        std::cout << "Model decompilation for " << Model.GetFilename() << " failed with an unknown exception.\n";
    }

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

    int nCounter = 0;
    while(*lpStrFiles != 0 && !bCancelMass){
        std::string sCurrentFile = std::string();
        if(sFolder.back() == '\\'){
            sCurrentFile += sFolder;
            sCurrentFile += lpStrFiles;
        }
        else sCurrentFile += sFolder;
        lpStrFiles = lpStrFiles + (strlen(lpStrFiles) + 1);

        std::string sFileNoExt = sCurrentFile.c_str();
        if(safesubstr(sCurrentFile, sCurrentFile.size() - 4, 4) == ".mdl") sFileNoExt = safesubstr(sCurrentFile, 0, sCurrentFile.size() - 4);

        std::string sStatic = "Processing " + std::string(PathFindFileName(sFileNoExt.c_str())) + "...";
        SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

        tempModel.FlushAll();

        bool bAscii = false;
        if(LoadFiles(tempModel, sFileNoExt, bAscii)){
            try{
                if(bAscii){
                    if(!tempModel.ReadAscii()) throw;
                    if(!tempModel.Compile()) throw;
                }
                else{
                    tempModel.DecompileModel();
                    if(tempModel.Wok){
                        tempModel.Wok->ProcessBWM();
                        tempModel.GetLytPositionFromWok();
                    }
                    if(tempModel.Pwk) tempModel.Pwk->ProcessBWM();
                    if(tempModel.Dwk0) tempModel.Dwk0->ProcessBWM();
                    if(tempModel.Dwk1) tempModel.Dwk1->ProcessBWM();
                    if(tempModel.Dwk2) tempModel.Dwk2->ProcessBWM();
                }
            }
            catch(...){
                std::cout << "Model (de)compilation for " << Model.GetFilename() << " failed.\n";
                nCounter++;
                SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
                continue;
            }

            sFileNoExt += "-mdledit-ascii";
            sCurrentFile = sFileNoExt + std::string(".mdl");

            SaveFiles(tempModel, sFileNoExt, IDM_ASCII_SAVE);
        }

        nCounter++;
        SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
        UpdateWindow(hProgressMass);
    }

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}

DWORD WINAPI ThreadMassAnalyze(LPVOID lpParam){
    int nMax = 0;
    char * lpstrCounter = lpStrFiles;
    while(*lpstrCounter != 0){
        lpstrCounter = lpstrCounter + (strlen(lpstrCounter) + 1);
        nMax++;
    }

    SendMessage(hProgressMass, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, nMax));

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

    /// Define lists of model names that have some property.
    std::array<std::vector<std::string>, 32> HasEmitterFlag;
    std::array<std::vector<std::string>, 8> HasClassification;
    std::vector<std::string> HasDepthTexture, HasRenderOrder, HasFrameBlending, HasFlareSize, HasFlarePosition, HasFlareTexture, HasFlareColor,
                             HasClassUnknown1, HasClassUnknown2, HasClassUnknown3, HasNoClassification, HasMultipleClassification, HasDirtEnabled,
                             HasBeaming, HasBackgroundGeometry;

    int nCounter = 0;
    while(*lpStrFiles != 0 && !bCancelMass){
        std::string sCurrentFile = std::string();
        if(sFolder.back() == '\\'){
            sCurrentFile += sFolder;
            sCurrentFile += lpStrFiles;
        }
        else sCurrentFile += sFolder;

        /// Make lpStrFiles point to next filename
        lpStrFiles = lpStrFiles + (strlen(lpStrFiles) + 1);

        std::string sFileNoExt = sCurrentFile.c_str();
        if(safesubstr(sCurrentFile, sCurrentFile.size() - 4, 4) == ".mdl") sFileNoExt = safesubstr(sCurrentFile, 0, sCurrentFile.size() - 4);

        std::string sStatic = "Analyzing " + std::string(PathFindFileName(sFileNoExt.c_str())) + "...";
        SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

        tempModel.FlushAll();

        bool bAscii = false;
        if(LoadFiles(tempModel, sFileNoExt, bAscii)){
            if(!bAscii){
                try{
                    tempModel.DecompileModel();
                    if(tempModel.Wok){
                        tempModel.Wok->ProcessBWM();
                        tempModel.GetLytPositionFromWok();
                    }
                    if(tempModel.Pwk) tempModel.Pwk->ProcessBWM();
                    if(tempModel.Dwk0) tempModel.Dwk0->ProcessBWM();
                    if(tempModel.Dwk1) tempModel.Dwk1->ProcessBWM();
                    if(tempModel.Dwk2) tempModel.Dwk2->ProcessBWM();
                }
                catch(...){
                    std::cout << "Model decompilation for " << Model.GetFilename() << " failed.\n";
                    nCounter++;
                    SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
                    UpdateWindow(hProgressMass);
                    continue;
                }

                tempModel.CheckPeculiarities(); //Finally, check for peculiarities

                /// Decompilation should now be done. Here we will perform our tests.
                ModelHeader & Data = tempModel.GetFileData()->MH;

                bool bFoundClassification = false;
                bool bAdded = false;
                for(int ef = 0; ef < HasClassification.size(); ef++){
                    if(Data.nClassification & pown(2, ef)){
                        HasClassification.at(ef).push_back(Data.GH.sName.c_str());
                        if(bFoundClassification && !bAdded){
                            HasMultipleClassification.push_back(Data.GH.sName.c_str());
                            bAdded = true;
                        }
                        else if(!bFoundClassification) bFoundClassification = true;
                    }
                }
                if(!bFoundClassification){
                    HasNoClassification.push_back(Data.GH.sName.c_str());
                }
                if(Data.nSubclassification != 0) HasClassUnknown1.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nSubclassification) + std::string(")"));
                if(Data.nUnknown != 0) HasClassUnknown2.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nUnknown) + std::string(")"));
                if(Data.nAffectedByFog != 1) HasClassUnknown3.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nAffectedByFog) + std::string(")"));

                for(int nNode = 0; nNode < Data.ArrayOfNodes.size(); nNode++){
                    Node & node = Data.ArrayOfNodes.at(nNode);
                    if(node.Head.nType & NODE_EMITTER){
                        for(int ef = 0; ef < HasEmitterFlag.size(); ef++){
                            if(node.Emitter.nFlags & pown(2, ef)) HasEmitterFlag.at(ef).push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str());
                        }
                        if(node.Emitter.nFrameBlending != 0) HasFrameBlending.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Emitter.nFrameBlending) + std::string(")"));
                        if(node.Emitter.cDepthTextureName.c_str() != std::string("NULL") &&
                           node.Emitter.cDepthTextureName.c_str() != std::string("") ) HasDepthTexture.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + node.Emitter.cDepthTextureName.c_str() + std::string(")"));
                        if(node.Emitter.nRenderOrder != 0) HasRenderOrder.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string(node.Emitter.nRenderOrder) + std::string(")"));
                    }
                    if(node.Head.nType & NODE_LIGHT){
                        if(node.Light.FlareSizes.size() > 0) HasFlareSize.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Light.FlareSizes.size()) + std::string(")"));
                        if(node.Light.FlarePositions.size() > 0) HasFlarePosition.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Light.FlarePositions.size()) + std::string(")"));
                        if(node.Light.FlareTextureNames.size() > 0) HasFlareTexture.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Light.FlareTextureNames.size()) + std::string(")"));
                        if(node.Light.FlareColorShifts.size() > 0) HasFlareColor.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Light.FlareColorShifts.size()) + std::string(")"));
                    }
                    if(node.Head.nType & NODE_MESH){
                        if(node.Mesh.nDirtEnabled != 0) HasDirtEnabled.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Mesh.nDirtEnabled) + std::string(")"));
                        if(node.Mesh.nBeaming != 0) HasBeaming.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Mesh.nBeaming) + std::string(")"));
                        if(node.Mesh.nBackgroundGeometry != 0) HasBackgroundGeometry.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Mesh.nBackgroundGeometry) + std::string(")"));
                    }
                }
            }
        }

        nCounter++;
        SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
        UpdateWindow(hProgressMass);
    }

    /// Now, let's report our findings.
    if(sFolder.back() == '\\'){
        std::ofstream reportfile(sFolder + "mdl_analysis.txt", std::fstream::out);

        if(true){
            reportfile << "FLAG STATS\n\n";
            reportfile << "dirt_enabled > 0:\n";
            for(int n = 0; n < HasDirtEnabled.size(); n++) reportfile << "    " << HasDirtEnabled.at(n) << "\n";
            reportfile << "beaming > 0:\n";
            for(int n = 0; n < HasBeaming.size(); n++) reportfile << "    " << HasBeaming.at(n) << "\n";
            reportfile << "backgroundgeometry > 0:\n";
            for(int n = 0; n < HasBackgroundGeometry.size(); n++) reportfile << "    " << HasBackgroundGeometry.at(n) << "\n";
        }
        if(false){
            reportfile << "\nCLASSIFICATION STATS\n\n";
            reportfile << "Classification 0:\n";
            for(int n = 0; n < HasNoClassification.size(); n++) reportfile << "    " << HasNoClassification.at(n) << "\n";
            for(int ef = 0; ef < HasClassification.size(); ef++){
                reportfile << "Classification " << pown(2, ef) << ":\n";
                for(int n = 0; n < HasClassification.at(ef).size(); n++) reportfile << "    " << HasClassification.at(ef).at(n) << "\n";
            }
            reportfile << "Mixed Classification:\n";
            for(int n = 0; n < HasMultipleClassification.size(); n++) reportfile << "    " << HasMultipleClassification.at(n) << "\n";
        }
        if(false){
            reportfile << "\nCLASSIFICATION BYTE STATS\n\n";
            reportfile << "Classification byte 2:\n";
            for(int n = 0; n < HasClassUnknown1.size(); n++) reportfile << "    " << HasClassUnknown1.at(n) << "\n";
            reportfile << "Classification byte 3:\n";
            for(int n = 0; n < HasClassUnknown2.size(); n++) reportfile << "    " << HasClassUnknown2.at(n) << "\n";
            reportfile << "Classification byte 4:\n";
            for(int n = 0; n < HasClassUnknown3.size(); n++) reportfile << "    " << HasClassUnknown3.at(n) << "\n";
        }
        if(false){
            reportfile << "\nEMITTER STATS\n\n";
            for(int ef = 0; ef < HasEmitterFlag.size(); ef++){
                reportfile << "Emitter flag " << ef << ":\n";
                for(int n = 0; n < HasEmitterFlag.at(ef).size(); n++) reportfile << "    " << HasEmitterFlag.at(ef).at(n) << "\n";
            }
            reportfile << "Frame Blending:\n";
            for(int n = 0; n < HasFrameBlending.size(); n++) reportfile << "    " << HasFrameBlending.at(n) << "\n";
            reportfile << "Depth Texture:\n";
            for(int n = 0; n < HasDepthTexture.size(); n++) reportfile << "    " << HasDepthTexture.at(n) << "\n";
            reportfile << "Render Order:\n";
            for(int n = 0; n < HasRenderOrder.size(); n++) reportfile << "    " << HasRenderOrder.at(n) << "\n";
        }
        if(false){
            reportfile << "\nLENS FLARE STATS\n\n";
            reportfile << "Flare Sizes:\n";
            for(int n = 0; n < HasFlareSize.size(); n++) reportfile << "    " << HasFlareSize.at(n) << "\n";
            reportfile << "Flare Positions:\n";
            for(int n = 0; n < HasFlarePosition.size(); n++) reportfile << "    " << HasFlarePosition.at(n) << "\n";
            reportfile << "Flare Textures:\n";
            for(int n = 0; n < HasFlareTexture.size(); n++) reportfile << "    " << HasFlareTexture.at(n) << "\n";
            reportfile << "Flare Color Shifts:\n";
            for(int n = 0; n < HasFlareColor.size(); n++) reportfile << "    " << HasFlareColor.at(n) << "\n";
        }

        reportfile.close();
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

    int nCounter = 0;
    while(*lpStrFiles != 0 && !bCancelMass){
        //ProgressSize(0, 1);
        ProgressPos(0);
        //std::string sCurrentFile = lpStrFiles;
        std::string sCurrentFile = std::string();
        if(sFolder.back() == '\\'){
            sCurrentFile += sFolder;
            sCurrentFile += lpStrFiles;
        }
        else sCurrentFile += sFolder;
        lpStrFiles = lpStrFiles + (strlen(lpStrFiles) + 1);

        std::string sFileNoExt = sCurrentFile.c_str();
        if(safesubstr(sCurrentFile, sCurrentFile.size() - 4, 4) == ".mdl") sFileNoExt = safesubstr(sCurrentFile, 0, sCurrentFile.size() - 4);

        std::string sStatic = "Processing " + std::string(PathFindFileName(sFileNoExt.c_str())) + "...";
        SetWindowText(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

        tempModel.FlushAll();

        bool bAscii = false;
        if(LoadFiles(tempModel, sFileNoExt, bAscii)){
            try{
                if(bAscii){
                    if(!tempModel.ReadAscii()) throw;
                    if(!tempModel.Compile()) throw;
                }
                else{
                    tempModel.DecompileModel();
                    if(tempModel.Wok){
                        tempModel.Wok->ProcessBWM();
                        tempModel.GetLytPositionFromWok();
                    }
                    if(tempModel.Pwk) tempModel.Pwk->ProcessBWM();
                    if(tempModel.Dwk0) tempModel.Dwk0->ProcessBWM();
                    if(tempModel.Dwk1) tempModel.Dwk1->ProcessBWM();
                    if(tempModel.Dwk2) tempModel.Dwk2->ProcessBWM();
                }
            }
            catch(...){
                std::cout << "Model (de)compilation for " << Model.GetFilename() << " failed.\n";
                nCounter++;
                SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
                continue;
            }

            sFileNoExt += "-mdledit-bin";
            sCurrentFile = sFileNoExt + std::string(".mdl");

            SaveFiles(tempModel, sFileNoExt, IDM_BIN_SAVE);
        }

        nCounter++;
        SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
    }

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}
