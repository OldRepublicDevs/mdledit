#ifndef MDL_H_INCLUDED
#define MDL_H_INCLUDED

#include "general.h"
#include "MDLdefinitions.h"

/**
    UNKNOWNS:

    1. Non-static data
     - Supermodel int32 in Header
     - last three bytes (padding?) after Classification in Header
     - last three bytes (padding?) after ModelType in Header and Animation
     - bytes after RenderOrder in EmitterHeader (possibly padding like above?)
     - short nUnknown2 and last three bytes (padding?) in Controller
     - nProperty in Aabb
     - an unknown array in LightHeader
     - data2 array (3 floats, likely coordinates) in DanglyHeader
     - Array8 in SkinHeader (I believe this one to be unused, any data inside is ignored. So it can just be anything. Need to test this.)
     - MeshHeader:
       - two unknown bytes after Render
       - "total area"
       - Unknown float at the end
       - Unknown lightsaber bytes
       - K2 unknown 1 (float)

    2. Static data - always 0 (usually it's 0)
     - short nUnknownEmpty1 in NodeHeader
     - 3 int32s in SkinHeader
     - K2 unknown 2 in MeshHeader
     - zero1 and zero2 in EmitterHeader
     - -1 -1 0 Array in MeshHeader

/**/

#define CONVERT_CONTROLLER_SINGLE        1
#define CONVERT_CONTROLLER_KEYED         2
#define CONVERT_HEADER                   3
#define CONVERT_LIGHT                    4
#define CONVERT_EMITTER                  5
#define CONVERT_MESH                     6
#define CONVERT_SKIN                     7
#define CONVERT_DANGLY                   8
#define CONVERT_AABB                     9
#define CONVERT_SABER                    10
#define CONVERT_ENDNODE                  11
#define CONVERT_ANIMATION                12
#define CONVERT_ANIMATION_NODE           13
#define CONVERT_MODEL_GEO                14
#define CONVERT_MODEL                    15

//Forward declarations
struct Face;
struct Aabb;
struct Controller;
struct Sound;
struct Name;
struct ConstraintsStruct;
struct DanglyData2Struct;
//struct BonemapStruct;
//struct QBoneStruct;
//struct TBoneStruct;
//struct Array8Struct;
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
struct Orientation;
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

struct Orientation{
    unsigned int nCompressed; //Since it is compressed, best to get it in a neutral type
    double qX;
    double qY;
    double qZ;
    double qW;
    double fX;
    double fY;
    double fZ;
    double fAngle;
    Orientation(){
        qX = 0.0;
        qY = 0.0;
        qZ = 0.0;
        qW = 1.0;
    }
    Orientation(const double & f1, const double & f2, const double & f3, const double & f4){
        qX = f1;
        qY = f2;
        qZ = f3;
        qW = f4;
    }
    void Quaternion(const double & f1, const double & f2, const double & f3, const double & f4){
        qX = f1;
        qY = f2;
        qZ = f3;
        qW = f4;
    }
    void AA(const double & f1, const double & f2, const double & f3, const double & f4){
        fX = f1;
        fY = f2;
        fZ = f3;
        fAngle = f4;
    }
    void ConvertToQuaternions(){
        double a = fAngle / 2.0;
        qW = cosf(a);
        qX = fX * sinf(a);
        qY = fY * sinf(a);
        qZ = fZ * sinf(a);
    }
    void ConvertToAA(){
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
    }
    void Decompress(){
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
    }
    void Compress(){

    }
    Orientation & operator*=(const Orientation & o){
        double fTempX =  qX * o.qW + qW * o.qX - qZ * o.qY + qY * o.qZ;
        double fTempY =  qY * o.qW + qZ * o.qX + qW * o.qY - qX * o.qZ;
        double fTempZ =  qZ * o.qW - qY * o.qX + qX * o.qY + qW * o.qZ;
        double fTempW =  qW * o.qW - qX * o.qX - qY * o.qY - qZ * o.qZ;
        qX = std::move(fTempX);
        qY = std::move(fTempY);
        qZ = std::move(fTempZ);
        qW = std::move(fTempW);
        ConvertToAA();
        return *this;
    }
    Orientation & ReverseW(){
        qW = - qW;
        return *this;
    }
    Orientation & Reverse(){
        qX = - qX;
        qY = - qY;
        qZ = - qZ;
        qW = - qW;
        return *this;
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

        double fTempX = fX * (o.qW * o.qW + o.qX * o.qX - o.qY * o.qY - o.qZ * o.qZ)
                     + fY * 2.0 * (o.qX * o.qY - o.qZ * o.qW)
                     + fZ * 2.0 * (o.qX * o.qZ + o.qY * o.qW);
        double fTempY = fX * 2.0 * (o.qY * o.qX + o.qZ * o.qW)
                     + fY * (o.qW * o.qW - o.qX * o.qX + o.qY * o.qY - o.qZ * o.qZ)
                     + fZ * 2.0 * (o.qY * o.qZ - o.qX * o.qW);
        double fTempZ = fX * 2.0 * (o.qZ * o.qX - o.qY * o.qW)
                     + fY * 2.0 * (o.qZ * o.qY + o.qX * o.qW)
                     + fZ * (o.qW * o.qW - o.qX * o.qX - o.qY * o.qY + o.qZ * o.qZ);
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
    bool Compare(const Vector & v1, double fDiff = 0.00001){
        if(abs(fX - v1.fX) < fDiff &&
           abs(fY - v1.fY) < fDiff &&
           abs(fZ - v1.fZ) < fDiff ) return true;
        else return false;
    }
};

Vector operator*(Vector v, const Matrix22 & m);
Vector operator*(Vector v, const double & f);
Vector operator/(Vector v, const double & f);
double operator*(const Vector & v, const Vector & v2); //dot product
Vector operator/(Vector v, const Vector & v2); //cross product
Vector operator+(Vector v, const Vector & v2);
Vector operator-(Vector v, const Vector & v2);
double Angle(const Vector & v, const Vector & v2);
Orientation operator*(Orientation o1, const Orientation & o2);

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
    int nLength = 32;
    Vector vNormal; // from farmboy0
    double fDistance; // from farmboy0
    int nMaterialID; //possibly imported as material group!
    short nAdjacentFaces [3]; // from farmboy0
    short nIndexVertex[3];  // from farmboy0

    //Added members
    int nSmoothingGroup = 1;
    double fArea = 0.0;
};


struct Aabb{
    //binary members
    Vector vBBmin;
    Vector vBBmax;
    unsigned int nChild1;
    unsigned int nChild2;
    int nID; //An index to the face of the walkmesh
    int nProperty; //0 if no children, otherwise 2^something. I have seen 1 2 4 8 16 32
    // 0x01 = Positive X //from nwntool
    // 0x02 = Positive Y
    // 0x04 = Positive Z
    // 0x08 = Negative X
    // 0x10 = Negative Y
    // 0x20 = Negative Z

    //Added members
    unsigned int nOffset;
    std::vector<Aabb> Child1;
    std::vector<Aabb> Child2;
    int nExtra;
};

struct Controller{
    //Binary members
    int nControllerType; // MDLOps: Controller type
    short nUnknown2;
    unsigned short nValueCount; // MDLOps: rows of data
    unsigned short nTimekeyStart; // MDLOps: Time start offset
    unsigned short nDataStart;  // MDLOps: Data start offset
    char nColumnCount; // MDLOps: columns of data
    char nPadding [3];  // MDLOps: unknown values

    //Added members
    int nLength = 16;
    int nNameIndex;
    int nAnimation;
};

struct Sound{
    double fTime;
    std::string cName;
};

struct Name{
    unsigned int nOffset;
    std::string cName;
};

struct SaberDataStruct{
    Vector vVertex;
    double fUV[2];
    Vector vNormal;
};

struct VertIndicesStruct{
    short nValues[3];
};

struct MDXDataStruct{
    int nNameIndex;
    Vector vVertex;
    Vector vNormal;
    double fUV1[2];
    double fUV2[2];
    double fUV3[2];
    double fUV4[2];
    Vector vTangent1[3];
    Vector vTangent2[3];
    Vector vTangent3[3];
    Vector vTangent4[3];
    double fSkin1[4];
    double fSkin2[4];
    MDXDataStruct(){
        for(int n = 0; n < 4; n++) fSkin2[n] = -1.0;
    }
};

struct ArrayHead{
    unsigned int nOffset;
    unsigned int nCount;
    unsigned int nCount2;
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
    float fFromRootX;
    float fFromRootY;
    float fFromRootZ;
    MDXDataStruct MDXData;
    int nLinkedFacesIndex = -1;
};
bool VerticesEqual(const Vertex & v1, const Vertex & v2);

struct Color{
    double fR;
    double fG;
    double fB;
    int nR;
    int nG;
    int nB;

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
    double fArray8;
};

/**** NODE ELEMENTS ****/

// if NODE_HAS_HEADER
struct Header{
    //Binary members
    unsigned short nType; //Check against NODE_HAS_* constants defined at the top
    unsigned short nID1;
    unsigned short nNameIndex;
    short nUnknownEmpty1;
    unsigned int nOffsetToRoot; // ???
    unsigned int nOffsetToParent;
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
    // According to Farmboy0, the arrays would include the following. Looks plausible.
    double fFlareRadius;
    ArrayHead UnknownArray;
    ArrayHead FlareSizeArray;
    ArrayHead FlarePositionArray;
    ArrayHead FlareColorShiftArray;
    ArrayHead FlareTextureNameArray;
    int nLightPriority;
    int nAmbientOnly;
    int nDynamicType;
    int nAffectDynamic;
    int nShadow;
    int nFlare;
    int nFadingLight;

    std::vector<Name> FlareTextureNames;
    std::vector<Color> FlareColorShifts;
    std::vector<double> FlarePositions;
    std::vector<double> FlareSizes;
};

// if NODE_HAS_EMITTER
struct EmitterHeader{
    //Binary Members
    unsigned int nZero1 = 0; //These aren't function pointers though
    unsigned int nZero2 = 0;
    double fDeadSpace;
    double fBlastRadius;
    double fBlastLength;
    int nxGrid;
    int nyGrid;
    int nSpawnType;
    std::string cUpdate; //32B
    std::string cRender; //32B
    std::string cBlend; //32B // By here 128B, left: 96B
    std::string cTexture; ///64B  ///There is a way to test this. Change v_shockwave_imp in w_rokplasma.mdl > Geo > shockwave > Header > Emitter > cTexture(per current convention)
    //char cChunkName [32];
    // For MDLOps, this string is part of cTexture
    // and cUnknownBytes is cChunkName. Same for Farmboy0.
    // If these are both normal 32B strings
    // then by now we've got 192B, left: 32B
    std::string cChunkName; //16B
    unsigned int nTwosidedTex;
    unsigned int nLoop;
    //The following two shorts might constitute a float, test on more models
    //Some really large floats would result, maybe a byte per byte approach?
    unsigned short nRenderOrder; // Possibly an int32?
    unsigned short nUnknown6; //Farmboy0: this is padding
    int nFlags;

    //Added Members
    int nLength = 224;
};

// if NODE_HAS_MESH
struct MeshHeader{
    //Binary members
    unsigned int nFunctionPointer0;
        /* 4216656 K1, 4216592 K1 SkinMesh, 4216640 K1 DanglyMesh
         * 4216880 K2, 4216816 K2 SkinMesh, 4216864 K2 DanglyMesh
         */
    unsigned int nFunctionPointer1;
        /* 4216672 K1, 4216608 K1 SkinMesh, 4216624 K1 DanglyMesh
         * 4216896 K2, 4216832 K2 SkinMesh, 4216848 K2 DanglyMesh
         */
    ArrayHead FaceArray;
    Vector vBBmin;
    Vector vBBmax;
    double fRadius;
    Vector vAverage;
    Color fDiffuse; //DiffuseColor
    Color fAmbient; //AmbientColor
    unsigned int nShininess; //Tentative!
        /* 0 (mostly), 1, 2, 3, 4, 5, 7, 8, 13
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
    int nUnknown3 [3]; // -1, -1, 0
    char nSaberUnknown1; // 3 for non-saber
    char nSaberUnknown2; // 0 for non-saber
    char nSaberUnknown3; // 0 for non-saber
    char nSaberUnknown4; // 0 for non-saber
    char nSaberUnknown5; // 0 for non-saber
    char nSaberUnknown6; // 0 for non-saber
    char nSaberUnknown7; // 0 for non-saber
    char nSaberUnknown8; // 0 for non-saber, 17 for sabers
    int nAnimateUV;  // 0
    double fUVDirectionX; // These four doubles together might be orientation or rotation (per MagnusII)
    double fUVDirectionY; // But per ndix UR, these are UV animation control values
    double fUVJitter; // 0
    double fUVJitterSpeed;
        /* 0 for most, 1 for water and clouds and
         * one piece of the GUI
         * Maybe some sparkle or reflex effects,
         * or a specular lighting modifier?
         */
    int nMdxDataSize; //MDLOps: Mdx Data Size... uhhh what?
        /* mostly 24, 32, 40, 76 with a smattering of other values
         * if textures = 0 almost always 24, except 15 models with -1 and 23 with 60
         * if textures = 1, almost always 32, with several 64 and 68, and a bunch of 100 and 0
         * if textures = 2, it's either 40 (mostly) or 76
         */
    unsigned int nMdxDataBitmap;
        /* which data is present in MDX
         * according to MDXDatabits
         */
         // Dastardly & LiliArch: to enable bumpmaps, set the first bit of MdxDataBitmap to 1.
         // Also: "The MDX bump map UV offset is 16 bytes away from the beginning of the
         // texture uv offset. While you *could* insert extra UV's into the .mdx file then
         // fix each node's .mdx offset, this is just not needed. Simply set the bump map's
         // UV offset equal to the texture UV offset."
    unsigned int nOffsetToVerticesInMDX; // = 0 for most, -1 for lightsabers or when verticescount = 0
    unsigned int nOffsetToNormalsInMDX; // = 12
    unsigned int nOffsetToUnknownInMDX; // = -1, never present
    unsigned int nOffsetToUVsInMDX; //MDLOps: Loc61
        // = 24 if present
    unsigned int nOffsetToUV2sInMDX; //MDLOps: Loc62
        /* -1, 24, 32
         * if mdxdatasize = 24 then it's always -1
         * if mdxdatasize = 32 it's almost always -1 except for 14 models where it's 24
         * if mdxdatasize = 40 or 76 then it's always 32
         */
    unsigned int nOffsetToUV3sInMDX; // = -1, never present
    unsigned int nOffsetToUV4sInMDX; // = -1, never present
    unsigned int nOffsetToUnknownStructInMDX; //MDLOps: Loc65
        /* -1, 24, 32, 40
        * if mdxdatasize = 24 or 32 or 40 then it's always -1
        * if mdxdatasize = 76 then it's always 40
        * this is basically whatever info is in the MDX
        * after the last texture UV coordinates and
        * before the bones. It is 36 bytes long.
        * the last three of these values seem to be some sort of
        * "weighted normals"... they definitely have something
        * to do with normals as at least one of these triplets
        * was equal to the normal in c_hutt --> eye01, and the others
        * have similar values
        */
    unsigned int nOffsetToUnusedMDXStructure1; // -1
    unsigned int nOffsetToUnusedMDXStructure2; // -1
    unsigned int nOffsetToUnusedMDXStructure3; // -1
    short nNumberOfVerts; //MDLOps: VertCoordNumber
    short nTextureNumber; //MDLOps: Texture Number
        /*
         * Some relevant nwmax options DarthParametric gathered and tested:
         * scale, transparencyhint, alpha, render, shadow, rotatetexture, beaming, inheritcolor, tilefade
         */
    char nHasLightmap; //0, 1 (per ndix UR, breaks down into two bytes, the first is for lightmaps, the second might be RotateTexture
    char nRotateTexture;
    char nBackgroundGeometry; //Apparently always 0 (per ndix UR needs study - further says: BackgroundGeometry)
    char nShadow; //MDLOps: Shadow
    char nBeaming; //Apparently always 0, might correspond to beaming in nwn (per ndix UR)
    char nRender; //MDLOps: Render
    char nUnknown30; //Still unknown
    char nUnknown31; //Still unknown
    short nUnknown32;
    short nUnknown33;
    double fUnknown7; // = 0 (not zero!)
    double fTotalArea; //K2 only!
    unsigned int nK2Unknown2 = 0; //K2 only!
    unsigned int nOffsetIntoMdx;
    unsigned int nOffsetToVertArray;

    //Added members
    std::vector<Face> Faces;
    std::vector<VertIndicesStruct> VertIndices;
    std::vector<Vertex> Vertices;
    MDXDataStruct MDXData;
    double fMdxConstant [100];
    unsigned int nVertIndicesCount;
    unsigned int nVertIndicesLocation;
    unsigned int nMeshInvertedCounter;
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
    int nUnknown1 = 0;
    int nUnknown2 = 0;
    int nUnknown3 = 0;
    int nPointerToStruct1InMDX;
    int nPointerToStruct2InMDX;
    unsigned int nOffsetToBonemap;
    unsigned int nNumberOfBonemap;
    ArrayHead QBoneArray;
    ArrayHead TBoneArray;
    //I believe that this array is empty.
    //Test this by erasing it to 00s in some skin, then try the model ingame
    ArrayHead Array8Array;
    short nBoneIndexes[18];

    std::vector<Bone> Bones;
    std::vector<int> BoneNameIndexes;
    /**
    INFO about weights from NWMax.
    the number after "weights" is the number of vertexes of the skin modifier
    ...whatever, just check aurora_fn_export.ms
    /**/
};

// if NODE_HAS_DANGLY
struct DanglymeshHeader{
    //Binary members
    ArrayHead ConstraintArray;
    double fDisplacement;
    double fTightness;
    double fPeriod;
    unsigned int nOffsetToData2;

    //Added members
    int nLength = 28;
    std::vector<double> Constraints;
    std::vector<Vector> Data2;
};

// if NODE_HAS_AABB
struct WalkmeshHeader{
    //Binary members
    unsigned int nOffsetToAabb;

    //Added members
    std::vector<Aabb> ArrayOfAabb;
    Aabb RootAabb;
};

// if NODE_HAS_SABER
struct SaberHeader{
    //Binary members
    unsigned int nOffsetToSaberData1;
    unsigned int nOffsetToSaberData2;
    unsigned int nOffsetToSaberData3;
    int nInvCount1;
    int nInvCount2;

    //Added members
    int nNumberOfSaberData;
    std::vector<SaberDataStruct> SaberData;
};

struct Node{
    unsigned int nOffset;
    unsigned int nSize;
    int nAnimation;

    Header Head;
    LightHeader Light;
    EmitterHeader Emitter;
    MeshHeader Mesh;
    SkinHeader Skin;
    DanglymeshHeader Dangly;
    WalkmeshHeader Walkmesh;
    SaberHeader Saber;

    //Node * Parent = nullptr; //Only to be used during compilation to make life easier
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
                location.oOrientation.nCompressed = Head.ControllerData.at(orictrl.nDataStart);
                location.oOrientation.Decompress();
            }
            location.oOrientation.ConvertToAA();
        }

        return location;
    }
};

/**** HIGHER ELEMENTS ****/

struct Animation{
    //Binary members
    //Geoheader part
    unsigned int nFunctionPointer0;
    unsigned int nFunctionPointer1;
    std::string cName;
    unsigned int nOffsetToRootAnimationNode;
    unsigned int nNumberOfObjects; //(same as number of names and nodes, why?)
    int nUnknownEmpty3 [6];
    int nUnknownEmpty4;
    unsigned char nModelType; //??
    unsigned char nPadding [3]; //??

    //Animation-specific part
    double fLength; //According to ascii, the Length
    double fTransition; //According to farmboy0, transition //MDLOps :Trans Time
    std::string cName2; //Name of parent? owner? MDLOps: AnimRoot
    ArrayHead SoundArray; //MDLOps: events
    int nUnknownEmpty6;

    //Added members
    int nNodeCount;
    unsigned int nOffset;
    int nLength = 136; //Header, not the whole thing
    std::vector<Sound> Sounds;
    Node RootAnimationNode;
    //std::vector<Node*> ArrayOfNodePointers;
    std::vector<Node> ArrayOfNodes;
};

struct GeometryHeader{
    //Binary members
    unsigned int nFunctionPointer0;
    unsigned int nFunctionPointer1;
    std::string cName;
    unsigned int nOffsetToRootNode; //Root Node or AuroraBase
    //It turns out that the following can sometimes be different than the other node counters. This is the "TotalNodeCount" (MDLOps)
    unsigned int nTotalNumberOfNodes; // Total number of nodes = Number of nodes in this model + Total number of nodes in supermodel + 1
    int nUnknownEmpty [6];
    int nRefCount; //?? is it?
    unsigned char nModelType; //??
    unsigned char nPadding [3]; //??

    //Added members
    int nLength = 80;
};

struct ModelHeader{
    //Binary members
    char nClassification;
    char nUnknown1 [3];
    unsigned int nChildModelCount; //??
    ArrayHead AnimationArray;

    unsigned int nUnknown2;
    /*Pointer to supermodel?
    Always present when there is a supermodel
    but very variable, models sharing a supermodel
    do not share this value.
    */
    Vector vBBmin;
    Vector vBBmax;
    double fRadius;
    double fScale; //MDLOps: Animation Scale
    std::string cSupermodelName;
    unsigned int nOffsetToHeadRootNode; //This one does not always refer to the root node !!!!!! Check handmaiden's head!
    int nUnknownEmpty3;
    unsigned int nMdxLength2;
    unsigned int nOffsetIntoMdx;
    ArrayHead NameArray;

    //Added members
    int nLength = 116; //Without geometry header and file header
    GeometryHeader GH;
    std::vector<Animation> Animations;
    std::vector<Name> Names;
    Node RootNode;
    std::vector<Node> ArrayOfNodes;
    std::vector<std::vector<LinkedFace>> LinkedFacesPointers;
    std::vector<std::vector<Patch>> PatchArrayPointers;
};

struct FileHeader{
    //Binary members
    unsigned int nID; //Always 00 00 00 00
    unsigned int nMdlLength; //Not exact
    unsigned int nMdxLength; //Not exact

    //Added members
    int nLength = 12;
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
    void CreatePatches();
    void DetermineSmoothing();
    bool bDetermineSmoothing = true;
    bool bSmoothAreaWeighting = true;
    bool bSmoothAngleWeighting = false;
    void GenerateSmoothingNumber(std::vector<int> & SmoothingGroup, const std::vector<unsigned long int> & nSmoothinGroupNumbers, const int & nSmoothinGroupCounter, const int & pg);
    bool FindNormal(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup, const Vector & vNormalBase, const Vector & vNormal, std::vector<int> & CurrentlySmoothedPatches, std::ofstream & file);
    Vector GetTransformedCoordinates(Vertex & vert, int nNameIndex);

    //Getters
    const std::string GetName(){
        return sName;
    }

  public:
    //Friends
    friend Ascii;
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
                if(FH[0].MH.Names.at(FH[0].MH.ArrayOfNodes.at(n).Head.nNameIndex).cName == "neck_g") return true;
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
                if(FH[0].MH.Names.at(n).cName == "neck_g") nNameIndex = n;
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
