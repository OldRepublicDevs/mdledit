#ifndef EDITORDLG_H_INCLUDED
#define EDITORDLG_H_INCLUDED

#include "general.h"
#include "MDL.h"

struct TokenDatum{
    std::string sToken;
    std::string sDataType;
    std::string sFile;
    void * data = nullptr;
    unsigned int nOffset = 0;
    unsigned int nBitflag = 0;
    unsigned int nMaxString = 0;
    int nBytes = 0;
    TokenDatum(){}
    TokenDatum(const std::string & s1, const std::string & s2, const std::string & s3, void * ptr, int n1): sToken(s1), sDataType(s2), sFile(s3), data(ptr), nOffset(n1)
    {
        if(s2 == "bool" || s2 == "char" || s2 == "unsigned char" || s2 == "signed char") nBytes = 1;
        else if(s2 == "short" || s2 == "unsigned short" || s2 == "signed short") nBytes = 2;
        else if(s2 == "bitflag" || s2 == "double" || s2 == "int" || s2 == "unsigned int" || s2 == "signed int") nBytes = 4;
    }
};

class EditorDlgWindow: public TextFile{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;
    MDL * MdlPtr = nullptr;

    bool SaveData();

public:
    HWND hMe;
    EditorDlgWindow();
    bool Run();
    friend LRESULT CALLBACK EditorDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void SetData(MDL & Model){
        MdlPtr = &Model;
    }
    std::vector<TokenDatum> TokenData;
    bool GetTokenData(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam, std::stringstream & ssName, int nFile);
};

#endif // EDITORDLG_H_INCLUDED
