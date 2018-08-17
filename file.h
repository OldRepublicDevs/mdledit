#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <string>
#include <vector>
#include "geometry.h"

HANDLE bead_CreateReadFile(const std::string & sFilename);
HANDLE bead_CreateReadFile(const std::wstring & sFilename);
HANDLE bead_CreateWriteFile(const std::string & sFilename);
HANDLE bead_CreateWriteFile(const std::wstring & sFilename);
long unsigned bead_GetFileLength(HANDLE hFile);
bool bead_ReadFile(HANDLE hFile, std::vector<char> & sBuffer, unsigned long nToRead = ~0);
bool bead_WriteFile(HANDLE hFile, const std::string & sBuffer, unsigned long nToWrite = ~0);

class Path{
    std::wstring sFullPath;
  public:
    std::wstring GetFullPath();
    std::wstring GetDirectory();
    std::wstring GetFilename();
    std::wstring GetFilenameNoExt();
    std::wstring GetExtension();

    void Set(const std::wstring & wsSet){
        sFullPath = wsSet.c_str();
    }
    void SetSize(unsigned n = 0){
        sFullPath.resize(n);
    }
    wchar_t * GetPtr(){
        return &sFullPath.front();
    }

};

class File{
  protected:
    Path path;
    std::vector<char> sBuffer;
    unsigned int nPosition = 0;
    unsigned int nLine = 1;
    std::vector<unsigned> SavedPosition, SavedLine;

    /// Functions
    void SavePosition(unsigned n);
    bool RestorePosition(unsigned n);
    File(){
        SavedPosition.reserve(5);
        SavedLine.reserve(5);
    }
  public:
    //Getters
    std::vector<char> & GetBuffer(){ return sBuffer; }
    std::wstring GetFilename(){ return path.GetFilename(); }
    std::wstring GetFullPath(){ return path.GetFullPath(); }
    virtual const std::string GetName(){ return ""; }
    bool empty(){ return !sBuffer.size(); }
    void Export(std::string &sExport){
        sExport = std::string(sBuffer.begin(), sBuffer.end());
    }

    //Setters
    void SetFilePath(std::wstring & sPath);

    //Loaders/Unloaders
    virtual std::vector<char> & CreateBuffer(long unsigned nSize){
        sBuffer.resize(nSize, 0);
        return sBuffer;
    }
    virtual void FlushAll(){
        sBuffer.clear();
        sBuffer.shrink_to_fit();
    }

};

struct BinaryPosition{
    unsigned nOffset = 0;
    unsigned nSize = 0;
    std::string sLabel;
    BinaryPosition(unsigned nOffset = 0, unsigned nSize = 0, const std::string & sLabel = "") : nOffset(nOffset), nSize(nSize), sLabel(sLabel) {}
};

class BinaryFile: public File{

  protected:
    //For coloring bytes
    std::vector<int> bKnown;
    std::vector<char> sCompareBuffer;
    std::vector<BinaryPosition> positions;
    void MarkBytes(unsigned int nOffset, int nLength, int nClass);
    void MarkDataBorder(unsigned nOffset);

    //Reading functions
    unsigned char * ReadBytes(unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * nPlaceholder = nullptr, bool bAdvancePlaceholder = true);
    template <class NumType>
    NumType ReadNumber(NumType * p_number, int nMarking, const std::string & sDesc, unsigned * nPlaceholder = nullptr, bool bAdvancePlaceholder = true){
        NumType result = * reinterpret_cast<NumType*>(ReadBytes(sizeof(NumType), nMarking, sDesc, nPlaceholder, bAdvancePlaceholder));
        if(p_number) *p_number = result;
        return result;
    }
    std::string ReadString(std::string * p_string, unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * p_insert = nullptr, bool bAdvancePlaceholder = true){
        std::string result;
        if(nBytes) result.assign(reinterpret_cast<const char*>(ReadBytes(nBytes, nMarking, sDesc, p_insert, bAdvancePlaceholder)), nBytes);
        else result = reinterpret_cast<const char*>(ReadBytes(strlen(reinterpret_cast<const char *>(p_insert != nullptr ? &sBuffer.at(*p_insert) : &sBuffer.at(nPosition))) + 1, nMarking, sDesc, p_insert, bAdvancePlaceholder));
        if(p_string){
            *p_string = result;
        }
        return result;
    }

    //Writing functions
    unsigned WriteBytes(unsigned char * p_bytes, unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * p_insert = nullptr);
    template <class NumType>
    unsigned WriteNumber(NumType * p_number, int nMarking, const std::string & sDesc, unsigned * p_insert = nullptr){
        return WriteBytes(reinterpret_cast<unsigned char*>(p_number), sizeof(NumType), nMarking, sDesc, p_insert);
    }
    unsigned WriteString(std::string * p_string, unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * p_insert = nullptr){
        return WriteBytes(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(p_string->c_str())), nBytes == 0 ? strlen(p_string->c_str()) + 1 : nBytes, nMarking, sDesc, p_insert);
    }
    unsigned WriteFloat(double * p_double, int nMarking, const std::string & sDesc, unsigned * p_insert = nullptr){
        float fHelper = static_cast<float>(*p_double);
        return WriteNumber(&fHelper, nMarking, sDesc, p_insert);
    }

    //Reading functions
    //int ReadInt(unsigned int * nPosition, int nMarking, int nBytes = 4);
    //float ReadFloat(unsigned int * nPosition, int nMarking, int nBytes = 4);
    //void ReadString(std::string & sArray1, unsigned int *nPosition, int nMarking, int nNumber);
    //Vector ReadVector(unsigned int * nPosition, int nMarking, int nBytes = 4);

    //Writing functions
    //void WriteInt(int nInt, int nKnown, int nBytes = 4);
    //void WriteFloat(float fFloat, int nKnown, int nBytes = 4);
    //void WriteString(std::string sString, int nKnown);
    //void WriteByte(char cByte, int nKnown);
    //void WriteIntToPH(int nInt, int nPH, unsigned int & nContainer); //PH is placeholder
    //void WriteAtOffset4(int nInt, unsigned int nOffset);
    //void WriteAtOffset2(short nShort, unsigned int nOffset);
    //void WriteAtOffset1(char nChar, unsigned int nOffset);
    //void WriteAtOffset(float fFloat, unsigned int nOffset);
    //void WriteAtOffset(const std::string & sString, unsigned int nOffset);

  public:
    //Friends
    friend class EditorDlgWindow;

    //Getters
    std::vector<int> & GetKnownData(){ return bKnown; }
    std::vector<char> & GetCompareData(){ return sCompareBuffer; }
    std::string GetPosition(unsigned nOffset){
        for(BinaryPosition & pos : positions){
            if(nOffset >= pos.nOffset && nOffset < (pos.nOffset + pos.nSize)) return pos.sLabel;
            //if(nOffset < pos.nOffset) break;
        }
        return "Unknown";
    }

    //Loaders/Unloaders
    std::vector<char> & CreateBuffer(long unsigned nSize) override {
        sBuffer.resize(nSize, 0);
        bKnown.resize(nSize, 0);
        positions.clear();
        positions.reserve(nSize / 2);
        return sBuffer;
    }
    void FlushAll() override {
        sBuffer.clear();
        sCompareBuffer.clear();
        bKnown.clear();
        positions.clear();
    }
    void Compare(File & file){
        sCompareBuffer = file.GetBuffer();
    }
};

class TextFile: public File{
  protected:
    bool ReadFloat(double & fNew, std::string * sGet = nullptr, bool bPrint = false);
    //bool ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint = false);
    bool ReadInt(int & nNew, std::string * sGet = nullptr, bool bPrint = false);
    bool ReadUInt(unsigned int & nNew, std::string * sGet = nullptr, bool bPrint = false);
    bool ReadUntilText(std::string & sHandle, bool bToLowercase = true);
    void SkipLine();
    bool EmptyRow();
};

enum DataType {
    DT_string,
    DT_int,
    DT_uint,
    DT_bool,
    DT_float
};
struct IniOption{
    std::string sToken;
    DataType nType = static_cast<DataType>(0);
    void * lpVariable = nullptr;

    IniOption(){}
    IniOption(const std::string & s, DataType t, void * lptr): sToken(s), nType(t), lpVariable(lptr) {}
};

class IniFile: public TextFile{
    std::vector<IniOption> Options;
  public:
    void AddIniOption(const std::string & s, DataType t, void * lptr){
        Options.push_back(IniOption(s, t, lptr));
    }
    void ClearIniOptions(){
        Options.clear();
    }
    void ReadIni(std::wstring &);
    void WriteIni(std::wstring &);
};
/*
//Convert unions
extern BB2 ByteBlock2;
extern BB4 ByteBlock4;
extern BB8 ByteBlock8;
*/
#endif // FILE_H_INCLUDED
