#include "MDL.h"

//This is the WOK counterpart to MDL's DecompileModel().
void WOK::ProcessWalkmesh(){
    if(sBuffer.empty() || nBufferSize == 0) return;
    unsigned int nPos = 0;

    //Mark version info
    MarkBytes(nPos, 8, 3);
    nPos+=8;

    //Skip to useful data
    nPos = 72;
    nNumberOfVerts = ReadInt(&nPos, 1); //This is not equal in the wok and mdl
    nOffsetToVerts = ReadInt(&nPos, 6);
    nNumberOfFaces = ReadInt(&nPos, 1); // In my test model this equals the number in the mdl
    nOffsetToIndexes = ReadInt(&nPos, 6);
    nOffsetToMaterials = ReadInt(&nPos, 6);
    nOffsetToNormals = ReadInt(&nPos, 6);
    nOffsetToDistances = ReadInt(&nPos, 6);
    nNumberOfAabb = ReadInt(&nPos, 1); // In my test model this equals the number of aabb in the mdl
    nOffsetToAabb = ReadInt(&nPos, 6);
    nPos+=4; //Skip a zero
    nNumberOfTriples = ReadInt(&nPos, 1);
    nOffsetToTriples = ReadInt(&nPos, 6);
    nNumberOfDoubles = ReadInt(&nPos, 1);
    nOffsetToDoubles = ReadInt(&nPos, 6);
    nNumberOfSingles = ReadInt(&nPos, 1);
    nOffsetToSingles = ReadInt(&nPos, 6);

    verts.resize(nNumberOfVerts);
    nPos = nOffsetToVerts;
    for(int n = 0; n < nNumberOfVerts; n++){
        //Collect verts
        verts[n].fX = ReadFloat(&nPos, 2);
        verts[n].fY = ReadFloat(&nPos, 2);
        verts[n].fZ = ReadFloat(&nPos, 2);
    }
    faces.resize(nNumberOfFaces);
    nPos = nOffsetToIndexes;
    for(int n = 0; n < nNumberOfFaces; n++){
        //Collect indexes
        faces[n].nIndexVertex[0] = ReadInt(&nPos, 4);
        faces[n].nIndexVertex[1] = ReadInt(&nPos, 4);
        faces[n].nIndexVertex[2] = ReadInt(&nPos, 4);
    }
    nPos = nOffsetToMaterials;
    for(int n = 0; n < nNumberOfFaces; n++){
        //Collect materials
        faces[n].nMaterialID = ReadInt(&nPos, 1);
    }
    nPos = nOffsetToNormals;
    for(int n = 0; n < nNumberOfFaces; n++){
        //Collect normals
        faces[n].vNormal.fX = ReadFloat(&nPos, 2);
        faces[n].vNormal.fY = ReadFloat(&nPos, 2);
        faces[n].vNormal.fZ = ReadFloat(&nPos, 2);
    }
    nPos = nOffsetToDistances;
    for(int n = 0; n < nNumberOfFaces; n++){
        //Collect distances
        faces[n].fDistance = ReadFloat(&nPos, 2);
    }
    aabb.resize(nNumberOfAabb);
    nPos = nOffsetToAabb;
    for(int n = 0; n < nNumberOfAabb; n++){
        //Collect aabb
        aabb[n].vBBmin.fX = ReadFloat(&nPos, 2);
        aabb[n].vBBmin.fY = ReadFloat(&nPos, 2);
        aabb[n].vBBmin.fZ = ReadFloat(&nPos, 2);
        aabb[n].vBBmax.fX = ReadFloat(&nPos, 2);
        aabb[n].vBBmax.fY = ReadFloat(&nPos, 2);
        aabb[n].vBBmax.fZ = ReadFloat(&nPos, 2);
        aabb[n].nID = ReadInt(&nPos, 4);
        aabb[n].nExtra = ReadInt(&nPos, 4);
        aabb[n].nProperty = ReadInt(&nPos, 4);
        aabb[n].nChild1 = ReadInt(&nPos, 4);
        aabb[n].nChild2 = ReadInt(&nPos, 4);
    }
    triples.resize(nNumberOfTriples);
    nPos = nOffsetToTriples;
    for(int n = 0; n < nNumberOfTriples; n++){
        //Collect triples
        triples[n].n1 = ReadInt(&nPos, 1);
        triples[n].n2 = ReadInt(&nPos, 1);
        triples[n].n3 = ReadInt(&nPos, 1);
    }
    doubles.resize(nNumberOfDoubles);
    nPos = nOffsetToDoubles;
    for(int n = 0; n < nNumberOfDoubles; n++){
        //Collect doubles
        doubles[n].n1 = ReadInt(&nPos, 4);
        doubles[n].n2 = ReadInt(&nPos, 4);
    }
    singles.resize(nNumberOfSingles);
    nPos = nOffsetToSingles;
    for(int n = 0; n < nNumberOfSingles; n++){
        //Collect singles
        singles[n] = ReadInt(&nPos, 1);
    }
}

HTREEITEM Append(const std::string & sString, LPARAM lParam = NULL, HTREEITEM hParentNew = NULL, HTREEITEM hAfterNew = NULL, UINT Flags = NULL);

void BuildTree(WOK & Walkmesh){
    HTREEITEM Root = Append(Walkmesh.GetFilename(), NULL, TVI_ROOT);
    HTREEITEM Verts = Append("Vertices", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfVerts; n++){
        std::stringstream ss;
        ss << "Vertex " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.verts[n], Verts);
    }
    HTREEITEM Faces = Append("Faces", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfFaces; n++){
        std::stringstream ss;
        ss << "Face " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.faces[n], Faces);
    }
    HTREEITEM Aabb = Append("Aabb", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfAabb; n++){
        std::stringstream ss;
        ss << "aabb " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.aabb[n], Aabb);
    }
    HTREEITEM Array1 = Append("Array 1", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfTriples; n++){
        std::stringstream ss;
        ss << "Triple " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.triples[n], Array1);
    }
    HTREEITEM Array2 = Append("Array 2", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfDoubles; n++){
        std::stringstream ss;
        ss << "Double " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.doubles[n], Array2);
    }
    HTREEITEM Array3 = Append("Array 3", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.nNumberOfSingles; n++){
        std::stringstream ss;
        ss << "Single " << n;
        Append(ss.str().c_str(), (LPARAM) &Walkmesh.singles[n], Array3);
    }

}
