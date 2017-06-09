#include "MDL.h"
#include <algorithm>

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
    Data.nOffsetToIndices = ReadInt(&nPos, 6);
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
    nPos = Data.nOffsetToIndices;
    for(int n = 0; n < Data.nNumberOfFaces; n++){
        //Collect indices
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
    nPos = Data.nOffsetToAdjacentFaces;
    for(int n = 0; n < Data.nNumberOfAdjacentFaces; n++){
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

void WOK::WriteWok(Node & node, Vector vLytPos, std::stringstream * ptrssFile){
    GetData().reset(new BWMHeader);
    BWMHeader & Data = *GetData();

    std::cout<< "Writing wok.\n";
    Data.nType = 1;
    Data.vPosition = node.Head.vPos;

    //First copy our faces over to the Wok data. Put all walkables to the front.
    std::vector<Face> unwalkable;
    for(int f = 0; f < node.Mesh.Faces.size(); f++){
        Face & face = node.Mesh.Faces.at(f);
        Vector v1 = node.Mesh.Vertices.at(face.nIndexVertex[0]).vFromRoot + vLytPos;
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

    //Calculate adjacent edges
    for(int f = 0; f < Data.faces.size(); f++){
        Face & face = Data.faces.at(f);

        face.nID = f;

        // Skip if none is -1
        if((face.nAdjacentFaces[0]==-1 ||
           face.nAdjacentFaces[1]==-1 ||
           face.nAdjacentFaces[2]==-1 ) && face.nMaterialID != 7){
            //Go through all the faces coming after this one
            for(int f2 = f+1; f2 < Data.faces.size(); f2++){
                Face & compareface = Data.faces.at(f2);
                if(compareface.nMaterialID != 7){
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
                    int vertmatch = -1;
                    int comparevertmatch = -1;
                    if(VertMatches.at(0) && VertMatches.at(1)) vertmatch = 0;
                    else if(VertMatches.at(1) && VertMatches.at(2)) vertmatch = 1;
                    else if(VertMatches.at(2) && VertMatches.at(0)) vertmatch = 2;
                    if(VertMatchesCompare.at(0) && VertMatchesCompare.at(1)) comparevertmatch = 0;
                    else if(VertMatchesCompare.at(1) && VertMatchesCompare.at(2)) comparevertmatch = 1;
                    else if(VertMatchesCompare.at(2) && VertMatchesCompare.at(0)) comparevertmatch = 2;
                    if(vertmatch != -1 && comparevertmatch != -1){
                        if(VertMatches.at(0) && VertMatches.at(1)){
                            if(face.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f<<") for edge 0...\n";
                            else face.nAdjacentFaces[0] = f2*3 + comparevertmatch;
                        }
                        else if(VertMatches.at(1) && VertMatches.at(2)){
                            if(face.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f<<") for edge 1...\n";
                            else face.nAdjacentFaces[1] = f2*3 + comparevertmatch;
                        }
                        else if(VertMatches.at(2) && VertMatches.at(0)){
                            if(face.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f<<") for edge 2...\n";
                            else face.nAdjacentFaces[2] = f2*3 + comparevertmatch;
                        }
                        if(VertMatchesCompare.at(0) && VertMatchesCompare.at(1)){
                            if(compareface.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f2<<") for edge 0...\n";
                            else compareface.nAdjacentFaces[0] = f*3 + vertmatch;
                        }
                        else if(VertMatchesCompare.at(1) && VertMatchesCompare.at(2)){
                            if(compareface.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f2<<") for edge 1...\n";
                            else compareface.nAdjacentFaces[1] = f*3 + vertmatch;
                        }
                        else if(VertMatchesCompare.at(2) && VertMatchesCompare.at(0)){
                            if(compareface.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent edges (to "<<f2<<") for edge 2...\n";
                            else compareface.nAdjacentFaces[2] = f*3 + vertmatch;
                        }
                    }
                    if(face.nAdjacentFaces[0]!=-1 &&
                       face.nAdjacentFaces[1]!=-1 &&
                       face.nAdjacentFaces[2]!=-1 ){
                        f2 = Data.faces.size(); //Found them all, maybe I finish early?
                    }
                }
            }
        }
    }

    //First, find the first -1
    int nFace = -1;
    int nEdge = -1;
    int nAdjacent = -1;
    for(int f = 0; f < Data.faces.size() && nFace == -1; f++){
        for(int i = 0; i < 3 && nEdge == -1; i++){
            if(Data.faces.at(f).nAdjacentFaces.at(i) == -1 && !Data.faces.at(f).bProcessed.at(i) && Data.faces.at(f).nMaterialID != 7){
                //std::cout<<"Found starting point at face "<<f<<", edge "<<i<<".\n";
                nFace = f;
                nEdge = i;
                nAdjacent = -1;
                while(nFace != -1 && nEdge != -1){
                    //std::cout<<"Looping through face "<<nFace<<" and edge "<<nEdge<<".\n";
                    nAdjacent = Data.faces.at(nFace).nAdjacentFaces.at(nEdge);
                    if(nAdjacent == -1){
                        if(!Data.faces.at(nFace).bProcessed.at(nEdge)){
                            Data.edges.push_back(Edge(3*nFace + nEdge, Data.faces.at(nFace).nEdgeTransitions.at(nEdge)));
                            Data.faces.at(nFace).bProcessed.at(nEdge) = true;
                            nEdge = (nEdge+1) % 3;
                        }
                        else{
                            nFace = -1;
                            nEdge = -1;
                            Data.perimeters.push_back(Data.edges.size());
                        }
                    }
                    else{
                        nFace = nAdjacent / 3;
                        nEdge = nAdjacent % 3;
                        nEdge = (nEdge+1) % 3;
                    }
                }
            }
        }
    }
    //std::cout<<"Done with edge loops.\n";

    for(int f = 0; f < Data.faces.size(); f++){
        Face & face = Data.faces.at(f);
        for(int i = 0; i < 3; i++){
            face.bProcessed.at(i) = false;
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
                vert += vLytPos;
                Data.verts.push_back(std::move(vert));
            }
        }
        face.vBBmax = Vector(-10000.0, -10000.0, -10000.0);
        face.vBBmin = Vector(10000.0, 10000.0, 10000.0);
        face.vCentroid = Vector(0.0, 0.0, 0.0);
        for(int i = 0; i < 3; i++){
            face.vBBmax.fX = std::max(face.vBBmax.fX, Data.verts.at(face.nIndexVertex[i]).fX);
            face.vBBmax.fY = std::max(face.vBBmax.fY, Data.verts.at(face.nIndexVertex[i]).fY);
            face.vBBmax.fZ = std::max(face.vBBmax.fZ, Data.verts.at(face.nIndexVertex[i]).fZ);
            face.vBBmin.fX = std::min(face.vBBmin.fX, Data.verts.at(face.nIndexVertex[i]).fX);
            face.vBBmin.fY = std::min(face.vBBmin.fY, Data.verts.at(face.nIndexVertex[i]).fY);
            face.vBBmin.fZ = std::min(face.vBBmin.fZ, Data.verts.at(face.nIndexVertex[i]).fZ);
            face.vCentroid += Data.verts.at(face.nIndexVertex[i]);
        }
        face.vCentroid /= 3.0;
        face.vBBmax += Vector(0.01, 0.01, 0.01);
        face.vBBmin -= Vector(0.01, 0.01, 0.01);
    }

    //Create aabb tree
    std::vector<Face*> allfaces;
    for(int f = 0; f < Data.faces.size(); f++){
        allfaces.push_back(&Data.faces.at(f));
    }
    Aabb rootaabb;
    BuildAabb(rootaabb, allfaces, ptrssFile);
    LinearizeAabb(rootaabb, Data.aabb);
}


struct FaceSort{
    Face * p_face = nullptr;
    double centroid = 0.0;
    double maxdiff = 0.0;
    double fMax = 0.0;
    double fMin = 0.0;
    bool operator<(const FaceSort & facesort){
        if(centroid == facesort.centroid && p_face != nullptr && facesort.p_face != nullptr){
            if(RoundDec(p_face->fDistance, 4) != RoundDec(facesort.p_face->fDistance, 4)){
                return (abs(p_face->fDistance) < abs(facesort.p_face->fDistance));
            }
            /*
            else{
                return (p_face->nID < facesort.p_face->nID);
            }
            */
        }
        return (centroid < facesort.centroid);
    }
    bool operator==(const FaceSort & facesort){
        return (centroid == facesort.centroid);
    }
};

struct AxisSort{
    std::string sName;
    double fSort;
    bool operator<(const AxisSort & axissort){
        return (fSort < axissort.fSort);
    }
    AxisSort(std::string sName = "", double fSort = 0.0): sName(sName), fSort(fSort) {}
};

struct AxisSort2{
    std::string sName;
    int nSize;
    bool operator<(const AxisSort2 & axissort){
        if(nSize == axissort.nSize){
            if(sName == "Z" && (axissort.sName == "Y" || axissort.sName == "X")) return true;
            if(sName == "Y" &&  axissort.sName == "X") return true;
        }
        return (nSize < axissort.nSize);
    }
    AxisSort2(std::string sName = "", int nSize = 0): sName(sName), nSize(nSize) {}
};

void BuildAabb(Aabb & aabb, const std::vector<Face*> & faces, std::stringstream * file){
    if(file != nullptr) (*file).precision(5);
    if(faces.size() == 1){
        //This is the leaf
        Face & face = *faces.front();
        aabb.nID = face.nID;
        aabb.vBBmax = face.vBBmax;
        aabb.vBBmin = face.vBBmin;
        aabb.nProperty = 0;
        aabb.nChild1 = 0;
        aabb.nChild2 = 0;
        if(file != nullptr) *file<<"Wrote leaf: "<<aabb.nID<<"\n";
    }
    else{
        if(file != nullptr) *file<<"Processing non-leaf, faces: "<<faces.size()<<"\n";
        aabb.nID = -1;
        aabb.vBBmax = Vector(-10000.0, -10000.0, -10000.0);
        aabb.vBBmin = Vector(10000.0, 10000.0, 10000.0);
        Vector vAxisBBmax = Vector(-10000.0, -10000.0, -10000.0);
        Vector vAxisBBmin = Vector(10000.0, 10000.0, 10000.0);
        Vector vAverage;
        Vector vAverageBB;
        for(int f = 0; f < faces.size(); f++){
            Face & face = *faces.at(f);
            aabb.vBBmax.fX = std::max(aabb.vBBmax.fX, face.vBBmax.fX);
            aabb.vBBmax.fY = std::max(aabb.vBBmax.fY, face.vBBmax.fY);
            aabb.vBBmax.fZ = std::max(aabb.vBBmax.fZ, face.vBBmax.fZ);
            aabb.vBBmin.fX = std::min(aabb.vBBmin.fX, face.vBBmin.fX);
            aabb.vBBmin.fY = std::min(aabb.vBBmin.fY, face.vBBmin.fY);
            aabb.vBBmin.fZ = std::min(aabb.vBBmin.fZ, face.vBBmin.fZ);
            vAxisBBmax.fX = std::max(vAxisBBmax.fX, face.vCentroid.fX);
            vAxisBBmax.fY = std::max(vAxisBBmax.fY, face.vCentroid.fY);
            vAxisBBmax.fZ = std::max(vAxisBBmax.fZ, face.vCentroid.fZ);
            vAxisBBmin.fX = std::min(vAxisBBmin.fX, face.vCentroid.fX);
            vAxisBBmin.fY = std::min(vAxisBBmin.fY, face.vCentroid.fY);
            vAxisBBmin.fZ = std::min(vAxisBBmin.fZ, face.vCentroid.fZ);
            vAverageBB.fX += aabb.vBBmin.fX + (aabb.vBBmax.fX - aabb.vBBmin.fX)/2.0;
            vAverageBB.fY += aabb.vBBmin.fY + (aabb.vBBmax.fY - aabb.vBBmin.fY)/2.0;
            vAverageBB.fZ += aabb.vBBmin.fZ + (aabb.vBBmax.fZ - aabb.vBBmin.fZ)/2.0;
            vAverage += face.vCentroid;
        }
        vAverage /= faces.size();
        vAverageBB /= faces.size();
        aabb.vBBmax += Vector(0.0001, 0.0001, 0.0001);
        if(file != nullptr) *file<<"Bounding box: "<<aabb.vBBmin.fX<<", "<<aabb.vBBmin.fY<<", "<<aabb.vBBmin.fZ<<", "<<aabb.vBBmax.fX<<", "<<aabb.vBBmax.fY<<", "<<aabb.vBBmax.fZ<<"\n";

        std::vector<AxisSort> axispriority;
        axispriority.push_back(AxisSort("X", aabb.vBBmax.fX - aabb.vBBmin.fX));
        axispriority.push_back(AxisSort("Y", aabb.vBBmax.fY - aabb.vBBmin.fY));
        axispriority.push_back(AxisSort("Z", aabb.vBBmax.fZ - aabb.vBBmin.fZ));
        sort(axispriority.begin(), axispriority.end());
        std::reverse(axispriority.begin(), axispriority.end());
        if(file != nullptr) *file<<"Priority List: "<<axispriority.at(0).sName<<": "<<axispriority.at(0).fSort<<", "<<axispriority.at(1).sName<<": "<<axispriority.at(1).fSort<<", "<<axispriority.at(2).sName<<": "<<axispriority.at(2).fSort<<"\n";
        int nCurrentPriority = 0;
        std::vector<double> vectorX;
        std::vector<double> vectorY;
        std::vector<double> vectorZ;
        for(int f = 0; f < faces.size(); f++){
            Face & face = *faces.at(f);
            if(std::find(vectorX.begin(), vectorX.end(), face.vCentroid.fX/* + (face.vBBmax.fX - face.vBBmin.fX)/2/**/) == vectorX.end()) vectorX.push_back(face.vCentroid.fX /*+ (face.vBBmax.fX - face.vBBmin.fX)/2/**/);
            if(std::find(vectorY.begin(), vectorY.end(), face.vCentroid.fY/* + (face.vBBmax.fY - face.vBBmin.fY)/2/**/) == vectorY.end()) vectorY.push_back(face.vCentroid.fY /*+ (face.vBBmax.fY - face.vBBmin.fY)/2/**/);
            if(std::find(vectorZ.begin(), vectorZ.end(), face.vCentroid.fZ/* + (face.vBBmax.fZ - face.vBBmin.fZ)/2/**/) == vectorZ.end()) vectorZ.push_back(face.vCentroid.fZ /*+ (face.vBBmax.fZ - face.vBBmin.fZ)/2/**/);
        }
        std::vector<AxisSort2> axispriority2;
        axispriority2.push_back(AxisSort2("X", vectorX.size()));
        axispriority2.push_back(AxisSort2("Y", vectorY.size()));
        axispriority2.push_back(AxisSort2("Z", vectorZ.size()));
        sort(axispriority2.begin(), axispriority2.end());
        std::reverse(axispriority2.begin(), axispriority2.end());
        if(file != nullptr) *file<<"Priority List 2: "<<axispriority2.at(0).sName<<": "<<axispriority2.at(0).nSize<<", "<<axispriority2.at(1).sName<<": "<<axispriority2.at(1).nSize<<", "<<axispriority2.at(2).sName<<": "<<axispriority2.at(2).nSize<<"\n";
        /*if(faces.size()%2 == 0){
            axispriority.at(0).sName = axispriority2.at(0).sName;
            axispriority.at(1).sName = axispriority2.at(1).sName;
            axispriority.at(2).sName = axispriority2.at(2).sName;
        }*/
        std::vector<AxisSort> axispriority3;
        axispriority3.push_back(AxisSort("X", vAxisBBmax.fX - vAxisBBmin.fX));
        axispriority3.push_back(AxisSort("Y", vAxisBBmax.fY - vAxisBBmin.fY));
        axispriority3.push_back(AxisSort("Z", vAxisBBmax.fZ - vAxisBBmin.fZ));
        sort(axispriority3.begin(), axispriority3.end());
        std::reverse(axispriority3.begin(), axispriority3.end());
        Vector vDeviance;
        Vector vDeviance2;
        if(file != nullptr) *file<<"Priority List 3: "<<axispriority3.at(0).sName<<": "<<axispriority3.at(0).fSort<<", "<<axispriority3.at(1).sName<<": "<<axispriority3.at(1).fSort<<", "<<axispriority3.at(2).sName<<": "<<axispriority3.at(2).fSort<<"\n";
        for(int f = 0; f < faces.size(); f++){
            Face & face = *faces.at(f);
            vDeviance += face.vCentroid - vAverage;
            vDeviance2.fX += (face.vBBmin.fX + (face.vBBmax.fX - face.vBBmin.fX)/2.0 - vAverageBB.fX);
            vDeviance2.fY += (face.vBBmin.fY + (face.vBBmax.fY - face.vBBmin.fY)/2.0 - vAverageBB.fY);
            vDeviance2.fZ += (face.vBBmin.fZ + (face.vBBmax.fZ - face.vBBmin.fZ)/2.0 - vAverageBB.fZ);
        }
        vDeviance /= (faces.size() );
        vDeviance2 /= (faces.size() );
        std::vector<AxisSort> axispriority4;
        axispriority4.push_back(AxisSort("X", vDeviance.fX));
        axispriority4.push_back(AxisSort("Y", vDeviance.fY));
        axispriority4.push_back(AxisSort("Z", vDeviance.fZ));
        sort(axispriority4.begin(), axispriority4.end());
        if(file != nullptr) *file<<"Priority List 4: "<<axispriority4.at(0).sName<<": "<<axispriority4.at(0).fSort<<", "<<axispriority4.at(1).sName<<": "<<axispriority4.at(1).fSort<<", "<<axispriority4.at(2).sName<<": "<<axispriority4.at(2).fSort<<"\n";

        std::vector<AxisSort> axispriority5;
        axispriority5.push_back(AxisSort("X", vDeviance2.fX));
        axispriority5.push_back(AxisSort("Y", vDeviance2.fY));
        axispriority5.push_back(AxisSort("Z", vDeviance2.fZ));
        sort(axispriority5.begin(), axispriority5.end());
        if(file != nullptr) *file<<"Priority List 5: "<<axispriority5.at(0).sName<<": "<<axispriority5.at(0).fSort<<", "<<axispriority5.at(1).sName<<": "<<axispriority5.at(1).fSort<<", "<<axispriority5.at(2).sName<<": "<<axispriority5.at(2).fSort<<"\n";

        //axispriority.at(0).sName = axispriority4.at(0).sName;
        //axispriority.at(1).sName = axispriority4.at(1).sName;
        //axispriority.at(2).sName = axispriority4.at(2).sName;

        std::vector<Face*> half1;
        std::vector<Face*> half2;
        std::vector<FaceSort> centroids;

        bool bNegative = false;
        bool bEven;
        int nIndex;
        int nTry = 0;
        bool bOk = false;
        std::stringstream ssFaces;
        while(nTry < 7 && !bOk){
            bOk = true;

            //Clear
            centroids.resize(0);
            ssFaces.str(std::string());

            //Fill centroids
            int nNegative = 0;
            for(int f = 0; f < faces.size(); f++){
                Face & face = *faces.at(f);
                FaceSort newfs;
                newfs.p_face = &face;
                double fMax = 0.0, fMin = 0.0, fAverage = 0.0;

                if(axispriority.at(nCurrentPriority).sName == "X"){
                    newfs.fMin = face.vBBmin.fX;
                    newfs.fMax = face.vBBmax.fX;
                    newfs.centroid = face.vCentroid.fX; //+ (face.vBBmax.fX - face.vBBmin.fX) / 2;
                    //newfs.centroid = /**/face.vBBmin.fX +/**/ (face.vBBmax.fX - face.vBBmin.fX)/2;
                    newfs.maxdiff = /**/face.vBBmin.fX +/**/ (face.vBBmax.fX - face.vBBmin.fX)/2;
                    fMax = aabb.vBBmax.fX;
                    fMin = aabb.vBBmin.fX;
                    fAverage = vAverage.fX;
                }
                else if(axispriority.at(nCurrentPriority).sName == "Y"){
                    newfs.fMin = face.vBBmin.fY;
                    newfs.fMax = face.vBBmax.fY;
                    newfs.centroid = face.vCentroid.fY; //+ (face.vBBmax.fY - face.vBBmin.fY) / 2;
                    //newfs.centroid = /**/face.vBBmin.fY +/**/ (face.vBBmax.fY - face.vBBmin.fY)/2;
                    newfs.maxdiff = /**/face.vBBmin.fY +/**/ (face.vBBmax.fY - face.vBBmin.fY)/2;
                    fMax = aabb.vBBmax.fY;
                    fMin = aabb.vBBmin.fY;
                    fAverage = vAverage.fY;
                }
                else if(axispriority.at(nCurrentPriority).sName == "Z"){
                    newfs.fMin = face.vBBmin.fZ;
                    newfs.fMax = face.vBBmax.fZ;
                    newfs.centroid = face.vCentroid.fZ; //+ (face.vBBmax.fZ - face.vBBmin.fZ) / 2;
                    //newfs.centroid = /**/face.vBBmin.fZ +/**/ (face.vBBmax.fZ - face.vBBmin.fZ)/2;
                    newfs.maxdiff = /**/face.vBBmin.fZ +/**/ (face.vBBmax.fZ - face.vBBmin.fZ)/2;
                    fMax = aabb.vBBmax.fZ;
                    fMin = aabb.vBBmin.fZ;
                    fAverage = vAverage.fZ;
                }
                else{
                    Error("Aabb significant plane not assigned!");
                    return;
                }
                //nNegative += (newfs.centroid < fMin + (fMax - fMin)/2)? -1 : 1;
                //nNegative += (newfs.centroid < fAverage)? -1 : 1;
                //nNegative += (newfs.maxdiff < fMin + (fMax - fMin)/2)? -1 : 1;
                nNegative += (newfs.maxdiff < fAverage)? -1 : 1;
                //nNegative += (newfs.fMin < fMin + (fMax - fMin)/2)? -1 : 1;
                //nNegative += (newfs.fMin < fAverage)? -1 : 1;
                centroids.push_back(std::move(newfs));
                //ssFaces<<"  "<<centroids.back().centroid<<" ("<<fMin<<" -> "<<fMax<<") - face "<<face.nID<<"\n";
            }
            //if(nNegative < 0) bNegative = true;
            //if(centroids.size() == 0) Error("Major error, centroid size 0!!");
            sort(centroids.begin(), centroids.end());
            bEven = (centroids.size() % 2 == 0);
            /*
            if(centroids.size() >= 2 && bEven){
                *file << centroids.at(centroids.size() / 2 - 1).fMin << " > " << centroids.at(centroids.size() / 2).fMin << "\n";
                if(!(centroids.at(centroids.size() / 2 - 1).fMin > centroids.at(centroids.size() / 2).fMin)){
                    bNegative = true;
                }
                else bNegative = false;
            }
            else bNegative = false;*/
            if(bNegative) std::reverse(centroids.begin(), centroids.end());

            if(bEven) nIndex = centroids.size() / 2 - 1;
            else nIndex = centroids.size() / 2;

            if(file != nullptr){
                *file << "Priority: "<<(bNegative? "-" : "+")<<axispriority.at(nCurrentPriority).sName<<"\n";
                *file<<"Faces:\n";
                for(int c = 0; c < centroids.size(); c++){
                    ssFaces<<"  "<<centroids.at(c).centroid<<", "<<centroids.at(c).fMin + (centroids.at(c).fMax - centroids.at(c).fMin)/2.0;
                    ssFaces<<" ("<<centroids.at(c).fMin<<" -> "<<centroids.at(c).fMax<<") - face "<<centroids.at(c).p_face->nID<<"\n";
                }
                *file << ssFaces.str();
            }

            bool bTryAxis = false;
            if(bTryAxis){
                if(nTry < 5){
                    if(!bEven){
                        /*
                        if(!bNegative && centroids.size() > 2){
                            if(centroids.at(nIndex).maxdiff == centroids.at(nIndex-1).maxdiff){
                                bNegative = true;
                                if(file != nullptr) *file << "To negative\n";
                                bOk = false;
                            }
                        }
                        else*/ if(centroids.size() > 2){
                            if(centroids.at(nIndex).centroid == centroids.at(nIndex-1).centroid){
                                bNegative = false;
                                if(file != nullptr) *file << "Increase priority\n";
                                if(nCurrentPriority < 2) nCurrentPriority++;
                                bOk = false;
                            }
                        }
                    }
                    else{
                        /*
                        if(!bNegative && centroids.size() > 2){
                            if(//centroids.at(nIndex).maxdiff == centroids.at(nIndex+1).maxdiff ||
                               centroids.at(nIndex).maxdiff == centroids.at(nIndex-1).maxdiff){
                                bNegative = true;
                                if(file != nullptr) *file << "To negative\n";
                                bOk = false;
                            }
                        }
                        else*/ if(centroids.at(nIndex).maxdiff == centroids.at(nIndex+1).maxdiff && centroids.size() > 2){
                            bNegative = false;
                            if(file != nullptr) *file << "Increase priority\n";
                            if(nCurrentPriority < 2) nCurrentPriority++;
                            bOk = false;
                        }
                    }
                }
                else{
                    nCurrentPriority = 0;
                    bNegative = false;
                    bOk = false;
                    if(file != nullptr) *file << "Resetting\n";
                }
            }
            nTry++;
        }

        double fMedian = centroids.at(nIndex).centroid;
        if(bEven){
            nIndex++;
            fMedian = (fMedian + centroids.at(nIndex).centroid) / 2;
        }
        if(file != nullptr){
            *file<<"Median: "<<fMedian<<" (";
            if(bEven) *file<<centroids.at(nIndex-1).p_face->nID<<", ";
            *file<<centroids.at(nIndex).p_face->nID;
            *file<<")\n";
        }

        if(axispriority.at(nCurrentPriority).sName == "X" && !bNegative) aabb.nProperty = AABB_POSITIVE_X;
        else if(axispriority.at(nCurrentPriority).sName == "X" && bNegative) aabb.nProperty = AABB_NEGATIVE_X;
        else if(axispriority.at(nCurrentPriority).sName == "Y" && !bNegative) aabb.nProperty = AABB_POSITIVE_Y;
        else if(axispriority.at(nCurrentPriority).sName == "Y" && bNegative) aabb.nProperty = AABB_NEGATIVE_Y;
        else if(axispriority.at(nCurrentPriority).sName == "Z" && !bNegative) aabb.nProperty = AABB_POSITIVE_Z;
        else if(axispriority.at(nCurrentPriority).sName == "Z" && bNegative) aabb.nProperty = AABB_NEGATIVE_Z;
        aabb.nProperty = 0;

        for(int c = 0; c < centroids.size(); c++){
            if(c < nIndex) half1.push_back(centroids.at(c).p_face);
            else half2.push_back(centroids.at(c).p_face);
        }
        centroids.resize(0);
        centroids.shrink_to_fit();

        if(!half1.empty() && !half2.empty()){
            aabb.Child1.resize(1);
            BuildAabb(aabb.Child1.front(), half1, file);
            aabb.Child2.resize(1);
            BuildAabb(aabb.Child2.front(), half2, file);
        }
        else{
            if(file != nullptr) *file<<"ERROR: One of the halves is empty!\n";
            Error("AABB tree: One of the halves is empty!");
        }
    }
}

void LinearizeAabb(Aabb & aabbroot, std::vector<Aabb> & aabbarray){
    unsigned int nIndex = aabbarray.size();
    aabbroot.nExtra = 4;
    aabbarray.push_back(std::move(aabbroot));

    if(aabbarray.at(nIndex).Child1.size() > 0){
        aabbarray.at(nIndex).nChild1 = aabbarray.size();
        LinearizeAabb(aabbarray.at(nIndex).Child1.front(), aabbarray);
    }
    else aabbarray.at(nIndex).nChild1 = -1;
    if(aabbarray.at(nIndex).Child2.size() > 0){
        aabbarray.at(nIndex).nChild2 = aabbarray.size();
        LinearizeAabb(aabbarray.at(nIndex).Child2.front(), aabbarray);
    }
    else aabbarray.at(nIndex).nChild2 = -1;
}

void MDL::BwmAsciiPostProcess(BWMHeader & data, std::vector<Vector> & vertices, bool bDwk){
    data.nType = 0;
    if(bDwk){
        data.vDwk1 = data.vUse1 + data.vPosition;
        data.vDwk2 = data.vUse2 + data.vPosition;
    }

    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        for(int i = 0; i < 3; i++){
            if(!face.bProcessed[i]){
                Vector vert;

                if(vertices.size() > 0)
                    vert = vertices.at(face.nIndexVertex[i]);

                //Find identical verts
                for(int f2 = f; f2 < data.faces.size(); f2++){
                    Face & face2 = data.faces.at(f2);
                    for(int i2 = 0; i2 < 3; i2++){
                        //Make sure that we're only changing what's past our current position if we are in the same face.
                        if(f2 != f || i2 > i){
                            if( (face2.nIndexVertex[i2] == face.nIndexVertex[i]) &&
                                !face2.bProcessed[i2] )
                            {
                                face2.nIndexVertex[i2] = data.verts.size();
                                face2.bProcessed[i2] = true;
                            }
                        }
                    }
                }

                //Now we're allowed to link the original vert as well
                face.nIndexVertex[i] = data.verts.size();
                face.bProcessed[i] = true;

                //Put the new vert into the array
                data.verts.push_back(std::move(vert));
            }
        }

        Vector & v1 = data.verts.at(face.nIndexVertex[0]);
        Vector & v2 = data.verts.at(face.nIndexVertex[1]);
        Vector & v3 = data.verts.at(face.nIndexVertex[2]);
        Vector Edge1 = v2 - v1;
        Vector Edge2 = v3 - v1;
        Vector Edge3 = v3 - v2;

        //This is for face normal
        face.vNormal = cross(Edge1, Edge2); //Cross product, unnormalized
        face.vNormal.Normalize();

        //This is for the distance.
        face.fDistance = - (face.vNormal.fX * v1.fX +
                            face.vNormal.fY * v1.fY +
                            face.vNormal.fZ * v1.fZ);
    }
}
bool ASCII::ReadWalkmesh(MDL & Mdl, bool bPwk){
	if(bPwk){
		Mdl.Pwk.reset(new PWK);
		std::string sPwk = GetFilename();
		Mdl.Pwk->SetFilePath(sPwk);
		Mdl.Pwk->GetData().reset(new BWMHeader);
	}
	else{
		Mdl.Dwk0.reset(new DWK);
		Mdl.Dwk1.reset(new DWK);
		Mdl.Dwk2.reset(new DWK);
		std::string sDwk;
		sDwk =  GetFilename()+" (closed)";
		Mdl.Dwk0->SetFilePath(sDwk);
		sDwk =  GetFilename()+" (open1)";
		Mdl.Dwk1->SetFilePath(sDwk);
		sDwk =  GetFilename()+" (open2)";
		Mdl.Dwk2->SetFilePath(sDwk);
		Mdl.Dwk0->GetData().reset(new BWMHeader);
		Mdl.Dwk1->GetData().reset(new BWMHeader);
		Mdl.Dwk2->GetData().reset(new BWMHeader);
	}

    std::string sID;
    nPosition = 0;

    int nConvert;
    double fConvert;
    bool bError = false;
    bool bFound = false;
    bool bVerts = false;
    bool bFaces = false;
    bool * lpbList = nullptr;
    int nNode = 0;
    int nDataMax = -1;
    int nDataCounter = -1;
    BWMHeader * DATA = nullptr;
    std::vector<Vector> * TempVerts = nullptr;
    std::vector<Vector> TempVerts0;
    std::vector<Vector> TempVerts1;
    std::vector<Vector> TempVerts2;
    std::string sNodeName;

    while(nPosition < sBuffer.size() && !bError){
        //First set our current list
        if(bVerts) lpbList = &bVerts;
        else if(bFaces) lpbList = &bFaces;
        else lpbList = nullptr;

        //First, check if we have a blank line, we'll just skip it here.
        if(EmptyRow()){
            SkipLine();
        }
        //Second, check if we are in some list of data. We should not look for a keyword then.
        else if(lpbList != nullptr && DATA != nullptr){
            //First, check if we're done. Save your position, cuz we'll need to revert if we're not done
            int nSavePos = nPosition;
            ReadUntilText(sID);
            if(sID == "endlist" || nDataMax == 0){
                *lpbList = false;
                SkipLine();
            }
            else{
                //Revert back to old position
                nPosition = nSavePos;

                /// Read the data
                if(bVerts && TempVerts != nullptr){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading verts data "<<nDataCounter<<".\n";
                    bFound = true;
                    Vector vert;
                    if(ReadFloat(fConvert)) vert.fX = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vert.fY = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vert.fZ = fConvert;
                    else bFound = false;
                    if(bFound){
                        TempVerts->push_back(std::move(vert));
                    }
                    else bError = true;
                }
                else if(bFaces){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading faces data"<<""<<".\n";
                    bFound = true;
                    Face face;
                    //std::cout << "Reading walk face.\n";

                    //Currently we read the regular NWMax version with only a single set of tvert indices
                    if(ReadInt(nConvert)) face.nIndexVertex[0] = nConvert;
                    else bFound = false;
                    if(ReadInt(nConvert)) face.nIndexVertex[1] = nConvert;
                    else bFound = false;
                    if(ReadInt(nConvert)) face.nIndexVertex[2] = nConvert;
                    else bFound = false;
                    if(!ReadInt(nConvert)) bFound = false;
                    if(!ReadInt(nConvert)) bFound = false;
                    if(!ReadInt(nConvert)) bFound = false;
                    if(!ReadInt(nConvert)) bFound = false;
                    if(ReadInt(nConvert)) face.nMaterialID = nConvert;
                    else bFound = false;

                    if(bFound){
                        //std::cout << "Added walk face to faces.\n";
                        DATA->faces.push_back(std::move(face));
                    }
                    else bError = true;
                }

                nDataCounter++;
                if(nDataCounter >= nDataMax && nDataMax != -1){
                    *lpbList = false;
                }
                SkipLine();
            }
        }
        //So no data. Find the next keyword then.
        else{
            //std::cout<<"No data to read. Read keyword instead"<<""<<".\n";
            bFound = ReadUntilText(sID);
            if(!bFound) SkipLine(); //This will have already been done above, no need to look for it again
            else{
                /// Common case for nodes
                if(sID == "node" && nNode == 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";

                    //Read type
                    int nType;
                    bFound = ReadUntilText(sID); //Get type
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: a node is without any other specification.\n";
                    }
                    if(sID == "dummy") nType = NODE_HAS_HEADER;
                    else if(sID == "trimesh") nType = NODE_HAS_HEADER | NODE_HAS_MESH;
                    else if(sID == "aabb") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_AABB;
                    else if(bFound){
                        std::cout<<"ReadUntilText() has found some text (type?) that we do not support: "<<sID<<"\n";
                        bError = true;
                    }

                    //Read name
                    bFound = ReadUntilText(sID, false); //Name
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: a node is without a name.\n";
                    }
                    else{
                        sNodeName = sID;
                        //std::cout << "Reading "<< sNodeName <<".\n";
						if(bPwk){
							DATA = Mdl.Pwk->GetData().get();
                            TempVerts = &TempVerts0;
						}
                        else if(sNodeName.find("closed") != std::string::npos){
                            DATA = Mdl.Dwk0->GetData().get();
                            TempVerts = &TempVerts0;
                        }
                        else if(sNodeName.find("open1") != std::string::npos){
                            DATA = Mdl.Dwk1->GetData().get();
                            TempVerts = &TempVerts1;
                        }
                        else if(sNodeName.find("open2") != std::string::npos){
                            DATA = Mdl.Dwk2->GetData().get();
                            TempVerts = &TempVerts2;
                        }
                    }

                    //Finish up
                    nNode = nType;
                    if(!bFound) bError;
                    SkipLine();
                }
                /// Next we have the DATA LISTS
                else if(sID == "verts" && nNode & NODE_HAS_MESH && (sNodeName.find("_wg") != std::string::npos)){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    //std::cout << "Reading verts.\n";
                    bVerts = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "faces" && nNode & NODE_HAS_MESH && (sNodeName.find("_wg") != std::string::npos)){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    //std::cout << "Reading faces.\n";
                    bFaces = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "position" && nNode && DATA != nullptr){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    double fX, fY, fZ;
                    if(ReadFloat(fConvert)) fX = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) fY = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) fZ = fConvert;
                    else bError = true;

                    if(sNodeName.find("pwk_use01") != std::string::npos || sNodeName.find("pwk_dp_use_01") != std::string::npos || sNodeName.find("DWK_dp_closed_01") != std::string::npos || sNodeName.find("DWK_dp_open1_01") != std::string::npos || sNodeName.find("DWK_dp_open2_01") != std::string::npos){
                        DATA->vUse1 = Vector(fX, fY, fZ);
                    }
                    else if(sNodeName.find("pwk_use02") != std::string::npos || sNodeName.find("pwk_dp_use_02") != std::string::npos || sNodeName.find("DWK_dp_closed_02") != std::string::npos || sNodeName.find("DWK_dp_open1_02") != std::string::npos || sNodeName.find("DWK_dp_open2_02") != std::string::npos){
                        DATA->vUse2 = Vector(fX, fY, fZ);
                    }
                    else if(sNodeName.find("_wg") != std::string::npos){
                        DATA->vPosition = Vector(fX, fY, fZ);
                    }

                    SkipLine();
                }
                /// General ending tokens
                else if(sID == "endnode" && nNode > 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    nNode = 0;
                    DATA = nullptr;
                    TempVerts = nullptr;
                    SkipLine();
                }
                //These are the names we should ignore, because we expect them to appear, but we have no use for them
                else if(sID == "multimaterial"){
                    if(ReadInt(nConvert)){
                        SkipLine();
                        for(int nj = 0; nj < nConvert; nj++){
                            SkipLine();
                        }
                    }
                    else SkipLine();
                }
                else if(sID == "parent") SkipLine();
                else if(sID == "orientation") SkipLine();
                else if(sID == "bitmap") SkipLine();
                else if(sID == "filedependancy") SkipLine();
                else if(sID == "specular") SkipLine();
                else if(sID == "wirecolor") SkipLine();
                else if(sID == "shininess") SkipLine();
                else if(sID == "name") SkipLine();
                else if(sID == "inheritcolor") SkipLine();
                else if(sID == "tilefade") SkipLine();
                else if(sID == "center") SkipLine();
                else{
                    std::cout<<"ReadUntilText() has found some text that we cannot interpret: "<<sID<<"\n";
                    SkipLine();
                }
            }
        }
    }
    std::cout<<"Done reading walkmesh ascii, checking for errors...\n";
    if(bError){
        Error("Some kind of error has occured! Check the console! The program will now cleanup what it has read since the data is now broken.");
        return false;
    }

	if(bPwk){
		Mdl.BwmAsciiPostProcess(*Mdl.Pwk->GetData(), TempVerts0, false);
	}
	else{
		Mdl.BwmAsciiPostProcess(*Mdl.Dwk0->GetData(), TempVerts0);
		Mdl.BwmAsciiPostProcess(*Mdl.Dwk1->GetData(), TempVerts1);
		Mdl.BwmAsciiPostProcess(*Mdl.Dwk2->GetData(), TempVerts2);
	}

    return true;
}
