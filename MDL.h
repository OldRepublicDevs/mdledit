#ifndef MDL_H_INCLUDED
#define MDL_H_INCLUDED

#include "general.h"
#include "file.h"

#define MDL_OFFSET 12
#define ANIM_OFFSET 136

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
#define CONVERT_WOK                      16
#define CONVERT_PWK                      17
#define CONVERT_DWK                      18

#define K1_FUNCTION_POINTER_0 4273776
#define K1_FUNCTION_POINTER_1 4216096
#define K2_FUNCTION_POINTER_0 4285200
#define K2_FUNCTION_POINTER_1 4216320

#define CLASS_OTHER           0x00
#define CLASS_EFFECT          0x01
#define CLASS_TILE            0x02
#define CLASS_CHARACTER       0x04
#define CLASS_DOOR            0x08
#define CLASS_SABER           0x10 //Probably more general, weapon or item
#define CLASS_PLACEABLE       0x20
//#define CLASS_40            0x40
//#define CLASS_80            0x80

#define NODE_HAS_HEADER       0x0001
#define NODE_HAS_LIGHT        0x0002
#define NODE_HAS_EMITTER      0x0004
//#define NODE_HAS_CAMERA     0x0008
//#define NODE_HAS_REFERENCE  0x0010
#define NODE_HAS_MESH         0x0020
#define NODE_HAS_SKIN         0x0040
//#define NODE_HAS_ANIM       0x0080
#define NODE_HAS_DANGLY       0x0100
#define NODE_HAS_AABB         0x0200
//#define NODE_HAS_400        0x0400
#define NODE_HAS_SABER        0x0800
//#define NODE_HAS_GOB        0x1000
//#define NODE_HAS_COLLISION  0x2000
//#define NODE_HAS_SPHERE     0x4000
//#define NODE_HAS_CAPSULE    0x8000

#define NODE_SIZE_HEADER      80
#define NODE_SIZE_LIGHT       92
#define NODE_SIZE_EMITTER     224
//#define NODE_SIZE_CAMERA    0
//#define NODE_SIZE_REFERENCE 0
#define NODE_SIZE_MESH        340
#define NODE_SIZE_SKIN        100
//#define NODE_SIZE_ANIM      0
#define NODE_SIZE_DANGLY      28
#define NODE_SIZE_AABB        1
//#define NODE_SIZE_400       0
#define NODE_SIZE_SABER       20   // Not supported by NWMax

#define EMITTER_FLAG_P2P                0x0001
#define EMITTER_FLAG_P2P_SEL            0x0002
#define EMITTER_FLAG_AFFECTED_WIND      0x0004
#define EMITTER_FLAG_TINTED             0x0008
#define EMITTER_FLAG_BOUNCE             0x0010
#define EMITTER_FLAG_RANDOM             0x0020
#define EMITTER_FLAG_INHERIT            0x0040
#define EMITTER_FLAG_INHERIT_VEL        0x0080
#define EMITTER_FLAG_INHERIT_LOCAL      0x0100
#define EMITTER_FLAG_SPLAT              0x0200
#define EMITTER_FLAG_INHERIT_PART       0x0400
#define EMITTER_FLAG_DEPTH_TEXTURE      0x0800 //ndix UR¸  // Not supported by NWMax
#define EMITTER_FLAG_RENDER_ORDER       0x1000 //ndix UR   // Not supported by NWMax
//#define EMITTER_FLAG_2000             0x2000
//#define EMITTER_FLAG_3000             0x4000
//#define EMITTER_FLAG_4000             0x8000

#define MDX_FLAG_VERTEX                0x0001
#define MDX_FLAG_HAS_UV1               0x0002
#define MDX_FLAG_HAS_UV2               0x0004
#define MDX_FLAG_HAS_UV3               0x0008
#define MDX_FLAG_HAS_UV4               0x0010
#define MDX_FLAG_HAS_NORMAL            0x0020
#define MDX_FLAG_0040         /*??*/   0x0040
#define MDX_FLAG_HAS_TANGENT1          0x0080
#define MDX_FLAG_HAS_TANGENT2 /*??*/   0x0100
#define MDX_FLAG_HAS_TANGENT3 /*??*/   0x0200
#define MDX_FLAG_HAS_TANGENT4 /*??*/   0x0400
//#define MDX_FLAG_0800                0x0800
//#define MDX_FLAG_1000                0x1000
//#define MDX_FLAG_2000                0x2000
//#define MDX_FLAG_3000                0x4000
//#define MDX_FLAG_4000                0x8000

#define AABB_NO_CHILDREN    0x00
#define AABB_POSITIVE_X     0x01
#define AABB_POSITIVE_Y     0x02
#define AABB_POSITIVE_Z     0x04
#define AABB_NEGATIVE_X     0x08
#define AABB_NEGATIVE_Y     0x10
#define AABB_NEGATIVE_Z     0x20
//#define AABB_40           0x40
//#define AABB_80           0x80

#define MATERIAL_NONE           0
#define MATERIAL_DIRT           1
#define MATERIAL_OBSCURING      2
#define MATERIAL_GRASS          3
#define MATERIAL_STONE          4
#define MATERIAL_WOOD           5
#define MATERIAL_WATER          6
#define MATERIAL_NONWALK        7
#define MATERIAL_TRANSPARENT    8
#define MATERIAL_CARPET         9
#define MATERIAL_METAL          10
#define MATERIAL_PUDDLES        11
#define MATERIAL_SWAMP          12
#define MATERIAL_MUD            13
#define MATERIAL_LEAVES         14
#define MATERIAL_LAVA           15
#define MATERIAL_BOTTOMLESSPIT  16
#define MATERIAL_DEEPWATER      17
#define MATERIAL_DOOR           18
#define MATERIAL_SNOW           19
#define MATERIAL_SAND           20

///All controller numbers apparently must be divisible by 4
#define CONTROLLER_HEADER_POSITION              8
#define CONTROLLER_HEADER_ORIENTATION           20
#define CONTROLLER_HEADER_SCALING               36
///------------------------------------------------
#define CONTROLLER_MESH_SELFILLUMCOLOR          100
#define CONTROLLER_MESH_ALPHA                   132
///------------------------------------------------
#define CONTROLLER_LIGHT_COLOR                  76
#define CONTROLLER_LIGHT_RADIUS                 88
#define CONTROLLER_LIGHT_SHADOWRADIUS           96   // Not supported by NWMax
#define CONTROLLER_LIGHT_VERTICALDISPLACEMENT   100  // Not supported by NWMax
#define CONTROLLER_LIGHT_MULTIPLIER             140
///------------------------------------------------
#define CONTROLLER_EMITTER_ALPHAEND             80
#define CONTROLLER_EMITTER_ALPHASTART           84
#define CONTROLLER_EMITTER_BRITHRATE            88
#define CONTROLLER_EMITTER_BOUNCE_CO            92
#define CONTROLLER_EMITTER_COMBINETIME          96
#define CONTROLLER_EMITTER_DRAG                 100
#define CONTROLLER_EMITTER_FPS                  104
#define CONTROLLER_EMITTER_FRAMEEND             108
#define CONTROLLER_EMITTER_FRAMESTART           112
#define CONTROLLER_EMITTER_GRAV                 116
#define CONTROLLER_EMITTER_LIFEEXP              120
#define CONTROLLER_EMITTER_MASS                 124
#define CONTROLLER_EMITTER_P2P_BEZIER2          128
#define CONTROLLER_EMITTER_P2P_BEZIER3          132
#define CONTROLLER_EMITTER_PARTICLEROT          136
#define CONTROLLER_EMITTER_RANDVEL              140
#define CONTROLLER_EMITTER_SIZESTART            144
#define CONTROLLER_EMITTER_SIZEEND              148
#define CONTROLLER_EMITTER_SIZESTART_Y          152
#define CONTROLLER_EMITTER_SIZEEND_Y            156
#define CONTROLLER_EMITTER_SPREAD               160
#define CONTROLLER_EMITTER_THRESHOLD            164
#define CONTROLLER_EMITTER_VELOCITY             168
#define CONTROLLER_EMITTER_XSIZE                172
#define CONTROLLER_EMITTER_YSIZE                176
#define CONTROLLER_EMITTER_BLURLENGTH           180
#define CONTROLLER_EMITTER_LIGHTNINGDELAY       184
#define CONTROLLER_EMITTER_LIGHTNINGRADIUS      188
#define CONTROLLER_EMITTER_LIGHTNINGSCALE       192
#define CONTROLLER_EMITTER_LIGHTNINGSUBDIV      196
#define CONTROLLER_EMITTER_LIGHTNINGZIGZAG      200  // Not supported by NWMax
//#define CONTROLLER_EMITTER_204                204
//#define CONTROLLER_EMITTER_208                208
//#define CONTROLLER_EMITTER_212                212
#define CONTROLLER_EMITTER_ALPHAMID             216  // Not supported by NWMax
#define CONTROLLER_EMITTER_PERCENTSTART         220  // Not supported by NWMax
#define CONTROLLER_EMITTER_PERCENTMID           224  // Not supported by NWMax
#define CONTROLLER_EMITTER_PERCENTEND           228  // Not supported by NWMax
#define CONTROLLER_EMITTER_SIZEMID              232  // Not supported by NWMax
#define CONTROLLER_EMITTER_SIZEMID_Y            236  // Not supported by NWMax
#define CONTROLLER_EMITTER_RANDOMBIRTHRATE      240  // Not supported by NWMax
//#define CONTROLLER_EMITTER 244                244
//#define CONTROLLER_EMITTER 248                248
#define CONTROLLER_EMITTER_TARGETSIZE           252  // Not supported by NWMax
#define CONTROLLER_EMITTER_NUMCONTROLPTS        256  // Not supported by NWMax
#define CONTROLLER_EMITTER_CONTROLPTRADIUS      260  // Not supported by NWMax
#define CONTROLLER_EMITTER_CONTROLPTDELAY       264  // Not supported by NWMax
#define CONTROLLER_EMITTER_TANGENTSPREAD        268  // Not supported by NWMax
#define CONTROLLER_EMITTER_TANGENTLENGTH        272  // Not supported by NWMax
//#define CONTROLLER_EMITTER_276                276
//#define CONTROLLER_EMITTER_280                280
#define CONTROLLER_EMITTER_COLORMID             284  // Not supported by NWMax
/// ... many controller numbers ... ///
#define CONTROLLER_EMITTER_COLOREND             380
//#define CONTROLLER_EMITTER_384                384
//#define CONTROLLER_EMITTER_388                388
#define CONTROLLER_EMITTER_COLORSTART           392
/// ... many controller numbers ... ///
#define CONTROLLER_EMITTER_DETONATE             502  // Not supported by NWMax

//Data structures
struct Face;
struct Aabb;
struct Controller;
struct Sound;
struct Name;
struct SaberDataStruct;
struct VertIndicesStruct;
struct MDXDataStruct;
struct Vertex;
struct Color;
struct Triples;
struct Bone;

//Node structs
struct Node;
struct Header;
struct LightHeader;
struct EmitterHeader;
struct MeshHeader;
struct SkinHeader;
struct DanglymeshHeader;
struct WalkmeshHeader;
struct SaberHeader;

//Higher structures
struct Animation;
struct GeometryHeader;
struct ModelHeader;
struct FileHeader;

//File structs
class MDL;
class MDX;
class WOK;
class PWK;
class DWK;
class Ascii;

//bool LoadSupermodel(MDL & curmdl, std::vector<MDL> & Supermodels);
bool LoadSupermodel(MDL & curmdl, std::unique_ptr<MDL> & Supermodel);


/**** DATA STRUCTS ****/

struct Location{
    Vector vPosition;
    Orientation oOrientation;
};

struct Edge{
    int nIndex;
    int nTransition;

    Edge(int nIndex = -1, int nTransition = -1): nIndex(nIndex), nTransition(nTransition) {}
};

struct Face{
    //Binary members
    Vector vNormal;
    double fDistance = 0.0;
    int nMaterialID = 0;
    std::array<short, 3> nAdjacentFaces = {-1, -1, -1}; //short nAdjacentFaces [3] = {-1, -1, -1};
    std::array<short, 3> nIndexVertex = {-1, -1, -1}; //short nIndexVertex[3] = {-1, -1, -1};

    //Added members
    std::array<short, 3> nIndexTvert = {-1, -1, -1}; //short nIndexTvert[3] = {-1, -1, -1};
    std::array<short, 3> nIndexTvert1 = {-1, -1, -1}; //short nIndexTvert1[3] = {-1, -1, -1};
    std::array<short, 3> nIndexTvert2 = {-1, -1, -1}; //short nIndexTvert2[3] = {-1, -1, -1};
    std::array<short, 3> nIndexTvert3 = {-1, -1, -1}; //short nIndexTvert3[3] = {-1, -1, -1};
    std::array<bool, 3> bProcessed = {false, false, false}; //bool bProcessed[3] = {false, false, false};
    bool bProcessedSG = false;
    std::array<short, 3> nEdgeTransitions = {-1, -1, -1};
    int nTextureCount = 0;
    int nSmoothingGroup = 1;
    double fArea = 0.0;
    double fAreaUV = 0.0;
    Vector vTangent;
    Vector vBitangent;
    Vector vBBmin;
    Vector vBBmax;
    Vector vCentroid;
    short nID = -1;
};


struct Aabb{
    //binary members
    Vector vBBmin;
    Vector vBBmax;
    unsigned int nChild1 = 0; //Offset (in the mdl) or Index (in the wok)
    unsigned int nChild2 = 0; //Offset (in the mdl) or Index (in the wok)
    int nID = -1; //An index to the face of the walkmesh
    int nProperty = 0; //AABB_* constants, most significant plane for Child 2

    //Added members
    unsigned int nOffset = 0;
    std::vector<Aabb> Child1;
    std::vector<Aabb> Child2;
    int nExtra = 0;
};

struct Controller{
    //Binary members
    int nControllerType = 0;
    short nUnknown2 = -1;
    unsigned short nValueCount = 0;
    unsigned short nTimekeyStart = 0;
    unsigned short nDataStart = 0;
    char nColumnCount = 0;
    std::array<char, 3> nPadding = {0, 0, 0};//char nPadding [3] = {0, 0, 0};

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
    int nOffsetVertex = 0;
    int nOffsetUV = 0;
    int nOffsetNormal = 0;
    Vector vVertex;
    Vector vUV;
    Vector vNormal;
    SaberDataStruct(){}
    SaberDataStruct(const Vector & v1, const Vector & v2): vVertex(v1), vUV(v2) {}
};

struct VertIndicesStruct{
    std::array<short, 3> nValues = {-1, -1, -1};//short nValues[3] = {-1, -1, -1};
};

struct Weight{
    std::array<double, 4> fWeightValue = {1.0, 0.0, 0.0, 0.0}; //double fWeightValue[4] = {1.0, 0.0, 0.0, 0.0};
    std::array<double, 4> fWeightIndex = {-1.0, -1.0, -1.0, -1.0}; //double fWeightIndex[4] = {-1.0, -1.0, -1.0, -1.0};
};

struct MDXDataStruct{
    //Binary members
    Vector vVertex;
    Vector vNormal;
    Vector vUV1;
    Vector vUV2;
    Vector vUV3;
    Vector vUV4;
    std::array<Vector, 3> vTangent1; //Vector vTangent1[3];
    std::array<Vector, 3> vTangent2; //Vector vTangent2[3];
    std::array<Vector, 3> vTangent3; //Vector vTangent3[3];
    std::array<Vector, 3> vTangent4; //Vector vTangent4[3];
    Weight Weights;

    //Added members
    int nNameIndex = -1;
};

// ArrayHeads should only be relevant during (de)compilation. Otherwise, use vector lengths.
struct ArrayHead{
    unsigned int nOffset = 0;
    unsigned int nCount = 0;
    unsigned int nCount2 = 0;

    bool GetDoCountsDiffer(){
        if(nCount == nCount2) return false;
        else return true; //We sure don't expect that!
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
    bool bUniform = false;
};

struct Vertex: public Vector{
    int nOffset = 0;
    MDXDataStruct MDXData;
    int nLinkedFacesIndex = -1;
    Vertex assign(const Vector & v){
        fX = v.fX;
        fY = v.fY;
        fZ = v.fZ;
        return *this;
    }
    Vector vFromRoot;
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
    unsigned int nOffsetToRoot = 0;
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
    std::vector<int> ChildIndices;
    int nParentIndex = -1;
    Vector vFromRoot;
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
    Vector vBBmin; //not exported
    Vector vBBmax; //not exported
    double fRadius = 0.0; //not exported
    Vector vAverage; //not exported
    Color fDiffuse;
    Color fAmbient;
    unsigned int nTransparencyHint = 0; // 0 (mostly), 1, 2, 3, 4, 5, 7, 8, 13
    std::string cTexture1;
    std::string cTexture2;
    std::string cTexture3;   // Not supported by NWMax
    std::string cTexture4;   // Not supported by NWMax
    ArrayHead IndexCounterArray;
    ArrayHead IndexLocationArray;
    ArrayHead MeshInvertedCounterArray;
        /* MagnusII:
         * = 1, always one UInt32
         * sequence goes like this: first
         * trimesh node is 98, then:
         * 98..00, 100
         * 199..101, 200
         * 399..?
         */
    std::array<int, 3> nUnknown3 = {-1, -1, 0}; //int nUnknown3 [3] = {-1, -1, 0};
    char nSaberUnknown1 = 3; // 3 for non-saber
    char nSaberUnknown2 = 0; // 0 for non-saber
    char nSaberUnknown3 = 0; // 0 for non-saber
    char nSaberUnknown4 = 0; // 0 for non-saber
    char nSaberUnknown5 = 0; // 0 for non-saber
    char nSaberUnknown6 = 0; // 0 for non-saber
    char nSaberUnknown7 = 0; // 0 for non-saber
    char nSaberUnknown8 = 0; // 0 for non-saber
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
    char nHasLightmap = 0;  // Not supported by NWMax
    char nRotateTexture = 0;
    char nBackgroundGeometry = 0;  // Not supported by NWMax
    char nShadow = 0;
    char nBeaming = 0;
    char nRender = 1;
    char nDirtEnabled = 0; //K2  // Not supported by NWMax
    char nUnknown1 = 0; //K2
    short nDirtTexture = 1; //K2  // Not supported by NWMax
    short nDirtCoordSpace = 1; //K2  // Not supported by NWMax
    char nHideInHolograms = 0; //K2  // Not supported by NWMax
    char nUnknown2 = 0; //K2
    short nUnknown4 = 0;
    double fTotalArea = 0.0; //not exported
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
    std::vector<Vector> TempVerts;
    std::vector<Vector> TempTverts;
    std::vector<Vector> TempTverts1;
    std::vector<Vector> TempTverts2;
    std::vector<Vector> TempTverts3;
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
    std::array<short, 18> nBoneIndexes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //short nBoneIndexes[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<Bone> Bones;
    std::vector<int> BoneNameIndexes;
    std::vector<Weight> TempWeights;
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
    std::vector<double> TempConstraints;
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
    Location GetLocation();
};

/**** HIGHER ELEMENTS ****/

struct Animation{
    //Binary members

    /// Geoheader part
    unsigned int nFunctionPointer0 = 0;
    unsigned int nFunctionPointer1 = 0;
    std::string sName;
    unsigned int nOffsetToRootAnimationNode = 0;
    unsigned int nNumberOfNames = 0;
    ArrayHead RuntimeArray1; //Always 0
    ArrayHead RuntimeArray2; //Always 0
    unsigned int nRefCount = 0; //Always 0
    unsigned char nModelType = 5; //1 - geometry, 2 - model, 5 - animation
    std::array<unsigned char, 3> nPadding = {0, 0, 0}; //unsigned char nPadding [3] = {0, 0, 0};

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
    std::array<unsigned char, 3> nPadding = {0, 0, 0}; //unsigned char nPadding[3] = {0, 0, 0};
};

struct ModelHeader{
    //Binary members
    unsigned char nClassification = 0;
    std::array<unsigned char, 3> nUnknown1 = {0, 0, 1}; //unsigned char nUnknown1 [3] = {0, 0, 1};
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
    int nTotalVertCount = 0;
    int nTotalTangent1Count = 0;
    int nTotalTangent2Count = 0;
    int nTotalTangent3Count = 0;
    int nTotalTangent4Count = 0;
    int nNodeCount = 0; //Only the nodes that actually exist, and only the ones in this model
    Vector vLytPosition;
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

struct BWMHeader{
    //Data
    int nType = 0;
    Vector vUse1;
    Vector vUse2;
    Vector vDwk1;
    Vector vDwk2;
    Vector vPosition;
    unsigned int nNumberOfVerts = 0;
    unsigned int nOffsetToVerts = 0;
    unsigned int nNumberOfFaces = 0;
    unsigned int nOffsetToIndexes = 0;
    unsigned int nOffsetToMaterials = 0;
    unsigned int nOffsetToNormals = 0;
    unsigned int nOffsetToDistances = 0;
    unsigned int nNumberOfAabb = 0;
    unsigned int nOffsetToAabb = 0;
    unsigned int nUnknown2 = 0;
    unsigned int nNumberOfAdjacentFaces = 0;
    unsigned int nOffsetToAdjacentFaces = 0;
    unsigned int nNumberOfEdges= 0;
    unsigned int nOffsetToEdges = 0;
    unsigned int nNumberOfPerimeters = 0;
    unsigned int nOffsetToPerimeters = 0;
    std::vector<Aabb> aabb;
    std::vector<Face> faces;
    std::vector<Vector> verts;
    std::vector<Edge> edges;
    std::vector<int> perimeters;
};

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
  /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

class MDX: public BinaryFile{
    static const std::string sClassName;
  public:
     //Getters
     const std::string GetName(){ return sClassName; }

    //Friends
    friend class MDL;
    //We need this so that MDL::functions can call protected members inherited from BinaryFile
};

class ASCII: public TextFile{
  public:
    bool Read(MDL & Mdl);
    bool ReadPwk(MDL & Mdl);
    bool ReadDwk(MDL & Mdl);
};

class MDL: public BinaryFile{
    static const std::string sClassName;
    std::unique_ptr<FileHeader> FH;

    //Reading
    void ParseNode(Node * NODE, int * nNodeCounter, Vector vFromRoot, bool bMinimal = false);
    void ParseAabb(Aabb * AABB, unsigned int nHighestOffset);
    void LinearizeGeometry(Node & NODE, std::vector<Node> & ArrayOfNodes);
    void LinearizeAnimations(Node & NODE, std::vector<Node> & ArrayOfNodes);

    bool CheckNodes(std::vector<Node> & node, std::stringstream & ssReturn, int nAnimation = -1);

    //Writing
    void WriteNodes(Node & node);
    void WriteAabb(Aabb & aabb);
    void GatherChildren(Node & NODE, std::vector<Node> & ArrayOfNodes, Vector vFromRoot);

    //Calculating
    void CreatePatches();
    void DetermineSmoothing();
    void GenerateSmoothingNumber(std::vector<int> & SmoothingGroup, const std::vector<unsigned long int> & nSmoothinGroupNumbers, const int & nSmoothinGroupCounter, const int & pg, std::stringstream & file);
    bool FindNormal(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup, const Vector & vNormalBase, const Vector & vNormal, std::vector<int> & CurrentlySmoothedPatches, std::stringstream & file);
    int FindTangentSpace(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup,
                           const Vector & vTangentBase, const Vector & vBitangentBase, const Vector & vNormalBase,
                           const Vector & vTangent, const Vector & vBitangent, const Vector & vNormal,
                           std::vector<int> & CurrentlySmoothedPatches, std::stringstream & file);
    void ConsolidateSmoothingGroups(int nPatchGroup, std::vector<std::vector<unsigned long int>> & Numbers, std::vector<bool> & DoneGroups);
    //void BuildAabb(Aabb & aabb, const std::vector<Face*> & faces, std::stringstream & file);
    std::string MakeUniqueName(int nNameIndex);

    //Getters
    const std::string GetName(){ return sClassName; }

    //Reporters
    void Report(std::string);
    void ProgressSize(int, int);
    void ProgressPos(int);

  public:
    std::unique_ptr<ASCII> Ascii;
    std::unique_ptr<ASCII> PwkAscii;
    std::unique_ptr<ASCII> DwkAscii;
    std::unique_ptr<MDX> Mdx;
    std::unique_ptr<WOK> Wok;
    std::unique_ptr<PWK> Pwk;
    std::unique_ptr<DWK> Dwk0;
    std::unique_ptr<DWK> Dwk1;
    std::unique_ptr<DWK> Dwk2;

    //Function pointers
    void (*PtrReport)(std::string) = nullptr;
    void (*PtrProgressSize)(int, int) = nullptr;
    void (*PtrProgressPos)(int) = nullptr;

    //Friends
    friend ASCII;
    friend Node;

    //Version
    bool bK2 = true;
    bool bDebug = false;
    bool bDetermineSmoothing = true;
    bool bSmoothAreaWeighting = true;
    bool bSmoothAngleWeighting = false;

    //Getters
    std::unique_ptr<FileHeader> & GetFileData();
    std::vector<char> & GetAsciiBuffer();
    Node & GetNodeByNameIndex(int nIndex, int nAnimation = -1);
    bool HeadLinked();
    bool NodeExists(const std::string & sNodeName);
    int GetNameIndex(std::string sName);
    void UpdateTexture(Node & node, const std::string & sNew, int nTex);
    void GetLytPositionFromWok();

    //Loaders
    bool Compile();
    void DecompileModel(bool bMinimal = false);
    void AsciiPostProcess();
    void BwmAsciiPostProcess(BWMHeader & data, std::vector<Vector> & vertices, bool bDwk = true);
    void CheckPeculiarities();
    void FlushData();
    void ConvertToAscii(int nDataType, std::stringstream & sReturn, void * Data);

    //Setters/general
    bool LinkHead(bool bLink);
    void WriteUintToPlaceholder(unsigned int nUint, int nOffset);
    void WriteByteToPlaceholder(unsigned char nByte, int nOffset);

    //ascii
    void ExportAscii(std::string &sExport);
    void ExportPwkAscii(std::string &sExport);
    void ExportDwkAscii(std::string &sExport);
    void ExportWokAscii(std::string &sExport);
    void FlushAscii();
    std::vector<char> & CreateAsciiBuffer(int nSize);
    bool ReadAscii();
};

class BWM: public BinaryFile{
    std::unique_ptr<BWMHeader> Bwm;

  public:
    //Getters
    std::unique_ptr<BWMHeader> & GetData() { return Bwm; }

    //Loaders
    void ProcessBWM();
    void Compile();
};

class PWK: public BWM{
    static const std::string sClassName;

  public:
    //Getters
    const std::string GetName(){ return sClassName; }
};

class DWK: public BWM{
    static const std::string sClassName;

  public:
    //Getters
    const std::string GetName(){ return sClassName; }
};

class WOK: public BWM{
    static const std::string sClassName;

  public:
    //Getters
    const std::string GetName(){ return sClassName; }
    void WriteWok(Node & node, Vector vLytPos, std::stringstream * ptrssFile);
};


int ReturnController(std::string sController);
std::string ReturnClassificationName(int nClassification);
std::string ReturnControllerName(int, int nType);

void BuildAabb(Aabb & aabb, const std::vector<Face*> & faces, std::stringstream * file = nullptr);
void LinearizeAabb(Aabb & aabbroot, std::vector<Aabb> & aabbarray);


#endif // MDL_H_INCLUDED
