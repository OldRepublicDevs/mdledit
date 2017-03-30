#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED

#define _WIN32_WINNT 0x0602
#define _WIN32_IE 0x601
#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include <iostream> //for std::cout
#include <vector> //for std::vector
#include <memory> //for std::unique_ptr - can be eliminated if I get rid of string_format
#include <string> //for std::string
#include <sstream> //for std::stringstream
#include "resource.h"

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
#define ME_DATA_LABEL_SIZE_X        50
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
#define ME_TREE_OFFSET_Y            280
#define ME_DISPLAY_OFFSET_Y         71
#define ME_DISPLAY_SIZE_Y           205

#define DEBUG_LEVEL 1

template<typename ... Args> std::string string_format( const std::string& format, Args ... args ){
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

extern HWND hFrame;
extern HWND hStatusBar;
extern HWND hTree;
extern HWND hTabs;
extern HWND hProgress;
extern HWND hDisplayEdit;
bool BVstrcmp(TCHAR string1[], TCHAR string2[], bool bDebug = false);
void BVright(TCHAR * string1, int n);
void BVleft(TCHAR * string1, int n);
char DecToHexDigit(int nDec);
void CharsToHex(char * cOutput, std::vector<char> & cInput, int nOffset, int nNumber);
void AddSignificantZeroes(char * cInt, int nSignificant);
void TruncateDec(TCHAR * tcString);
void PrepareCharForDisplay(char * cChar);
void CopyBuffer(std::vector<char> & cCopyTo, char * cCopyFrom, int nCount);
int Error(std::string sErrorMessage);
int WarningCancel(std::string sWarningMessage);
int Warning(std::string sWarningMessage);
int pown(int base, int exp);
float powf(float base, float exp);
float deg(float rad);
float rad(float deg);
double deg(double rad);
double rad(double deg);
float acosf(float fVal);
float cosf(float fVal);
float sinf(float fVal);
float tanf(float fVal);
float sqrtf(float fVal);
void QuaternionToAA(float * fQuaternion, float * fAA);
void AAToQuaternion(float * fAA, float * fQuaternion);
void QuaternionToAA(double * fQuaternion, double * fAA);
void AAToQuaternion(double * fAA, double * fQuaternion);
double RoundDec(double fNumber, int nDecPlaces);
double RoundDec(float fNumber, int nDecPlaces);
bool bCursorOnLine(POINT pt, POINT ptLine1, POINT ptLine2, int nOffset);

union ByteBlock2{
    short int i;
    unsigned short int ui;
    char bytes [2];
};

union ByteBlock4{
    float f;
    int i;
    unsigned int ui;
    char bytes [4];
};

union ByteBlock8{
    double d;
    long int i;
    unsigned long int ui;
    char bytes [8];
};
struct MenuLineAdder{
    HMENU hMenu;
    int nIndex;
};

#endif // GENERAL_H_INCLUDED
