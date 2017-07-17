#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <string>
#include <vector>
#include "geometry.h"

union BB2{
    short int i;
    unsigned short int ui;
    char bytes [2];
};

union BB4{
    float f;
    int i;
    unsigned int ui;
    char bytes [4];
};

union BB8{
    double d;
    long int i;
    unsigned long int ui;
    char bytes [8];
};

class File{
  protected:
    bool bLoaded = false;
    std::string sFile;
    std::string sFullPath;
    std::vector<char> sBuffer;
    unsigned int nPosition = 0;
  public:
    //Getters
    std::vector<char> & GetBuffer(){ return sBuffer; }
    const std::string & GetFilename(){ return sFile; }
    const std::string & GetFullPath(){ return sFullPath; }
    virtual const std::string GetName(){ return ""; }
    bool empty(){ return !bLoaded; }
    void Export(std::string &sExport){
        sExport = std::string(sBuffer.begin(), sBuffer.end());
    }

    //Setters
    void SetFilePath(std::string & sPath);

    //Loaders/Unloaders
    virtual std::vector<char> & CreateBuffer(int nSize){
        bLoaded = true;
        sBuffer.resize(nSize, 0);
        return sBuffer;
    }
    virtual void FlushAll(){
        sBuffer.clear();
        sBuffer.shrink_to_fit();
        bLoaded = false;
    }

};

class BinaryFile: public File{

  protected:
    //For coloring bytes
    std::vector<int> bKnown;
    std::vector<int> bDifferent;
    void MarkBytes(unsigned int nOffset, int nLength, int nClass);

    //Reading functions
    int ReadInt(unsigned int * nPosition, int nMarking, int nBytes = 4);
    float ReadFloat(unsigned int * nPosition, int nMarking, int nBytes = 4);
    void ReadString(std::string & sArray1, unsigned int *nPosition, int nMarking, int nNumber);
    Vector ReadVector(unsigned int * nPosition, int nMarking, int nBytes = 4);

    //Writing functions
    void WriteInt(int nInt, int nKnown, int nBytes = 4);
    void WriteFloat(float fFloat, int nKnown, int nBytes = 4);
    void WriteString(std::string sString, int nKnown);
    void WriteByte(char cByte, int nKnown);
    void WriteIntToPH(int nInt, int nPH, unsigned int & nContainer); //PH is placeholder
    void WriteAtOffset4(int nInt, unsigned int nOffset);
    void WriteAtOffset2(short nShort, unsigned int nOffset);
    void WriteAtOffset1(char nChar, unsigned int nOffset);
    void WriteAtOffset(float fFloat, unsigned int nOffset);
    void WriteAtOffset(const std::string & sString, unsigned int nOffset);

  public:
    //Friends
    friend class EditorDlgWindow;

    //Getters
    std::vector<int> & GetKnownData(){ return bKnown; }
    std::vector<int> & GetDifferenceData(){ return bDifferent; }

    //Loaders/Unloaders
    std::vector<char> & CreateBuffer(int nSize){
        bLoaded = true;
        sBuffer.resize(nSize, 0);
        bKnown.resize(nSize, 0);
        //bDifferent.resize(nSize, 0);
        return sBuffer;
    }
    void FlushAll(){
        sBuffer.clear();
        bKnown.clear();
        bDifferent.clear();
        bLoaded = false;
    }
    void Compare(File & file){
        bDifferent.resize(sBuffer.size(), 0);
        std::vector<char> & sBuffer2 = file.GetBuffer();
        for(int i = 0; i < sBuffer.size(); i++){
            if(i < sBuffer2.size()){
                if(sBuffer.at(i) != sBuffer2.at(i)) bDifferent.at(i) = 1;
            }
            else bDifferent.at(i) = 2;
        }
    }
};

class TextFile: public File{
  protected:
    bool ReadFloat(double & fNew, std::string * sGet = nullptr, bool bPrint = false);
    //bool ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint = false);
    bool ReadInt(int & nNew, std::string * sGet = nullptr, bool bPrint = false);
    bool ReadUInt(unsigned int & nNew, std::string * sGet = nullptr, bool bPrint = false);
    bool ReadUntilText(std::string & sHandle, bool bToLowercase = true, bool bStrictNoNewLine = false);
    void SkipLine();
    bool EmptyRow();
};

enum DataType {
    DT_string,
    DT_int,
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
    void ReadIni(std::string &);
    void WriteIni(std::string &);
};

//Convert unions
extern BB2 ByteBlock2;
extern BB4 ByteBlock4;
extern BB8 ByteBlock8;

#endif // FILE_H_INCLUDED
