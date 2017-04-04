#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <string>
#include <vector>

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
    void MarkBytes(unsigned int nOffset, int nLength, int nClass);

    //Reading functions
    int ReadInt(unsigned int * nPosition, int nMarking, int nBytes = 4);
    float ReadFloat(unsigned int * nPosition, int nMarking, int nBytes = 4);
    void ReadString(std::string & sArray1, unsigned int *nPosition, int nMarking, int nNumber);

    //Writing functions
    void WriteInt(int nInt, int nKnown, int nBytes = 4);
    void WriteFloat(float fFloat, int nKnown, int nBytes = 4);
    void WriteString(std::string sString, int nKnown);
    void WriteByte(char cByte, int nKnown);
    void WriteIntToPH(int nInt, int nPH, unsigned int & nContainer); //PH is placeholder

  public:
    //Getters
    std::vector<int> & GetKnownData(){ return bKnown; }

    //Loaders/Unloaders
    std::vector<char> & CreateBuffer(int nSize){
        bLoaded = true;
        sBuffer.resize(nSize, 0);
        bKnown.resize(nSize, 0);
        return sBuffer;
    }
    void FlushAll(){
        sBuffer.clear();
        bKnown.clear();
        bLoaded = false;
    }
};

class TextFile: public File{
  protected:
    bool ReadFloat(double & fNew, bool bPrint = false);
    bool ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint = false);
    bool ReadInt(int & nNew, bool bPrint = false);
    bool ReadUntilText(std::string & sHandle, bool bToLowercase = true, bool bStrictNoNewLine = false);
    void SkipLine();
    bool EmptyRow();
};

//Convert unions
extern BB2 ByteBlock2;
extern BB4 ByteBlock4;
extern BB8 ByteBlock8;

#endif // FILE_H_INCLUDED
