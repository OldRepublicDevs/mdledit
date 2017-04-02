#ifndef EDITORDLG_H_INCLUDED
#define EDITORDLG_H_INCLUDED

#include "general.h"
#include "MDL.h"

class EditorDlgWindow{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;
    std::string sBuffer;
    unsigned int nPosition = 0;
    unsigned int nDataType = 0;
    LPVOID lpData = nullptr;
    MDL * MdlPtr = nullptr;

    bool SaveData();
    bool ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint = false);
    bool ReadInt(int & nNew, bool bPrint = false);
    void SkipLine();
    bool EmptyRow();
    bool ReadUntilText(std::string & sHandle, bool bToLowercase = false, bool bStrictNoNewLine = false);


public:
    HWND hMe;
    EditorDlgWindow();
    bool Run();
    friend LRESULT CALLBACK EditorDlgWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void SetData(unsigned int nDataSet, LPVOID lpDataSet, MDL & Model){
        nDataType = nDataSet;
        lpData = lpDataSet;
        MdlPtr = &Model;
    }
};

#define EDITOR_DLG_VERTEX          1
#define EDITOR_DLG_SABER_DATA      2
#define EDITOR_DLG_TRIMESH_FLAGS   3

#endif // EDITORDLG_H_INCLUDED
