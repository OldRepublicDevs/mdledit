#include "general.h"

bool BVstrcmp(TCHAR * string1, TCHAR * string2, bool bDebug){
    //std::cout<<string_format("BVstrcmp: to compare <%ls> and <%ls>: \n", string1, string2);
    int n = 0;
    bool bReturn = true;
    //std::cout<<string_format("Comparing %ls and %ls \n", string1, string2);
    if(string1 == NULL || string2 == NULL) return false;
    while(n < 5000){
        //std::cout<<string_format("BVstrcmp: in while...  \n");
        if(string1[n] != string2[n]) return false;
        if(bDebug) std::cout<<string_format("BVstrcmp: comparing <%lc> and <%lc> ... result: %i \n", string1[n], string2[n], bReturn);
        if(string1[n] == L'\0' && string2[n] == L'\0') return true;
        n++;
    }
    return true;
}

void BVright(TCHAR * string1, int n){
    if(strlen(string1)<=n) return;
    int f = 0;
    while(f<=n){
        string1[f] = string1[strlen(string1)-(n-f)];
        f++;
    }
}

void BVleft(TCHAR * string1, int n){
    if(strlen(string1)<=n) return;
    string1[n] = L'\0';
}

int pown(int base, int exp){
    int result = 1;
    for(int n = 0; n < exp; n++){
        result *= base;
    }
    /*
    while(exp){
        if(exp & 1) result *= base;
        exp /= 2;
        base *= base;
    }
    */
    return result;
}
/*
float powf(float base, float exp){
    return (float) pow((double) base, (double) exp);
}*/

float deg(float rad){
    return rad * 180.0 / 3.141592653589793;

}

float rad(float deg){
    return deg * 3.141592653589793 / 180.0;
}

double deg(double rad){
    return rad * 180.0 / 3.141592653589793;

}

double rad(double deg){
    return deg * 3.141592653589793 / 180.0;
}

void QuaternionToAA(float * fQuaternion, float * fAA){
    if(fQuaternion[3] > 1.0){
        //Normalize
    }
    fAA[3] = 2.0 * acosf(fQuaternion[3]);
    float s = sqrtf( (1.0 - powf(fQuaternion[3], 2.0)));
    if(s < 0.001){
        //Without normalization
        // if it is important that axis is normalised then replace with x=1; y=z=0;
        // MDLOps turns them into 0,0,0. Need to check what the best solution for NWMax is (maybe it's irrelevant)
        fAA[0] = 1.0; //fQuaternion[0];
        fAA[1] = 0.0; //fQuaternion[1];
        fAA[2] = 0.0; //fQuaternion[2];
    }
    else{
        fAA[0] = fQuaternion[0] / s;
        fAA[1] = fQuaternion[1] / s;
        fAA[2] = fQuaternion[2] / s;
    }
}

void QuaternionToAA(double * fQuaternion, double * fAA){
    if(fQuaternion[3] > 1.0){
        //Normalize
    }
    fAA[3] = 2.0 * acosf(fQuaternion[3]);
    double s = sqrtf( (1.0 - powf(fQuaternion[3], 2.0)));
    if(s < 0.001){
        //Without normalization
        // if it is important that axis is normalised then replace with x=1; y=z=0;
        // MDLOps turns them into 0,0,0. Need to check what the best solution for NWMax is (maybe it's irrelevant)
        fAA[0] = 1.0; //fQuaternion[0];
        fAA[1] = 0.0; //fQuaternion[1];
        fAA[2] = 0.0; //fQuaternion[2];
    }
    else{
        fAA[0] = fQuaternion[0] / s;
        fAA[1] = fQuaternion[1] / s;
        fAA[2] = fQuaternion[2] / s;
    }
}

void AAToQuaternion(float * fAA, float * fQuaternion){
    float a = fAA[3] / 2.0;
    fQuaternion[3] = cosf(a);
    fQuaternion[0] = fAA[0] * sinf(a);
    fQuaternion[1] = fAA[1] * sinf(a);
    fQuaternion[2] = fAA[2] * sinf(a);
}

void AAToQuaternion(double * fAA, double * fQuaternion){
    double a = fAA[3] / 2.0;
    fQuaternion[3] = cosf(a);
    fQuaternion[0] = fAA[0] * sinf(a);
    fQuaternion[1] = fAA[1] * sinf(a);
    fQuaternion[2] = fAA[2] * sinf(a);
}

char DecToHexDigit(int nDec){
    if(nDec == 0) return '0';
    else if(nDec == 1) return '1';
    else if(nDec == 2) return '2';
    else if(nDec == 3) return '3';
    else if(nDec == 4) return '4';
    else if(nDec == 5) return '5';
    else if(nDec == 6) return '6';
    else if(nDec == 7) return '7';
    else if(nDec == 8) return '8';
    else if(nDec == 9) return '9';
    else if(nDec == 10) return 'A';
    else if(nDec == 11) return 'B';
    else if(nDec == 12) return 'C';
    else if(nDec == 13) return 'D';
    else if(nDec == 14) return 'E';
    else if(nDec == 15) return 'F';
}

void AddSignificantZeroes(char * cInt, int nSignificant){
    //string max length must be greater or equal to nSignificant. Otherwise, expect unpredictable results.
    int nLen = strlen(cInt);
    if(nLen >= nSignificant) return;
    int n = 0;
    char * cString = new char [nSignificant+1];
    while(n < nSignificant){
        if(n < nSignificant - nLen) cString[n] = '0';
        else cString[n] = cInt[n - (nSignificant - nLen)];
        n++;
    }
    cString[nSignificant] = '\0';
    sprintf(cInt, cString);
    delete [] cString;
}

//Removes final zeros, unless a decimal operator precedes it.
void TruncateDec(TCHAR * tcString){
    int nLen = strlen(tcString);
    int n = 0;
    while(tcString[nLen-n]=='\0' || tcString[nLen-n]=='0'){
        if(tcString[nLen-n]=='0' && isdigit(tcString[nLen-n-1])) tcString[nLen-n] = '\0';
        n++;
    }
}

//This replaces chars that display weirdly in font Consolas with spaces and replaces \0 with period.
void PrepareCharForDisplay(char * cChar){
    switch((unsigned char)*cChar){
        case 0x00:
            *cChar = '.';
        break;
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x7F:
        case 0x81:
        case 0x83:
        case 0x88:
        case 0x90:
        case 0x98:
            *cChar = ' ';
        break;
    }
}

void CharsToHex(char * cOutput, std::vector<char> & cInput, int nOffset, int nNumber){
    int n = 0;
    int nDigit1;
    int nDigit2;
    while(n < nNumber){
        nDigit1 = (unsigned char) cInput[nOffset + n] / 16;
        nDigit2 = (unsigned char) cInput[nOffset + n] - nDigit1 * 16;
        cOutput[n*3 + 0] = DecToHexDigit(nDigit1);
        cOutput[n*3 + 1] = DecToHexDigit(nDigit2);
        if(n+1 == nNumber) cOutput[n*3 + 2] = '\0';
        else cOutput[n*3 + 2] = ' ';
        n++;
    }
}

double RoundDec(double fNumber, int nDecPlaces){
    if(nDecPlaces < 0){
        //Error
        printf("RoundDec() ERROR: number of decimal places indicated as a negative number. \n");
        return fNumber;
    }
    double fFactor = pow(10.0, -1.0 * (double) nDecPlaces);
    double fReturn = fNumber / fFactor;
    fReturn = round(fReturn);
    fReturn = fReturn * fFactor;
    return fReturn;
}

double RoundDec(float fNumber, int nDecPlaces){
    if(nDecPlaces < 0){
        //Error
        printf("RoundDec() ERROR: number of decimal places indicated as a negative number. \n");
        return fNumber;
    }
    double fFactor = pow(10.0, -1.0 * (double) nDecPlaces);
    double fReturn = ((double) fNumber) / fFactor;
    fReturn = round(fReturn);
    fReturn = fReturn * fFactor;
    return fReturn;
}

int Error(std::string sErrorMessage){
    return MessageBox(hFrame, sErrorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
}

int WarningCancel(std::string sWarningMessage){
    return MessageBox(hFrame, sWarningMessage.c_str(), "Warning!", MB_OKCANCEL | MB_ICONWARNING);
}

int Warning(std::string sWarningMessage){
    return MessageBox(hFrame, sWarningMessage.c_str(), "Warning!", MB_OK | MB_ICONWARNING);
}

void CopyBuffer(std::vector<char> & cCopyTo, char * cCopyFrom, int nCount){
    int n = 0;
    while(n<nCount){
        cCopyTo[n] = cCopyFrom[n];
        n++;
    }
}

char * operator&(std::string & sStr){
    if(sStr.length() > 0) return &sStr.at(0);
    else return nullptr;
}

bool bCursorOnLine(POINT pt, POINT ptLine1, POINT ptLine2, int nOffset){
    int nx = ptLine1.x;
    int nx2 = ptLine2.x;
    int ny = ptLine1.y;
    int ny2 = ptLine2.y;
    if(nx==nx2){
        if(pt.x >= nx-nOffset && pt.x <= nx+nOffset && pt.y >= ny && pt.y <= ny2 && ny2 > ny) return true;
        else if(pt.x >= nx-nOffset && pt.x <= nx+nOffset && pt.y <= ny && pt.y >= ny2 && ny2 < ny) return true;
        else return false;
    }
    else if(ny==ny2){
        if(pt.y >= ny-nOffset && pt.y <= ny+nOffset && pt.x >= nx && pt. x<= nx2 && nx2 > nx) return true;
        else if(pt.y >= ny-nOffset && pt.y <= ny+nOffset && pt.x <= nx && pt.x >= nx2 && nx2 < nx) return true;
        else return false;
    }

    double fx1;
    double fy1;
    double fx2;
    double fy2;
    if(nx < nx2){
        fx1 = (double) nx;
        fx2 = (double) nx2;
        fy1 = (double) ny;
        fy2 = (double) ny2;
    }
    else{
        fx1 = (double) nx2;
        fx2 = (double) nx;
        fy1 = (double) ny2;
        fy2 = (double) ny;
    }
    double k = 1.0 * (fy2 - fy1) / (fx2 - fx1);
    double n = fy1 - k * fx1;
    double k2 = -1.0 * (fx2 - fx1) / (fy2 - fy1);
    double n2a = fy1 - k2 * fx1;
    double n2b = fy2 - k2 * fx2;
    double d = sqrt(pow((fy2 - fy1), 2.0) + pow((fx2 - fx1), 2.0));
    double p = (double) nOffset;
    double r = d * p / (fx2 - fx1);
    double x = (double) pt.x;
    double y = (double) pt.y;
    if(y >= k*x+n-r &&
        y <= k*x+n+r &&
        y >= k2*x+n2a &&
        y <= k2*x+n2b &&
        fy1 < fy2)
    {
        return true;
    }
    else if(y >= k*x+n-r &&
            y <= k*x+n+r &&
            y <= k2*x+n2a &&
            y >= k2*x+n2b &&
            fy1 > fy2)
    {
        return true;
    }
    else return false;
}

std::string safesubstr(const std::string & sParam, size_t nStart, size_t nEnd){
    if(nStart > sParam.length() || nEnd < 0 || nStart < 0) return sParam;
    else return sParam.substr(nStart, nEnd);
}
