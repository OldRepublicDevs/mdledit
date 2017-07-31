#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED

#include "general.h"
#include <commctrl.h>
#include "MDL.h"

#define ME_TABS_OFFSET_Y_TOP        22
#define ME_TABS_OFFSET_X_RIGHT      2
#define ME_TABS_OFFSET_Y_BOTTOM     2
#define ME_TABS_SIZE_X              615
#define ME_HEX_WIN_SIZE_X           612
#define ME_HEX_WIN_SIZE_Y           200
#define ME_ASCII_WIN_SIZE_X         150
#define ME_ASCII_WIN_SIZE_Y         200
#define ME_HEX_WIN_OFFSET_X         0
#define ME_ASCII_WIN_OFFSET_X       2
#define ME_EDIT_PADDING_TOP         3
#define ME_EDIT_PADDING_BOTTOM      8
#define ME_EDIT_PADDING_LEFT        4
#define ME_EDIT_NEXT_ROW            15
#define ME_EDIT_CHAR_SIZE_X         8
#define ME_SCROLLBAR_X              17
#define ME_DATA_LABEL_OFFSET_X      10
#define ME_DATA_EDIT_OFFSET_X       2
#define ME_DATA_NEXT_ROW            22
#define ME_BASIC_OFFSET_Y           5
#define ME_DATA_LABEL_SIZE_X        25
#define ME_DATA_LABEL_SIZE_Y        15
#define ME_DATA_EDIT_SIZE_X         280
#define ME_DATA_EDIT_SIZE_Y         20
#define ME_DATA_LABEL_ROW_OFFSET_Y  3
#define ME_EDIT_ROWNUM_OFFSET       67
#define ME_EDIT_SEPARATOR_OFFSET    62
#define ME_EDIT_SEPARATOR_2_OFFSET  447
#define ME_EDIT_CHARSET_OFFSET      455
#define ME_STATUSBAR_Y              23
#define ME_STATUSBAR_PART_X         150
#define ME_TREE_SIZE_X              334
#define ME_TREE_SIZE_DIFF_X         10
#define ME_TREE_SIZE_DIFF_Y         10
#define ME_TREE_OFFSET_X            620
#define ME_TREE_OFFSET_Y            302
#define ME_DISPLAY_OFFSET_Y         93
#define ME_DISPLAY_SIZE_Y           220

#define ACTION_UPDATE_DISPLAY      0
#define ACTION_ADD_MENU_LINES      1
#define ACTION_OPEN_VIEWER         2
#define ACTION_OPEN_EDITOR         3

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

extern bool bDotAsciiDefault;

enum IniConst {
    INI_READ,
    INI_WRITE
};
void ManageIni(IniConst Action);

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
void DetermineDisplayText(std::vector<std::string> cItem, std::stringstream & sPrint, LPARAM lParam);
void AddMenuLines(std::vector<std::string> cItem, LPARAM lParam, MenuLineAdder * pmla, int nFile);
void OpenGeoViewer(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam);
void OpenViewer(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam);
void OpenEditorDlg(MDL & Mdl, std::vector<std::string> cItem, LPARAM lParam, int nFile);
//bool AppendTab(HWND hTabControl, std::string sName);
void Report(std::string sMessage);
void ProgressSize(int nMin, int nMax);
void ProgressPos(int nPos);

#endif // FRAME_H_INCLUDED
