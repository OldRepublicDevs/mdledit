#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED

#define _WIN32_WINNT 0x0501 //0x0602
#define _WIN32_IE 0x0500 //0x601
#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include <array> //for std::array
#include <iostream> //for std::cout
#include <vector> //for std::vector
#include <memory> //for std::unique_ptr
#include <string> //for std::string
#include <sstream> //for std::stringstream
#include <exception> //for std::exception
#include "resource.h"

#define DEBUG_LEVEL 0

extern HWND hFrame;
extern HWND hStatusBar;
extern HWND hTree;
extern HWND hTabs;
extern HWND hProgress;
extern HWND hDisplayEdit;
extern double PI;
extern bool bShowHex;
char DecToHexDigit(int nDec);
void CharsToHex(char * cOutput, std::vector<char> & cInput, int nOffset, int nNumber);
void AddSignificantZeroes(char * cInt, int nSignificant);
void TruncateDec(TCHAR * tcString);
std::string TruncateDec(std::string sCopy);
void PrepareCharForDisplay(char * cChar);
void CopyBuffer(std::vector<char> & cCopyTo, char * cCopyFrom, int nCount);
int Error(std::string sErrorMessage);
int WarningCancel(std::string sWarningMessage);
int WarningYesNoCancel(std::string sWarningMessage);
int Warning(std::string sWarningMessage);
int pown(int base, int exp);
float deg(float rad);
float rad(float deg);
double deg(double rad);
double rad(double deg);
void QuaternionToAA(double * fQuaternion, double * fAA);
void AAToQuaternion(double * fAA, double * fQuaternion);
double RoundDec(double fNumber, int nDecPlaces);
double RoundDec(float fNumber, int nDecPlaces);
bool bCursorOnLine(POINT pt, POINT ptLine1, POINT ptLine2, int nOffset);
std::string PrepareFloat(double fFloat, bool bFiniteOnly = true);
unsigned int stou(std::string const & str, size_t * idx = 0, int base = 10);

struct MenuLineAdder{
    HMENU hMenu;
    int nIndex;
};

class Timer{
    DWORD nReferenceTime = 0;
  public:
    Timer(){
        nReferenceTime = timeGetTime();
    }
    void StartTimer(){
        nReferenceTime = timeGetTime();
    }
    std::string GetTime(bool bRestart = false){
        DWORD nNewTime = timeGetTime();
        int nSeconds = 0;
        int nMiliseconds = 0;
        if(nNewTime < nReferenceTime){
            DWORD dwMaxVal = 0;
            dwMaxVal = ~dwMaxVal;
            nSeconds = ((dwMaxVal - nReferenceTime) + nNewTime)/1000;
            nMiliseconds = ((dwMaxVal - nReferenceTime) + nNewTime)%1000;
        }
        else{
            nSeconds = (nNewTime - nReferenceTime)/1000;
            nMiliseconds = (nNewTime - nReferenceTime)%1000;
        }
        if(bRestart) nReferenceTime = nNewTime;
        return (std::to_string(nSeconds) + "." + (nMiliseconds < 100 ? "0" : "") + (nMiliseconds < 10 ? "0" : "") + std::to_string(nMiliseconds) + "s");
    }
};

class mdlexception: public std::exception {
    std::string sException;
  public:
    mdlexception() {}
    mdlexception(const std::string & sNew): sException(sNew) {}
    virtual const char* what() const throw() {
        return sException.c_str();
    }
    void SetText(const std::string & sNew){
        sException = sNew;
    }
};

//char * operator&(std::string & sStr);
std::string safesubstr(const std::string & sParam, size_t nStart, size_t nLen = std::string::npos);

#endif // GENERAL_H_INCLUDED
