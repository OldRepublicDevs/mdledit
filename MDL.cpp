#include "MDL.h"
#include <fstream>
#include <Shlwapi.h>
/// This file should be a general initializer/implementor of MDL.h

void MDL::Report(std::string sMessage){
    if(PtrReport != nullptr) PtrReport(sMessage);
}
void MDL::ProgressSize(int nMin, int nMax){
    if(PtrProgressSize != nullptr) PtrProgressSize(nMin, nMax);
}
void MDL::ProgressPos(int nPos){
    if(PtrProgressPos != nullptr) PtrProgressPos(nPos);
}

int MDL::GetNameIndex(std::string sName){
    if(FH){
        std::vector<Name> & Names = FH->MH.Names;
        int n = 0;
        while(n < Names.size()){
            if(Names[n].sName == sName) return n;
            n++;
        }
    }
    return -1;
}

void MDL::LinearizeGeometry(Node & NODE, std::vector<Node> & ArrayOfNodes){
    for(int n = 0; n < NODE.Head.Children.size(); n++){
        LinearizeGeometry(NODE.Head.Children[n], ArrayOfNodes);
    }
    ArrayOfNodes.at(NODE.Head.nNameIndex) = std::move(NODE);
}

void MDL::LinearizeAnimations(Node & NODE, std::vector<Node> & ArrayOfNodes){
    ArrayOfNodes.push_back(std::move(NODE));
    Node & node = ArrayOfNodes.back();
    for(int n = 0; n < node.Head.Children.size(); n++){
        LinearizeAnimations(node.Head.Children[n], ArrayOfNodes);
    }
}

std::unique_ptr<FileHeader> & MDL::GetFileData(){
    return FH;
}

std::string MDL::MakeUniqueName(int nNameIndex){
    std::vector<Name> & Names = FH->MH.Names;
    std::string sReturn = Names.at(nNameIndex).sName.c_str();
    for(int n = 0; n < nNameIndex; n++){
        if(std::string(Names.at(n).sName.c_str()) == sReturn) return sReturn + "__dpl" + std::to_string(nNameIndex);
    }
    return sReturn;
}

Node & MDL::GetNodeByNameIndex(int nIndex, int nAnimation){
    if(nAnimation == -1){
        return FH->MH.ArrayOfNodes.at(nIndex);
    }
    else{
        std::vector<Node> & Array = FH->MH.Animations[nAnimation].ArrayOfNodes;
        for(int n = 0; n < Array.size(); n++){
            //std::cout<<"Looping through animation nodes for our node\n";
            if(Array[n].Head.nNameIndex == nIndex) return Array[n];
        }
    }
}

bool MDL::HeadLinked(){
    for(int n = 0; n < FH->MH.ArrayOfNodes.size(); n++){
        if(FH->MH.ArrayOfNodes.at(n).nOffset == FH->MH.nOffsetToHeadRootNode){
            if(FH->MH.Names.at(FH->MH.ArrayOfNodes.at(n).Head.nNameIndex).sName == "neck_g") return true;
            else return false;
        }
    }
    return false;
}

bool MDL::NodeExists(const std::string & sNodeName){
    if(GetNameIndex(sNodeName) != -1) return true;
    return false;
}

void MDL::FlushData(){
    FH.reset();
    Ascii.reset();
    Mdx.reset();
    Wok.reset();
    Pwk.reset();
    Dwk.reset();
    FlushAll();
}

//Setters/general
bool MDL::LinkHead(bool bLink){
    unsigned int nOffset;
    if(bLink){
        int nNameIndex = -1;
        for(int n = 0; n < FH->MH.Names.size() && nNameIndex == -1; n++){
            if(FH->MH.Names.at(n).sName == "neck_g") nNameIndex = n;
        }
        if(nNameIndex != -1){
            nOffset = GetNodeByNameIndex(nNameIndex).nOffset;
        }
        else return false;
    }
    else{
        nOffset = FH->MH.GH.nOffsetToRootNode;
    }
    FH->MH.nOffsetToHeadRootNode = nOffset;
    WriteIntToPH(nOffset, 180, nOffset);
    return true;
}

void MDL::UpdateTexture(Node & node, const std::string & sNew, int nTex){
    std::string sNewTex = sNew.c_str();
    if(nTex == 1){
        node.Mesh.cTexture1 = sNewTex;
        sNewTex.resize(32);
        int nPos = node.nOffset + 12 + 168;
        for(int n = 0; n < sNewTex.size(); n++){
            sBuffer.at(nPos + n) = sNewTex.at(n);
        }
    }
    else if(nTex == 2){
        node.Mesh.cTexture2 = sNewTex;
        sNewTex.resize(32);
        int nPos = node.nOffset + 12 + 200;
        for(int n = 0; n < sNewTex.size(); n++){
            sBuffer.at(nPos + n) = sNewTex.at(n);
        }
    }
    else if(nTex == 3){
        node.Mesh.cTexture3 = sNewTex;
        sNewTex.resize(12);
        int nPos = node.nOffset + 12 + 232;
        for(int n = 0; n < sNewTex.size(); n++){
            sBuffer.at(nPos + n) = sNewTex.at(n);
        }
    }
    else if(nTex == 4){
        node.Mesh.cTexture4 = sNewTex;
        sNewTex.resize(12);
        int nPos = node.nOffset + 12 + 244;
        for(int n = 0; n < sNewTex.size(); n++){
            sBuffer.at(nPos + n) = sNewTex.at(n);
        }
    }
}

void MDL::WriteUintToPlaceholder(unsigned int nUint, int nOffset){
    WriteIntToPH(nUint, nOffset, nUint);
}

void MDL::WriteByteToPlaceholder(unsigned char nByte, int nOffset){
    sBuffer.at(nOffset) = nByte;
}

//ascii
void MDL::FlushAscii(){
    if(Ascii) Ascii->FlushAll();
}

std::vector<char> & MDL::CreateAsciiBuffer(int nSize){
    return Ascii->CreateBuffer(nSize);
}

bool MDL::ReadAscii(){
    //CreateDataStructure
    FH.reset(new FileHeader);
    bool bReturn = Ascii->Read(*this);
    if(!bReturn){
        FlushData();
    }
    else std::cout<<"Ascii read succesfully!\n";
    return bReturn;
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
            location.oOrientation.SetQuaternion(DecompressQuaternion((unsigned int) Head.ControllerData.at(orictrl.nDataStart)));
        }
    }

    return location;
}

const std::string MDL::sClassName = "MDL";
const std::string MDX::sClassName = "MDX";
const std::string WOK::sClassName = "WOK";
const std::string PWK::sClassName = "PWK";
const std::string DWK::sClassName = "DWK";

bool LoadSupermodel(MDL & curmdl, std::vector<MDL> & Supermodels){
    std::string sSMname = curmdl.GetFileData()->MH.cSupermodelName;
    sSMname.resize(strlen(sSMname.c_str()));
    if(sSMname != "NULL"){
        MDL newmdl;
        std::string sNewMdl = curmdl.GetFullPath().c_str();
        sNewMdl.reserve(MAX_PATH);
        PathRemoveFileSpec(&sNewMdl[0]);
        sNewMdl.resize(strlen(sNewMdl.c_str()));
        sNewMdl += "\\";
        sNewMdl += curmdl.GetFileData()->MH.cSupermodelName.c_str();
        sNewMdl += ".mdl";

        //Create file
        std::ifstream file(sNewMdl, std::ios::binary);

        //Check for problems
        bool bOpen = true;
        bool bWrongGame = false;
        if(!file.is_open()) bOpen = false;
        if(bOpen){
            file.seekg(0, std::ios::beg);
            char cBinary [4];
            file.read(cBinary, 4);
            //Make sure that what we've read is a binary .mdl as far as we can tell
            if(cBinary[0]!='\0' || cBinary[1]!='\0' || cBinary[2]!='\0' || cBinary[3]!='\0') bOpen = false;
            //If we pass, then the file is definitely ready to be read.
        }
        if(bOpen){
            file.seekg(12);
            file.read(ByteBlock4.bytes, 4);
            if(ByteBlock4.ui != curmdl.GetFileData()->MH.GH.nFunctionPointer0){
                bOpen = false;
                bWrongGame = true;
            }
        }
        bool bReturn = true;
        if(bOpen){
            std::cout<<"Reading supermodel: \n"<<sNewMdl<<"\n";
            file.seekg(0, std::ios::end);
            std::streampos length = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<char> & sBufferRef = newmdl.CreateBuffer(length);
            file.read(&sBufferRef[0], length);
            newmdl.SetFilePath(sNewMdl);
            file.close();

            Supermodels.push_back(std::move(newmdl));
            Supermodels.back().DecompileModel(true);

            bReturn = LoadSupermodel(Supermodels.back(), Supermodels); //Go recursive
        }
        else{
            file.close();
            if(bWrongGame) Warning("Binary supermodel " + std::string(sNewMdl.c_str()) + " belongs to the wrong game and couldn't be read! The supernode numbers will be wrong!");
            else Warning("Could not find binary supermodel " + std::string(sNewMdl.c_str()) + " in the directory! The supernodes numbers will be wrong!");
            return false;
        }
        return bReturn;
    }
    else return true;
}

/// This function is to be used both when compiling and decompiling (to determine smoothing groups)
/// It is very expensive, so modify with care to keep it efficient. Any calculations that can be performed outside, should be.
void MDL::CreatePatches(){
    FileHeader & Data = *FH;

    //SendMessage(hProgress, PBM_SETRANGE, (WPARAM) NULL, MAKELPARAM(0, Data.MH.ArrayOfNodes.size()));
    ProgressSize(0, Data.MH.ArrayOfNodes.size());
    //SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, (LPARAM) NULL);
    Report("Building LinkedFaces array... (this may take a while)");
    std::cout<<"Building LinkedFaces array... (this may take a while)\n";
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        //std::cout<<"Linking faces for node "<<n+1<<"/"<<Data.MH.ArrayOfNodes.size()<<".\n";
        //Currently, this takes all meshes, including skins, danglymeshes, walkmeshes and sabers
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_MESH && !(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_SABER)){
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
                    vert.nLinkedFacesIndex = Data.MH.LinkedFacesPointers.size(); // Update reference to new vector that is about to be created
                    Data.MH.LinkedFacesPointers.push_back(std::vector<LinkedFace>()); // Create new vector
                    std::vector<LinkedFace> & LinkedFaceArray = Data.MH.LinkedFacesPointers.back(); // Get reference to the new vector

                    Vector & vCoords = vert.vFromRoot;

                    //We've already gone through the nodes up to n and linked any vertices, so we can skip those
                    for(int n2 = n; n2 < Data.MH.ArrayOfNodes.size(); n2++){

                        if(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_HAS_MESH && !(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_HAS_SABER)){
                            Node & node2 = Data.MH.ArrayOfNodes.at(n2);

                            //Loop through all the faces in the mesh and look for matching vertices - theoretically there is no way to optimize this part
                            for(int f = 0; f < node2.Mesh.Faces.size(); f++){
                                Face & face = node2.Mesh.Faces.at(f);

                                //We are now checking the three vertices
                                for(int i = 0; i < 3; i++){
                                    //Check if vertices are equal (enough)
                                    Vector & vCoords2 = node2.Mesh.Vertices.at(face.nIndexVertex[i]).vFromRoot;
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
            ProgressPos(n);
            //SendMessage(hProgress, PBM_SETPOS, (WPARAM) n, (LPARAM) NULL);
        }
    }
    std::cout<<"Done building LinkedFaces array!\n";
    ProgressPos(Data.MH.ArrayOfNodes.size());
    //SendMessage(hProgress, PBM_SETPOS, (WPARAM) Data.MH.ArrayOfNodes.size(), (LPARAM) NULL);
    /*
    for(int dbg = 0; dbg < Data.MH.LinkedFacesPointers.size(); dbg++){
        if(Data.MH.LinkedFacesPointers.at(dbg).size() == 0) Warning("A linked face group is empty. This is gonna cause a crash in a bit...");
    }
    */
    std::cout<<"Creating patches... \n";
    Report("Building patches...");
    for(int v = 0; v < Data.MH.LinkedFacesPointers.size(); v++){
        //For every vector of linked faces, create a vector of patches
        std::vector<LinkedFace> & LinkedFaceVector = Data.MH.LinkedFacesPointers.at(v);
        Data.MH.PatchArrayPointers.push_back(std::vector<Patch>());
        std::vector<Patch> & PatchVector = Data.MH.PatchArrayPointers.back();

        for(int lf = 0; lf < LinkedFaceVector.size(); lf++){
            if(!LinkedFaceVector.at(lf).bAssignedToPatch){
                Patch newpatch;
                LinkedFace & patchhead = LinkedFaceVector.at(lf);
                newpatch.nNameIndex = patchhead.nNameIndex;
                newpatch.nVertex = patchhead.nVertex;
                patchhead.bAssignedToPatch = true;
                newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(patchhead.nNameIndex).Mesh.Faces.at(patchhead.nFace).nSmoothingGroup;
                newpatch.FaceIndices.push_back(patchhead.nFace); //Assign first linked face index to the patch

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
                            //ssReport << linked.nFace << " ";
                        }
                    }
                }
                //file<<"   patch "<<PatchVector.size()<<" ("<<Data.MH.Names.at(newpatch.nNameIndex).sName.c_str()<<", vert "<<newpatch.nVertex<<") contains faces: "<<ssReport.str()<<"\n";
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
    FileHeader & Data = *FH;
    std::stringstream ssReturn;
    bool bUpdate = false;
    Report("Checking for peculiarities...");
    ssReturn<<"Lucky you! Your model has some rare peculiarities:";
    if(!Data.MH.GH.RuntimeArray1.empty()){
        ssReturn<<"\r\n - First empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(!Data.MH.GH.RuntimeArray2.empty()){
        ssReturn<<"\r\n - Second empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(Data.MH.GH.nRefCount != 0){
        ssReturn<<"\r\n - RefCount has a value!";
        bUpdate = true;
    }
    if(Data.MH.GH.nModelType != 2){
        ssReturn<<"\r\n - Header ModelType different than 2!";
        bUpdate = true;
    }
    if(Data.MH.nUnknown1[1] != 0){
        ssReturn<<"\r\n - Second classification padding number different than 0!";
        bUpdate = true;
    }
    if(Data.MH.nUnknown1[2] != 1){
        ssReturn<<"\r\n - Third classification padding number different than 1!";
        bUpdate = true;
    }
    if(Data.MH.nChildModelCount != 0){
        ssReturn<<"\r\n - ChildModelCount has a value!";
        bUpdate = true;
    }
    if(Data.MH.AnimationArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - AnimationArray counts differ!";
        bUpdate = true;
    }
    if(Data.MH.nUnknown2 != 0){
        ssReturn<<"\r\n - Unknown int32 after the Root Head Node Offset in the MH has a value!";
        bUpdate = true;
    }
    if(Data.MH.nMdxLength2 != Data.nMdxLength){
        ssReturn<<"\r\n - MdxLength in FH and MdxLength2 in MH don't have the same value!";
        bUpdate = true;
    }
    if(Data.MH.nOffsetIntoMdx != 0){
        ssReturn<<"\r\n - OffsetIntoMdx after the MdxLength2 in the MH has a value!";
        bUpdate = true;
    }
    if(Data.MH.NameArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - NameArray counts differ!";
        bUpdate = true;
    }
    for(int a = 0; a < Data.MH.AnimationArray.nCount; a++){
        if(!Data.MH.GH.RuntimeArray1.empty()){
            ssReturn<<"\r\n   - First empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(!Data.MH.GH.RuntimeArray2.empty()){
            ssReturn<<"\r\n   - Second empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nRefCount != 0){
            ssReturn<<"\r\n   - Animation counterpart to RefCount has a value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).SoundArray.GetDoCountsDiffer()){
            ssReturn<<"\r\n   - SoundArray counts differ!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nPadding2 != 0){
            ssReturn<<"\r\n   - Unknown int32 after SoundArrayHead has a value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nModelType != 5){
            ssReturn<<"\r\n   - Animation ModelType is not 5!";
            bUpdate = true;
        }
        if(CheckNodes(Data.MH.Animations.at(a).ArrayOfNodes, ssReturn, a)) bUpdate = true;
    }
    if(CheckNodes(Data.MH.ArrayOfNodes, ssReturn, -1)) bUpdate = true;
    if(!bUpdate){
        std::cout<<"Checked for peculiarities, nothing to report.\n";
        return;
    }
    MessageBox(NULL, ssReturn.str().c_str(), "Notification", MB_OK);
}

bool MDL::CheckNodes(std::vector<Node> & NodeArray, std::stringstream & ssReturn, int nAnimation){
    bool bMasterUpdate = false;
    for(int b = 0; b < NodeArray.size(); b++){
        if(NodeArray.at(b).Head.nType == 0){
            //Ghost node
        }
        else{
            bool bUpdate = false;
            std::stringstream ssAdd;
            std::string sCont;
            if(nAnimation == -1) sCont = "geometry";
            else sCont = FH->MH.Animations.at(nAnimation).sName.c_str();
            ssAdd<<"\r\n - "<<FH->MH.Names.at(NodeArray.at(b).Head.nNameIndex).sName<<" ("<<sCont<<")";
            if(NodeArray.at(b).Head.nType & NODE_HAS_HEADER){
                if(NodeArray.at(b).Head.nPadding1 != 0){
                    ssAdd<<"\r\n     - Header: Unknown short after NameIndex has a value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ChildrenArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Header: ChildArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ControllerArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Header: ControllerArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ControllerDataArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Header: ControllerDataArray counts differ!";
                    bUpdate = true;
                }
                for(int c = 0; c < NodeArray.at(b).Head.Controllers.size(); c++){
                    Controller & ctrl = NodeArray.at(b).Head.Controllers.at(c);

                    if( (ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                         ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION) &&
                        ctrl.nAnimation != -1 &&
                        ctrl.nUnknown2 == ctrl.nControllerType + 8){}
                    else if(ctrl.nUnknown2 == -1 &&
                            ( (ctrl.nControllerType != CONTROLLER_HEADER_POSITION &&
                               ctrl.nControllerType != CONTROLLER_HEADER_ORIENTATION) ||
                              ctrl.nAnimation == -1) ){}
                    else{
                        std::string sLoc;
                        if(ctrl.nAnimation == -1) sLoc = "geometry";
                        else sLoc = FH->MH.Animations.at(ctrl.nAnimation).sName.c_str();
                        ssAdd<<"\r\n     - Header: New controller unknown2 value! ("<<ctrl.nUnknown2<<" - "<<ReturnControllerName(ctrl.nControllerType, FH->MH.ArrayOfNodes.at(ctrl.nNameIndex).Head.nType)<<" controller in node "<<FH->MH.Names.at(ctrl.nNameIndex).sName<<" in "<<sLoc<<")";
                        bUpdate = true;
                    }
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
                        ssAdd<<"\r\n     - Header: Previously unseen controller padding! ("<<(int)ctrl.nPadding[0]<<", "<<(int)ctrl.nPadding[1]<<", "<<(int)ctrl.nPadding[2]<<")";
                        bUpdate = true;
                    }
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_HAS_LIGHT){
                if(NodeArray.at(b).Light.UnknownArray.nCount != 0){
                    ssAdd<<"\r\n     - Light: Unknown array not empty!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareSizeArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Light: FlareSizeArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlarePositionArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Light: FlarePositionArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareColorShiftArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Light: FlareColorShiftArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareTextureNameArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Light: FlareTextureNameArray counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_HAS_EMITTER){
                if(NodeArray.at(b).Emitter.nUnknown1 != 0){
                    ssAdd<<"\r\n     - Emitter: Unknown short after Loop has a value!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_HAS_MESH){
                if(NodeArray.at(b).Mesh.nUnknown3[0] != -1 || NodeArray.at(b).Mesh.nUnknown3[1] != -1 || NodeArray.at(b).Mesh.nUnknown3[2] != 0){
                    ssAdd<<"\r\n     - Mesh: The unknown -1 -1 0 array has a different value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.FaceArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Mesh: FaceArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.IndexCounterArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Mesh: IndexCounterArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.IndexLocationArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Mesh: IndexLocationArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.MeshInvertedCounterArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Mesh: MeshInvertedCounterArray counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_HAS_SKIN){
                if(!NodeArray.at(b).Skin.UnknownArray.empty()){
                    ssAdd<<"\r\n     - Skin: Unknown empty array has some nonzero value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.QBoneArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Skin: QBoneArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.TBoneArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Skin: TBoneArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.Array8Array.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Skin: Array8Array counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_HAS_DANGLY){
                if(NodeArray.at(b).Dangly.ConstraintArray.GetDoCountsDiffer()){
                    ssAdd<<"\r\n     - Dangly: ConstraintArray counts differ!";
                    bUpdate = true;
                }
            }
            if(bUpdate){
                bMasterUpdate = true;
                ssReturn<<ssAdd.str();
            }
        }
    }
    return bMasterUpdate;
}


std::string ReturnClassificationName(int nClassification){
    switch(nClassification){
        case CLASS_OTHER: return "Other";
        case CLASS_EFFECT: return "Effect";
        case CLASS_TILE: return "Tile";
        case CLASS_CHARACTER: return "Character";
        case CLASS_DOOR: return "Door";
        case CLASS_SABER: return "";
        case CLASS_PLACEABLE: return "Placeable";
    }
    std::cout<<"ReturnClassification() ERROR: Unknown classification "<<nClassification<<".\n";
    return "unknown";
}

int ReturnController(std::string sController){
    if(sController == "position") return CONTROLLER_HEADER_POSITION;
    else if(sController == "orientation") return CONTROLLER_HEADER_ORIENTATION;
    else if(sController == "scale") return CONTROLLER_HEADER_SCALING;
    else if(sController == "color") return CONTROLLER_LIGHT_COLOR;
    else if(sController == "radius") return CONTROLLER_LIGHT_RADIUS;
    else if(sController == "shadowradius") return CONTROLLER_LIGHT_SHADOWRADIUS;          //Missing from NWmax
    else if(sController == "verticaldisplacement") return CONTROLLER_LIGHT_VERTICALDISPLACEMENT;  //Missing from NWmax
    else if(sController == "multipler") return CONTROLLER_LIGHT_MULTIPLIER;             //NWmax reads 'multipler', not 'multiplier'
    else if(sController == "multiplier") return CONTROLLER_LIGHT_MULTIPLIER;             //NWmax reads 'multipler', not 'multiplier'
    else if(sController == "alphaend") return CONTROLLER_EMITTER_ALPHAEND;
    else if(sController == "alphastart") return CONTROLLER_EMITTER_ALPHASTART;
    else if(sController == "birthrate") return CONTROLLER_EMITTER_BRITHRATE;
    else if(sController == "bounce_co") return CONTROLLER_EMITTER_BOUNCE_CO;
    else if(sController == "combinetime") return CONTROLLER_EMITTER_COMBINETIME;
    else if(sController == "drag") return CONTROLLER_EMITTER_DRAG;
    else if(sController == "fps") return CONTROLLER_EMITTER_FPS;
    else if(sController == "frameend") return CONTROLLER_EMITTER_FRAMEEND;
    else if(sController == "framestart") return CONTROLLER_EMITTER_FRAMESTART;
    else if(sController == "grav") return CONTROLLER_EMITTER_GRAV;
    else if(sController == "lifeexp") return CONTROLLER_EMITTER_LIFEEXP;
    else if(sController == "mass") return CONTROLLER_EMITTER_MASS;
    else if(sController == "p2p_bezier2") return CONTROLLER_EMITTER_P2P_BEZIER2;
    else if(sController == "p2p_bezier3") return CONTROLLER_EMITTER_P2P_BEZIER3;
    else if(sController == "particlerot") return CONTROLLER_EMITTER_PARTICLEROT;
    else if(sController == "randvel") return CONTROLLER_EMITTER_RANDVEL;
    else if(sController == "sizestart") return CONTROLLER_EMITTER_SIZESTART;
    else if(sController == "sizeend") return CONTROLLER_EMITTER_SIZEEND;
    else if(sController == "sizestart_y") return CONTROLLER_EMITTER_SIZESTART_Y;
    else if(sController == "sizeend_y") return CONTROLLER_EMITTER_SIZEEND_Y;
    else if(sController == "spread") return CONTROLLER_EMITTER_SPREAD;
    else if(sController == "threshold") return CONTROLLER_EMITTER_THRESHOLD;
    else if(sController == "velocity") return CONTROLLER_EMITTER_VELOCITY;
    else if(sController == "xsize") return CONTROLLER_EMITTER_XSIZE;
    else if(sController == "ysize") return CONTROLLER_EMITTER_YSIZE;
    else if(sController == "blurlength") return CONTROLLER_EMITTER_BLURLENGTH;
    else if(sController == "lightningdelay") return CONTROLLER_EMITTER_LIGHTNINGDELAY;
    else if(sController == "lightningradius") return CONTROLLER_EMITTER_LIGHTNINGRADIUS;
    else if(sController == "lightningscale") return CONTROLLER_EMITTER_LIGHTNINGSCALE;
    else if(sController == "lightningsubdiv") return CONTROLLER_EMITTER_LIGHTNINGSUBDIV;
    else if(sController == "lightningzigzag") return CONTROLLER_EMITTER_LIGHTNINGZIGZAG;    //Missing from NWmax
    else if(sController == "alphamid") return CONTROLLER_EMITTER_ALPHAMID;           //Missing from NWmax
    else if(sController == "percentstart") return CONTROLLER_EMITTER_PERCENTSTART;       //Missing from NWmax
    else if(sController == "percentmid") return CONTROLLER_EMITTER_PERCENTMID;         //Missing from NWmax
    else if(sController == "percentend") return CONTROLLER_EMITTER_PERCENTEND;         //Missing from NWmax
    else if(sController == "sizemid") return CONTROLLER_EMITTER_SIZEMID;            //Missing from NWmax
    else if(sController == "sizemid_y") return CONTROLLER_EMITTER_SIZEMID_Y;          //Missing from NWmax
    else if(sController == "m_frandombirthrate") return CONTROLLER_EMITTER_RANDOMBIRTHRATE;    //Missing from NWmax
    else if(sController == "targetsize") return CONTROLLER_EMITTER_TARGETSIZE;         //Missing from NWmax
    else if(sController == "numcontrolpts") return CONTROLLER_EMITTER_NUMCONTROLPTS;      //Missing from NWmax
    else if(sController == "controlptradius") return CONTROLLER_EMITTER_CONTROLPTRADIUS;    //Missing from NWmax
    else if(sController == "controlptdelay") return CONTROLLER_EMITTER_CONTROLPTDELAY;     //Missing from NWmax
    else if(sController == "tangentspread") return CONTROLLER_EMITTER_TANGENTSPREAD;      //Missing from NWmax
    else if(sController == "tangentlength") return CONTROLLER_EMITTER_TANGENTLENGTH;      //Missing from NWmax
    else if(sController == "colormid") return CONTROLLER_EMITTER_COLORMID;           //Missing from NWmax
    else if(sController == "colorend") return CONTROLLER_EMITTER_COLOREND;
    else if(sController == "colorstart") return CONTROLLER_EMITTER_COLORSTART;
    else if(sController == "detonate") return CONTROLLER_EMITTER_DETONATE;           //Missing from NWmax
    else if(sController == "selfillumcolor") return CONTROLLER_MESH_SELFILLUMCOLOR;
    else if(sController == "alpha") return CONTROLLER_MESH_ALPHA;
    else return 0;
}

std::string ReturnControllerName(int nController, int nType){
    switch(nController){
        case CONTROLLER_HEADER_POSITION:            return "position";
        case CONTROLLER_HEADER_ORIENTATION:         return "orientation";
        case CONTROLLER_HEADER_SCALING:             return "scale";
    }

    if(nType & NODE_HAS_LIGHT){
        switch(nController){
        case CONTROLLER_LIGHT_COLOR:                return "color";
        case CONTROLLER_LIGHT_RADIUS:               return "radius";
        case CONTROLLER_LIGHT_SHADOWRADIUS:         return "shadowradius";          //Missing from NWmax
        case CONTROLLER_LIGHT_VERTICALDISPLACEMENT: return "verticaldisplacement";  //Missing from NWmax
        case CONTROLLER_LIGHT_MULTIPLIER:           return "multiplier";
        }
    }
    else if(nType & NODE_HAS_EMITTER){
        switch(nController){
        case CONTROLLER_EMITTER_ALPHAEND:           return "alphaEnd";
        case CONTROLLER_EMITTER_ALPHASTART:         return "alphaStart";
        case CONTROLLER_EMITTER_BRITHRATE:          return "birthrate";
        case CONTROLLER_EMITTER_BOUNCE_CO:          return "bounce_co";
        case CONTROLLER_EMITTER_COMBINETIME:        return "combinetime";
        case CONTROLLER_EMITTER_DRAG:               return "drag";
        case CONTROLLER_EMITTER_FPS:                return "fps";
        case CONTROLLER_EMITTER_FRAMEEND:           return "frameEnd";
        case CONTROLLER_EMITTER_FRAMESTART:         return "frameStart";
        case CONTROLLER_EMITTER_GRAV:               return "grav";
        case CONTROLLER_EMITTER_LIFEEXP:            return "lifeExp";
        case CONTROLLER_EMITTER_MASS:               return "mass";
        case CONTROLLER_EMITTER_P2P_BEZIER2:        return "p2p_bezier2";
        case CONTROLLER_EMITTER_P2P_BEZIER3:        return "p2p_bezier3";
        case CONTROLLER_EMITTER_PARTICLEROT:        return "particleRot";
        case CONTROLLER_EMITTER_RANDVEL:            return "randvel";
        case CONTROLLER_EMITTER_SIZESTART:          return "sizeStart";
        case CONTROLLER_EMITTER_SIZEEND:            return "sizeEnd";
        case CONTROLLER_EMITTER_SIZESTART_Y:        return "sizeStart_y";
        case CONTROLLER_EMITTER_SIZEEND_Y:          return "sizeEnd_y";
        case CONTROLLER_EMITTER_SPREAD:             return "spread";
        case CONTROLLER_EMITTER_THRESHOLD:          return "threshold";
        case CONTROLLER_EMITTER_VELOCITY:           return "velocity";
        case CONTROLLER_EMITTER_XSIZE:              return "xsize";
        case CONTROLLER_EMITTER_YSIZE:              return "ysize";
        case CONTROLLER_EMITTER_BLURLENGTH:         return "blurlength";
        case CONTROLLER_EMITTER_LIGHTNINGDELAY:     return "lightningDelay";
        case CONTROLLER_EMITTER_LIGHTNINGRADIUS:    return "lightningRadius";
        case CONTROLLER_EMITTER_LIGHTNINGSCALE:     return "lightningScale";
        case CONTROLLER_EMITTER_LIGHTNINGSUBDIV:    return "lightningSubDiv";
        case CONTROLLER_EMITTER_LIGHTNINGZIGZAG:    return "lightningzigzag";    //Missing from NWmax
        case CONTROLLER_EMITTER_ALPHAMID:           return "alphaMid";           //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTSTART:       return "percentStart";       //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTMID:         return "percentMid";         //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTEND:         return "percentEnd";         //Missing from NWmax
        case CONTROLLER_EMITTER_SIZEMID:            return "sizeMid";            //Missing from NWmax
        case CONTROLLER_EMITTER_SIZEMID_Y:          return "sizeMid_y";          //Missing from NWmax
        case CONTROLLER_EMITTER_RANDOMBIRTHRATE:    return "m_fRandomBirthRate";    //Missing from NWmax
        case CONTROLLER_EMITTER_TARGETSIZE:         return "targetsize";         //Missing from NWmax
        case CONTROLLER_EMITTER_NUMCONTROLPTS:      return "numcontrolpts";      //Missing from NWmax
        case CONTROLLER_EMITTER_CONTROLPTRADIUS:    return "controlptradius";    //Missing from NWmax
        case CONTROLLER_EMITTER_CONTROLPTDELAY:     return "controlptdelay";     //Missing from NWmax
        case CONTROLLER_EMITTER_TANGENTSPREAD:      return "tangentspread";      //Missing from NWmax
        case CONTROLLER_EMITTER_TANGENTLENGTH:      return "tangentlength";      //Missing from NWmax
        case CONTROLLER_EMITTER_COLORMID:           return "colorMid";           //Missing from NWmax
        case CONTROLLER_EMITTER_COLOREND:           return "colorEnd";
        case CONTROLLER_EMITTER_COLORSTART:         return "colorStart";
        case CONTROLLER_EMITTER_DETONATE:           return "detonate";           //Missing from NWmax
        }
    }
    else{
        switch(nController){
        case CONTROLLER_MESH_SELFILLUMCOLOR:        return "selfillumcolor";
        case CONTROLLER_MESH_ALPHA:                 return "alpha";
        }
    }
    std::cout<<"ReturnController() ERROR: Unknown controller "<<nController<<" (type "<<nType<<").\n";
    return "unknown";
}
