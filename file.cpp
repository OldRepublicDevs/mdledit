#include "general.h"
#include "file.h"
#include <iostream>
#include <algorithm>
#include <Shlwapi.h>

BB2 ByteBlock2;
BB4 ByteBlock4;
BB8 ByteBlock8;

void File::SetFilePath(std::string & sPath){
    sFullPath = sPath;
    sFile = sPath;    PathStripPath(&sFile.front());
    sFile.resize(strlen(sFile.c_str()));
}

int BinaryFile::ReadInt(unsigned int * nCurPos, int nMarking, int nBytes){
    //std::cout<<string_format("ReadInt() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > sBuffer.size()){
        std::cout<<"ReadInt(): Reading past buffer size in "<<GetName()<<", aborting and returning -1.\n";
        return -1;
    }
    if(nBytes == 4){
        int n = 0;
        while(n < 4){
            ByteBlock4.bytes[n] = sBuffer[*nCurPos + n];
            n++;
        }
        MarkBytes(*nCurPos, 4, nMarking);
        *nCurPos += 4;
        //std::cout<<"ReadInt() return: "<<ByteBlock4.i<<"\n";
        return ByteBlock4.i;
    }
    else if(nBytes ==2){
        int n = 0;
        while(n < 2){
            ByteBlock2.bytes[n] = sBuffer[*nCurPos + n];
            n++;
        }
        MarkBytes(*nCurPos, 2, nMarking);
        *nCurPos += 2;
        return ByteBlock2.i;
    }
    else if(nBytes == 1){
        int nReturn = (int) sBuffer[*nCurPos];
        MarkBytes(*nCurPos, 1, nMarking);
        *nCurPos += 1;
        return nReturn;
    }
    else return -1;
}

float BinaryFile::ReadFloat(unsigned int * nCurPos, int nMarking, int nBytes){
    //std::cout<<string_format("ReadFloat() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > sBuffer.size()){
        std::cout<<"ReadFloat(): Reading past buffer size in "<<GetName()<<", aborting and returning -1.0.\n";
        return -1.0;
    }
    int n = 0;
    while(n < 4){
        ByteBlock4.bytes[n] = sBuffer[*nCurPos + n];
        n++;
    }
    MarkBytes(*nCurPos, 4, nMarking);
    *nCurPos += 4;
    return ByteBlock4.f;
}

Vector BinaryFile::ReadVector(unsigned int * nCurPos, int nMarking, int nBytes){
    if(*nCurPos+nBytes*3 > sBuffer.size()){
        std::cout<<"ReadVector(): Reading past buffer size in "<<GetName()<<", aborting and returning -1.0.\n";
        return Vector(-1.0, -1.0, -1.0);
    }
    int n = 0;
    double Coords[3];
    for(int m = 0; m < 3; m++){
        n = 0;
        while(n < 4){
            ByteBlock4.bytes[n] = sBuffer[*nCurPos + m*4 + n];
            n++;
        }
        Coords[m] = ByteBlock4.f;
    }
    MarkBytes(*nCurPos, 12, nMarking);
    *nCurPos += 12;
    return Vector(Coords[0], Coords[1], Coords[2]);
}

void BinaryFile::ReadString(std::string & sArray1, unsigned int * nCurPos, int nMarking, int nNumber){
    if(*nCurPos+nNumber > sBuffer.size()){
        std::cout<<"ReadString(): Reading past buffer size in "<<GetName()<<", aborting.\n";
        return;
    }
    sArray1.assign(&sBuffer[*nCurPos], nNumber);
    //std::cout<<"ReadString(): "<<sArray1<<"\n";
    MarkBytes(*nCurPos, nNumber, nMarking);
    *nCurPos += nNumber;
}

void BinaryFile::MarkBytes(unsigned int nOffset, int nLength, int nClass){
    int n = 0;
    //std::cout<<"Setting known: offset "<<nOffset<<" length "<<nLength<<" class "<<nClass<<"\n.";
    while(n < nLength && n < sBuffer.size()){
        if(bKnown[nOffset + n] != 0) std::cout<<"MarkBytes(): Warning! Data already interpreted as "<<bKnown[nOffset + n]<<" at offset "<<nOffset + n<<" in "<<GetName()<<"!\n";
        bKnown[nOffset + n] = nClass;
        n++;
    }
}

void BinaryFile::WriteIntToPH(int nInt, int nPH, unsigned int & nContainer){
    ByteBlock4.i = nInt;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer[nPH+n] = (ByteBlock4.bytes[n]);
    }
    nContainer = nInt;
}

void BinaryFile::WriteInt(int nInt, int nKnown, int nBytes){
    if(nBytes == 1){
        sBuffer.push_back((char) nInt);
        bKnown.push_back(nKnown);
        nPosition++;
    }
    else if(nBytes == 2){
        ByteBlock2.i = nInt;
        int n = 0;
        for(n = 0; n < 2; n++){
            sBuffer.push_back(ByteBlock2.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else if(nBytes == 4){
        ByteBlock4.i = nInt;
        int n = 0;
        for(n = 0; n < 4; n++){
            sBuffer.push_back(ByteBlock4.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else if(nBytes == 8){
        ByteBlock8.i = nInt;
        int n = 0;
        for(n = 0; n < 8; n++){
            sBuffer.push_back(ByteBlock8.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else Error("Cannot convert an integer to anything but 1, 2, 4 and 8 byte representations!");
}

void BinaryFile::WriteFloat(float fFloat, int nKnown, int nBytes){
    ByteBlock4.f = fFloat;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer.push_back(ByteBlock4.bytes[n]);
        bKnown.push_back(nKnown);
    }
    nPosition+=n;
}

void BinaryFile::WriteString(std::string sString, int nKnown){
    int n = 0;
    for(n = 0; n < sString.length(); n++){
        sBuffer.push_back(sString.at(n));
        bKnown.push_back(nKnown);
    }
    nPosition+=n;
}

void BinaryFile::WriteByte(char cByte, int nKnown){
    sBuffer.push_back(cByte);
    bKnown.push_back(nKnown);
    nPosition++;
}

bool TextFile::ReadFloat(double & fNew, std::string & sGetFloat, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading float at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading float at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"EditorDlgWindow::ReadFloat(): Reading: "<<sCheck<<". ";

    try{
        fNew = std::stof(sCheck, (size_t*) NULL);
        sGetFloat = sCheck;
    }
    catch(std::invalid_argument){
        std::cout<<"EditorDlgWindow::ReadFloat(): There was an error converting the string: "<<sCheck<<". Printing 0.0. \n";
        fNew = 0.0;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<fNew<<".\n";
    return true;
}

bool TextFile::ReadFloat(double & fNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading float at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading float at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Ascii::ReadFloat(): Reading: "<<sCheck<<". ";

    try{
        fNew = std::stof(sCheck, (size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout<<"Ascii::ReadFloat(): There was an error converting the string: "<<sCheck<<". Printing 0.0. \n";
        fNew = 0.0;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<fNew<<".\n";
    return true;
}

bool TextFile::ReadInt(int & nNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading int at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Reading int at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Ascii::ReadInt(): Reading: "<<sCheck<<". ";

    try{
        nNew = stoi(sCheck,(size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout<<"Ascii::ReadInt(): There was an error converting the string: "<<sCheck<<". Printing 0xFFFFFFFF. \n";
        nNew = 0xFFFFFFFF;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<nNew<<".\n";
    return true;
}

void TextFile::SkipLine(){
    if(sBuffer[nPosition] == 0x0A){
        nPosition+=1;
        return;
    }
    else if(sBuffer[nPosition] == 0x0D && sBuffer[nPosition+1] == 0x0A){
        nPosition+=2;
        return;
    }
    bool bStop = false;
    while(nPosition + 1 < sBuffer.size() && !bStop){
        if(sBuffer[nPosition] != 0x0D && sBuffer[nPosition+1] != 0x0A) nPosition++;
        else bStop = true;
    }
    nPosition+=2;
}

bool TextFile::EmptyRow(){
    int n = nPosition; //Do not use the iterator
    while( sBuffer[n] != 0x0D &&
           sBuffer[n+1] != 0x0A &&
           sBuffer[n] != '#' &&
           n+1 < sBuffer.size())
    {
        if(sBuffer[n] != 0x20 || (sBuffer[n+1] != 0x20 && sBuffer[n+1] != 0x0D && sBuffer[n+1] != '#')) return false;
        n++;
    }
    return true;
}

bool TextFile::ReadUntilText(std::string & sHandle, bool bToLowercase, bool bStrictNoNewLine){
    sHandle = ""; //Make sure the handle is cleared
    while(nPosition < sBuffer.size()){
        //std::cout<<"Looping in ReadUntilText main while(), nPosition="<<nPosition<<".\n";
        if(sBuffer[nPosition] == 0x20){
            //Skip space
            nPosition++;
            if(nPosition >= sBuffer.size()) return false;
        }
        else if(sBuffer[nPosition] == 0x0A){
            if(bStrictNoNewLine) return false;
            nPosition++;
            if(nPosition >= sBuffer.size()) return false;
        }
        else if(sBuffer[nPosition] == '#'){
            //Return because end of line and nothing was found
            return false;
        }
        else{
            if(nPosition + 1 < sBuffer.size()){
                if(sBuffer[nPosition] == 0x0D &&
                 sBuffer[nPosition+1] == 0x0A)
                {
                    //Return because end of line and nothing was found
                    return false;
                }
            }
            //Now it gets interesting - we may actually have relevant text now
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
            do{
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
                sHandle.push_back(sBuffer[nPosition]);
                nPosition++;
            }
            while(sBuffer[nPosition] != 0x20 &&
                  sBuffer[nPosition] != '#' &&
                  sBuffer[nPosition] != 0x0D &&
                  sBuffer[nPosition] != 0x0A &&
                  nPosition < sBuffer.size());

            //Report
            //if(sHandle != "") std::cout<<"ReadUntilText() found the following string: "<<sHandle<<".\n";

            //convert to lowercase
            if(bToLowercase) std::transform(sHandle.begin(), sHandle.end(), sHandle.begin(), ::tolower);

            //Go back and tell them you've found something
            return true;
        }
    }
    //Go back and tell them you're done
    return false;
}
