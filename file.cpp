#include "file.h"
#include <iostream>
#include <algorithm>
//#include "general.h"
#define DEBUG_LEVEL 0

bool TextFile::ReadFloat(double & fNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' ||
       sBuffer[nPosition] == 0x0D ||
       sBuffer[nPosition] == 0x0A)
    {
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
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
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' ||
           sBuffer[nPosition] == 0x0D ||
           sBuffer[nPosition] == 0x0A)
        {
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
    while(nPosition + 1 < nBufferSize && !bStop){
        if(sBuffer[nPosition] != 0x0D && sBuffer[nPosition+1] != 0x0A) nPosition++;
        else bStop = true;
    }
    nPosition+=2;
}

bool TextFile::EmptyRow(){
    int n = nPosition; //Do not use the iterator
    while(sBuffer[n] != 0x0D &&
        sBuffer[n+1] != 0x0A &&
                n+1 < nBufferSize)
    {
        if(sBuffer[n] != 0x20 || (sBuffer[n+1] != 0x20 && sBuffer[n+1] != 0x0D)) return false;
        n++;
    }
    return true;
}

bool TextFile::ReadUntilText(std::string & sHandle, bool bToLowercase, bool bStrictNoNewLine){
    sHandle = ""; //Make sure the handle is cleared
    while(nPosition < nBufferSize){
        //std::cout<<"Looping in ReadUntilText main while(), nPosition="<<nPosition<<".\n";
        if(sBuffer[nPosition] == 0x20){
            //Skip space
            nPosition++;
            if(nPosition >= nBufferSize) return false;
        }
        else if(sBuffer[nPosition] == 0x0A){
            if(bStrictNoNewLine) return false;
            nPosition++;
            if(nPosition >= nBufferSize) return false;
        }
        else if(sBuffer[nPosition] == '#'){
            //Return because end of line and nothing was found
            return false;
        }
        else{
            if(nPosition + 1 < nBufferSize){
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
                  nPosition < nBufferSize);

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
