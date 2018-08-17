#include "general.h"
#include "file.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <Shlwapi.h>

std::wstring Path::GetFullPath(){
    return sFullPath;
}
std::wstring Path::GetDirectory(){
    std::wstring sReturn = sFullPath;
    PathRemoveFileSpecW(&sReturn.front());
    sReturn = sReturn.c_str();
    sReturn += L"\\";
    return sReturn;
}
std::wstring Path::GetFilename(){
    std::wstring sReturn = sFullPath;    PathStripPathW(&sReturn.front());
    sReturn = sReturn.c_str();
    return sReturn;
}
std::wstring Path::GetFilenameNoExt(){
    std::wstring sReturn = sFullPath;
    sReturn = PathFindFileNameW(&sReturn.front());
    PathRemoveExtensionW(&sReturn.front());
    sReturn = sReturn.c_str();
    return sReturn;
}
std::wstring Path::GetExtension(){
    std::wstring sReturn = sFullPath;
    sReturn = PathFindExtensionW(&sReturn.front());
    return sReturn;
}

HANDLE bead_CreateReadFile(const std::string & sFilename){
    return CreateFileA(sFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
HANDLE bead_CreateReadFile(const std::wstring & sFilename){
    return CreateFileW(sFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
HANDLE bead_CreateWriteFile(const std::string & sFilename){
    return CreateFileA(sFilename.c_str(), GENERIC_WRITE, 0x00, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}
HANDLE bead_CreateWriteFile(const std::wstring & sFilename){
    return CreateFileW(sFilename.c_str(), GENERIC_WRITE, 0x00, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}
long unsigned bead_GetFileLength(HANDLE hFile){
    LARGE_INTEGER lnSize;
    if(GetFileSizeEx(hFile, &lnSize)){
        return static_cast<long unsigned>(lnSize.QuadPart);
    }
    return 0;
}

DWORD g_BytesTransferred = 0;
VOID CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped){
    std::cout << "Error code:\t" << dwErrorCode << "\n";
    std::cout << "Number of bytes:\t" << dwNumberOfBytesTransfered << "\n";
    g_BytesTransferred = dwNumberOfBytesTransfered;
}

bool bead_ReadFile(HANDLE hFile, std::vector<char> & sBuffer, unsigned long nToRead){
    OVERLAPPED ol = {0};
    unsigned long length = nToRead;
    if(nToRead > bead_GetFileLength(hFile)) length = bead_GetFileLength(hFile);
    return ReadFileEx(hFile, &sBuffer.front(), length, &ol, FileIOCompletionRoutine);
}
bool bead_WriteFile(HANDLE hFile, const std::string & sBuffer, unsigned long nToWrite){
    OVERLAPPED ol = {0};
    unsigned long length = nToWrite;
    if(nToWrite > sBuffer.length()) length = sBuffer.length();
    return WriteFileEx(hFile, &sBuffer.front(), length, &ol, FileIOCompletionRoutine);
}

void File::SetFilePath(std::wstring & sPath){
    path.Set(sPath);
}

unsigned char * BinaryFile::ReadBytes(unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * p_insert, bool bAdvancePlaceholder){
    unsigned nCurPos = p_insert == nullptr ? nPosition : *p_insert;

    if(sDesc.length() && nBytes) positions.emplace_back(nCurPos, nBytes, sDesc);

    if(nCurPos + nBytes > sBuffer.size()){
        throw mdlexception("Attempting to read in " + GetName() + " at offset " + std::to_string(nCurPos) + ", which would read past the buffer size (" + std::to_string(sBuffer.size()) + ").");
    }

    MarkBytes(nCurPos, nBytes, nMarking);

    if(p_insert == nullptr) nPosition += nBytes;
    else if(bAdvancePlaceholder) *p_insert += nBytes;

    return (unsigned char*) &sBuffer.at(nCurPos);
}
/*
int BinaryFile::ReadInt(unsigned int * nCurPos, int nMarking, int nBytes){
    //std::cout << string_format("ReadInt() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > sBuffer.size()){
        throw mdlexception("Attempting to read integer in " + GetName() + " at offset " + std::to_string(*nCurPos) + ", which would read past the buffer size (" + std::to_string(sBuffer.size()) + ").");
        std::cout << "ReadInt(): Reading past buffer size in " << GetName() << ", aborting and returning -1.\n";
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
        //std::cout << "ReadInt() return: " << ByteBlock4.i << "\n";
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
    //std::cout << string_format("ReadFloat() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > sBuffer.size()){
        throw mdlexception("Attempting to read float in " + GetName() + " at offset " + std::to_string(*nCurPos) + ", which would read past the buffer size (" + std::to_string(sBuffer.size()) + ").");
        std::cout << "ReadFloat(): Reading past buffer size in " << GetName() << ", aborting and returning -1.0.\n";
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
        throw mdlexception("Attempting to read vector in " + GetName() + " at offset " + std::to_string(*nCurPos) + ", which would read past the buffer size (" + std::to_string(sBuffer.size()) + ").");
        std::cout << "ReadVector(): Reading past buffer size in " << GetName() << ", aborting and returning -1.0.\n";
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
        MarkBytes(*nCurPos + m * 4, 4, nMarking);
    }
    *nCurPos += 12;
    return Vector(Coords[0], Coords[1], Coords[2]);
}

void BinaryFile::ReadString(std::string & sArray1, unsigned int * nCurPos, int nMarking, int nNumber){
    if(*nCurPos+nNumber > sBuffer.size()){
        throw mdlexception("Attempting to read string in " + GetName() + " at offset " + std::to_string(*nCurPos) + ", which would read past the buffer size (" + std::to_string(sBuffer.size()) + ").");
        std::cout << "ReadString(): Reading past buffer size in " << GetName() << ", aborting.\n";
        return;
    }
    sArray1.assign(&sBuffer[*nCurPos], nNumber);
    //std::cout << "ReadString(): " << sArray1 << "\n";
    MarkBytes(*nCurPos, nNumber, nMarking);
    *nCurPos += nNumber;
}
*/

void BinaryFile::MarkDataBorder(unsigned nOffset){
    bKnown.at(nOffset) = bKnown.at(nOffset) | 0x20000;
}

void BinaryFile::MarkBytes(unsigned int nOffset, int nLength, int nClass){
    //std::cout << "Setting known: offset " << nOffset << " length " << nLength << " class " << nClass << ".\n";
    if(nOffset > 0) bKnown.at(nOffset - 1) = bKnown.at(nOffset - 1) | 0x10000;
    for(unsigned n = 0; n < nLength && n < sBuffer.size(); n++){
        if((bKnown.at(nOffset + n) & 0xFFFF) != 0){
            std::cout << "MarkBytes(): Warning! Data already interpreted as " << bKnown.at(nOffset + n) << " at offset " << nOffset + n << " in " << GetName() << "! Trying to reinterpret as " << nClass << ".\n";
            throw mdlexception("Interpreting already interpreted data in " + GetName() + ".");
        }
        bKnown.at(nOffset + n) = (nClass & 0xFFFF) + (bKnown.at(nOffset + n) & 0xFFFF0000);
        if(n + 1 == nLength) bKnown.at(nOffset + n) = bKnown.at(nOffset + n) | 0x10000;
    }
}

/**
    Writes nBytes number of bytes from the pointer p_bytes. If p_insert is nullptr,
    it appends the bytes to the buffer vector. If p_insert is valid, then it points
    to a position in the buffer, and that's where the bytes will be written.
**/
unsigned BinaryFile::WriteBytes(unsigned char * p_bytes, unsigned nBytes, int nMarking, const std::string & sDesc, unsigned * p_insert){
    //std::cout << "Writing data " << nBytes << " bytes long to " << (p_insert ? *p_insert : nPosition) << ".\n";
    unsigned nReturn = nPosition;
    for(int n = 0; n < nBytes; n++){
        if(p_insert == nullptr){
            sBuffer.push_back(*(p_bytes + n));
            bKnown.push_back(nMarking | (n + 1 == nBytes ? 0x10000 : 0));
        }
        else sBuffer.at((*p_insert) + n) = *(p_bytes + n);
    }
    //if(nMarking > 0) MarkBytes(nReturn, nBytes, nMarking);
    if(p_insert == nullptr){
        if(sDesc.length() && nBytes) positions.emplace_back(nPosition, nBytes, sDesc);
        nPosition += nBytes;
    }
    return nReturn;
}
/*
void BinaryFile::WriteIntToPH(int nInt, int nPH, unsigned int & nContainer){
    ByteBlock4.i = nInt;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer[nPH+n] = (ByteBlock4.bytes[n]);
    }
    nContainer = nInt;
}

void BinaryFile::WriteAtOffset4(int nInt, unsigned int nOffset){
    ByteBlock4.i = nInt;
    for(int n = 0; n < 4; n++){
        sBuffer[nOffset+n] = (ByteBlock4.bytes[n]);
    }
}
void BinaryFile::WriteAtOffset2(short nShort, unsigned int nOffset){
    ByteBlock2.i = nShort;
    for(int n = 0; n < 2; n++){
        sBuffer[nOffset+n] = (ByteBlock2.bytes[n]);
    }
}
void BinaryFile::WriteAtOffset1(char nChar, unsigned int nOffset){
    sBuffer[nOffset] = nChar;
}
void BinaryFile::WriteAtOffset(float fFloat, unsigned int nOffset){
    ByteBlock4.f = fFloat;
    for(int n = 0; n < 4; n++){
        sBuffer[nOffset+n] = (ByteBlock4.bytes[n]);
    }
}
void BinaryFile::WriteAtOffset(const std::string & sString, unsigned int nOffset){
    for(int n = 0; n < sString.length(); n++){
        sBuffer[nOffset+n] = sString.at(n);
    }
}

void BinaryFile::WriteInt(int nInt, int nKnown, int nBytes){
    if(nBytes == 1){
        if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
        sBuffer.push_back((char) nInt);
        bKnown.push_back(nKnown | 0x10000);
        nPosition++;
    }
    else if(nBytes == 2){
        ByteBlock2.i = nInt;
        if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
        int n = 0;
        for(n = 0; n < 2; n++){
            sBuffer.push_back(ByteBlock2.bytes[n]);
            bKnown.push_back(nKnown | (n + 1 == 2 ? 0x10000 : 0));
        }
        nPosition+=n;
    }
    else if(nBytes == 4){
        ByteBlock4.i = nInt;
        if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
        int n = 0;
        for(n = 0; n < 4; n++){
            sBuffer.push_back(ByteBlock4.bytes[n]);
            bKnown.push_back(nKnown | (n + 1 == 4 ? 0x10000 : 0));
        }
        nPosition+=n;
    }
    else if(nBytes == 8){
        ByteBlock8.i = nInt;
        if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
        int n = 0;
        for(n = 0; n < 8; n++){
            sBuffer.push_back(ByteBlock8.bytes[n]);
            bKnown.push_back(nKnown | (n + 1 == 8 ? 0x10000 : 0));
        }
        nPosition+=n;
    }
    else Error("Cannot convert an integer to anything but 1, 2, 4 and 8 byte representations!");
}

void BinaryFile::WriteFloat(float fFloat, int nKnown, int nBytes){
    ByteBlock4.f = fFloat;
    if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer.push_back(ByteBlock4.bytes[n]);
        bKnown.push_back(nKnown | (n + 1 == 4 ? 0x10000 : 0));
    }
    nPosition+=n;
}

void BinaryFile::WriteString(std::string sString, int nKnown){
    if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
    int n = 0;
    for(n = 0; n < sString.length(); n++){
        sBuffer.push_back(sString.at(n));
        bKnown.push_back(nKnown | (n + 1 == sString.length() ? 0x10000 : 0));
    }
    nPosition+=n;
}

void BinaryFile::WriteByte(char cByte, int nKnown){
    if(bKnown.size() > 0) bKnown.back() = bKnown.back() | 0x10000;
    sBuffer.push_back(cByte);
    bKnown.push_back(nKnown | 0x10000);
    nPosition++;
}
*/
bool TextFile::ReadFloat(double & fNew, std::string * sGet, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A ||
       sBuffer[nPosition] == 0x00)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading float at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A ||
           sBuffer[nPosition] == 0x00)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading float at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A &&
            sBuffer[nPosition] != 0x00)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "TextFile::ReadFloat(): Reading: " << sCheck << ". ";

    if(sGet != nullptr) *sGet = sCheck;

    try{
        fNew = std::stod(sCheck, (size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout << "TextFile::ReadFloat(): There was an error converting the string: " << sCheck << ". \n";
        throw mdlexception("Could not convert '" + sCheck + "' to float.");
    }
    catch(std::out_of_range){
        std::cout << "TextFile::ReadFloat(): The float is out of range: " << sCheck << ".\n";
        throw mdlexception("The float " + sCheck + " is out of range.");
    }
    catch(...){
        std::cout << "TextFile::ReadFloat(): An unknown exception occurred while converting: " << sCheck << ". \n";
        throw mdlexception("An unknown exception occurred while converting '" + sCheck + "' to float.");
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "Converted: " << fNew << ".\n";
    return true;
}

bool TextFile::ReadInt(int & nNew, std::string * sGet, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A ||
       sBuffer[nPosition] == 0x00)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading int at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A ||
           sBuffer[nPosition] == 0x00)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading int at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A &&
            sBuffer[nPosition] != 0x00)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "TextFile::ReadInt(): Reading: " << sCheck << ". ";

    if(sGet != nullptr) *sGet = sCheck;

    try{
        nNew = stoi(sCheck,(size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout << "TextFile::ReadInt(): There was an error converting the string: " << sCheck << ". Printing 0xFFFFFFFF. \n";
        throw mdlexception("Could not convert '" + sCheck + "' to integer.");
    }
    catch(std::out_of_range){
        std::cout << "TextFile::ReadInt(): The integer is out of range: " << sCheck << ".\n";
        throw mdlexception("The integer " + sCheck + " is out of range.");
    }
    catch(...){
        std::cout << "TextFile::ReadInt(): An unknown exception occurred while converting: " << sCheck << ". Printing 0xFFFFFFFF. \n";
        throw mdlexception("An unknown exception occurred while converting '" + sCheck + "' to integer.");
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "Converted: " << nNew << ".\n";
    return true;
}

bool TextFile::ReadUInt(unsigned int & nNew, std::string * sGet, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A ||
       sBuffer[nPosition] == 0x00)
    {
        if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading int at end of line!\n";
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A ||
           sBuffer[nPosition] == 0x00)
        {
            if(bPrint || DEBUG_LEVEL > 5) std::cout << "Reading int at end of line!\n";
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A &&
            sBuffer[nPosition] != 0x00)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "TextFile::ReadInt(): Reading: " << sCheck << ". ";

    if(sGet != nullptr) *sGet = sCheck;

    try{
        nNew = stou(sCheck,(size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout << "TextFile::ReadInt(): There was an error converting the string: " << sCheck << ". Printing 0xFFFFFFFF. \n";
        throw mdlexception("Could not convert '" + sCheck + "' to integer.");
    }
    catch(std::out_of_range){
        std::cout << "TextFile::ReadInt(): The integer is out of range: " << sCheck << ".\n";
        throw mdlexception("The integer " + sCheck + " is out of range.");
    }
    catch(...){
        std::cout << "TextFile::ReadInt(): An unknown exception occurred while converting: " << sCheck << ". Printing 0xFFFFFFFF. \n";
        throw mdlexception("An unknown exception occurred while converting '" + sCheck + "' to integer.");
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout << "Converted: " << nNew << ".\n";
    return true;
}

void TextFile::SkipLine(){
    if(nPosition + 1 >= sBuffer.size()){
        /// End of file, set position to eof
        nPosition = sBuffer.size();
        //throw mdlexception("Error: Trying to skip a line at the end of the file.");
    }

    while(nPosition + 1 < sBuffer.size()){
        if(sBuffer[nPosition] != 0x0D && sBuffer[nPosition] != 0x0A) nPosition++;
        else break;
    }

    if(sBuffer[nPosition] == 0x0A){
        nPosition+=1;
        nLine++;
        return;
    }
    else if(sBuffer[nPosition] == 0x0D && sBuffer[nPosition+1] == 0x0A){
        nPosition+=2;
        nLine++;
        return;
    }
    else if(sBuffer[nPosition] == 0x0D){
        nPosition+=1;
        nLine++;
        return;
    }
}

void File::SavePosition(unsigned n){
    if(SavedPosition.size() <= n) SavedPosition.resize(n+1);
    if(SavedLine.size() <= n) SavedLine.resize(n+1);
    SavedPosition.at(n) = nPosition;
    SavedLine.at(n) = nLine;
}

bool File::RestorePosition(unsigned n){
    if(SavedPosition.size() <= n || SavedLine.size() <= n) return false;
    nPosition = SavedPosition.at(n);
    nLine = SavedLine.at(n);
    return true;
}

bool TextFile::EmptyRow(){
    int n = nPosition; //Do not use the iterator
    while( n < sBuffer.size() &&
           sBuffer[n] != 0x0A &&
           sBuffer[n] != 0x0D &&
           sBuffer[n] != 0x00 &&
           sBuffer[n] != '#' )
    {
        if(sBuffer[n] != 0x20) return false;
        n++;
    }
    return true;
}

bool TextFile::ReadUntilText(std::string & sHandle, bool bToLowercase){
    sHandle = ""; //Make sure the handle is cleared
    while(nPosition < sBuffer.size() &&
          sBuffer[nPosition] != '#' &&
          sBuffer[nPosition] != 0x0A &&
          sBuffer[nPosition] != 0x0D &&
          sBuffer[nPosition] != 0x00 )
    {
        if(sBuffer[nPosition] == 0x20){
            //Skip space
            nPosition++;
        }
        else{
            //Now it gets interesting - we may actually have relevant text now
            do{
                sHandle.push_back(sBuffer[nPosition]);
                nPosition++;
            }
            while(nPosition < sBuffer.size() &&
                  sBuffer[nPosition] != 0x20 &&
                  sBuffer[nPosition] != '#' &&
                  sBuffer[nPosition] != 0x0D &&
                  sBuffer[nPosition] != 0x0A);

            //convert to lowercase
            if(bToLowercase) std::transform(sHandle.begin(), sHandle.end(), sHandle.begin(), ::tolower);

            //Go back and tell them you've found something
            return true;
        }
    }
    //Go back and tell them you're done
    return false;
}

void IniFile::ReadIni(std::wstring & sIni){
    //std::ifstream fIni (sIni.c_str(), std::ios::binary);
    HANDLE fIni = bead_CreateReadFile(sIni); //CreateFileW(sIni.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    //if(!fIni.is_open()){
    if(fIni == INVALID_HANDLE_VALUE){
        throw mdlexception("Error opening .ini file.");
    }

    //fIni.seekg(0, std::ios::end);
    //std::streampos length = fIni.tellg();
    //fIni.seekg(0, std::ios::beg);
    //LARGE_INTEGER lnSize;
    //if(!GetFileSizeEx(fIni, &lnSize)){
    //    throw mdlexception("Error: .ini file empty .");
    //}
    //long unsigned length = lnSize.QuadPart;

    std::vector<char> & sBufferRef = CreateBuffer(bead_GetFileLength(fIni));

    //fIni.read(&sBufferRef[0], length);
    if(!bead_ReadFile(fIni, sBufferRef)){ //if(!ReadFileEx(fIni, &sBufferRef.front(), length, NULL, NULL)){
        throw mdlexception("Error reading .ini file.");
    }

    //fIni.close();
    CloseHandle(fIni);

    SetFilePath(sIni);

    std::string sID;
    int nConvert = 0;
    unsigned uConvert = 0;
    double fConvert = 0.0;

    while(nPosition < sBuffer.size()){
        if(EmptyRow()){
            SkipLine();
        }
        else{
            ReadUntilText(sID, true);
            for(IniOption & option : Options){
                std::string sToken = option.sToken;
                std::transform(sToken.begin(), sToken.end(), sToken.begin(), ::tolower);

                if(sID != sToken) continue;

                switch(option.nType){
                    case DT_bool: if(ReadInt(nConvert)) *((bool*) option.lpVariable) = (nConvert == 0 ? false : true);
                    break;
                    case DT_int: if(ReadInt(nConvert)) *((int*) option.lpVariable) = nConvert;
                    break;
                    case DT_uint: if(ReadUInt(uConvert)) *((unsigned*) option.lpVariable) = uConvert;
                    break;
                    case DT_float: if(ReadFloat(fConvert)) *((double*) option.lpVariable) = fConvert;
                    break;
                    case DT_string: if(ReadUntilText(sID, false)) *((std::string*) option.lpVariable) = sID;
                    break;
                }
            }
        }
    }
}

void IniFile::WriteIni(std::wstring & sIni){
    //std::ofstream fIni (sIni, std::ios::binary);
    //if(!fIni.is_open()){
    //    throw mdlexception("Error opening .ini file.");
    //}
    std::stringstream fIni;

    for(IniOption & option : Options){
        fIni << option.sToken << " ";

        switch(option.nType){
            case DT_bool: fIni << (*((bool*) option.lpVariable) ? 1 : 0);
            break;
            case DT_int: fIni << *((int*) option.lpVariable);
            break;
            case DT_uint: fIni << *((unsigned*) option.lpVariable);
            break;
            case DT_float: fIni << PrepareFloat(*((double*) option.lpVariable), true);
            break;
            case DT_string: fIni << *((std::string*) option.lpVariable);
            break;
        }

        if(&option != &Options.back()) fIni << "\r\n";
    }

    HANDLE hIni = bead_CreateWriteFile(sIni); //CreateFileW(sIni.c_str(), GENERIC_WRITE, 0x00, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hIni == INVALID_HANDLE_VALUE){
        throw mdlexception("Error opening .ini file.");
    }

    if(!bead_WriteFile(hIni, fIni.str())){ //if(!WriteFileEx(hIni, &fIni.str().front(), fIni.str().length(), NULL, NULL)){
        throw mdlexception("Error writing .ini file.");
    }

    CloseHandle(hIni);
}
