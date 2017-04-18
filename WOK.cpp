#include "MDL.h"

//This is the BWM counterpart to MDL's DecompileModel().
void BWM::ProcessBWM(){
    if(sBuffer.empty()) return;
    unsigned int nPos = 0;

    Bwm.reset(new BWMHeader);

    BWMHeader & Data = *Bwm;

    //Mark version info
    MarkBytes(nPos, 8, 3);
    nPos+=8;

    Data.nType = ReadInt(&nPos, 4);
    Data.vUse1 = ReadVector(&nPos, 2);
    Data.vUse2 = ReadVector(&nPos, 2);
    Data.vDwk1 = ReadVector(&nPos, 2);
    Data.vDwk2 = ReadVector(&nPos, 2);
    Data.vPosition = ReadVector(&nPos, 2);

    //Skip to useful data
    Data.nNumberOfVerts = ReadInt(&nPos, 1); //This is not equal in the wok and mdl
    Data.nOffsetToVerts = ReadInt(&nPos, 6);
    Data.nNumberOfFaces = ReadInt(&nPos, 1); // In my test model this equals the number in the mdl
    Data.nOffsetToIndexes = ReadInt(&nPos, 6);
    Data.nOffsetToMaterials = ReadInt(&nPos, 6);
    Data.nOffsetToNormals = ReadInt(&nPos, 6);
    Data.nOffsetToDistances = ReadInt(&nPos, 6);
    Data.nNumberOfAabb = ReadInt(&nPos, 1); // In my test model this equals the number of aabb in the mdl
    Data.nOffsetToAabb = ReadInt(&nPos, 6);
    Data.nUnknown2 = ReadInt(&nPos, 10);
    Data.nNumberOfAdjacentFaces = ReadInt(&nPos, 1);
    Data.nOffsetToAdjacentFaces = ReadInt(&nPos, 6);
    Data.nNumberOfEdges = ReadInt(&nPos, 1);
    Data.nOffsetToEdges = ReadInt(&nPos, 6);
    Data.nNumberOfPerimeters = ReadInt(&nPos, 1);
    Data.nOffsetToPerimeters = ReadInt(&nPos, 6);

    Data.verts.resize(Data.nNumberOfVerts);
    nPos = Data.nOffsetToVerts;
    for(int n = 0; n < Data.nNumberOfVerts; n++){
        //Collect verts
        Data.verts.at(n).fX = ReadFloat(&nPos, 2);
        Data.verts.at(n).fY = ReadFloat(&nPos, 2);
        Data.verts.at(n).fZ = ReadFloat(&nPos, 2);
    }
    Data.faces.resize(Data.nNumberOfFaces);
    nPos = Data.nOffsetToIndexes;
    for(int n = 0; n < Data.nNumberOfFaces; n++){
        //Collect indexes
        Data.faces.at(n).nIndexVertex[0] = ReadInt(&nPos, 4);
        Data.faces.at(n).nIndexVertex[1] = ReadInt(&nPos, 4);
        Data.faces.at(n).nIndexVertex[2] = ReadInt(&nPos, 4);
    }
    nPos = Data.nOffsetToMaterials;
    for(int n = 0; n < Data.nNumberOfFaces; n++){
        //Collect materials
        Data.faces.at(n).nMaterialID = ReadInt(&nPos, 4);
    }
    nPos = Data.nOffsetToNormals;
    for(int n = 0; n < Data.nNumberOfFaces; n++){
        //Collect normals
        Data.faces.at(n).vNormal.fX = ReadFloat(&nPos, 2);
        Data.faces.at(n).vNormal.fY = ReadFloat(&nPos, 2);
        Data.faces.at(n).vNormal.fZ = ReadFloat(&nPos, 2);
    }
    nPos = Data.nOffsetToDistances;
    for(int n = 0; n < Data.nNumberOfFaces; n++){
        //Collect distances
        Data.faces.at(n).fDistance = ReadFloat(&nPos, 2);
    }
    Data.aabb.resize(Data.nNumberOfAabb);
    nPos = Data.nOffsetToAabb;
    for(int n = 0; n < Data.nNumberOfAabb; n++){
        //Collect aabb
        Data.aabb.at(n).vBBmin.fX = ReadFloat(&nPos, 2);
        Data.aabb.at(n).vBBmin.fY = ReadFloat(&nPos, 2);
        Data.aabb.at(n).vBBmin.fZ = ReadFloat(&nPos, 2);
        Data.aabb.at(n).vBBmax.fX = ReadFloat(&nPos, 2);
        Data.aabb.at(n).vBBmax.fY = ReadFloat(&nPos, 2);
        Data.aabb.at(n).vBBmax.fZ = ReadFloat(&nPos, 2);
        Data.aabb.at(n).nID = ReadInt(&nPos, 4);
        Data.aabb.at(n).nExtra = ReadInt(&nPos, 4);
        Data.aabb.at(n).nProperty = ReadInt(&nPos, 4);
        Data.aabb.at(n).nChild1 = ReadInt(&nPos, 4);
        Data.aabb.at(n).nChild2 = ReadInt(&nPos, 4);
    }
    //Data.adjacentfaces.resize(Data.nNumberOfAdjacentFaces);
    nPos = Data.nOffsetToAdjacentFaces;
    for(int n = 0; n < Data.nNumberOfAdjacentFaces; n++){
        //Collect triples
        //Data.adjacentfaces.at(n).n1 = ReadInt(&nPos, 1);
        //Data.adjacentfaces.at(n).n2 = ReadInt(&nPos, 1);
        //Data.adjacentfaces.at(n).n3 = ReadInt(&nPos, 1);

        if(n < Data.faces.size()){
            Data.faces.at(n).nAdjacentFaces[0] = ReadInt(&nPos, 4);
            Data.faces.at(n).nAdjacentFaces[1] = ReadInt(&nPos, 4);
            Data.faces.at(n).nAdjacentFaces[2] = ReadInt(&nPos, 4);
        }
        else Error("More adjacent faces than faces!");
    }
    Data.edges.resize(Data.nNumberOfEdges);
    nPos = Data.nOffsetToEdges;
    for(int n = 0; n < Data.nNumberOfEdges; n++){
        Data.edges.at(n).nIndex = ReadInt(&nPos, 4);
        Data.edges.at(n).nTransition = ReadInt(&nPos, 4);
    }
    Data.perimeters.resize(Data.nNumberOfPerimeters);
    nPos = Data.nOffsetToPerimeters;
    for(int n = 0; n < Data.nNumberOfPerimeters; n++){
        Data.perimeters.at(n) = ReadInt(&nPos, 4);
    }
}

void WOK::WriteWok(Node & node){
    GetData().reset(new BWMHeader);
    BWMHeader & Data = *GetData();

    std::cout<< "Writing wok.\n";
    Data.nType = 1;
    Data.vPosition = node.Head.vPos;

    //First copy our faces over to the Wok data. Put all walkables to the front.
    std::vector<Face> unwalkable;
    for(int f = 0; f < node.Mesh.Faces.size(); f++){
        Face & face = node.Mesh.Faces.at(f);
        Vector v1 = node.Mesh.Vertices.at(face.nIndexVertex[0]).vFromRoot + Vector(41.0, 50.0, 0.0);
        if(face.nMaterialID != 7){
            Data.faces.push_back(face);
            Data.faces.back().bProcessed[0] = false;
            Data.faces.back().bProcessed[1] = false;
            Data.faces.back().bProcessed[2] = false;
            Data.faces.back().nAdjacentFaces[0] = -1;
            Data.faces.back().nAdjacentFaces[1] = -1;
            Data.faces.back().nAdjacentFaces[2] = -1;
            Data.faces.back().fDistance = - (face.vNormal.fX * v1.fX +
                                             face.vNormal.fY * v1.fY +
                                             face.vNormal.fZ * v1.fZ);
        }
        else{
            unwalkable.push_back(face);
            unwalkable.back().bProcessed[0] = false;
            unwalkable.back().bProcessed[1] = false;
            unwalkable.back().bProcessed[2] = false;
            unwalkable.back().nAdjacentFaces[0] = -1;
            unwalkable.back().nAdjacentFaces[1] = -1;
            unwalkable.back().nAdjacentFaces[2] = -1;
            unwalkable.back().fDistance = - (face.vNormal.fX * v1.fX +
                                             face.vNormal.fY * v1.fY +
                                             face.vNormal.fZ * v1.fZ);
        }
    }
    for(int f = 0; f < unwalkable.size(); f++){
        Data.faces.push_back(unwalkable.at(f));
    }

    //Calculate adjacent faces again
    for(int f = 0; f < Data.faces.size(); f++){
        Face & face = Data.faces.at(f);

        // Skip if none is -1
        if(face.nAdjacentFaces[0]==-1 ||
           face.nAdjacentFaces[1]==-1 ||
           face.nAdjacentFaces[2]==-1 ){
            //Go through all the faces coming after this one
            for(int f2 = f+1; f2 < Data.faces.size(); f2++){
                Face & compareface = Data.faces.at(f2);
                std::vector<bool> VertMatches(3, false);
                std::vector<bool> VertMatchesCompare(3, false);
                for(int i = 0; i < 3; i++){
                    int nVertIndex = face.nIndexVertex[i];
                    Vector & ourvect = node.Mesh.Vertices.at(nVertIndex).vFromRoot;
                    for(int i2 = 0; i2 < 3; i2++){
                        Vector & othervect = node.Mesh.Vertices.at(compareface.nIndexVertex[i2]).vFromRoot;
                        if(ourvect.Compare(othervect)){
                            VertMatches.at(i) = true;
                            VertMatchesCompare.at(i2) = true;
                            i2 = 3; // we can only have one matching vert in a face per vert. Once we find a match, we're done.
                        }
                    }
                }
                if(VertMatches.at(0) && VertMatches.at(1)){
                    if(face.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 0...\n";
                    else face.nAdjacentFaces[0] = f2;
                }
                else if(VertMatches.at(1) && VertMatches.at(2)){
                    if(face.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 1...\n";
                    else face.nAdjacentFaces[1] = f2;
                }
                else if(VertMatches.at(2) && VertMatches.at(0)){
                    if(face.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 2...\n";
                    else face.nAdjacentFaces[2] = f2;
                }
                if(VertMatchesCompare.at(0) && VertMatchesCompare.at(1)){
                    if(compareface.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f2<<") for edge 0...\n";
                    else compareface.nAdjacentFaces[0] = f;
                }
                else if(VertMatchesCompare.at(1) && VertMatchesCompare.at(2)){
                    if(compareface.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f2<<") for edge 1...\n";
                    else compareface.nAdjacentFaces[1] = f;
                }
                else if(VertMatchesCompare.at(2) && VertMatchesCompare.at(0)){
                    if(compareface.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f2<<") for edge 2...\n";
                    else compareface.nAdjacentFaces[2] = f;
                }
                if(face.nAdjacentFaces[0]!=-1 &&
                   face.nAdjacentFaces[1]!=-1 &&
                   face.nAdjacentFaces[2]!=-1 ){
                    f2 = Data.faces.size(); //Found them all, maybe I finish early?
                }
            }
        }
    }

    //Build verts array and udpate the indices
    for(int f = 0; f < Data.faces.size(); f++){
        Face & face = Data.faces.at(f);
        for(int i = 0; i < 3; i++){
            if(!face.bProcessed[i]){
                Vector vert (node.Mesh.Vertices.at(face.nIndexVertex[i]).vFromRoot);

                for(int f2 = f; f2 < Data.faces.size(); f2++){
                    Face & face2 = Data.faces.at(f2);
                    for(int i2 = 0; i2 < 3; i2++){
                        //Make sure that we're only changing what's past our current position if we are in the same face.
                        if(f2 != f || i2 > i){
                            if(vert.Compare(node.Mesh.Vertices.at(face2.nIndexVertex[i2]).vFromRoot) &&
                               !face2.bProcessed[i2] ){
                                face2.bProcessed[i2] = true;
                                face2.nIndexVertex[i2] = Data.verts.size();
                            }
                        }
                    }
                }

                face.nIndexVertex[i] = Data.verts.size();
                face.bProcessed[i] = true;
                vert += Vector(41.0, 50.0, 0.0);
                Data.verts.push_back(std::move(vert));
            }
        }
        face.vBBmax = Vector(-10000.0, -10000.0, -10000.0);
        face.vBBmin = Vector(10000.0, 10000.0, 10000.0);
        for(int i = 0; i < 3; i++){
            face.vBBmax.fX = std::max(face.vBBmax.fX, Data.verts.at(face.nIndexVertex[i]).fX);
            face.vBBmax.fY = std::max(face.vBBmax.fY, Data.verts.at(face.nIndexVertex[i]).fY);
            face.vBBmax.fZ = std::max(face.vBBmax.fZ, Data.verts.at(face.nIndexVertex[i]).fZ);
            face.vBBmin.fX = std::min(face.vBBmin.fX, Data.verts.at(face.nIndexVertex[i]).fX);
            face.vBBmin.fY = std::min(face.vBBmin.fY, Data.verts.at(face.nIndexVertex[i]).fY);
            face.vBBmin.fZ = std::min(face.vBBmin.fZ, Data.verts.at(face.nIndexVertex[i]).fZ);
        }
        face.vBBmax += Vector(0.01, 0.01, 0.01);
        face.vBBmin -= Vector(0.01, 0.01, 0.01);
    }

    //Create aabb tree
    std::vector<Face*> allfaces;
    for(int f = 0; f < Data.faces.size(); f++){
        allfaces.push_back(&Data.faces.at(f));
    }
    Aabb rootaabb;
    BuildAabb(rootaabb, allfaces);
    LinearizeAabb(rootaabb, Data.aabb);
}
