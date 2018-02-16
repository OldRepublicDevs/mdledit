#include "frame.h"
//#include "edits.h"
#include <Shlwapi.h>
#include <fstream>
#include <algorithm>
#include "MDL.h"

extern Edits Edit1;
HANDLE hThread;
HWND hProgress;
HWND hProgressMass;
wchar_t * lpStrFiles = nullptr;
std::wstring sFolder;
DWORD WINAPI ThreadProcessing(LPVOID lpParam);
bool bCancelMass = false;
bool bCancelSG = false;
bool bDotAsciiDefault = true;

enum MdlProcessing {
    PROCESSING_ASCII,
    PROCESSING_BINARY,
    PROCESSING_MASS_ASCII,
    PROCESSING_MASS_BINARY,
    PROCESSING_MASS_ANALYZE
} CurrentProcess;

void ReportGetOpenFileNameError(){
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

bool LoadFiles(MDL & mdl, const std::wstring & sFileNoExt, bool & bAscii){
    bool bDisplay = false;
    if(&mdl == &Model) bDisplay = true;
    ReportObject ReportMdl(mdl);

    /// Create file
    std::wstring sMdl = sFileNoExt + (bAscii ? L".mdl.ascii" : L".mdl");

    //std::ifstream file(sMdl, std::ios::binary);
    HANDLE file = bead_CreateReadFile(sMdl);

    if(file == INVALID_HANDLE_VALUE){ //if(!file.is_open()){
        ReportMdl << "File creation/opening failed for " << to_ansi(sMdl) << ". Aborting.\n";
        return false;
    }

    /// If everything checks out, we may begin reading
    //file.seekg(0, std::ios::end);
    //std::streampos filelength = file.tellg();
    unsigned long filelength = bead_GetFileLength(file);
    if(filelength < 12){
        ReportMdl << "File too short. Aborting.\n";
        return false;
    }

    /// First check whether it's an ascii or a binary
    //file.seekg(0,std::ios::beg);
    std::vector<char> cBinary (4, 0);
    //char cBinary [4];
    //file.read(cBinary, 4);
    bead_ReadFile(file, cBinary, 4);
    if(cBinary[0] != '\0' && cBinary[1] != '\0' && cBinary[2] != '\0' && cBinary[3] != '\0'){
        bAscii = true;
    }
    else{
        if(bAscii){
            if(bDisplay) Error("The selected .ascii file does not seem to be in the ASCII MDL format!");
            else ReportMdl << "The selected .ascii file does not seem to be in the ASCII MDL format!\n";
            return false;
        }
        bAscii = false;
    }

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
        ReportMdl << "Reading ascii...\n";
        //file.seekg(0, std::ios::beg);
        mdl.Ascii.reset(new ASCII());
        std::vector<char> & sBufferRef = mdl.CreateAsciiBuffer(filelength);
        //file.read(&sBufferRef[0], filelength);
        bead_ReadFile(file, sBufferRef);
        //file.close();
        CloseHandle(file);
        if(bDisplay){
            TabCtrl_AppendTab(hTabs, "MDL");
            TabCtrl_AppendTab(hTabs, "MDX");
        }

        /// Open and process .pwk if it exists
        std::wstring cPwk;
        if(bDotAsciiDefault && PathFileExistsW(std::wstring(sFileNoExt + L".pwk.ascii").c_str())) cPwk = sFileNoExt + L".pwk.ascii";
        else if(PathFileExistsW(std::wstring(sFileNoExt + L".pwk").c_str())) cPwk = sFileNoExt + L".pwk";
        else if(PathFileExistsW(std::wstring(sFileNoExt + L".pwk.ascii").c_str())) cPwk = sFileNoExt + L".pwk.ascii";
        if(cPwk != L""){
            //file.open(cPwk, std::ifstream::binary);
            file = bead_CreateReadFile(cPwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed (pwk) for " << to_ansi(cPwk) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cPwk) << " too short. Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryPwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryPwk, 8);
                    if(std::string(&cBinaryPwk.front(), 8) != "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.PwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferPwk = mdl.PwkAscii->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferPwk);
                        mdl.PwkAscii->SetFilePath(cPwk);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "PWK");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }

        //Open and process .dwk if it exists
        std::wstring cDwk = L"";
        if(bDotAsciiDefault && PathFileExistsW(std::wstring(sFileNoExt + L".dwk.ascii").c_str())) cDwk = sFileNoExt + L".dwk.ascii";
        else if(PathFileExistsW(std::wstring(sFileNoExt + L".dwk").c_str())) cDwk = sFileNoExt + L".dwk";
        else if(PathFileExistsW(std::wstring(sFileNoExt + L".dwk.ascii").c_str())) cDwk = sFileNoExt + L".dwk.ascii";
        if(cDwk != L""){
            //file.open(cDwk);
            file = bead_CreateReadFile(cDwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cDwk) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cDwk) << " too short. Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryDwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryDwk, 8);
                    if(std::string(&cBinaryDwk.front(), 8) != "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.DwkAscii.reset(new ASCII());
                        std::vector<char> & sBufferDwk = mdl.DwkAscii->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferDwk);
                        mdl.DwkAscii->SetFilePath(cDwk);
                        if(bDisplay){
                            TabCtrl_AppendTab(hTabs, "DWK 0");
                            TabCtrl_AppendTab(hTabs, "DWK 1");
                            TabCtrl_AppendTab(hTabs, "DWK 2");
                        }
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }
    }
    else{
        ReportMdl << "Reading binary...\n";

        //file.seekg(0,std::ios::beg);
        std::vector<char> & sBufferMdl = mdl.CreateBuffer(filelength);
        //file.read(&sBufferMdl[0], filelength);
        //file.close();
        bead_ReadFile(file, sBufferMdl);
        CloseHandle(file);
        if(bDisplay) TabCtrl_AppendTab(hTabs, "MDL");

        /// Open and process .mdx if it exists
        std::wstring cMdx = sFileNoExt + L".mdx";
        if(PathFileExistsW(cMdx.c_str())){
            //file.open(cMdx, std::ios::binary);
            file = bead_CreateReadFile(cMdx);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cMdx) << ". Aborting.\n";
            }
            else{
                //We may begin reading
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0,std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                mdl.Mdx.reset(new MDX());
                std::vector<char> & sBufferRef = mdl.Mdx->CreateBuffer(length);
                //file.read(&sBufferRef[0], length);
                bead_ReadFile(file, sBufferRef);
                //file.close();
                CloseHandle(file);
                mdl.Mdx->SetFilePath(cMdx);
                if(bDisplay) TabCtrl_AppendTab(hTabs, "MDX");
            }
        }
        else if(bDisplay){            PathStripPathW(&cMdx.front());
            Warning("Could not find " + std::string(std::string(cMdx.begin(), cMdx.end()).c_str()) + " in the same directory. Will load without the MDX data.");
        }

        /// Open and process .wok if it exists
        std::wstring cWok = sFileNoExt + L".wok";
        if(PathFileExistsW(cWok.c_str())){
            //file.open(cWok, std::ios::binary);
            file = bead_CreateReadFile(cWok);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cWok) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cWok) << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryWok (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryWok, 8);
                    if(std::string(&cBinaryWok.front(), 8) == "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.Wok.reset(new WOK());
                        std::vector<char> & sBufferRef = mdl.Wok->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferRef);
                        mdl.Wok->SetFilePath(cWok);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "WOK");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }

        //Open and process .pwk if it exists
        std::wstring cPwk = sFileNoExt + L".pwk";
        if(PathFileExistsW(cPwk.c_str())){
            //file.open(cPwk, std::ios::binary);
            file = bead_CreateReadFile(cPwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cPwk) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cPwk) << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryPwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryPwk, 8);
                    if(std::string(&cBinaryPwk.front(), 8) == "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.Pwk.reset(new PWK());
                        std::vector<char> & sBufferRef = mdl.Pwk->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferRef);
                        mdl.Pwk->SetFilePath(cPwk);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "PWK");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }

        //Open and process .dwk if it exists
        std::wstring cDwk = sFileNoExt + L"0.dwk";
        if(PathFileExistsW(cDwk.c_str())){
            //file.open(cDwk, std::ios::binary);
            file = bead_CreateReadFile(cDwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cDwk) << ". Aborting.\n";
            }
            else{
                bAscii = false;
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cDwk) << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryDwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryDwk, 8);
                    if(std::string(&cBinaryDwk.front(), 8) == "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.Dwk0.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk0->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferRef);
                        mdl.Dwk0->SetFilePath(cDwk);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "DWK 0");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }
        cDwk = sFileNoExt + L"1.dwk";
        if(PathFileExistsW(cDwk.c_str())){
            //file.open(cDwk, std::ios::binary);
            file = bead_CreateReadFile(cDwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cDwk) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cDwk) << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryDwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryDwk, 8);
                    if(std::string(&cBinaryDwk.front(), 8) == "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.Dwk1.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk1->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferRef);
                        mdl.Dwk1->SetFilePath(cDwk);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "DWK 1");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }
        cDwk = sFileNoExt + L"2.dwk";
        if(PathFileExistsW(cDwk.c_str())){
            //file.open(cDwk, std::ios::binary);
            file = bead_CreateReadFile(cDwk);
            //if(!file.is_open()){
            if(file == INVALID_HANDLE_VALUE){
                ReportMdl << "File creation/opening failed for " << to_ansi(cDwk) << ". Aborting.\n";
            }
            else{
                //file.seekg(0, std::ios::end);
                //std::streampos length = file.tellg();
                //file.seekg(0, std::ios::beg);
                unsigned long length = bead_GetFileLength(file);
                if(length < 12) ReportMdl << "File " << to_ansi(cDwk) << " too short! Aborting.\n";
                else{
                    /// First check whether it's an ascii or a binary
                    //char cBinary [8];
                    std::vector<char> cBinaryDwk (8, 0);
                    //file.read(cBinary, 8);
                    bead_ReadFile(file, cBinaryDwk, 8);
                    if(std::string(&cBinaryDwk.front(), 8) == "BWM V1.0"){
                        /// We may begin reading
                        //file.seekg(0, std::ios::beg);
                        mdl.Dwk2.reset(new DWK());
                        std::vector<char> & sBufferRef = mdl.Dwk2->CreateBuffer(length);
                        //file.read(&sBufferRef[0], length);
                        bead_ReadFile(file, sBufferRef);
                        mdl.Dwk2->SetFilePath(cDwk);
                        if(bDisplay) TabCtrl_AppendTab(hTabs, "DWK 2");
                    }
                }
                //file.close();
                CloseHandle(file);
            }
        }
    }

    return true;
}

bool SaveFiles(MDL & mdl, const std::wstring & sFileNoExt, int nID, bool bAscii = false){
    std::wstring sMdl = sFileNoExt + L".mdl";
    ReportObject ReportMdl(mdl);
    if(nID == IDM_ASCII_SAVE){
        if(bAscii) sMdl += L".ascii";

        /// Convert the data and put it into a string
        std::string sAsciiExport;
        mdl.ExportAscii(sAsciiExport);

        /// Create file
        //std::ofstream file(sMdl, std::fstream::out);
        HANDLE file = bead_CreateWriteFile(sMdl);

        //if(!file.is_open()){
        if(file == INVALID_HANDLE_VALUE){
            ReportMdl << "File creation failed for " << to_ansi(sMdl) << ". Aborting.\n";
            return false;
        }

        /// Write and close file
        //file << sAsciiExport;
        bead_WriteFile(file, sAsciiExport);
        //file.close();
        CloseHandle(file);

        sAsciiExport.clear();
        sAsciiExport.shrink_to_fit();

        /// Save Pwk
        if(mdl.Pwk){
            std::wstring cPwk = sFileNoExt + L".pwk";
            if(bAscii) cPwk += L".ascii";

            sAsciiExport.clear();
            mdl.ExportPwkAscii(sAsciiExport);

            //file.open(cPwk, std::fstream::out);
            file = bead_CreateWriteFile(cPwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cPwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sAsciiExport;
                bead_WriteFile(file, sAsciiExport);
                //file.close();
                CloseHandle(file);

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }

        /// Save Dwk
        if(mdl.Dwk0 || mdl.Dwk1 || mdl.Dwk2){
            std::wstring cDwk = sFileNoExt + L".dwk";
            if(bAscii) cDwk += L".ascii";

            sAsciiExport.clear();
            mdl.ExportDwkAscii(sAsciiExport);

            //file.open(cDwk, std::fstream::out);
            file = bead_CreateWriteFile(cDwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cDwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sAsciiExport;
                bead_WriteFile(file, sAsciiExport);
                //file.close();
                CloseHandle(file);

                sAsciiExport.clear();
                sAsciiExport.shrink_to_fit();
            }
        }

        //Save Wok
        if(mdl.Wok && mdl.bExportWok){
            std::wstring cWok = sFileNoExt + L".wok";
            if(bAscii) cWok += L".ascii";

            sAsciiExport.clear();
            mdl.ExportWokAscii(sAsciiExport);

            //file.open(cWok, std::fstream::out);
            file = bead_CreateWriteFile(cWok);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cWok) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sAsciiExport;
                bead_WriteFile(file, sAsciiExport);
                //file.close();
                CloseHandle(file);

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
        //std::ofstream file(sMdl, std::ios::binary | std::fstream::out);
        HANDLE file = bead_CreateWriteFile(sMdl);

        //if(!file.is_open()){
        if(file == INVALID_HANDLE_VALUE){
            ReportMdl << "File creation failed for " << to_ansi(sMdl) << ". Aborting.\n";
            return false;
        }

        /// Write and close file
        //file << sBinaryExport;
        bead_WriteFile(file, sBinaryExport);
        //file.close();
        CloseHandle(file);

        /// Save mdx
        if(mdl.Mdx){
            std::wstring cMdx = sFileNoExt + L".mdx";

            sBinaryExport.clear();
            mdl.Mdx->Export(sBinaryExport);

            //file.open(cMdx, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cMdx);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cMdx) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Wok
        if(mdl.Wok){
            std::wstring cWok = sFileNoExt + L".wok";

            sBinaryExport.clear();
            mdl.Wok->Export(sBinaryExport);

            //file.open(cWok, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cWok);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cWok) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Pwk
        if(mdl.Pwk){
            std::wstring cPwk = sFileNoExt + L".pwk";

            sBinaryExport.clear();
            mdl.Pwk->Export(sBinaryExport);

            //file.open(cPwk, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cPwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cPwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }

        /// Save Dwk
        if(mdl.Dwk0){
            std::wstring cDwk = sFileNoExt + L"0.dwk";

            sBinaryExport.clear();
            mdl.Dwk0->Export(sBinaryExport);

            //file.open(cDwk, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cDwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cDwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
        if(mdl.Dwk1){
            std::wstring cDwk = sFileNoExt + L"1.dwk";

            sBinaryExport.clear();
            mdl.Dwk1->Export(sBinaryExport);

            //file.open(cDwk, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cDwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cDwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
        if(mdl.Dwk2){
            std::wstring cDwk = sFileNoExt + L"2.dwk";

            sBinaryExport.clear();
            mdl.Dwk2->Export(sBinaryExport);

            //file.open(cDwk, std::ios::binary | std::fstream::out);
            file = bead_CreateWriteFile(cDwk);

            //if(!file.is_open())
            if(file == INVALID_HANDLE_VALUE)
                ReportMdl << "File creation failed for " << to_ansi(cDwk) << ". Aborting.\n";
            else{
                /// Write and close file
                //file << sBinaryExport;
                bead_WriteFile(file, sBinaryExport);
                //file.close();
                CloseHandle(file);

                sBinaryExport.clear();
                sBinaryExport.shrink_to_fit();
            }
        }
    }
    else return false;
    return true;
}

bool FileEditor(HWND hwnd, int nID, std::wstring & cFile){
    OPENFILENAMEW ofn;
    std::string cExt;
    bool bReturn = false;
    const unsigned int MAX_PATH_MASS = 0x0FFFFFFF;
    ZeroMemory(&ofn, sizeof(ofn));    ofn.lStructSize = sizeof(ofn);    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = nullptr;    ofn.nMaxFile = MAX_PATH;    ofn.lpstrFilter = L"MDL Format (*.mdl, *.mdl.ascii)\0*.mdl;*.mdl.ascii\0";    ofn.nFilterIndex = 1;

    cFile.resize(MAX_PATH);
    if(nID == IDM_ASCII_SAVE){
        bool bValidFileName = false;
        while(!bValidFileName){            ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path            ofn.lpstrFilter = L"ASCII MDL Format (*.mdl)\0*.mdl\0ASCII MDL Format (*.mdl.ascii)\0*.mdl.ascii\0";            ofn.nFilterIndex = bDotAsciiDefault ? 2 : 1;            ofn.Flags = OFN_PATHMUSTEXIST;            if(GetSaveFileNameW(&ofn)){                std::cout << "Selected File:\n" << to_ansi(cFile.c_str()) << "\n";
                if(ofn.nFileExtension != 0) std::cout << "File extension: " << to_ansi(&cFile[ofn.nFileExtension - 1]) << "\n";

                /// The bAscii bool will (at this point) tell us, whether we're saving as .mdl or as .mdl.ascii.
                bool bAscii = false;
                if(ofn.nFilterIndex == 2) bAscii = true;
                std::wstring sFileNoExt = cFile.c_str();

                /// If the user manually entered a different extension, this overrides the currently selected one
                //if(safesubstr(sFileNoExt, sFileNoExt.size() - 10, 10) == ".mdl.ascii") bAscii = true;
                //if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == ".mdl") bAscii = false;

                /// Remove .ascii and .mdl extensions.
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 6, 6) == L".ascii") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 6);
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 4);

                /// Re-construct the save file name
                if(bAscii) cFile = sFileNoExt + std::wstring(L".mdl.ascii");
                else cFile = sFileNoExt + std::wstring(L".mdl");

                /// Now we check if this file exists already
                if(PathFileExistsW(cFile.c_str())){
                    int nDecision = WarningYesNoCancel(L"The file '" + std::wstring(PathFindFileNameW(cFile.c_str())) + L"' already exists! Do you want to overwrite?");
                    if(nDecision == IDCANCEL) return false;
                    else if(nDecision == IDNO) continue; /// This will run the file selection dialog a second time
                }
                bValidFileName = true;

                Timer t1;
                bReturn = SaveFiles(Model, sFileNoExt, IDM_ASCII_SAVE, bAscii);
                ReportModel << "Wrote files in: " << t1.GetTime() << "\n";            }
            else{
                ReportGetOpenFileNameError();
                break;
            }
        }
    }
    else if(nID == IDM_BIN_SAVE){
        bool bValidFileName = false;
        while(!bValidFileName){            ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path            ofn.lpstrFilter = L"Binary MDL Format (*.mdl)\0*.mdl\0";            ofn.Flags = OFN_PATHMUSTEXIST;            if(GetSaveFileNameW(&ofn)){                std::cout << "Selected File:\n" << to_ansi(cFile.c_str()) << "\n";
                if(ofn.nFileExtension != 0) std::cout << "File extension: " << to_ansi(&cFile[ofn.nFileExtension - 1]) << "\n";

                std::wstring sFileNoExt = cFile.c_str();

                /// Remove .ascii and .mdl extensions.
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 6, 6) == L".ascii") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 6);
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 4);

                /// Re-construct the save file name
                cFile = sFileNoExt + std::wstring(L".mdl");

                /// Now we check if this file exists already
                if(PathFileExistsW(cFile.c_str())){
                    int nDecision = WarningYesNoCancel(L"The file '" + std::wstring(PathFindFileNameW(cFile.c_str())) + L"' already exists! Do you want to overwrite?");
                    if(nDecision == IDCANCEL) return false;
                    else if(nDecision == IDNO) continue; /// This will run the file selection dialog a second time
                }
                bValidFileName = true;

                Timer t1;
                bReturn = SaveFiles(Model, sFileNoExt, IDM_BIN_SAVE);
                ReportModel << "Wrote files in: " << t1.GetTime() << "\n";            }
            else{
                ReportGetOpenFileNameError();
                break;
            }
        }
    }
    else if(nID == IDM_MDL_OPEN){
        bool bValidFileName = false;
        while(!bValidFileName){            ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;            if(GetOpenFileNameW(&ofn)){                std::cout << "Selected File:\n" << to_ansi(cFile.c_str()) << "\n";
                if(ofn.nFileExtension != 0) std::cout << "File extension: " << to_ansi(&cFile[ofn.nFileExtension - 1]) << "\n";

                std::wstring sFileNoExt = cFile.c_str();

                /// Get whether we have an .ascii extension or not
                bool bAscii = false;
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 10, 10) == L".mdl.ascii") bAscii = true;
                else if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") bAscii = false;
                else{
                    Error("The specified file is not an MDL file!");
                    continue;
                }
                bValidFileName = true;

                /// Remove .ascii and .mdl extensions.
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 6, 6) == L".ascii") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 6);
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 4);

                Timer t1;
                Model.SetFilePath(cFile);
                bReturn = LoadFiles(Model, sFileNoExt, bAscii);
                ReportModel << "Read files in: " << t1.GetTime() << "\n";

                /// Process the data
                CurrentProcess = bAscii ? PROCESSING_ASCII : PROCESSING_BINARY;
                if(DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESS), hFrame, ProgressProc, bAscii ? 1 : 2)){
                    bReturn = true;
                    ReportModel << "Total processing time: " << t1.GetTime() << "\n";
                    if(bSaveReport) Model.SaveReport();
                }
                else{
                    ReportModel << "Total processing time: " << t1.GetTime() << "\n";
                    if(bSaveReport) Model.SaveReport();

                    /// We failed reading the ascii, so we need to clean up
                    TabCtrl_DeleteAllItems(hTabs);
                    TreeView_DeleteAllItems(hTree);
                    Model.FlushData();
                    Edit1.LoadData();
                    ProcessTreeAction(NULL, ACTION_UPDATE_DISPLAY, nullptr);
                    bReturn = false;
                }            }
            else{
                ReportGetOpenFileNameError();
                break;
            }
        }    }
    else if(nID == IDM_MASS_TO_ASCII){
        std::wstring sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front(); //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileNameW(&ofn)){
            Timer tTotal;
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += L"\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::wstring(lpStrFiles);
            CurrentProcess = PROCESSING_MASS_ASCII;
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 1);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nConverted files in: " << to_ansi(sFolder) << "\n";
            else std::cout << "\nConverted file: " << to_ansi(cFile) << "\n";
            std::cout << "Total processing time: " << tTotal.GetTime() << "\n";        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_MASS_TO_BIN){
        std::wstring sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front();        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileNameW(&ofn)){
            Timer tTotal;
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += L"\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::wstring(lpStrFiles);
            CurrentProcess = PROCESSING_MASS_BINARY;
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 2);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nConverted files in: " << to_ansi(sFolder) << "\n";
            else std::cout << "\nConverted file: " << to_ansi(cFile) << "\n";
            std::cout << "Total processing time: " << tTotal.GetTime() << "\n";        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_MASS_ANALYZE){
        std::wstring sMassFiles (MAX_PATH_MASS, 0);
        int nOffsetToFirst = 0;        ofn.lpstrFile = &sMassFiles.front(); //The open dialog will update cFile with the file path        ofn.nMaxFile = MAX_PATH_MASS;        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;        if(GetOpenFileNameW(&ofn)){
            Timer tTotal;
            lpStrFiles = &sMassFiles[0] + ofn.nFileOffset;
            cFile = sMassFiles.c_str();
            if(ofn.nFileExtension == 0) cFile += L"\\";
            sFolder = cFile;
            if(ofn.nFileExtension == 0) cFile += std::wstring(lpStrFiles);
            CurrentProcess = PROCESSING_MASS_ANALYZE;
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_PROGRESSMASS), hFrame, ProgressMassProc, 3);
            bReturn = true;
            if(ofn.nFileExtension == 0) std::cout << "\nAnalyzed files in: " << to_ansi(sFolder) << "\n";
            else std::cout << "\nAnalyzed file: " << to_ansi(cFile) << "\n";
            std::cout << "Total processing time: " << tTotal.GetTime() << "\n";        }
        else ReportGetOpenFileNameError();    }
    else if(nID == IDM_BIN_COMPARE){        ofn.lpstrFile = &cFile.front(); //The open dialog will update cFile with the file path        ofn.lpstrFilter = L"Binary MDL Format (*.mdl)\0*.mdl\0";        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        bool bValidFileName = false;
        while(!bValidFileName){            if(GetOpenFileNameW(&ofn)){                std::cout << "Selected File:\n" << to_ansi(cFile.c_str()) << "\n";
                if(ofn.nFileExtension != 0) std::cout << "File extension: " << to_ansi(&cFile[ofn.nFileExtension - 1]) << "\n";

                std::wstring sFileNoExt = cFile.c_str();

                /// Check the extension.
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) != L".mdl"){
                    Error("The specified file is not a valid MDL file!");
                    continue;
                }

                /// Remove .ascii and .mdl extensions.
                if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 4);

                MDL compModel;

                Timer t1;
                compModel.SetFilePath(cFile);
                bool bAscii = false;
                bReturn = LoadFiles(compModel, sFileNoExt, bAscii);
                std::cout << "Read files in: " << t1.GetTime() << "\n";

                if(bAscii){
                    Error("The specified file is not a binary MDL file!");
                    continue;
                }
                bValidFileName = true;

                if(bReturn){
                    Model.Compare(compModel);
                    if(Model.Mdx && compModel.Mdx) Model.Mdx->Compare(*compModel.Mdx);
                    if(Model.Wok && compModel.Wok) Model.Wok->Compare(*compModel.Wok);
                    if(Model.Pwk && compModel.Pwk) Model.Pwk->Compare(*compModel.Pwk);
                    if(Model.Dwk0 && compModel.Dwk0) Model.Dwk0->Compare(*compModel.Dwk0);
                }
                std::cout << "Total processing time: " << t1.GetTime() << "\n";            }
            else{
                ReportGetOpenFileNameError();
                break;
            }
        }    }
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
            bCancelSG = false;

            hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessing, hwnd, 0, NULL);
        }
        break;
        case WM_COMMAND:
        {
            if(wParam == IDCANCEL){
                std::cout << "Escape pressed! Canceling the processing!\n";
                bCancelSG = true;
            }
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

            hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcessing, hwnd, 0, NULL);
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

DWORD WINAPI ThreadProcessing(LPVOID lpParam){
    if(CurrentProcess == PROCESSING_ASCII){
        bool bReadWell = Model.ReadAscii();
        //Just read it.
        if(!bReadWell){
            SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
            return 0;
        }

        /// Now we know whether we have WOK data
        if(Model.Wok) TabCtrl_AppendTab(hTabs, "WOK");

        Model.Compile();
        //This should bring us to a state where all data is ready,
        //even the binary-file-specific data

        //Load the data
        Timer tLoad;
        Report("Loading data...");
        SetWindowText(hDisplayEdit, "");
        Edit1.LoadData(); //Loads up the binary file onto the screen
        BuildTree(Model); //Fills the TreeView control
        if(Model.Pwk) BuildTree(*Model.Pwk);
        if(Model.Dwk0) BuildTree(*Model.Dwk0);
        if(Model.Dwk1) BuildTree(*Model.Dwk1);
        if(Model.Dwk2) BuildTree(*Model.Dwk2);
        if(Model.Wok){
            std::wstring sWok = L".wok";
            Model.Wok->SetFilePath(sWok);
            BuildTree(*Model.Wok);
        }
        ReportModel << "Data loaded in " << tLoad.GetTime() << ".\n";
    }
    else if(CurrentProcess == PROCESSING_BINARY){
        try{
            Model.DecompileModel();
        }
        catch(const std::exception & e){
            Error(L"Model decompilation for " + Model.GetFilename() + L" failed with the following exception:\n\n" + to_wide(e.what()));
            std::cout << "Model decompilation for " << to_ansi(Model.GetFilename()) << " failed with the following exception:\n" << e.what() << "\n";
            SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
            return 0;
        }
        catch(...){
            Error(L"Model decompilation for " + Model.GetFilename() + L" failed with an unknown exception.");
            std::cout << "Model decompilation for " << to_ansi(Model.GetFilename()) << " failed with an unknown exception.\n";
            SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
            return 0;
        }

        Report("Processing walkmesh...");
        try{
            if(Model.Wok){
                Model.Wok->ProcessBWM();
                Model.GetLytPositionFromWok();
            }
            if(Model.Pwk) Model.Pwk->ProcessBWM();
            if(Model.Dwk0) Model.Dwk0->ProcessBWM();
            if(Model.Dwk1) Model.Dwk1->ProcessBWM();
            if(Model.Dwk2) Model.Dwk2->ProcessBWM();
        }
        catch(const std::exception & e){
            Error(L"Walkmesh decompilation for " + Model.GetFilename() + L" failed with the following exception:\n\n" + to_wide(e.what()));
            std::cout << "Walkmesh decompilation for " << to_ansi(Model.GetFilename()) << " failed with the following exception:\n" << e.what() << "\n";
            SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
            return 0;
        }
        catch(...){
            Error(L"Walkmesh decompilation for " + Model.GetFilename() + L" failed with an unknown exception.");
            std::cout << "Walkmesh decompilation for " << to_ansi(Model.GetFilename()) << " failed with an unknown exception.\n";
            SendMessage((HWND)lpParam, 69, 1, NULL); //Abort, return error=1
            return 0;
        }

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
        ReportModel << "Data loaded!\n";

        Model.CheckPeculiarities(); //Finally, check for peculiarities
    }
    else if(CurrentProcess == PROCESSING_MASS_ASCII || CurrentProcess == PROCESSING_MASS_BINARY || CurrentProcess == PROCESSING_MASS_ANALYZE){
        int nMax = 0;
        wchar_t * lpstrCounter = lpStrFiles;
        while(*lpstrCounter != L'\0'){
            lpstrCounter = lpstrCounter + (wcslen(lpstrCounter) + 1);
            nMax++;
        }

        SendMessage(hProgressMass, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, nMax));

        /// Set tempModel and its settings
        MDL tempModel;
        ReportObject ReportTempModel(tempModel);
        tempModel.PtrReport = nullptr;
        tempModel.PtrProgressSize = ProgressSize;
        tempModel.PtrProgressPos = ProgressPos;
        tempModel.bExportWok = Model.bExportWok;
        tempModel.bDetermineSmoothing = Model.bDetermineSmoothing;
        tempModel.bLightsaberToTrimesh = Model.bLightsaberToTrimesh;
        tempModel.bSkinToTrimesh = Model.bSkinToTrimesh;
        tempModel.bBezierToLinear = Model.bBezierToLinear;
        tempModel.bSmoothAngleWeighting = Model.bSmoothAngleWeighting;
        tempModel.bSmoothAreaWeighting = Model.bSmoothAreaWeighting;
        tempModel.bWriteAnimations = Model.bWriteAnimations;
        tempModel.bMinimizeVerts = Model.bMinimizeVerts;
        tempModel.bXbox = Model.bXbox;
        tempModel.bK2 = Model.bK2;

        /// Define lists of model names that have some property.
        std::array<std::vector<std::string>, 32> HasEmitterFlag;
        std::array<std::vector<std::string>, 8> HasClassification;
        std::vector<std::string> HasDepthTexture, HasRenderOrder, HasFrameBlending, HasFlareSize, HasFlarePosition, HasFlareTexture, HasFlareColor,
                                 HasClassUnknown1, HasClassUnknown2, HasClassUnknown3, HasNoClassification, HasMultipleClassification, HasDirtEnabled,
                                 HasBeaming, HasBackgroundGeometry, HasSubclass2, HasSubclass4, HasNoSubclass4, HasBwmUnknown, HasBwmExtra;
        std::vector<std::string> CompressedOrientation, ControllerlessAnimationData, HeadLink, WalkablePwkDwk, DanglyValues, SupernodeNums, ModelNames;
        std::vector<std::vector<std::string>> BigEmitterControllers;
        std::vector<std::vector<int>> BigEmitterControllerNums;

        int nCounter = 0;
        while(*lpStrFiles != 0 && !bCancelMass){
            std::wstring sCurrentFile;
            if(sFolder.back() == L'\\'){
                sCurrentFile += sFolder;
                sCurrentFile += lpStrFiles;
            }
            else sCurrentFile += sFolder;
            lpStrFiles = lpStrFiles + (wcslen(lpStrFiles) + 1);

            std::wstring sFileNoExt = sCurrentFile.c_str();
            ModelNames.push_back(to_ansi(sFileNoExt));

            bool bAscii = false;
            if(safesubstr(sFileNoExt, sFileNoExt.size() - 10, 10) == L".mdl.ascii") bAscii = true;
            else if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") bAscii = false;
            else{
                ReportTempModel << "File " << to_ansi(Model.GetFilename().c_str()) << " is in the wrong format.\n";
                nCounter++;
                SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
                continue;
            }

            /// Remove .ascii and .mdl extensions.
            if(safesubstr(sFileNoExt, sFileNoExt.size() - 6, 6) == L".ascii") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 6);
            if(safesubstr(sFileNoExt, sFileNoExt.size() - 4, 4) == L".mdl") sFileNoExt = safesubstr(sFileNoExt, 0, sFileNoExt.size() - 4);

            std::wstring sStatic = L"Processing " + std::wstring(PathFindFileNameW(sCurrentFile.c_str())) + L"...";
            SetWindowTextW(GetDlgItem(GetParent(hProgress), DLG_ID_STATIC), sStatic.c_str());

            /// Flush tempModel
            tempModel.FlushAll();

            Timer tRead;
            tempModel.SetFilePath(sCurrentFile);
            if(LoadFiles(tempModel, sFileNoExt, bAscii)){
                ReportTempModel << "Read files in: " << tRead.GetTime() << "\n";
                try{
                    if(bAscii){
                        if(!tempModel.ReadAscii()) throw;
                        if(!tempModel.Compile()) throw;
                        //ReportTempModel << "Decompiled successfully!\n";
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
                    ReportTempModel << "Total processing time: " << tRead.GetTime() << "\n";
                }
                catch(...){
                    ReportTempModel << "Model (de)compilation for " << Model.GetFilename().c_str() << " failed.\n";
                    nCounter++;
                    SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
                    continue;
                }

                //ReportTempModel << "Saving now!\n";

                if(bSaveReport) tempModel.SaveReport();

                if(CurrentProcess == PROCESSING_MASS_ASCII){
                    sFileNoExt += L"-mdledit";
                    sCurrentFile = sFileNoExt + (bDotAsciiDefault ? std::wstring(L".mdl.ascii") : std::wstring(L".mdl"));
                    SaveFiles(tempModel, sFileNoExt, IDM_ASCII_SAVE, bDotAsciiDefault);
                }
                else if(CurrentProcess == PROCESSING_MASS_BINARY){
                    sFileNoExt += L"-mdledit";
                    sCurrentFile = sFileNoExt + std::wstring(L".mdl");
                    SaveFiles(tempModel, sFileNoExt, IDM_BIN_SAVE);
                }
                else if(CurrentProcess == PROCESSING_MASS_ANALYZE && !bAscii){
                    //tempModel.CheckPeculiarities(); //Finally, check for peculiarities

                    /// Decompilation should now be done. Here we will perform our tests.
                    ModelHeader & Data = tempModel.GetFileData()->MH;

                    /// We do BWM first
                    /* * /
                    for(int nBwm = 0; nBwm < 5; nBwm++){
                        BWMHeader * bwm_ptr = nullptr;
                        if(nBwm == 0 && tempModel.Wok) bwm_ptr = tempModel.Wok->GetData().get();
                        if(nBwm == 1 && tempModel.Pwk) bwm_ptr = tempModel.Pwk->GetData().get();
                        if(nBwm == 2 && tempModel.Dwk0) bwm_ptr = tempModel.Dwk0->GetData().get();
                        if(nBwm == 3 && tempModel.Dwk1) bwm_ptr = tempModel.Dwk1->GetData().get();
                        if(nBwm == 4 && tempModel.Dwk2) bwm_ptr = tempModel.Dwk2->GetData().get();
                        if(bwm_ptr == nullptr) continue;
                        std::string sFileName;
                        if(nBwm == 0) sFileName = tempModel.Wok->GetFilename();
                        if(nBwm == 1) sFileName = tempModel.Pwk->GetFilename();
                        if(nBwm == 2) sFileName = tempModel.Dwk0->GetFilename();
                        if(nBwm == 3) sFileName = tempModel.Dwk1->GetFilename();
                        if(nBwm == 4) sFileName = tempModel.Dwk2->GetFilename();

                        BWMHeader bwm = *bwm_ptr;
                        if(bwm.nPadding != 0) HasBwmUnknown.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(bwm.nPadding) + std::string(")"));
                        for(Aabb & aabb : bwm.aabb){
                            if(aabb.nExtra != 4){
                                HasBwmExtra.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(aabb.nExtra) + std::string(")"));
                                break;
                            }
                        }
                        if(nBwm > 0){
                            std::vector<int> FoundIDs;
                            for(Face & face : bwm.faces){
                                if(face.nMaterialID != MATERIAL_NONWALK && std::find(FoundIDs.begin(), FoundIDs.end(), face.nMaterialID) == FoundIDs.end()){
                                    WalkablePwkDwk.push_back(sFileName + std::string(" (") + std::to_string(face.nMaterialID) + std::string(")"));
                                    FoundIDs.push_back(face.nMaterialID);
                                }
                            }
                        }
                    }
                    /* * /
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
                    if(Data.nSubclassification == 2 && Data.nClassification != CLASS_CHARACTER) HasSubclass2.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nClassification) + std::string(")"));
                    if(Data.nSubclassification == 4 && Data.nClassification != CLASS_PLACEABLE) HasSubclass4.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nClassification) + std::string(")"));
                    if(Data.nSubclassification != 4 && Data.nClassification == CLASS_PLACEABLE) HasNoSubclass4.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nSubclassification) + std::string(")"));
                    if(Data.nSubclassification != 0) HasClassUnknown1.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nSubclassification) + std::string(")"));
                    if(Data.nUnknown != 0) HasClassUnknown2.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nUnknown) + std::string(")"));
                    if(Data.nAffectedByFog != 1) HasClassUnknown3.push_back(Data.GH.sName.c_str() + std::string(" (") + std::to_string(Data.nAffectedByFog) + std::string(")"));
                    /* */
                    BigEmitterControllers.push_back(std::vector<std::string>());
                    std::vector<std::string> & EmitterControllers = BigEmitterControllers.back();
                    BigEmitterControllerNums.push_back(std::vector<int>());
                    std::vector<int> & EmitterControllerNums = BigEmitterControllerNums.back();
                    /* * /
                    unsigned int nCompressed = 0, nUncompressed = 0;
                    for(int nAnim = 0; nAnim < Data.Animations.size(); nAnim++){
                        for(int nNode = 0; nNode < Data.Animations.at(nAnim).ArrayOfNodes.size(); nNode++){
                            Node & anim_node = Data.Animations.at(nAnim).ArrayOfNodes.at(nNode);
                            if(anim_node.Head.Controllers.empty()) continue;
                            std::string sControllers;
                            for(Controller & ctrl : anim_node.Head.Controllers){
                                if(!sControllers.empty()) sControllers += ", ";
                                sControllers += ReturnControllerName(ctrl.nControllerType, NODE_HEADER | NODE_EMITTER);
                            }
                            bool bFound = false;
                            for(int n = 0; n < EmitterControllers.size(); n++){
                                if(EmitterControllers.at(n) == sControllers){
                                    EmitterControllerNums.at(n)++;
                                    bFound = true;
                                }
                            }
                            if(!bFound){
                                EmitterControllers.push_back(sControllers);
                                EmitterControllerNums.push_back(1);
                            }
                            /* * /
                            for(int nCtrl = 0; nCtrl < anim_node.Head.Controllers.size(); nCtrl++){
                                Controller & ctrl = anim_node.Head.Controllers.at(nCtrl);
                                if(ctrl.nControllerType != CONTROLLER_HEADER_ORIENTATION) continue;
                                if(ctrl.nColumnCount == 2) nCompressed++;
                                else if(ctrl.nColumnCount == 4) nUncompressed++;
                                else Warning("An orientation controller's column count is neither 2 nor 4!");
                            }
                            if(anim_node.Head.Controllers.size() == 0 && anim_node.Head.ControllerData.size() > 0){
                                ControllerlessAnimationData.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Animations.at(nAnim).sName.c_str() + std::string("->") + Data.Names.at(anim_node.Head.nNodeNumber).sName.c_str() + std::string(" (") + std::to_string(anim_node.Head.ControllerData.size()) + std::string(")"));
                            }
                            /* * /
                        }
                    }
                    CompressedOrientation.push_back(Data.GH.sName.c_str() + std::string(" - compressed ") + std::to_string(nCompressed) + " : uncompressed " + std::to_string(nUncompressed));
                    /* */
                    for(int nNode = 0; nNode < Data.ArrayOfNodes.size(); nNode++){
                        Node & node = Data.ArrayOfNodes.at(nNode);
                        /* * /
                        //SupernodeNums.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(": ") + std::to_string(node.Head.nNodeNumber) + std::string(", ") + std::to_string(node.Head.nSupernodeNumber));
                        if(Data.nOffsetToHeadRootNode != Data.GH.nOffsetToRootNode && node.nOffset == Data.nOffsetToHeadRootNode) HeadLink.push_back(Data.GH.sName.c_str() + std::string(": ") + Data.Names.at(nNode).sName.c_str());
                        /* */
                        if(node.Head.nType & NODE_EMITTER){
                            /* * /
                            for(int ef = 0; ef < HasEmitterFlag.size(); ef++){
                                if(node.Emitter.nFlags & pown(2, ef)) HasEmitterFlag.at(ef).push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str());
                            }
                            if(node.Emitter.nFrameBlending != 0) HasFrameBlending.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string((int) node.Emitter.nFrameBlending) + std::string(")"));
                            if(node.Emitter.cDepthTextureName.c_str() != std::string("NULL") &&
                               node.Emitter.cDepthTextureName.c_str() != std::string("") ) HasDepthTexture.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + node.Emitter.cDepthTextureName.c_str() + std::string(")"));
                            if(node.Emitter.nRenderOrder != 0) HasRenderOrder.push_back(Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName.c_str() + std::string(" (") + std::to_string(node.Emitter.nRenderOrder) + std::string(")"));
                            /* */
                            std::string sControllers;
                            for(Controller & ctrl : node.Head.Controllers){
                                if(!sControllers.empty()) sControllers += ", ";
                                sControllers += ReturnControllerName(ctrl.nControllerType, NODE_HEADER | NODE_EMITTER);
                            }
                            bool bFound = false;
                            for(int n = 0; n < EmitterControllers.size(); n++){
                                if(EmitterControllers.at(n) == sControllers){
                                    EmitterControllerNums.at(n)++;
                                    bFound = true;
                                }
                            }
                            if(!bFound){
                                EmitterControllers.push_back(sControllers);
                                EmitterControllerNums.push_back(1);
                            }
                        }
                        /* * /
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
                        /* * /
                        if(node.Head.nType & NODE_DANGLY){
                            std::vector<double> DanglyConstraints;
                            for(double constr : node.Dangly.Constraints){
                                if(std::find(DanglyConstraints.begin(), DanglyConstraints.end(), constr) == DanglyConstraints.end()) DanglyConstraints.push_back(constr);
                            }
                            std::string sNew = Data.GH.sName.c_str() + std::string("->") + Data.Names.at(nNode).sName + ":\n"
                                                   "      period: " + PrepareFloat(node.Dangly.fPeriod) + "\n"
                                                   "      tightness: " + PrepareFloat(node.Dangly.fTightness) + "\n"
                                                   "      displacement: " + PrepareFloat(node.Dangly.fDisplacement) + "\n"
                                                   "      constraints:";
                            for(double constr : DanglyConstraints) sNew += " " + PrepareFloat(constr);
                            DanglyValues.push_back(std::move(sNew));
                        }
                        /* */
                    }
                }
            }

            nCounter++;
            SendMessage(hProgressMass, PBM_SETPOS, (WPARAM) nCounter, (LPARAM) NULL);
            UpdateWindow(hProgressMass);
        }

        /// Now, let's report our findings.
        if(CurrentProcess == PROCESSING_MASS_ANALYZE){
            //std::ofstream reportfile(sFolder + "mdl_analysis.txt", std::fstream::out);
            std::stringstream reportfile;

            if(true){
                reportfile << "EMITTER CONTROLLERS DATA\r\n";
                for(int i = 0; i < BigEmitterControllers.size(); i++){
                    reportfile << " " << ModelNames.at(i) << ":\r\n";
                    for(int n = 0; n < BigEmitterControllers.at(i).size(); n++){
                        reportfile << (BigEmitterControllerNums.at(i).at(n) > 9 ? " " : "  ") << BigEmitterControllerNums.at(i).at(n) << "x --> " << BigEmitterControllers.at(i).at(n) << "\r\n";
                    }
                }
                reportfile << "\r\n";
            }
            if(false){
                reportfile << "SUPERNODE NUMS\r\n";
                for(int n = 0; n < SupernodeNums.size(); n++) reportfile << "    " << SupernodeNums.at(n) << "\r\n";
                reportfile << "\r\n";
            }
            if(false){
                reportfile << "DANGLY VALUES\n\n";
                for(int n = 0; n < DanglyValues.size(); n++) reportfile << "    " << DanglyValues.at(n) << "\n";
                reportfile << "\n";
            }
            if(false){
                reportfile << "HEAD LINK\n\n";
                for(int n = 0; n < HeadLink.size(); n++) reportfile << "    " << HeadLink.at(n) << "\n";
                reportfile << "\n";
            }
            if(false){
                reportfile << "PWK/DWK \"WALKABLE\" MATERIAL IDS\n\n";
                for(int n = 0; n < WalkablePwkDwk.size(); n++) reportfile << "    " << WalkablePwkDwk.at(n) << "\n";
                reportfile << "\n";
            }
            if(false){
                reportfile << "CONTROLLERLESS ANIMATION DATA\n\n";
                for(int n = 0; n < ControllerlessAnimationData.size(); n++) reportfile << "    " << ControllerlessAnimationData.at(n) << "\n";
            }
            if(false){
                reportfile << "COMPRESSED QUATERNIONS\n\n";
                for(int n = 0; n < CompressedOrientation.size(); n++) reportfile << "    " << CompressedOrientation.at(n) << "\n";
            }
            if(false){
                reportfile << "BWM STATS\n\n";
                reportfile << "nPadding != 0:\n";
                for(int n = 0; n < HasBwmUnknown.size(); n++) reportfile << "    " << HasBwmUnknown.at(n) << "\n";
                reportfile << "nExtra != 4:\n";
                for(int n = 0; n < HasBwmExtra.size(); n++) reportfile << "    " << HasBwmExtra.at(n) << "\n";
            }
            if(false){
                reportfile << "FLAG STATS\n\n";
                reportfile << "dirt_enabled > 0:\n";
                for(int n = 0; n < HasDirtEnabled.size(); n++) reportfile << "    " << HasDirtEnabled.at(n) << "\n";
                reportfile << "beaming > 0:\n";
                for(int n = 0; n < HasBeaming.size(); n++) reportfile << "    " << HasBeaming.at(n) << "\n";
                reportfile << "backgroundgeometry > 0:\n";
                for(int n = 0; n < HasBackgroundGeometry.size(); n++) reportfile << "    " << HasBackgroundGeometry.at(n) << "\n";
            }
            if(false){
                reportfile << "\nSUBCLASSIFICATION STATS\n\n";
                //reportfile << "Subclassification 2:\n";
                //for(int n = 0; n < HasSubclass2.size(); n++) reportfile << "    " << HasSubclass2.at(n) << "\n";
                reportfile << "Placeables without Subclassification 4:\n";
                for(int n = 0; n < HasNoSubclass4.size(); n++) reportfile << "    " << HasNoSubclass4.at(n) << "\n";
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
            std::wstring sFoldTemp = sFolder;
            if(sFoldTemp.back() != L'\\'){
                PathRemoveFileSpecW(&sFoldTemp[0]);
                sFoldTemp = &sFoldTemp[0];
                sFoldTemp += L"\\";
            }
            //std::cout << "Writing to folder: " << to_ansi(sFoldTemp) << "\n";
            std::cout << "\nWriting analysis to file: " << to_ansi(sFoldTemp + L"mdl_analysis_result.txt") << "\n";
            HANDLE hReportfile = bead_CreateWriteFile(sFoldTemp + L"mdl_analysis_result.txt");
            bead_WriteFile(hReportfile, reportfile.str());
            //reportfile.close();
            CloseHandle(hReportfile);
        }
    }

    SendMessage((HWND)lpParam, 69, NULL, NULL); //Done
}
