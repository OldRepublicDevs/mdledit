#ifndef MDL_H_INCLUDED
#define MDL_H_INCLUDED

#include "general.h"
#include "MDLdefinitions.h"

/**
    UNKNOWNS:

    1. Non-static data
     - Supermodel int32 in Header
     - last three bytes (padding?) after Classification in Header
       - ndix UR: unconfirmed theory, the byte immediately following == 4 for placeables,
                  2 for most characaters and a few other random things, in NWN it was
                  'is this model affected by fog' I've spent a little bit of time trying to see
                  if this might be the same, inconclusive. my current logic for this is if
                  classification = placeable, then set to 4, otherwise set to 0.
     - last three bytes (padding?) after ModelType in Header and Animation
       - ndix UR: padding
     - bytes after RenderOrder in EmitterHeader (possibly padding like above?)
     - short nUnknown2 and last three bytes (padding?) in Controller
     - an unknown array in LightHeader (remains unknown and unused)
     - MeshHeader:
       - two unknown bytes after Render
       - Unknown float at the end
       - Unknown lightsaber bytes
       - K2 unknown 1 (float)

    2. Static data - always 0 (usually it's 0)
     - short nPadding1 in NodeHeader
     - 3 int32s in SkinHeader
     - K2 unknown 2 in MeshHeader
     - zero1 and zero2 in EmitterHeader
     - -1 -1 0 Array in MeshHeader

/**/

//Forward declarations
struct Face;
struct Aabb;
struct Controller;
struct Sound;
struct Name;
struct SaberDataStruct;
struct VertIndicesStruct;
struct MDXDataStruct;
struct Vertex;
struct Node;
struct Header;
struct LightHeader;
struct EmitterHeader;
struct MeshHeader;
struct SkinHeader;
struct DanglymeshHeader;
struct WalkmeshHeader;
struct SaberHeader;
struct Animation;
struct GeometryHeader;
struct ModelHeader;
struct FileHeader;
struct Vector;
class Orientation;
struct Color;
struct Triples;
struct Bone;

extern ByteBlock2 ByteBlock2;
extern ByteBlock4 ByteBlock4;
extern ByteBlock8 ByteBlock8;

/**** DATA STRUCTS ****/

struct Matrix22{
    double f11;
    double f12;
    double f21;
    double f22;

    Matrix22(const double & f1, const double & f2, const double & f3, const double & f4){
        f11 = f1;
        f12 = f2;
        f21 = f3;
        f22 = f4;
    }
};

extern const char QU_X;
extern const char QU_Y;
extern const char QU_Z;
extern const char QU_W;
extern const char AA_X;
extern const char AA_Y;
extern const char AA_Z;
extern const char AA_A;
class Orientation{
    //unsigned int nCompressed = 0; //Since it is compressed, best to get it in a neutral type
    double qX = 0.0;
    double qY = 0.0;
    double qZ = 0.0;
    double qW = 0.0;
    double fX = 0.0;
    double fY = 0.0;
    double fZ = 0.0;
    double fAngle = 0.0;
    bool bQuaternion = false, bAA = false;

  public:
    Orientation(){
        qX = 0.0;
        qY = 0.0;
        qZ = 0.0;
        qW = 1.0;
        bQuaternion = true;
        bAA = false;
    }
    Orientation(const double & f1, const double & f2, const double & f3, const double & f4){
        qX = f1;
        qY = f2;
        qZ = f3;
        qW = f4;
        bQuaternion = true;
        bAA = false;
    }
    void Quaternion(const double & f1, const double & f2, const double & f3, const double & f4){
        qX = f1;
        qY = f2;
        qZ = f3;
        qW = f4;
        bQuaternion = true;
        bAA = false;
    }
    void AA(const double & f1, const double & f2, const double & f3, const double & f4){
        fX = f1;
        fY = f2;
        fZ = f3;
        fAngle = f4;
        bQuaternion = false;
        bAA = true;
    }
    void ConvertToQuaternions(){
        if(!bAA) std::cout<<"Orientation::ConvertToQuaternions(): Calculating with invalid AA values\n";
        double a = fAngle / 2.0;
        qW = cosf(a);
        qX = fX * sinf(a);
        qY = fY * sinf(a);
        qZ = fZ * sinf(a);
        bQuaternion = true;
    }
    void ConvertToAA(){
        if(!bQuaternion) std::cout<<"Orientation::ConvertToAA(): Calculating with invalid quaternion values\n";
        if(qW > 1.0){
            //Normalize
        }
        fAngle = 2.0 * acosf(qW);
        double s = sqrtf( (1.0 - powf(qW, 2.0)));
        if(s < 0.001){
            //Without normalization
            // if it is important that axis is normalised then replace with x=1; y=z=0;
            // MDLOps turns them into 0,0,0. Need to check what the best solution for NWMax is (maybe it's irrelevant)
            fX = 1.0; //qX;
            fY = 0.0; //qY;
            fZ = 0.0; //yZ;
        }
        else{
            fX = qX / s;
            fY = qY / s;
            fZ = qZ / s;
        }
        bQuaternion = true;
    }
    void Decompress(unsigned int nCompressed){
        //Special compressed quaternion - from MDLOps
        /// Get only the first 11 bits (max value 0x07FF or 2047)
        /// Divide by half to get values in range from 0.0 to 2.0
        /// Subtract from 1.0 so we get values in range from 1.0 to -1.0
        /// This also seems to invert it, previously the highest number
        /// is now the lowest.
        qX = 1.0 - ((double) (nCompressed&0x07FF) / 1023.0);
        /// Move the bits by 11 and repeat
        qY = 1.0 - ((double) ((nCompressed>>11)&0x07FF) / 1023.0);
        /// Move the bits again and repeat
        /// This time, there are only 10 bits left, so our division number
        /// is smaller (511). Also since these are the only bits left
        /// there is no need to use & to clear higher bits,
        /// because they're 0 anyway, per >>.
        qZ = 1.0 - ((double) (nCompressed>>22) / 511.0);
        /// Now we get the w from the other three through the formula:
        /// x^2 + y^2 + z^2 + w^2 == 1 (unit)
        double fSquares = powf(qX, 2.0) + powf(qY, 2.0) + powf(qZ, 2.0);
        if(fSquares < 1.0){
            /// -1.0 to invert it like the coordinates are also inverted?
            qW = -1.0 * sqrtf(1.0 - fSquares);
        }
        else{
            /// If the sum is more than 1.0, we'd get a complex number for w
            /// Instead, set w to 0.0, then recalculate the vector accordingly
            qW = 0.0;
            qX /= sqrtf(fSquares);
            qY /= sqrtf(fSquares);
            qZ /= sqrtf(fSquares);
            /// Now the equation still holds:
            /// (x / sqrt(fSq))^2 + (y / sqrt(fSq))^2 + (z / sqrt(fSq))^2 + 0.0^2 == 1
            /// x^2 / fSq + y^2 / fSq + z^2 / fSq + 0.0 == 1
            /// x^2 + y^2 + z^2 == fSq (which is the definition of fSquare)
        }
        bQuaternion = true;
        bAA = false;
    }
    void Compress(){

    }
    Orientation & operator*=(const Orientation & o){
        if(!bQuaternion) std::cout<<"Orientation::operator*=(): Calculating with invalid quaternion values\n";
        double fTempX =  qX * o.Get(QU_W) + qW * o.Get(QU_X) - qZ * o.Get(QU_Y) + qY * o.Get(QU_Z);
        double fTempY =  qY * o.Get(QU_W) + qZ * o.Get(QU_X) + qW * o.Get(QU_Y) - qX * o.Get(QU_Z);
        double fTempZ =  qZ * o.Get(QU_W) - qY * o.Get(QU_X) + qX * o.Get(QU_Y) + qW * o.Get(QU_Z);
        double fTempW =  qW * o.Get(QU_W) - qX * o.Get(QU_X) - qY * o.Get(QU_Y) - qZ * o.Get(QU_Z);
        qX = std::move(fTempX);
        qY = std::move(fTempY);
        qZ = std::move(fTempZ);
        qW = std::move(fTempW);
        ConvertToAA();
        return *this;
    }
    Orientation & ReverseW(){
        if(!bQuaternion) std::cout<<"Orientation::ReverseW(): Calculating with invalid quaternion values\n";
        qW = - qW;
        return *this;
    }
    Orientation & Reverse(){
        if(!bQuaternion) std::cout<<"Orientation::Reverse(): Calculating with invalid quaternion values\n";
        qX = - qX;
        qY = - qY;
        qZ = - qZ;
        qW = - qW;
        return *this;
    }
    const double & Get(const char cID) const {
        if(cID == 0){
            std::cout<<"Orientation::Get(): invalid ID\n";
            return 0.0;
        }
        else if(cID <= QU_W && !bQuaternion){
            std::cout<<"Orientation::Get(): trying to get quaternion value when it's not set!\n";
            return 0.0;
        }
        else if(cID == QU_X) return qX;
        else if(cID == QU_Y) return qY;
        else if(cID == QU_Z) return qZ;
        else if(cID == QU_W) return qW;
        else if(cID <= AA_A && !bAA){
            std::cout<<"Orientation::Get(): trying to get quaternion value when it's not set!\n";
            return 0.0;
        }
        else if(cID == AA_X) return fX;
        else if(cID == AA_Y) return fY;
        else if(cID == AA_Z) return fZ;
        else if(cID == AA_A) return fAngle;
        else{
            std::cout<<"Orientation::Get(): invalid ID\n";
            return 0.0;
        }
    }
};

struct Vector{
    double fX;
    double fY;
    double fZ;

    Vector(const double & f1, const double & f2, const double & f3){
        fX = f1;
        fY = f2;
        fZ = f3;
    }
    void Set(const double & f1, const double & f2, const double & f3){
        fX = f1;
        fY = f2;
        fZ = f3;
    }

    std::string Print(){
        std::stringstream ss;
        ss<<"("<<fX<<", "<<fY<<", "<<fZ<<")";
        return ss.str();
    }

    void print(const std::string & sMsg){
        std::cout<<sMsg<<" fX="<<fX<<", fY="<<fY<<".\n";
    }

    Vector(const POINT & pt){
        fX = pt.x;
        fY = pt.y;
        fZ = 0.0;
    }

    Vector(){
        fX = 0.0;
        fY = 0.0;
        fZ = 0.0;
    }
    Vector(const double & f1, const double & f2){
        fX = f1;
        fY = f2;
        fZ = 0.0;
    }
    Vector & operator*=(const Matrix22 & m){
        fX = m.f11 * fX + m.f12 * fY;
        fY = m.f21 * fX + m.f22 * fY;
        return *this;
    }
    Vector & operator*=(const double & f){
        fX *= f;
        fY *= f;
        fZ *= f;
        return *this;
    }
    Vector & operator/=(const double & f){
        fX /= f;
        fY /= f;
        fZ /= f;
        return *this;
    }
    Vector & operator+=(const Vector & v){
        fX += v.fX;
        fY += v.fY;
        fZ += v.fZ;
        return *this;
    }
    Vector & operator-=(const Vector & v){
        fX -= v.fX;
        fY -= v.fY;
        fZ -= v.fZ;
        return *this;
    }
    Vector & operator/=(const Vector & v){ //cross product
        double fcrossx = fY * v.fZ - fZ * v.fY;
        double fcrossy = fZ * v.fX - fX * v.fZ;
        double fcrossz = fX * v.fY - fY * v.fX;
        fX = std::move(fcrossx);
        fY = std::move(fcrossy);
        fZ = std::move(fcrossz);
        return *this;
    }
    Vector & Rotate(const Orientation & o){
        if(fX == 0.0 && fY == 0.0 && fZ == 0.0) return *this;

        double fTempX = fX * (o.Get(QU_W) * o.Get(QU_W) + o.Get(QU_X) * o.Get(QU_X) - o.Get(QU_Y) * o.Get(QU_Y) - o.Get(QU_Z) * o.Get(QU_Z))
                     + fY * 2.0 * (o.Get(QU_X) * o.Get(QU_Y) - o.Get(QU_Z) * o.Get(QU_W))
                     + fZ * 2.0 * (o.Get(QU_X) * o.Get(QU_Z) + o.Get(QU_Y) * o.Get(QU_W));
        double fTempY = fX * 2.0 * (o.Get(QU_Y) * o.Get(QU_X) + o.Get(QU_Z) * o.Get(QU_W))
                     + fY * (o.Get(QU_W) * o.Get(QU_W) - o.Get(QU_X) * o.Get(QU_X) + o.Get(QU_Y) * o.Get(QU_Y) - o.Get(QU_Z) * o.Get(QU_Z))
                     + fZ * 2.0 * (o.Get(QU_Y) * o.Get(QU_Z) - o.Get(QU_X) * o.Get(QU_W));
        double fTempZ = fX * 2.0 * (o.Get(QU_Z) * o.Get(QU_X) - o.Get(QU_Y) * o.Get(QU_W))
                     + fY * 2.0 * (o.Get(QU_Z) * o.Get(QU_Y) + o.Get(QU_X) * o.Get(QU_W))
                     + fZ * (o.Get(QU_W) * o.Get(QU_W) - o.Get(QU_X) * o.Get(QU_X) - o.Get(QU_Y) * o.Get(QU_Y) + o.Get(QU_Z) * o.Get(QU_Z));
        fX = std::move(fTempX);
        fY = std::move(fTempY);
        fZ = std::move(fTempZ);
        return *this;
    }
    Vector & Reverse(){
        fX = - fX;
        fY = - fY;
        fZ = - fZ;
        return *this;
    }
    double GetLength() const {
        return sqrtf(powf(fX, 2.0) + powf(fY, 2.0) + powf(fZ, 2.0));
    }
    void Normalize(){
        double fNorm = GetLength();
        if(fNorm == 0.0){
            /// 1.0 0.0 0.0 based on vanilla normals (in 003ebof for example) ?????????
            *this = Vector(0.0, 0.0, 0.0);
        }
        else *this /= fNorm;
    }
    bool Compare(const Vector & v1, double fDiff = 0.0001){
        if(abs(fX - v1.fX) < fDiff &&
           abs(fY - v1.fY) < fDiff &&
           abs(fZ - v1.fZ) < fDiff ) return true;
        else return false;
    }
    bool Null(double fDiff = 0.0){
        if(abs(fX) <= fDiff &&
           abs(fY) <= fDiff &&
           abs(fZ) <= fDiff ) return true;
        else return false;
    }
};

Vector operator*(Vector v, const Matrix22 & m);
Vector operator*(Vector v, const double & f);
Vector operator/(Vector v, const double & f);
Vector operator*(const double & f, Vector v);
double operator*(const Vector & v, const Vector & v2); //dot product
Vector operator/(Vector v, const Vector & v2); //cross product
Vector operator+(Vector v, const Vector & v2);
Vector operator-(Vector v, const Vector & v2);
double Angle(const Vector & v, const Vector & v2);
Orientation operator*(Orientation o1, const Orientation & o2);
double HeronFormula(const Vector & e1, const Vector & e2, const Vector & e3);

struct Location{
    Vector vPosition;
    Orientation oOrientation;
};

struct Triples{
    int n1;
    int n2;
    int n3;
};

struct Face{
    //Binary members
    Vector vNormal;
    double fDistance = 0.0;
    int nMaterialID = 0;
    short nAdjacentFaces [3] = {-1, -1, -1};
    short nIndexVertex[3] = {-1, -1, -1};

    //Added members
    int nSmoothingGroup = 1;
    double fArea = 0.0;
    double fAreaUV = 0.0;
    Vector vTangent;
    Vector vBitangent;
};


struct Aabb{
    //binary members
    Vector vBBmin;
    Vector vBBmax;
    unsigned int nChild1 = 0;
    unsigned int nChild2 = 0;
    int nID = -1; //An index to the face of the walkmesh
    int nProperty = 0;
    // 0x01 = Positive X //from nwntool
    // 0x02 = Positive Y
    // 0x04 = Positive Z
    // 0x08 = Negative X
    // 0x10 = Negative Y
    // 0x20 = Negative Z

    //Added members
    unsigned int nOffset = 0;
    std::vector<Aabb> Child1;
    std::vector<Aabb> Child2;
    int nExtra = 0;
};

struct Controller{
    //Binary members
    int nControllerType = 0; // MDLOps: Controller type
    short nUnknown2 = -1;
    unsigned short nValueCount = 0; // MDLOps: rows of data
    unsigned short nTimekeyStart = 0; // MDLOps: Time start offset
    unsigned short nDataStart = 0;  // MDLOps: Data start offset
    char nColumnCount = 0; // MDLOps: columns of data
    char nPadding [3] = {0, 0, 0};  // MDLOps: unknown values

    //Added members
    int nNameIndex = -1;
    int nAnimation = -1;
};

struct Sound{
    double fTime = 0.0;
    std::string sName;
};

struct Name{
    unsigned int nOffset = 0;
    std::string sName;
};

struct SaberDataStruct{
    Vector vVertex;
    Vector vUV;
    Vector vNormal;
};

struct VertIndicesStruct{
    short nValues[3] = {-1, -1, -1};
};

struct MDXDataStruct{
    //Binary members
    Vector vVertex;
    Vector vNormal;
    Vector vUV1;
    Vector vUV2;
    Vector vUV3;
    Vector vUV4;
    Vector vTangent1[3];
    Vector vTangent2[3];
    Vector vTangent3[3];
    Vector vTangent4[3];
    double fWeightValue[4] = {1.0, 0.0, 0.0, 0.0};
    double fWeightIndex[4] = {-1.0, -1.0, -1.0, -1.0};

    //Added members
    int nNameIndex = -1;
};

struct ArrayHead{
    unsigned int nOffset = 0;
    unsigned int nCount = 0;
    unsigned int nCount2 = 0;

    bool GetDoCountsDiffer(){
        if(nCount == nCount2) return false;
        else return true; //We sure don't expect that!
    }
    int GetCount(){
        return nCount;
    }
    void ResetToSize(int nSize){
        nOffset = 0;
        nCount = nSize;
        nCount2 = nSize;
    }
    bool empty(){
        if(nOffset == 0 && nCount == 0 && nCount2 == 0) return true;
        return false;
    }
};

struct LinkedFace{
    unsigned int nNameIndex = -1; //This is the index of the mesh with our face
    unsigned int nFace = -1; //This is the index of our face
    unsigned int nVertex = -1; //This is the index of the matchin vertex of our face
    bool bAssignedToPatch = false;

    LinkedFace(){}
    LinkedFace(const int & name, const int & face, const int & vert){
        nNameIndex = name;
        nFace = face;
        nVertex = vert;
    }
};

struct Patch{
    unsigned int nNameIndex = -1; //This is the index of the mesh with our face
    unsigned int nVertex = -1; //This is the index of the matchin vertex of our face
    unsigned int nSmoothingGroups = 0;
    std::vector<unsigned int> FaceIndices;
    std::vector<int> SmoothedPatches;
    std::vector<unsigned long int*> SmoothingGroupNumbers;
};

struct Vertex: public Vector{
    float fFromRootX = 0.0;
    float fFromRootY = 0.0;
    float fFromRootZ = 0.0;
    MDXDataStruct MDXData;
    int nLinkedFacesIndex = -1;
};

struct Color{
    double fR = 0.0;
    double fG = 0.0;
    double fB = 0.0;
    int nR = 0;
    int nG = 0;
    int nB = 0;

    Color(){}
    Color(double f1, double f2, double f3){
        fR = f1;
        fG = f2;
        fB = f3;
    }
    ConvertToByte(){
        nR = (int) roundf(255.0 * fR);
        nG = (int) roundf(255.0 * fG);
        nB = (int) roundf(255.0 * fB);
    }
};

struct Bone{
    double fBonemap = -1.0;
    Orientation QBone;
    Vector TBone;
};

/**** NODE ELEMENTS ****/

// if NODE_HAS_HEADER
struct Header{
    //Binary members
    unsigned short nType = 0;
    short nID1 = -1;
    short nNameIndex = -1;
    unsigned short nPadding1 = 0;
    unsigned int nOffsetToRoot = 0; // ???
    unsigned int nOffsetToParent = 0;
    Vector vPos;
    Orientation oOrient;
    ArrayHead ChildrenArray;
    ArrayHead ControllerArray;
    ArrayHead ControllerDataArray;

    //Added members
    std::vector<Node> Children;
    std::vector<Controller> Controllers;
    std::vector<double> ControllerData;
    int nParentIndex = -1;
};

// if NODE_HAS_LIGHT
struct LightHeader{
    //Binary members
    double fFlareRadius = 0.0;
    ArrayHead UnknownArray;
    ArrayHead FlareSizeArray;
    ArrayHead FlarePositionArray;
    ArrayHead FlareColorShiftArray;
    ArrayHead FlareTextureNameArray;
    int nLightPriority = 0;
    int nAmbientOnly = 0;
    int nDynamicType = 0;
    int nAffectDynamic = 0;
    int nShadow = 0;
    int nFlare = 0;
    int nFadingLight = 0;

    //Added members
    std::vector<Name> FlareTextureNames;
    std::vector<Color> FlareColorShifts;
    std::vector<double> FlarePositions;
    std::vector<double> FlareSizes;
};

// if NODE_HAS_EMITTER
struct EmitterHeader{
    /***
    ndix UR's emitter
    # item offset size (bytes) data type notes
    1 Deadspace 0 4 float
    2 Blast Radius 4 4 float
    3 Blast Length 8 4 float
    4 Number of Branches 12 4 uint32
    5 Control Point Smoothing 16 4 float
    6 X Grid 20 4 uint32
    7 Y Grid 24 4 uint32
    8 Spawn Type 28 4 uint32
    9 Update 32 32 string Fountain, Single, Explosion, Lightning
    10 Render 64 32 string Normal, Billboard_to_Local_Z, Billboard_to_World_Z, Aligned_to_World_Z, Aligned_to_Particle_Dir, Motion_Blur, Linked
    11 Blend 96 32 string Normal, Lighten, PunchThrough, Punch-Through
    12 Texture 128 32 string
    13 Chunkname 160 16 string particle model?
    14 Two Sided Texture 176 4 uint16
    15 Loop 180 4 uint16
    16 Render Order ??? 184 2 uint16 Flags?
    17 Frame Blending 186 1 ubyte
    18 Depth Texture Name 187 32 string
    19 Padding ??? 219 1 ubyte
    20 Flags ??? 220 4? uint32 Maybe this is 2 uint16? Just padding? Render order?
    /***/
    //Binary Members
    double fDeadSpace = 0.0;
    double fBlastRadius = 0.0;
    double fBlastLength = 0.0;
    unsigned int nBranchCount = 0;
    double fControlPointSmoothing = 0.0;
    unsigned int nxGrid = 0;
    unsigned int nyGrid = 0;
    unsigned int nSpawnType = 0;
    std::string cUpdate; //32B
    std::string cRender; //32B
    std::string cBlend; //32B
    std::string cTexture; //32B
    std::string cChunkName; //16B
    unsigned int nTwosidedTex = 0;
    unsigned int nLoop = 0;
    unsigned short nUnknown1 = 0;
    unsigned char nFrameBlending = 0;
    std::string cDepthTextureName; //32B
    unsigned char nUnknown2 = 0;
    unsigned int nFlags = 0; //unsure
};

// if NODE_HAS_MESH
struct MeshHeader{
    //Binary members
    unsigned int nFunctionPointer0 = 0;
        /* MagnusII:
         * 4216656 K1, 4216592 K1 SkinMesh, 4216640 K1 DanglyMesh
         * 4216880 K2, 4216816 K2 SkinMesh, 4216864 K2 DanglyMesh
         */
    unsigned int nFunctionPointer1 = 0;
        /* MagnusII:
         * 4216672 K1, 4216608 K1 SkinMesh, 4216624 K1 DanglyMesh
         * 4216896 K2, 4216832 K2 SkinMesh, 4216848 K2 DanglyMesh
         */
    ArrayHead FaceArray;
    Vector vBBmin;
    Vector vBBmax;
    double fRadius = 0.0;
    Vector vAverage;
    Color fDiffuse;
    Color fAmbient;
    unsigned int nShininess = 0;
        /* MagnusII:
         * 0 (mostly), 1, 2, 3, 4, 5, 7, 8, 13
         * Blend Factor, SamplerStageStates, TextureOperation?
         */
    std::string cTexture1;
    std::string cTexture2;
    std::string cTexture3;
    std::string cTexture4;
    ArrayHead IndexCounterArray;
    ArrayHead IndexLocationArray;
    ArrayHead MeshInvertedCounterArray;
        /*
         * = 1, always one UInt32
         * sequence goes like this: first
         * trimesh node is 98, then:
         * 98..00, 100
         * 199..101, 200
         * 399..?
         */
    int nUnknown3 [3] = {-1, -1, 0};
    char nSaberUnknown1 = 0; // 3 for non-saber
    char nSaberUnknown2 = 0; // 0 for non-saber
    char nSaberUnknown3 = 0; // 0 for non-saber
    char nSaberUnknown4 = 0; // 0 for non-saber
    char nSaberUnknown5 = 0; // 0 for non-saber
    char nSaberUnknown6 = 0; // 0 for non-saber
    char nSaberUnknown7 = 0; // 0 for non-saber
    char nSaberUnknown8 = 0; // 0 for non-saber, 17 for sabers
    int nAnimateUV = 0;
    double fUVDirectionX = 0.0;
    double fUVDirectionY = 0.0;
    double fUVJitter = 0.0;
    double fUVJitterSpeed = 0.0;

    unsigned int nMdxDataSize = 0;
    unsigned int nMdxDataBitmap = 0;
    int nOffsetToVerticesInMDX = -1;
    int nOffsetToNormalsInMDX = -1;
    int nOffsetToUnknownInMDX = -1; //never present
    int nOffsetToUVsInMDX = -1;
    int nOffsetToUV2sInMDX = -1;
    int nOffsetToUV3sInMDX = -1; //never present
    int nOffsetToUV4sInMDX = -1; //never present
    int nOffsetToTangentSpaceInMDX  = -1;
    int nOffsetToTangentSpace2InMDX  = -1; //never present
    int nOffsetToTangentSpace3InMDX  = -1; //never present
    int nOffsetToTangentSpace4InMDX  = -1; //never present

    unsigned short nNumberOfVerts = 0;
    unsigned short nTextureNumber = 0;
    char nHasLightmap = 0;
    char nRotateTexture = 0;
    char nBackgroundGeometry = 0;
    char nShadow = 0;
    char nBeaming = 0;
    char nRender = 0;
    char nDirtEnabled = 0; //K2
    char nUnknown1 = 0; //K2
    short nDirtTexture = 0; //K2
    short nDirtCoordSpace = 0; //K2
    char nHideInHolograms = 0; //K2
    char nUnknown2 = 0; //K2
    short nUnknown4 = 0;
    double fTotalArea = 0.0;
    unsigned int nPadding = 0;
    unsigned int nOffsetIntoMdx = 0;
    unsigned int nOffsetToVertArray = 0;

    /*** ndix UR's knowleadge
    # item offset size (bytes) data type notes
    73 Dirt Enabled 314 1 ubyte K2ONLY
    74 Padding? 315 1 ubyte K2ONLY
    75 Dirt Texture 316 2 uint16 K2ONLY
    76 Dirt Coord space 318 2 uint16 K2ONLY
    77 Hide in holograms 320 1 ubyte K2ONLY
    78 Padding? 321 1 ubyte K2ONLY
    79 Padding? 322 1 uint16 K1@314
    80 Total Surface Area 324 4 float K1@316
    81 Padding 328 4 uint32 K1@320
    82 MDX data location 332 4 uint32 K1@324
    83 Vertex data location 336 4 uint32 K1@328
    /***/

    //Added members
    std::vector<Face> Faces;
    std::vector<VertIndicesStruct> VertIndices;
    std::vector<Vertex> Vertices;
    MDXDataStruct MDXData;
    unsigned int nVertIndicesCount = 0;
    unsigned int nVertIndicesLocation = 0;
    unsigned int nMeshInvertedCounter = 0;
    char * GetTexture(int nTex){
        if(nTex == 1) return (char*) cTexture1.c_str();
        if(nTex == 2) return (char*) cTexture2.c_str();
        if(nTex == 3) return (char*) cTexture3.c_str();
        if(nTex == 4) return (char*) cTexture4.c_str();
        return "Invalid texture!";
    }
};

// if NODE_HAS_SKIN
struct SkinHeader{
    ArrayHead UnknownArray; //Always 0
    int nOffsetToWeightValuesInMDX = -1;
    int nOffsetToBoneIndexInMDX = -1;
    unsigned int nOffsetToBonemap = 0;
    unsigned int nNumberOfBonemap = 0;
    ArrayHead QBoneArray;
    ArrayHead TBoneArray;
    ArrayHead Array8Array; //empty, data irrelevant
    short nBoneIndexes[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<Bone> Bones;
    std::vector<int> BoneNameIndexes;
};

// if NODE_HAS_DANGLY
struct DanglymeshHeader{
    //Binary members
    ArrayHead ConstraintArray;
    double fDisplacement = 0.0;
    double fTightness = 0.0;
    double fPeriod = 0.0;
    unsigned int nOffsetToData2 = 0;

    //Added members
    std::vector<double> Constraints;
    std::vector<Vector> Data2;
};

// if NODE_HAS_AABB
struct WalkmeshHeader{
    //Binary members
    unsigned int nOffsetToAabb = 0;

    //Added members
    std::vector<Aabb> ArrayOfAabb;
    Aabb RootAabb;
};

// if NODE_HAS_SABER
struct SaberHeader{
    //Binary members
    unsigned int nOffsetToSaberData1 = 0;
    unsigned int nOffsetToSaberData2 = 0;
    unsigned int nOffsetToSaberData3 = 0;
    int nInvCount1 = 0;
    int nInvCount2 = 0;

    //Added members
    int nNumberOfSaberData = 0;
    std::vector<SaberDataStruct> SaberData;
};

struct Node{
    unsigned int nOffset = 0;
    //unsigned int nSize;
    int nAnimation = -1;

    Header Head;
    LightHeader Light;
    EmitterHeader Emitter;
    MeshHeader Mesh;
    SkinHeader Skin;
    DanglymeshHeader Dangly;
    WalkmeshHeader Walkmesh;
    SaberHeader Saber;

    std::string & GetName();
    Location GetLocation(){
        Location location;

        int nCtrlPos = -1;
        int nCtrlOri = -1;
        for(int c = 0; c < Head.Controllers.size(); c++){
            if(Head.Controllers.at(c).nControllerType == CONTROLLER_HEADER_POSITION){
                nCtrlPos = c;
            }
            if(Head.Controllers.at(c).nControllerType == CONTROLLER_HEADER_ORIENTATION){
                nCtrlOri = c;
            }
        }

        if(nCtrlPos != -1){
            Controller & posctrl = Head.Controllers.at(nCtrlPos);
            location.vPosition = Vector(Head.ControllerData.at(posctrl.nDataStart + 0),
                                        Head.ControllerData.at(posctrl.nDataStart + 1),
                                        Head.ControllerData.at(posctrl.nDataStart + 2));
        }
        if(nCtrlOri != -1){
            Controller & orictrl = Head.Controllers.at(nCtrlOri);
            if(orictrl.nColumnCount == 4){
                location.oOrientation = Orientation(Head.ControllerData.at(orictrl.nDataStart + 0),
                                                    Head.ControllerData.at(orictrl.nDataStart + 1),
                                                    Head.ControllerData.at(orictrl.nDataStart + 2),
                                                    Head.ControllerData.at(orictrl.nDataStart + 3));
            }
            else if(orictrl.nColumnCount == 2){
                location.oOrientation.Decompress((unsigned int) Head.ControllerData.at(orictrl.nDataStart));
            }
            location.oOrientation.ConvertToAA();
        }

        return location;
    }
};

/**** HIGHER ELEMENTS ****/

struct Animation{
    //Binary members

    /// Geoheader part
    unsigned int nFunctionPointer0 = 0;
    unsigned int nFunctionPointer1 = 0;
    std::string sName;
    unsigned int nOffsetToRootAnimationNode = 0;
    unsigned int nNumberOfObjects = 0;
    ArrayHead RuntimeArray1; //Always 0
    ArrayHead RuntimeArray2; //Always 0
    unsigned int nRefCount = 0; //Always 0
    unsigned char nModelType = 5; //1 - geometry, 2 - model, 5 - animation
    unsigned char nPadding [3] = {0, 0, 0};

    /// Animation-specific part
    double fLength = 0.0;
    double fTransition = 0.0;
    std::string sAnimRoot;
    ArrayHead SoundArray; //MDLOps: events
    unsigned int nPadding2 = 0; //Always 0

    //Added members
    unsigned int nOffset = 0;
    Node RootAnimationNode;
    std::vector<Sound> Sounds;
    std::vector<Node> ArrayOfNodes;
};

struct GeometryHeader{
    //Binary members
    unsigned int nFunctionPointer0 = 0;
    unsigned int nFunctionPointer1 = 0;
    std::string sName;
    unsigned int nOffsetToRootNode = 0;
    unsigned int nTotalNumberOfNodes = 0; // Total number of nodes = Number of nodes in this model + Total number of nodes in supermodel + 1
    ArrayHead RuntimeArray1; //Always 0
    ArrayHead RuntimeArray2; //Always 0
    unsigned int nRefCount = 0;  //Always 0
    unsigned char nModelType = 2; //1 - geometry, 2 - model, 5 - animation
    unsigned char nPadding[3] = {0, 0, 0};
};

struct ModelHeader{
    //Binary members
    unsigned char nClassification = 0;
    unsigned char nUnknown1 [3] = {0, 0, 1};
    unsigned int nChildModelCount = 0; //Always 0
    ArrayHead AnimationArray;

    unsigned int nSupermodelReference = 0;
    /*Pointer to supermodel?
    Always present when there is a supermodel
    but very variable, models sharing a supermodel
    do not share this value.
    */
    Vector vBBmin;
    Vector vBBmax;
    double fRadius = 0.0;
    double fScale = 0.0;
    std::string cSupermodelName;
    unsigned int nOffsetToHeadRootNode;
    int nUnknown2 = 0; //Always 0
    unsigned int nMdxLength2 = 0;
    unsigned int nOffsetIntoMdx = 0; //Always 0
    ArrayHead NameArray;

    //Added members
    GeometryHeader GH;
    Node RootNode;
    std::vector<Animation> Animations;
    std::vector<Name> Names;
    std::vector<Node> ArrayOfNodes;
    std::vector<std::vector<LinkedFace>> LinkedFacesPointers;
    std::vector<std::vector<Patch>> PatchArrayPointers;
};

struct FileHeader{
    //Binary members
    unsigned int nZero; //Always 0
    unsigned int nMdlLength;
    unsigned int nMdxLength;

    //Added members
    ModelHeader MH;
};

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
  /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

int ReturnController(std::string sController);
std::string ReturnClassificationName(int nClassification);
std::string ReturnControllerName(int, int nType);

class File{
  protected:
    bool bLoaded = false;
    std::string sFile;
    std::string sFullPath;
    std::vector<char> sBuffer;
    unsigned int nBufferSize;
    unsigned int nPosition;
  public:
    //Getters
    std::vector<char> & GetBuffer(){
        return sBuffer;
    }
    unsigned int GetBufferLength(){
        return nBufferSize;
    }
    const std::string & GetFilename(){
        return sFile;
    }
    const std::string & GetFullPath(){
        return sFile;
    }
    virtual const std::string GetName(){
        return "";
    }
    bool empty(){
        return !bLoaded;
    }
    void Export(std::string &sExport){
        sExport = std::string(sBuffer.begin(), sBuffer.end());
    }

    //Setters
    void SetFilePath(std::string & sPath);

    //Loaders/Unloaders
    virtual std::vector<char> & CreateBuffer(int nSize){
        nBufferSize = nSize;
        bLoaded = true;
        sBuffer.resize(nSize, '\0');
        return sBuffer;
    }
    virtual void FlushAll(){
        sBuffer.clear();
        sBuffer.shrink_to_fit();
        nBufferSize = 0;
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
    std::vector<int> & GetKnownData(){
        return bKnown;
    }

    //Loaders/Unloaders
    std::vector<char> & CreateBuffer(int nSize){
        nBufferSize = nSize;
        bLoaded = true;
        sBuffer.resize(nSize, '\0');
        bKnown.resize(nSize, 0);
        return sBuffer;
    }
    void FlushAll(){
        sBuffer.clear();
        bKnown.clear();
        bLoaded = false;
        nBufferSize = 0;
    }
};

class MDL;

class MDX: public BinaryFile{
    static const std::string sName;
  public:
     //Getters
     const std::string GetName(){
        return sName;
     }

    //Friends
    friend class MDL;
};

class WOK: public BinaryFile{
    static const std::string sName;

    //Data
    unsigned int nNumberOfVerts;
    unsigned int nOffsetToVerts;
    unsigned int nNumberOfFaces;
    unsigned int nOffsetToIndexes;
    unsigned int nOffsetToMaterials;
    unsigned int nOffsetToNormals;
    unsigned int nOffsetToDistances;
    unsigned int nNumberOfAabb;
    unsigned int nOffsetToAabb;
    unsigned int nNumberOfTriples;
    unsigned int nOffsetToTriples;
    unsigned int nNumberOfDoubles;
    unsigned int nOffsetToDoubles;
    unsigned int nNumberOfSingles;
    unsigned int nOffsetToSingles;
    std::vector<Aabb> aabb;
    std::vector<Face> faces;
    std::vector<Vertex> verts;
    std::vector<Triples> triples;
    std::vector<Triples> doubles;
    std::vector<int> singles;

  public:
    //Getters
    const std::string GetName(){
        return sName;
    }

    //Loaders
    void ProcessWalkmesh();
    void BuildTree();
};

extern MDX Mdx;
extern WOK Walkmesh;

class Ascii: public File{
    //Reading
    bool ReadFloat(double & fNew, bool bPrint = false);
    bool ReadInt(int & nNew, bool bPrint = false);
    bool ReadUntilText(std::string & sHandle, bool bToLowercase = true, bool bStrictNoNewLine = false);
    void SkipLine();
    bool EmptyRow();
    void BuildAabb(Aabb & AABB, std::vector<Aabb> & ArrayOfAabb, int & nCounter);

    //Getters
    int GetNameIndex(std::string sName, std::vector<Name> Names);
    Node & GetNodeByNameIndex(FileHeader & Data, int nNameIndex, int nAnimation = -1);

  public:
    bool Read(FileHeader * FH);
};

class MDL: public BinaryFile{
    static const std::string sName;
    std::vector<FileHeader> FH;
    Ascii AsciiReader;

    //Reading
    void ParseNode(Node * NODE, int * nNodeCounter, Vector vFromRoot);
    void ParseAabb(Aabb * AABB, unsigned int nHighestOffset);
    void LinearizeGeometry(Node & NODE, std::vector<Node> & ArrayOfNodes){
        for(int n = 0; n < NODE.Head.Children.size(); n++){
            LinearizeGeometry(NODE.Head.Children[n], ArrayOfNodes);
        }
        ArrayOfNodes.at(NODE.Head.nNameIndex) = std::move(NODE);
    }
    void LinearizeAnimations(Node & NODE, std::vector<Node> & ArrayOfNodes){
        ArrayOfNodes.push_back(std::move(NODE));
        Node & node = ArrayOfNodes.back();
        for(int n = 0; n < node.Head.Children.size(); n++){
            LinearizeAnimations(node.Head.Children[n], ArrayOfNodes);
        }
    }

    //Display
    void DetermineDisplayText(std::vector<std::string> cItem, std::stringstream & sPrint, LPARAM lParam);

    //Context menu
    void AddMenuLines(std::vector<std::string> cItem, LPARAM lParam, MenuLineAdder * pmla);

    //Viewers
    void OpenViewer(std::vector<std::string> cItem, LPARAM lParam);
    void OpenGeoViewer(std::vector<std::string> cItem, LPARAM lParam);

    void ConvertToAscii(int nDataType, std::stringstream & sReturn, void * Data);
    bool CheckNodes(std::vector<Node> & node, std::stringstream & ssReturn, int nAnimation = -1);

    //Writing
    void WriteNodes(Node & node);
    void WriteAabb(Aabb & aabb);
    void GatherChildren(Node & NODE, std::vector<Node> & ArrayOfNodes);
    void FillBinaryFields(Node & NODE, int & nMeshCounter);

    //Calculating
    void CreatePatches(int & nNumOfVerts, int & nNumOfTS, bool bPrint);
    void DetermineSmoothing();
    bool bDetermineSmoothing = true;
    bool bSmoothAreaWeighting = true;
    bool bSmoothAngleWeighting = false;
    void GenerateSmoothingNumber(std::vector<int> & SmoothingGroup, const std::vector<unsigned long int> & nSmoothinGroupNumbers, const int & nSmoothinGroupCounter, const int & pg);
    bool FindNormal(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup, const Vector & vNormalBase, const Vector & vNormal, std::vector<int> & CurrentlySmoothedPatches, std::ofstream & file);
    int FindTangentSpace(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup,
                           const Vector & vTangentBase, const Vector & vBitangentBase, const Vector & vNormalBase,
                           const Vector & vTangent, const Vector & vBitangent, const Vector & vNormal,
                           std::vector<int> & CurrentlySmoothedPatches, std::ofstream & file);

    //Getters
    const std::string GetName(){
        return sName;
    }

  public:
    //Friends
    friend Ascii;
    friend Node;
    friend void ProcessTreeAction(HTREEITEM hItem, const int & nAction, void * Pointer);

    //Version
    bool bK2 = true;

    //Getters
    FileHeader * GetFileData(){
        if(FH.empty()) return nullptr;
        else return &FH[0];
    }
    std::vector<char> & GetAsciiBuffer(){
        return AsciiReader.GetBuffer();
    }
    Node & GetNodeByNameIndex(int nIndex, int nAnimation = -1){
        if(nAnimation == -1){
            return FH[0].MH.ArrayOfNodes.at(nIndex);
        }
        else{
            std::vector<Node> & Array = FH[0].MH.Animations[nAnimation].ArrayOfNodes;
            for(int n = 0; n < Array.size(); n++){
                //std::cout<<"Looping through animation nodes for our node\n";
                if(Array[n].Head.nNameIndex == nIndex) return Array[n];
            }
        }
    }
    bool HeadLinked(){
        for(int n = 0; n < FH[0].MH.ArrayOfNodes.size(); n++){
            if(FH[0].MH.ArrayOfNodes.at(n).nOffset == FH[0].MH.nOffsetToHeadRootNode){
                if(FH[0].MH.Names.at(FH[0].MH.ArrayOfNodes.at(n).Head.nNameIndex).sName == "neck_g") return true;
                else return false;
            }
        }
        return false;
    }


    //Loaders
    bool Compile();
    void DecompileModel();
    void PrepareForBinary();
    void CleanupAfterCompilation(){}
    void CheckPeculiarities();
    void BuildTree();
    void UpdateDisplay(HTREEITEM hItem);
    void FlushData(){
        FH.clear();
    }

    //Setters/general
    bool LinkHead(bool bLink){
        unsigned int nOffset;
        if(bLink){
            int nNameIndex = -1;
            for(int n = 0; n < FH[0].MH.Names.size() && nNameIndex == -1; n++){
                if(FH[0].MH.Names.at(n).sName == "neck_g") nNameIndex = n;
            }
            if(nNameIndex != -1){
                nOffset = GetNodeByNameIndex(nNameIndex).nOffset;
            }
            else return false;
        }
        else{
            nOffset = FH[0].MH.GH.nOffsetToRootNode;
        }
        FH[0].MH.nOffsetToHeadRootNode = nOffset;
        WriteIntToPH(nOffset, 180, nOffset);
        return true;
    }

    //ascii
    void ExportAscii(std::string &sExport);
    void FlushAscii(){
        AsciiReader.FlushAll();
    }
    std::vector<char> & CreateAsciiBuffer(int nSize){
        return AsciiReader.CreateBuffer(nSize);
    }
    bool ReadAscii(){
        //CreateDataStructure
        FH.resize(1);
        bool bReturn = AsciiReader.Read(&FH[0]);
        if(!bReturn){
            FlushData();
        }
        else std::cout<<"Ascii read succesfully!\n";
        return bReturn;
    }
};

extern MDL Model;


#endif // MDL_H_INCLUDED
