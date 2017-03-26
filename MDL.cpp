#include "MDL.h"
#include <Shlwapi.h>
#include <fstream>
/// This file should be a general initializer/implementor of MDL.h

void File::SetFilePath(std::string & sPath){
    sFullPath = sPath;
    sFile = sPath;    PathStripPath(&sFile.front());
    sFile.resize(strlen(sFile.c_str()));
}


std::string & Node::GetName(){
    return Model.FH.at(0).MH.Names.at(Head.nNameIndex).sName;
}

Location Node::GetLocation(){
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

const std::string MDL::sName = "MDL";
const std::string MDX::sName = "MDX";
const std::string WOK::sName = "WOK";

const char QU_X = 0x01;
const char QU_Y = 0x02;
const char QU_Z = 0x03;
const char QU_W = 0x04;
const char AA_X = 0x05;
const char AA_Y = 0x06;
const char AA_Z = 0x07;
const char AA_A = 0x08;

Vector operator*(Vector v, const Matrix22 & m){
    v *= m;
    return v;
}

Vector operator*(Vector v, const double & f){
    v *= f;
    return v;
}

Vector operator/(Vector v, const double & f){
    v /= f;
    return v;
}

Vector operator*(const double & f, Vector v){
    v *= f;
    return v;
}

double operator*(const Vector & v, const Vector & v2){ //dot product
    return (v.fX * v2.fX + v.fY * v2.fY + v.fZ * v2.fZ);
}

double Angle(const Vector & v, const Vector & v2){
    return acos((v*v2)/(v.GetLength() * v2.GetLength()));
}

Vector operator/(Vector v, const Vector & v2){ //cross product
    v /= v2;
    return v;
}

Vector operator-(Vector v, const Vector & v2){
    v -= v2;
    return v;
}

Vector operator+(Vector v, const Vector & v2){
    v += v2;
    return v;
}

Orientation operator*(Orientation o1, const Orientation & o2){
    o1 *= o2;
    return o1;
}

double HeronFormula(const Vector & e1, const Vector & e2, const Vector & e3){
    double fA = e1.GetLength();
    double fB = e2.GetLength();
    double fC = e3.GetLength();
    double fS = (fA + fB + fC) / 2.0;
    return sqrt(fS * (fS - fA) * (fS - fB) * (fS - fC));
}

/// This function is to be used both when compiling and decompiling (to determine smoothing groups)
void MDL::CreatePatches(bool bPrint, std::ofstream & file){
    if(!file.is_open()) bPrint = false;
    FileHeader & Data = FH[0];

    SendMessage(hProgress, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, Data.MH.ArrayOfNodes.size() - 1));
    SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);
    std::cout<<"Building LinkedFaces array... (this may take a while)\n";
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        std::cout<<"Linking faces for node "<<n+1<<"/"<<Data.MH.ArrayOfNodes.size()<<".\n";
        //Currently, this takes all meshes, including skins, danglymeshes, walkmeshes and sabers
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_MESH){
            Node & node = Data.MH.ArrayOfNodes.at(n);
            Data.MH.nTotalVertCount += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1) Data.MH.nTotalTangent1Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2) Data.MH.nTotalTangent2Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3) Data.MH.nTotalTangent3Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4) Data.MH.nTotalTangent4Count += node.Mesh.Vertices.size();

            for(int v = 0; v < node.Mesh.Vertices.size(); v++){
                //For every vertex of every mesh

                //Proceed only if this vertex group hasn't been processed yet
                if(node.Mesh.Vertices.at(v).nLinkedFacesIndex == -1){
                    Vertex & vert = node.Mesh.Vertices.at(v); // Make life easier
                    vert.nLinkedFacesIndex = Data.MH.LinkedFacesPointers.size(); //Update reference to new vector that is about to be created
                    Data.MH.LinkedFacesPointers.push_back(std::vector<LinkedFace>()); //Create new vector
                    std::vector<LinkedFace> & LinkedFaceArray = Data.MH.LinkedFacesPointers.back(); //Get reference to the new vector

                    Vector vCoords = vert;
                    Location loc = node.GetLocation();
                    vCoords.Rotate(loc.oOrientation);
                    vCoords += node.Head.vFromRoot;

                    //We've already gone through the nodes up to n and linked any vertices, so we can skip those
                    for(int n2 = n; n2 < Data.MH.ArrayOfNodes.size(); n2++){

                        if(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_HAS_MESH){
                            Node & node2 = Data.MH.ArrayOfNodes.at(n2);

                            //Loop through all the faces in the mesh and look for matching vertices - theoretically there is no way to optimize this part
                            for(int f = 0; f < node2.Mesh.Faces.size(); f++){
                                Face & face = node2.Mesh.Faces.at(f);

                                //We are now checking the three vertices
                                for(int i = 0; i < 3; i++){
                                    //Check if vertices are equal (enough)
                                    Vector vCoords2 = node2.Mesh.Vertices.at(face.nIndexVertex[i]);
                                    Location loc2 = node2.GetLocation();
                                    vCoords2.Rotate(loc2.oOrientation);
                                    vCoords2 += node2.Head.vFromRoot;
                                    if(vCoords.Compare(vCoords2)){

                                        //If they are equal, regardless of weldedness, add the face to the linked faces array
                                        LinkedFaceArray.push_back(LinkedFace(n2, f, face.nIndexVertex[i]));

                                        //Also update the other vert's linked face array reference index, so we can skip processing it later.
                                        node2.Mesh.Vertices.at(face.nIndexVertex[i]).nLinkedFacesIndex = vert.nLinkedFacesIndex;

                                        //Only one vertex in a face can match our vertex, so exit this small loop
                                        ///Well, it turns out that's not true <_< *swears*
                                        //i = 3;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        SendMessage(hProgress, PBM_STEPIT, (WPARAM) NULL, (LPARAM) NULL);
    }
    std::cout<<"Done building LinkedFaces array!\n";
    std::cout<<"Creating patches... \n";
    for(int v = 0; v < Data.MH.LinkedFacesPointers.size(); v++){
        //For every vector of linked faces, create a vector of patches
        std::vector<LinkedFace> & LinkedFaceVector = Data.MH.LinkedFacesPointers.at(v);
        Data.MH.PatchArrayPointers.push_back(std::vector<Patch>());
        std::vector<Patch> & PatchVector = Data.MH.PatchArrayPointers.back();

        //Write it down
        int ngroup = Data.MH.PatchArrayPointers.size() - 1;
        Node & node = GetNodeByNameIndex(LinkedFaceVector.at(0).nNameIndex);
        Vertex & vert = node.Mesh.Vertices.at(LinkedFaceVector.at(0).nVertex);
        Vector vCoords = vert;
        Location loc = node.GetLocation();
        vCoords.Rotate(loc.oOrientation);
        vCoords += node.Head.vFromRoot;
        file<<"Group "<<ngroup<<" "<<vCoords.Print()<<"\n";

        for(int lf = 0; lf < LinkedFaceVector.size(); lf++){
            if(!LinkedFaceVector.at(lf).bAssignedToPatch){
                Patch newpatch;
                LinkedFace & patchhead = LinkedFaceVector.at(lf);
                newpatch.nNameIndex = patchhead.nNameIndex;
                newpatch.nVertex = patchhead.nVertex;
                patchhead.bAssignedToPatch = true;
                std::stringstream ssReport;
                newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(patchhead.nNameIndex).Mesh.Faces.at(patchhead.nFace).nSmoothingGroup;
                newpatch.FaceIndices.push_back(patchhead.nFace); //Assign first linked face index to the patch
                ssReport << patchhead.nFace << " ";

                for(int plf = lf+1; plf < LinkedFaceVector.size(); plf++){
                    if(!LinkedFaceVector.at(plf).bAssignedToPatch){
                        //The following check is the whole point of this whole patch business
                        //The patch only contains those faces which are welded, ie. they have one and the same vertex, ie.
                        //they have the same mesh index and vert index as the point we are constructing this patch for
                        if(LinkedFaceVector.at(plf).nNameIndex == newpatch.nNameIndex &&
                           LinkedFaceVector.at(plf).nVertex == newpatch.nVertex){
                            LinkedFace & linked = LinkedFaceVector.at(plf);
                            linked.bAssignedToPatch = true;
                            newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(linked.nNameIndex).Mesh.Faces.at(linked.nFace).nSmoothingGroup;
                            newpatch.FaceIndices.push_back(linked.nFace); //Assign linked face index to the patch
                            ssReport << linked.nFace << " ";
                        }
                    }
                }
                file<<"   patch "<<PatchVector.size()<<" ("<<Data.MH.Names.at(newpatch.nNameIndex).sName.c_str()<<", vert "<<newpatch.nVertex<<") contains faces: "<<ssReport.str()<<"\n";
                PatchVector.push_back(std::move(newpatch));
            }
        }
    }
    Data.MH.LinkedFacesPointers.resize(0);
    Data.MH.LinkedFacesPointers.shrink_to_fit();
    std::cout<<"Done creating patches! LinkedFace array deleted!\n";
}

//This function together with the next one, checks the currently loaded data in MDL for any special properties
void MDL::CheckPeculiarities(){
    FileHeader * Data = &FH[0];
    std::stringstream ssReturn;
    bool bUpdate = false;
    ssReturn<<"Lucky you! Your model has some rare peculiarities:";
    if(!Data->MH.GH.RuntimeArray1.empty()){
        ssReturn<<"\r\n - First empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(!Data->MH.GH.RuntimeArray2.empty()){
        ssReturn<<"\r\n - Second empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(Data->MH.GH.nRefCount != 0){
        ssReturn<<"\r\n - RefCount has a value!";
        bUpdate = true;
    }
    if(Data->MH.GH.nModelType != 2){
        ssReturn<<"\r\n - Header ModelType different than 2!";
        bUpdate = true;
    }
    if(Data->MH.nChildModelCount != 0){
        ssReturn<<"\r\n - ChildModelCount has a value!";
        bUpdate = true;
    }
    if(Data->MH.AnimationArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - AnimationArray counts differ!";
        bUpdate = true;
    }
    if(Data->MH.nUnknown2 != 0){
        ssReturn<<"\r\n - Unknown int32 after the Root Head Node Offset in the MH has a value!";
        bUpdate = true;
    }
    if(Data->MH.nMdxLength2 != Data->nMdxLength){
        ssReturn<<"\r\n - MdxLength in FH and MdxLength2 in MH don't have the same value!";
        bUpdate = true;
    }
    if(Data->MH.nOffsetIntoMdx != 0){
        ssReturn<<"\r\n - OffsetIntoMdx after the MdxLength2 in the MH has a value!";
        bUpdate = true;
    }
    if(Data->MH.NameArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - NameArray counts differ!";
        bUpdate = true;
    }
    for(int a = 0; a < Data->MH.AnimationArray.nCount; a++){
        if(!Data->MH.GH.RuntimeArray1.empty()){
            ssReturn<<"\r\n   - First empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(!Data->MH.GH.RuntimeArray2.empty()){
            ssReturn<<"\r\n   - Second empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].nRefCount != 0){
            ssReturn<<"\r\n   - Animation counterpart to RefCount has a value!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].SoundArray.GetDoCountsDiffer()){
            ssReturn<<"\r\n   - SoundArray counts differ!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].nPadding2 != 0){
            ssReturn<<"\r\n   - Unknown int32 after SoundArrayHead has a value!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].nModelType != 5){
            ssReturn<<"\r\n   - Animation ModelType is not 5!";
            bUpdate = true;
        }
        if(CheckNodes(Data->MH.Animations[a].ArrayOfNodes, ssReturn, a)) bUpdate = true;
    }
    if(CheckNodes(Data->MH.ArrayOfNodes, ssReturn, -1)) bUpdate = true;
    if(!bUpdate){
        std::cout<<"Checked for peculiarities, nothing to report.\n";
        return;
    }
    MessageBox(NULL, ssReturn.str().c_str(), "Notification", MB_OK);
}

bool MDL::CheckNodes(std::vector<Node> & NodeArray, std::stringstream & ssReturn, int nAnimation){
    bool bMasterUpdate = false;
    for(int b = 0; b < NodeArray.size(); b++){
        bool bUpdate = false;
        std::stringstream ssAdd;
        std::string sCont;
        if(nAnimation == -1) sCont = "geometry";
        else sCont = FH[0].MH.Animations.at(nAnimation).sName.c_str();
        ssAdd<<"\r\n - "<<FH[0].MH.Names[NodeArray[b].Head.nNameIndex].sName<<" ("<<NodeArray[b].Head.nType<<", "<<sCont<<")";
        if(NodeArray[b].Head.nType & NODE_HAS_HEADER){
            if(NodeArray[b].Head.nPadding1 != 0){
                ssAdd<<"\r\n     - Header: Unknown short after NameIndex has a value!";
                bUpdate = true;
            }
            if(NodeArray[b].Head.ChildrenArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - ChildArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Head.ControllerArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - ControllerArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Head.ControllerDataArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - ControllerDataArray counts differ!";
                bUpdate = true;
            }
            for(int c = 0; c < NodeArray[b].Head.Controllers.size(); c++){
                Controller & ctrl = NodeArray.at(b).Head.Controllers.at(c);

                /***
                    This if for checking controller "padding" values. These numbers are in no way random.
                    Header and light controllers always have 0 for the third number, while emitter and mesh controllers have it greater than 0.
                    In keyed controllers, light and emitter seem to group together against header and mesh.
                    Selfillumcolor usually has the same padding values as scale, but they are different
                    in for example: n_admrlsaulkar or 003ebof
                /**/
                if(ctrl.nControllerType==CONTROLLER_HEADER_POSITION ||
                   ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION ||
                   ctrl.nControllerType==CONTROLLER_HEADER_SCALING && ctrl.nAnimation == -1 &&
                    (ctrl.nPadding[0] == 12 &&
                     ctrl.nPadding[1] == 76 &&
                     ctrl.nPadding[2] == 0   )){}
                else if(ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION && ctrl.nAnimation == -1 &&
                         (ctrl.nPadding[0] == -59 &&
                          ctrl.nPadding[1] == 73 &&
                          ctrl.nPadding[2] == 0   )){}
                else if(ctrl.nControllerType==CONTROLLER_HEADER_SCALING && ctrl.nAnimation == -1 &&
                         (ctrl.nPadding[0] == 49 &&
                          ctrl.nPadding[1] == 18 &&
                          ctrl.nPadding[2] == 0   )){}
                else if( (ctrl.nControllerType==CONTROLLER_LIGHT_COLOR || ctrl.nControllerType==CONTROLLER_LIGHT_MULTIPLIER || ctrl.nControllerType==CONTROLLER_LIGHT_RADIUS ||
                          ctrl.nControllerType==CONTROLLER_LIGHT_SHADOWRADIUS || ctrl.nControllerType==CONTROLLER_LIGHT_VERTICALDISPLACEMENT) && ctrl.nAnimation == -1 &&
                         (ctrl.nPadding[0] == -5 &&
                          ctrl.nPadding[1] == 54 &&
                          ctrl.nPadding[2] == 0   )){}
                else if( (ctrl.nControllerType==CONTROLLER_HEADER_POSITION ||
                          ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION ||
                          ctrl.nControllerType==CONTROLLER_HEADER_SCALING ||
                          GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_HAS_MESH) && ctrl.nAnimation != -1 &&
                         (ctrl.nPadding[0] == 50 &&
                          ctrl.nPadding[1] == 18 &&
                          ctrl.nPadding[2] == 0   )){}
                else if(  ctrl.nAnimation != -1 &&
                         (ctrl.nPadding[0] == 51 &&
                          ctrl.nPadding[1] == 18 &&
                          ctrl.nPadding[2] == 0   )){}
                /// the following are all emitter and mesh single controllers (as long as the last value is non-0)
                else if( (GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_HAS_EMITTER ||
                          GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_HAS_MESH) &&
                          ctrl.nPadding[2] > 0 && ctrl.nAnimation == -1 ){}
                else{
                    ssAdd<<"\r\n     - Previously unseen controller padding! ("<<(int)ctrl.nPadding[0]<<", "<<(int)ctrl.nPadding[1]<<", "<<(int)ctrl.nPadding[2]<<")";
                    bUpdate = true;
                }
            }
        }
        if(NodeArray[b].Head.nType & NODE_HAS_LIGHT){
            if(NodeArray[b].Light.UnknownArray.nCount != 0){
                ssAdd<<"\r\n     - Light: Unknown array not empty!";
                bUpdate = true;
            }
            if(NodeArray[b].Light.FlareSizeArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - FlareSizeArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Light.FlarePositionArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - FlarePositionArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Light.FlareColorShiftArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - FlareColorShiftArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Light.FlareTextureNameArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - FlareTextureNameArray counts differ!";
                bUpdate = true;
            }
        }
        if(NodeArray[b].Head.nType & NODE_HAS_EMITTER){
            if(NodeArray[b].Emitter.nUnknown1 != 0){
                ssAdd<<"\r\n     - Emitter: Unknown short after Loop has a value!";
                bUpdate = true;
            }
        }
        if(NodeArray[b].Head.nType & NODE_HAS_MESH){
            if(NodeArray[b].Mesh.nUnknown3[0] != -1 || NodeArray[b].Mesh.nUnknown3[1] != -1 || NodeArray[b].Mesh.nUnknown3[2] != 0){
                ssAdd<<"\r\n     - Mesh: The unknown -1 -1 0 array has a different value!";
                bUpdate = true;
            }
            if(NodeArray[b].Mesh.FaceArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - FaceArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Mesh.IndexCounterArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - IndexCounterArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Mesh.IndexLocationArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - IndexLocationArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Mesh.MeshInvertedCounterArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - MeshInvertedCounterArray counts differ!";
                bUpdate = true;
            }
        }
        if(NodeArray[b].Head.nType & NODE_HAS_SKIN){
            if(!NodeArray[b].Skin.UnknownArray.empty()){
                ssAdd<<"\r\n     - Skin: Unknown empty array has some nonzero value!";
                bUpdate = true;
            }
            if(NodeArray[b].Skin.QBoneArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - QBoneArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Skin.TBoneArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - TBoneArray counts differ!";
                bUpdate = true;
            }
            if(NodeArray[b].Skin.Array8Array.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - Array8Array counts differ!";
                bUpdate = true;
            }
        }
        if(NodeArray[b].Head.nType & NODE_HAS_DANGLY){
            if(NodeArray[b].Dangly.ConstraintArray.GetDoCountsDiffer()){
                ssAdd<<"\r\n     - ConstraintArray counts differ!";
                bUpdate = true;
            }
        }
        if(bUpdate){
            bMasterUpdate = true;
            ssReturn<<ssAdd.str();
        }
    }
    return bMasterUpdate;
}
