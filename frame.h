#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED

#include "general.h"
#include <commctrl.h>
#include "MDL.h"

class Frame{
    //Main Window Creation
    WNDCLASSEX WindowClass;
    static HINSTANCE hInstance;
    static char cClassName[];
    HWND hMe;

    public:
    //Main Window Creation
    static LRESULT CALLBACK FrameProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    Frame(HINSTANCE hInstanceCreate);
    bool Run(int nCmdShow);
};

#define ACTION_UPDATE_DISPLAY      0
#define ACTION_ADD_MENU_LINES      1
#define ACTION_OPEN_VIEWER         2
#define ACTION_OPEN_EDITOR         3

void ProcessTreeAction(HTREEITEM hItem, const int & nAction, void * Pointer = NULL);
bool FileEditor(HWND hwnd, int nID, std::string & cFile);
bool AppendTab(HWND hTabControl, std::string sName);
INT_PTR CALLBACK AboutProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TexturesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressMassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void BuildTree(MDL & Mdl);
void BuildTree(BWM & Bwm);
void DetermineDisplayText(std::vector<std::string>cItem, std::stringstream & sPrint, LPARAM lParam);
void AddMenuLines(std::vector<std::string>cItem, LPARAM lParam, MenuLineAdder * pmla);
void OpenGeoViewer(MDL & Mdl, std::vector<std::string>cItem, LPARAM lParam);
void OpenViewer(MDL & Mdl, std::vector<std::string>cItem, LPARAM lParam);
void OpenEditorDlg(MDL & Mdl, std::vector<std::string>cItem, LPARAM lParam);
//bool AppendTab(HWND hTabControl, std::string sName);
void Report(std::string sMessage);
void ProgressSize(int nMin, int nMax);
void ProgressPos(int nPos);

#endif // FRAME_H_INCLUDED
