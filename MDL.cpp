#include "MDL.h"
#include <fstream>
#include <algorithm>
#include <Shlwapi.h>
/// This file should be a general initializer/implementor of MDL.h

double Patch::fDiff = 0.0;
MDL * Patch::ptr_mdl = nullptr;

const unsigned short INVALID_SHORT = 0xFFFF;
const unsigned int INVALID_INT = 0xFFFFFFFF;
bool bReportValidness = false;

void MDL::Report(std::string sMessage){
    if(PtrReport != nullptr) PtrReport(sMessage);
}
void MDL::ProgressSize(int nMin, int nMax){
    if(PtrProgressSize != nullptr) PtrProgressSize(nMin, nMax);
}
void MDL::ProgressPos(int nPos){
    if(PtrProgressPos != nullptr) PtrProgressPos(nPos);
}
void MDL::ProgressSetStep(int nStep){
    if(PtrProgressSetStep != nullptr) PtrProgressSetStep(nStep);
}
void MDL::ProgressStepIt(){
    if(PtrProgressStepIt != nullptr) PtrProgressStepIt();
}

MdlInteger<unsigned short> MDL::GetNameIndex(const std::string & sName){
    if(!FH) throw mdlexception("MDL::GetNameIndex() ERROR: File header is not available.");
    std::vector<Name> & Names = FH->MH.Names;
    MdlInteger<unsigned short> nReturn;
    for(int n = 0; n < Names.size(); n++){
        if(StringEqual(Names[n].sName, sName)){
            nReturn = n;
            break;
        }
    }
    return nReturn;
}


std::string & MDL::GetNodeName(Node & node){
    if(!FH) throw mdlexception("MDL::GetNodeName() ERROR: File header is not available.");
    if(!node.Head.nNameIndex.Valid()) throw mdlexception("MDL::GetNodeName() ERROR: Name index (" + node.Head.nNameIndex.Print() + ") is invalid.");
    else if(node.Head.nNameIndex >= FH->MH.Names.size()) throw mdlexception("MDL::GetNodeName() ERROR: Name index (" + node.Head.nNameIndex.Print() + ") is too large to find a name.");
    return FH->MH.Names.at(node.Head.nNameIndex).sName;
}

unsigned int GetFunctionPointer(unsigned int FN_PTR, int n, bool bXbox, bool bK2){
    if(bXbox){
        if(bK2){
            if(n == 1){
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_XBOX_K2_MODEL_1;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_XBOX_K2_ANIM_1;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_XBOX_K2_MESH_1;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_XBOX_K2_SKIN_1;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_XBOX_K2_DANGLY_1;
            }
            else{
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_XBOX_K2_MODEL_2;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_XBOX_K2_ANIM_2;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_XBOX_K2_MESH_2;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_XBOX_K2_SKIN_2;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_XBOX_K2_DANGLY_2;
            }
        }
        else{
            if(n == 1){
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_XBOX_K1_MODEL_1;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_XBOX_K1_ANIM_1;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_XBOX_K1_MESH_1;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_XBOX_K1_SKIN_1;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_XBOX_K1_DANGLY_1;
            }
            else{
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_XBOX_K1_MODEL_2;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_XBOX_K1_ANIM_2;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_XBOX_K1_MESH_2;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_XBOX_K1_SKIN_2;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_XBOX_K1_DANGLY_2;
            }
        }
    }
    else{
        if(bK2){
            if(n == 1){
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_PC_K2_MODEL_1;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_PC_K2_ANIM_1;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_PC_K2_MESH_1;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_PC_K2_SKIN_1;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_PC_K2_DANGLY_1;
            }
            else{
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_PC_K2_MODEL_2;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_PC_K2_ANIM_2;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_PC_K2_MESH_2;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_PC_K2_SKIN_2;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_PC_K2_DANGLY_2;
            }
        }
        else{
            if(n == 1){
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_PC_K1_MODEL_1;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_PC_K1_ANIM_1;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_PC_K1_MESH_1;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_PC_K1_SKIN_1;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_PC_K1_DANGLY_1;
            }
            else{
                if(FN_PTR == FN_PTR_MODEL) return FN_PTR_PC_K1_MODEL_2;
                else if(FN_PTR == FN_PTR_ANIM) return FN_PTR_PC_K1_ANIM_2;
                else if(FN_PTR == FN_PTR_MESH) return FN_PTR_PC_K1_MESH_2;
                else if(FN_PTR == FN_PTR_SKIN) return FN_PTR_PC_K1_SKIN_2;
                else if(FN_PTR == FN_PTR_DANGLY) return FN_PTR_PC_K1_DANGLY_2;
            }
        }
    }
    return -1;
}

unsigned int MDL::FunctionPointer1(unsigned int FN_PTR){
    return GetFunctionPointer(FN_PTR, 1, bXbox, bK2);
}

unsigned int MDL::FunctionPointer2(unsigned int FN_PTR){
    return GetFunctionPointer(FN_PTR, 2, bXbox, bK2);
}

std::vector<Vertex> MDL::GetWokVertData(Node & node){
    std::vector<Vertex> verts_wok;
    if(!(Wok && Wok->GetData())) return verts_wok;
    BWMHeader & data = *Wok->GetData();
    Vector vLyt;
    if(FH) vLyt = FH->MH.vLytPosition;
    if(data.faces.size() != node.Mesh.Faces.size()) return verts_wok;

    /// All the checks have passed, get to business. First, build a vector of Faces in the order of the wok.
    std::vector<Face> faces_wok;
    std::vector<Face> unwalkable;
    for(int f = 0; f < node.Mesh.Faces.size(); f++){
        Face & face = node.Mesh.Faces.at(f);
        if(IsMaterialWalkable(face.nMaterialID)) faces_wok.push_back(face);
        else unwalkable.push_back(face);
    }
    for(int f = 0; f < unwalkable.size(); f++){
        faces_wok.push_back(unwalkable.at(f));
    }
    /// Now go through these faces and the wok faces simultaneously and copy the vert positions.
    verts_wok.resize(node.Mesh.Vertices.size());
    for(int f = 0; f < node.Mesh.Faces.size(); f++){
        for(int i = 0; i < 3; i++){
            verts_wok.at(faces_wok.at(f).nIndexVertex.at(i)).assign(data.verts.at(data.faces.at(f).nIndexVertex.at(i)) - vLyt - node.Head.vFromRoot, true);
        }
    }
    for(Vertex & vert : verts_wok) std::cout << vert.Print() << "\n";
    return verts_wok;
}

void MDL::GetLytPositionFromWok(){
    if(!Wok) return;
    if(!FH) return;
    if(!Wok->GetData()) return;
    FileHeader & Data = *FH;
    BWMHeader & data = *Wok->GetData();
    if(data.faces.size() == 0) return;

    int nSearchMaterial = data.faces.front().nMaterialID;
    for(Node & node : Data.MH.ArrayOfNodes){
        if(node.Head.nType & NODE_AABB){
            for(Face & face : node.Mesh.Faces){
                if(face.nMaterialID == nSearchMaterial){
                    Data.MH.vLytPosition =
                        Vector(data.verts.at(data.faces.front().nIndexVertex.at(0)).fX - node.Mesh.Vertices.at(face.nIndexVertex.at(0)).vFromRoot.fX,
                               data.verts.at(data.faces.front().nIndexVertex.at(0)).fY - node.Mesh.Vertices.at(face.nIndexVertex.at(0)).vFromRoot.fY,
                               data.verts.at(data.faces.front().nIndexVertex.at(0)).fZ - node.Mesh.Vertices.at(face.nIndexVertex.at(0)).vFromRoot.fZ);
                    return;
                }
            }
            return;
        }
    }
}

std::unique_ptr<FileHeader> & MDL::GetFileData(){
    return FH;
}

std::string MDL::MakeUniqueName(MdlInteger<unsigned short> nNameIndex){
    if(!FH) throw mdlexception("MDL::MakeUniqueName() failed because we're running it on a model without FileHeader data.");
    if(!nNameIndex.Valid()) throw mdlexception("MDL::MakeUniqueName() failed because nNameIndex is invalid (-1).");
    std::vector<Name> & Names = FH->MH.Names;
    std::string sReturn = Names.at(nNameIndex).sName.c_str();
    std::vector<Node> & Nodes = FH->MH.ArrayOfNodes;
    if(Nodes.size() > nNameIndex){
        if(Nodes.at(nNameIndex).Head.nType & NODE_SABER && bLightsaberToTrimesh) sReturn = "2081__" + sReturn;
    }
    for(int n = 0; n < nNameIndex; n++){
        if(StringEqual(Names.at(n).sName, sReturn)) return sReturn + "__dpl" + std::to_string(nNameIndex);
    }
    return sReturn;
}

MdlInteger<unsigned short> MDL::GetNodeIndexByNameIndex(MdlInteger<unsigned short> nNameIndex, MdlInteger<unsigned int> nAnimation){
    if(!FH)
        throw mdlexception("MDL::GetNodeIndexByNameIndex() ERROR: File header not available.");
    FileHeader & Data = *FH;

    if(!nNameIndex.Valid() || nNameIndex >= Data.MH.Names.size())
        throw mdlexception("MDL::GetNodeIndexByNameIndex() ERROR: The name index " + std::to_string(nNameIndex) + " lies outside the range of valid indices (0-" + std::to_string(Data.MH.Names.size()-1) + ").");

    MdlInteger<unsigned short> nReturn;
    if(!nAnimation.Valid()){
        for(unsigned short n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
            if(Data.MH.ArrayOfNodes.at(n).Head.nNameIndex == nNameIndex){
                nReturn = n;
                break;
            }
        }
    }
    else{
        for(unsigned short n = 0; n < Data.MH.Animations.at(nAnimation).ArrayOfNodes.size(); n++){
            if(Data.MH.Animations.at(nAnimation).ArrayOfNodes.at(n).Head.nNameIndex == nNameIndex){
                nReturn = n;
                break;
            }
        }
    }
    return nReturn;
}

bool MDL::HeadLinked(){
    if(FH->MH.GH.nOffsetToRootNode != FH->MH.nOffsetToHeadRootNode) return true;
    return false;
}

bool MDL::NodeExists(const std::string & sNodeName){
    if(GetNameIndex(sNodeName).Valid()) return true;
    return false;
}

void ModelHeader::BuildTreeOrderArray(Node & node){
    NameIndicesInTreeOrder.push_back(node.Head.nNameIndex);
    for(auto n : node.Head.ChildIndices){
        if(n >= Names.size()) throw mdlexception("BuildTreeOrderArray() error, child name index out of scope!");
        for(Node & child : ArrayOfNodes){
            if(child.Head.nNameIndex == n) BuildTreeOrderArray(child);
        }
    }
}

std::stringstream & MDL::GetReport(){
    return ssReport;
}

void MDL::SaveReport(){
    std::wstring sFile = GetFullPath();
    if(safesubstr(sFile, sFile.size() - 6, 6) == L".ascii") sFile = safesubstr(sFile, 0, sFile.size() - 6);
    if(safesubstr(sFile, sFile.size() - 4, 4) == L".mdl") sFile = safesubstr(sFile, 0, sFile.size() - 4);
    sFile += L"_report.txt";

    /// Create file
    HANDLE hFile = bead_CreateWriteFile(sFile);

    if(hFile == INVALID_HANDLE_VALUE){
        std::cout << "File creation failed for " << sFile.c_str() << ". Aborting.\n";
        return;
    }

    /// Write and close file
    bead_WriteFile(hFile, ssReport.str());
    CloseHandle(hFile);
}

void MDL::FlushData(){
    FH.reset();
    Ascii.reset();
    Mdx.reset();
    Wok.reset();
    Pwk.reset();
    Dwk0.reset();
    Dwk1.reset();
    Dwk2.reset();
    PwkAscii.reset();
    DwkAscii.reset();
    FlushAll();
    ClearStringstream(ssReport);

    src = NoSource;
}

//Setters/general
bool MDL::LinkHead(bool bLink){
    unsigned int nOffset;
    if(bLink){
        MdlInteger<unsigned short> nNameIndex;
        for(unsigned short n = 0; n < FH->MH.Names.size() && !nNameIndex.Valid(); n++){
            if(StringEqual(FH->MH.Names.at(n).sName, "neck_g")) nNameIndex = n;
        }
        if(nNameIndex.Valid()){
            MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(nNameIndex);
            if(!nNodeIndex.Valid()) throw mdlexception("MDL::LinkHead() error: dealing with a name index that does not have a node in geometry.");
            nOffset = FH->MH.ArrayOfNodes.at(nNodeIndex).nOffset;
        }
        else return false;
    }
    else{
        nOffset = FH->MH.GH.nOffsetToRootNode;
    }
    FH->MH.nOffsetToHeadRootNode = nOffset;
    unsigned nHeadLinkOffset = 180;
    WriteNumber(&nOffset, 0, "", &nHeadLinkOffset);
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

void MDL::WriteUintToPlaceholder(unsigned int nUint, unsigned nOffset){
    WriteNumber(&nUint, 0, "", &nOffset);
}

void MDL::WriteByteToPlaceholder(unsigned char nByte, unsigned nOffset){
    WriteNumber(&nByte, 0, "", &nOffset);
}

//ascii
void MDL::FlushAscii(){
    if(Ascii) Ascii->FlushAll();
}

std::vector<char> & MDL::CreateAsciiBuffer(int nSize){
    return Ascii->CreateBuffer(nSize);
}

bool MDL::ReadAscii(){
    ReportObject ReportMdl(*this);
    if(Ascii){
        if(!Ascii->Read(*this)){
            //FlushData();
            return false;
        }
        else ReportMdl << "Mdl ascii read succesfully!\n";
    }
    if(PwkAscii){
        if(!PwkAscii->ReadWalkmesh(*this, true)){
            //FlushData();
            return false;
        }
        else ReportMdl << "Pwk ascii read succesfully!\n";
    }
    if(DwkAscii){
        if(!DwkAscii->ReadWalkmesh(*this, false)){
            //FlushData();
            return false;
        }
        else ReportMdl << "Dwk ascii read succesfully!\n";
    }
    return true;
}

Location Node::GetLocation(){
    Location location;

    MdlInteger<unsigned short> nCtrlPos;
    MdlInteger<unsigned short> nCtrlOri;
    for(unsigned short c = 0; c < Head.Controllers.size(); c++){
        if(Head.Controllers.at(c).nControllerType == CONTROLLER_HEADER_POSITION){
            nCtrlPos = c;
        }
        if(Head.Controllers.at(c).nControllerType == CONTROLLER_HEADER_ORIENTATION){
            nCtrlOri = c;
        }
    }

    if(nCtrlPos.Valid()){
        Controller & posctrl = Head.Controllers.at(nCtrlPos);
        location.vPosition = Vector(Head.ControllerData.at(posctrl.nDataStart + 0),
                                    Head.ControllerData.at(posctrl.nDataStart + 1),
                                    Head.ControllerData.at(posctrl.nDataStart + 2));
    }
    if(nCtrlOri.Valid()){
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

bool LoadSupermodel(MDL & curmdl, std::unique_ptr<MDL> & Supermodel){
    ReportObject ReportMdl(curmdl);
    std::string sSMname = curmdl.GetFileData()->MH.cSupermodelName.c_str();
    if(sSMname != "NULL" && sSMname != ""){
        std::wstring sNewMdl = curmdl.GetFullPath().c_str();
        sNewMdl.reserve(MAX_PATH);
        PathRemoveFileSpecW(&sNewMdl[0]);
        sNewMdl.resize(wcslen(sNewMdl.c_str()));
        sNewMdl += L"\\";
        sNewMdl += to_wide(curmdl.GetFileData()->MH.cSupermodelName);
        sNewMdl += L".mdl";

        //Create file
        HANDLE hFile = bead_CreateReadFile(sNewMdl);

        //Check for problems
        bool bOpen = true;
        bool bWrongGame = false;
        if(hFile == INVALID_HANDLE_VALUE)
            bOpen = false;
        if(bOpen){
            std::vector<char> cBinary(4,0);
            bead_ReadFile(hFile, cBinary, 4);
            /// Make sure that what we've read is a binary .mdl as far as we can tell
            if(cBinary[0]!='\0' || cBinary[1]!='\0' || cBinary[2]!='\0' || cBinary[3]!='\0') bOpen = false;

        }

        /// If we pass, then the file is definitely ready to be read.
        if(bOpen){
            std::vector<char> cBinary(17,0);
            bead_ReadFile(hFile, cBinary, 16);
            unsigned nFunctionPointer = * (unsigned *) &cBinary.at(12);

            if(!((nFunctionPointer == FN_PTR_PC_K2_MODEL_1 || nFunctionPointer == FN_PTR_XBOX_K2_MODEL_1) && curmdl.bK2) &&
               !((nFunctionPointer == FN_PTR_PC_K1_MODEL_1 || nFunctionPointer == FN_PTR_XBOX_K1_MODEL_1) && !curmdl.bK2)){
                bOpen = false;
                bWrongGame = true;
            }
        }
        bool bReturn = true;
        if(bOpen){
            Supermodel.reset(new MDL);
            ReportMdl << "Reading supermodel: \n" << to_ansi(sNewMdl.c_str()) << "\n";
            unsigned long length = bead_GetFileLength(hFile);
            std::vector<char> & sBufferRef = Supermodel->CreateBuffer(length);
            bead_ReadFile(hFile, sBufferRef);
            Supermodel->SetFilePath(sNewMdl);
            CloseHandle(hFile);

            Supermodel->DecompileModel(true);
            return true;
        }
        else{
            CloseHandle(hFile);
            if(bWrongGame) Warning(L"Binary supermodel " + std::wstring(sNewMdl.c_str()) + L" belongs to " + (curmdl.bK2 ? L"K1" : L"K2") + L", while the model is being loaded for " + (curmdl.bK2 ? L"K2" : L"K1") + L". "
                                   L"The supermodel must belong to the game you are targeting, otherwise the supermodel animations will not work on the new model! The model will now be loaded without its supermodel.");
            else Warning(L"Could not find binary supermodel " + std::wstring(sNewMdl.c_str()) + L" in the directory! "
                         L"The new model must be compiled with access to its supermodel, otherwise the supermodel animations will not work on the new model! The model will now be loaded without its supermodel.");
            return false;
        }
    }
    return false;
}

/// This function is to be used both when compiling and decompiling (to determine smoothing groups)
/// It is very expensive, so modify with care to keep it efficient. Any calculations that can be performed outside, should be.
void MDL::CreatePatches(){
    FileHeader & Data = *FH;
    Patch::ptr_mdl = this;
    ReportObject ReportMdl(*this);
    Timer tPatches;
    extern bool bCancelSG;

    Report("Building Patch array...");
    ReportMdl << "Building Patch array...";
    //std::cout << " (this may take a while)";
    ReportMdl << "\n";

    /// Start algorithm
    /** KOTORMAX STYLE **/
    /// First record number of verts
    for(Node & node : Data.MH.ArrayOfNodes) if(node.Head.nType & NODE_MESH) {
        /// Record vert and tvert sizes
        Data.MH.nTotalVertCount += node.Mesh.Vertices.size();
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1) Data.MH.nTotalTangent1Count += node.Mesh.Vertices.size();
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2) Data.MH.nTotalTangent2Count += node.Mesh.Vertices.size();
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3) Data.MH.nTotalTangent3Count += node.Mesh.Vertices.size();
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4) Data.MH.nTotalTangent4Count += node.Mesh.Vertices.size();
    }
    std::vector<Vector> DoneVerts;
    DoneVerts.reserve(Data.MH.nTotalVertCount);
    Data.MH.PatchArrayPointers.reserve(Data.MH.nTotalVertCount);

    ProgressSize(0, 100);
    unsigned long nStepper = 0;
    unsigned nUnit = std::max(1, Data.MH.nTotalVertCount / 100);
    ProgressPos(0);
    /// Currently, this takes all meshes, including skins, danglymeshes and walkmeshes, with render on or off
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);
        if(node.Head.nType & NODE_MESH && !(node.Head.nType & NODE_SABER)){

            /// For every vertex of every mesh
            for(int v = 0; v < node.Mesh.Vertices.size(); v++){
                Vertex & vert = node.Mesh.Vertices.at(v);
                Vector & vCoords = vert.vFromRoot;

                /// Get the index of the vert in DoneVerts, in case it's already done.
                auto iter = std::find_if(DoneVerts.begin(), DoneVerts.end(), [&](Vector vComp){ return vComp.Compare(vCoords, 0.0005); });
                long nIndex = std::distance(DoneVerts.begin(), iter);
                bool bFound = !(iter == DoneVerts.end());

                /// If the vert location hasn't been processed yet, then we need a new patchGroup
                if(!bFound){
                    /// Record group
                    Data.MH.PatchArrayPointers.resize(Data.MH.PatchArrayPointers.size() + 1);
                    Data.MH.PatchArrayPointers.back().reserve(50);
                    DoneVerts.push_back(vCoords);
                }
                /// Get the patch group we are currently working on
                std::vector<Patch> & CurrentPatchGroup = (!bFound ? Data.MH.PatchArrayPointers.back() : Data.MH.PatchArrayPointers.at(nIndex));
                vert.nLinkedFacesIndex = nIndex;

                /// Create a new patch for this vert
                Patch CurrentPatch;
                CurrentPatch.nNodeArrayIndex = n;
                CurrentPatch.nVertex = v;
                CurrentPatch.nPatchGroup = (int) nIndex;

                /// For every face, check if it references our vertex, if it does, add its index and SGs to the patch being created.
                for(int f = 0; f < node.Mesh.Faces.size(); f++){
                    Face & face = node.Mesh.Faces.at(f);
                    for(MdlInteger<unsigned short> & fv : face.nIndexVertex) if(fv == v){
                        CurrentPatch.FaceIndices.push_back(f);
                        CurrentPatch.nSmoothingGroups = (CurrentPatch.nSmoothingGroups | face.nSmoothingGroup);
                        break;
                    }
                    /// This is the most embedded level, if ESC then abort here
                    if(bCancelSG) return;
                }

                /// Record the patch
                CurrentPatchGroup.push_back(std::move(CurrentPatch));

                nStepper++;
                //std::cout << "About to progress... nUnit: " << nUnit << "\n";
                if(nStepper % nUnit == 0){
                    //std::cout << "Progressing\n";
                    ProgressStepIt();
                }
            }
        }
    }
    /** MDLEDIT STYLE
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        /// Currently, this takes all meshes, including skins, danglymeshes and walkmeshes, with render on or off
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_MESH && !(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_SABER)){
            Node & node = Data.MH.ArrayOfNodes.at(n);

            /// Record vert and tvert sizes
            Data.MH.nTotalVertCount += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1) Data.MH.nTotalTangent1Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2) Data.MH.nTotalTangent2Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3) Data.MH.nTotalTangent3Count += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4) Data.MH.nTotalTangent4Count += node.Mesh.Vertices.size();

            /// For every vertex of every mesh
            for(int v = 0; v < node.Mesh.Vertices.size(); v++){
                /// Proceed only if this vertex hasn't been processed yet
                if(node.Mesh.Vertices.at(v).nLinkedFacesIndex == -1){
                    Vertex & vert = node.Mesh.Vertices.at(v);
                    Vector & vCoords = vert.vFromRoot;

                    /// Create new vector
                    std::vector<Patch> CurrentPatchGroup;

                    /// We've already gone through the nodes up to n and linked any vertices, so we can skip those
                    for(int n2 = n; n2 < Data.MH.ArrayOfNodes.size(); n2++){
                        if(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_MESH && !(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_SABER)){
                            Node & node2 = Data.MH.ArrayOfNodes.at(n2);

                            /// Loop through all the verts in the mesh and look for matching vertices - theoretically there is no way to optimize this part
                            for(int v2 = v; v2 < node2.Mesh.Vertices.size(); v2++){
                                /// Skip if this vertex has been processed
                                if(node2.Mesh.Vertices.at(v2).nLinkedFacesIndex >= 0) continue;

                                /// Check if vertices are equal (enough)
                                Vector & vCoords2 = node2.Mesh.Vertices.at(v2).vFromRoot;
                                if(vCoords.Compare(vCoords2, 0.0001)){ /// 20180623bead-v: changed precision to four places.
                                    /// Update reference to new vector
                                    node2.Mesh.Vertices.at(v2).nLinkedFacesIndex = Data.MH.PatchArrayPointers.size();

                                    /// Build patch and add it to the group
                                    Patch OtherPatch;
                                    OtherPatch.nNodeArrayIndex = n2;
                                    OtherPatch.nVertex = v2;
                                    for(int f = 0; f < node2.Mesh.Faces.size(); f++){
                                        Face & face = node2.Mesh.Faces.at(f);

                                        /// We are now checking the three vert indices
                                        for(int i = 0; i < 3; i++){
                                            if(face.nIndexVertex.at(i) == v2){
                                                OtherPatch.FaceIndices.push_back(f);
                                                OtherPatch.nSmoothingGroups = OtherPatch.nSmoothingGroups | face.nSmoothingGroup;
                                            }

                                            /// This is the most embedded level, if ESC then abort here
                                            if(bCancelSG) return;
                                        }
                                    }
                                    CurrentPatchGroup.push_back(std::move(OtherPatch));
                                }
                            }
                        }
                    }

                    /// Record group
                    Data.MH.PatchArrayPointers.push_back(std::move(CurrentPatchGroup));
                }
            }
            ProgressPos(n);
        }
    }
    **/

    /// Algorithm done
    ProgressPos(Data.MH.nTotalVertCount);
    int nPatches = 0;
    for(std::vector<Patch> & patch : Data.MH.PatchArrayPointers) nPatches += patch.size();
    ReportMdl << "Done creating patches in " << tPatches.GetTime() << ". Found " << nPatches << " patches!\n"; //"! Total: " << nPatches << ", vs " << Data.MH.nTotalVertCount << " verts \n";
}

std::vector<Patch> & Patch::GetPatchGroup(){
    if(!ptr_mdl || !nPatchGroup.Valid()) throw mdlexception("Patch::GetPatchGroup() error: either the model pointer is null or the patch group index is -1.");
    if(!ptr_mdl->GetFileData()) throw mdlexception("Patch::GetPatchGroup() error: the file data is null.");
    FileHeader & Data = *ptr_mdl->GetFileData();
    return Data.MH.PatchArrayPointers.at(nPatchGroup);
}

Node & Patch::GetNode(){
    if(!ptr_mdl || !nNodeArrayIndex.Valid()) throw mdlexception("Patch::GetNode() error: either the model pointer is null or the node array index is -1.");
    if(!ptr_mdl->GetFileData()) throw mdlexception("Patch::GetPatchGroup() error: the file data is null.");
    FileHeader & Data = *ptr_mdl->GetFileData();
    return Data.MH.ArrayOfNodes.at(nNodeArrayIndex);
}

bool Patch::CompareNormal(){
    Node & node = GetNode();
    Vertex & vert = node.Mesh.Vertices.at(nVertex);
    if(fDiff == 0.0) throw mdlexception("Patch::CompareNormal() error: fDiff is still 0.0!");
    if(vVertexNormal.Compare(vert.MDXData.vNormal, fDiff)) return true;
    return false;
}

bool Patch::CompareTangentSpace(char cVec){
    Node & node = GetNode();
    Vertex & vert = node.Mesh.Vertices.at(nVertex);
    if(fDiff == 0.0) throw mdlexception("Patch::CompareNormal() error: fDiff is still 0.0!");
    if(cVec == TS_BITANGENT && vVertexB.Compare(vert.MDXData.vTangent1.at(0), fDiff)) return true;
    if(cVec == TS_TANGENT && vVertexT.Compare(vert.MDXData.vTangent1.at(1), fDiff)) return true;
    if(cVec == TS_NORMAL && vVertexN.Compare(vert.MDXData.vTangent1.at(2), fDiff)) return true;
    return false;
}

void Patch::CalculateWorld(bool bNormal, bool bTangent){
    if(!bNormal && !bTangent) return;
    Node & patch_node = GetNode();
    Vertex & patch_vert = patch_node.Mesh.Vertices.at(nVertex);

    /// Go through all faces in this patch and add up their face normals and face tangent space vectors.
    /// They will form the base vectors that will later be used to calculate the actual vectors.
    /// The faces within the same patch will always smooth between each other
    /// and must therefore all be included. The face normals will be freshly calculated.
    for(int face_ind : FaceIndices){
        Face & face = patch_node.Mesh.Faces.at(face_ind);

        /// Get the verts
        Vertex & v1 = patch_node.Mesh.Vertices.at(face.nIndexVertex[0]);
        Vertex & v2 = patch_node.Mesh.Vertices.at(face.nIndexVertex[1]);
        Vertex & v3 = patch_node.Mesh.Vertices.at(face.nIndexVertex[2]);

        /// Definitely use world space coordinates
        Vector Edge1 = v2.vFromRoot - v1.vFromRoot;
        Vector Edge2 = v3.vFromRoot - v1.vFromRoot;
        Vector Edge3 = v3.vFromRoot - v2.vFromRoot;

        /// Mark bad patch
        if(face.fAreaUV <= 0.0) bBadUV = true;
        if(face.fArea <= 0.0) bBadGeo = true;

        /// Prepare face normal candidate
        Vector vAdd = cross(Edge1, Edge2); //Cross product, unnormalized
        vAdd.Normalize();

        /// Area Weighting
        if(ptr_mdl->bSmoothAreaWeighting) vAdd *= (face.fArea > 0.0 ? face.fArea : 0.0);

        /// Angle Weighting
        if(ptr_mdl->bSmoothAngleWeighting){
            if(nVertex == face.nIndexVertex[0]){
                vAdd *= Angle(Edge1, Edge2);
            }
            else if(nVertex == face.nIndexVertex[1]){
                vAdd *= Angle(Edge1, Edge3);
            }
            else if(nVertex == face.nIndexVertex[2]){
                vAdd *= Angle(Edge2, Edge3);
            }
        }

        /// Crease Angle Test
        /// if enabled, and the angle between the current normal and our candidate is greater than the crease angle, skip it
        /** THIS IS POSSIBLY NOT A GOOD IDEA, BECAUSE THE NORMALS WITHIN THE SAME PATCH SHOULD ALL BE REPRESENTED, NO? **/
        if(ptr_mdl->bCreaseAngle && Angle(vWorldNormal, vAdd) > static_cast<double>(ptr_mdl->nCreaseAngle)) continue;

        /// Normal incorporation
        if(bNormal) vWorldNormal += vAdd;
        //if(bDebug) file << "\r\n    Added component for face " << face_ind << ": " << vAdd.Print();

        /// Normalize the normal candidate for use in further calculations
        vAdd.Normalize();

        if(bTangent){
            /// Also incorporate tangent space vectors
            Vector & v1UV = v1.MDXData.vUV1;
            Vector & v2UV = v2.MDXData.vUV1;
            Vector & v3UV = v3.MDXData.vUV1;
            Vector EUV1 = v2UV - v1UV;
            Vector EUV2 = v3UV - v1UV;
            Vector EUV3 = v3UV - v2UV;

            /// Tangent and Bitangent calculation
            /// Now comes the calculation. Will be using edges 1 and 2
            double r = (EUV1.fX * EUV2.fY - EUV1.fY * EUV2.fX);

            if(r != 0){ /// This is division, need to check for 0
                r = 1.0 / r;
            }
            else{
                /**
                It can be 0 in several ways.
                1. any of the two edges is zero (ie. we're dealing with a line, not a triangle) - this happens
                2. both x's or both y's are zero, implying parallel edges, but we cannot have any in a triangle
                3. both x's are the same and both y's are the same, therefore they have the same angle and are parallel
                4. both edges have the same x and y, they both have a 45° angle and are therefore parallel

                Pretty much the only one relevant to us is the first case.
                /**/

                /// ndix UR's magic factor
                r = 2406.6388;
            }
            Vector vAddT = r * (Edge1 * EUV2.fY - Edge2 * EUV1.fY);
            Vector vAddB = r * (Edge2 * EUV1.fX - Edge1 * EUV2.fX);
            vAddT.Normalize();
            vAddB.Normalize();
            if(vAddT.Null()){
                vAddT = Vector(1.0, 0.0, 0.0);
            }
            if(vAddB.Null()){
                vAddB = Vector(1.0, 0.0, 0.0);
            }

            /// Handedness
            Vector vCross = cross(vAdd, vAddT);
            double fDot = dot(vCross, vAddB);
            if(fDot > 0.0000000001){
                vAddT *= -1.0;
            }

            /// Now check if we need to invert  T and B. But first we need a UV normal
            Vector vNormalUV = cross(EUV1, EUV2); //cross product
            if(vNormalUV.fZ < 0.0){
                vAddT *= -1.0;
                vAddB *= -1.0;
            }

            vWorldT += vAddT;
            vWorldB += vAddB;
            vWorldN += cross(vAddB, vAddT);
        }
    }
}

void Patch::Calculate(bool bNormal, bool bTangent, std::vector<MdlInteger<unsigned int>> * patches){
    if(!bNormal && !bTangent) return;
    if(!patches) patches = &SmoothedPatches;

    std::vector<Patch> & patch_group = GetPatchGroup();
    std::vector<MdlInteger<unsigned int>> & smoothed_patches = *patches;
    Node & node = GetNode();

    /// Define our candidates
    Vector vWorking, vWorkingTST, vWorkingTSB, vWorkingTSN;

    /// Reset group flags
    bGroupBadGeo = false;
    bGroupBadUV = false;

    /// The new algorithm
    /// Go through our patches
    std::vector<int> CheckedPatches;
    CheckedPatches.reserve(smoothed_patches.size());
    for(int n = 0; n < smoothed_patches.size(); n++){
        Patch & curpatch = patch_group.at(smoothed_patches.at(n));

        /// If we have done this patch already, continue
        if(std::find(CheckedPatches.begin(), CheckedPatches.end(), curpatch.nNodeArrayIndex) != CheckedPatches.end()) continue;

        /// Add the node index to checked patches
        CheckedPatches.push_back(curpatch.nNodeArrayIndex);

        /// The counter of patch normals that will be united in this step.
        /// The counter does not yet include the current patch, it will be added later.
        int nNormals = 0;

        /// Define the vectors to get the mesh normal and TS vectors for this step
        Vector vMeshNormal, vMeshTST, vMeshTSB, vMeshTSN;

        /// Go through the remaining patches (including this one!)
        for(int n2 = n; n2 < smoothed_patches.size(); n2++){
            Patch & curpatch2 = patch_group.at(smoothed_patches.at(n2));

            /// If this patch belongs to a different node, continue
            if(curpatch.nNodeArrayIndex != curpatch2.nNodeArrayIndex) continue;

            /// Make sure to set the group flags if necessary
            if(!bGroupBadGeo && curpatch2.bBadGeo) bGroupBadGeo = true;
            if(!bGroupBadUV && curpatch2.bBadUV) bGroupBadUV = true;

            /// Once we get to this point, we have to collect the normal and TS vectors. Increase the counter.
            nNormals++;

            /// Collect the normal and TS vectors
            if(bNormal) vMeshNormal += curpatch2.vWorldNormal;
            if(bTangent) vMeshTSB += curpatch2.vWorldB;
            if(bTangent) vMeshTST += curpatch2.vWorldT;
            if(bTangent) vMeshTSN += curpatch2.vWorldN;
        }

        /// Once we have gone through all the patches in the current setup, pack them up and add them to the working normal
        if(bNormal) vWorking += (nNormals * vMeshNormal / vMeshNormal.GetLength());
        if(bTangent) vWorkingTSB += (vMeshTSB / vMeshTSB.GetLength());
        if(bTangent) vWorkingTST += (vMeshTST / vMeshTST.GetLength());
        if(bTangent) vWorkingTSN += (vMeshTSN / vMeshTSN.GetLength());
    }

    /// Rotate the normal to object space
    if(bNormal) vWorking.Rotate(node.Head.qFromRoot.inverse());
    if(bTangent) vWorkingTSB.Rotate(node.Head.qFromRoot.inverse());
    if(bTangent) vWorkingTST.Rotate(node.Head.qFromRoot.inverse());
    if(bTangent) vWorkingTSN.Rotate(node.Head.qFromRoot.inverse());

    /// Normalize the vectors
    if(bNormal) vWorking.Normalize();
    if(bTangent) vWorkingTSB.Normalize();
    if(bTangent) vWorkingTST.Normalize();
    if(bTangent) vWorkingTSN.Normalize();

    /// Apply the candidates
    if(bNormal) vVertexNormal = vWorking;
    if(bTangent) vVertexB = vWorkingTSB;
    if(bTangent) vVertexT = vWorkingTST;
    if(bTangent) vVertexN = vWorkingTSN;
}

unsigned MDL::GetHeaderOffset(const Node & node, unsigned short nHeader){
    unsigned nReturn = 0;

    if(nHeader == NODE_HEADER) return nReturn;
    else if(node.Head.nType & NODE_HEADER) nReturn += NODE_SIZE_HEADER;
    if(nHeader == NODE_LIGHT) return nReturn;
    else if(node.Head.nType & NODE_LIGHT) nReturn += NODE_SIZE_LIGHT;
    if(nHeader == NODE_EMITTER) return nReturn;
    else if(node.Head.nType & NODE_EMITTER) nReturn += NODE_SIZE_EMITTER;
    if(nHeader == NODE_REFERENCE) return nReturn;
    else if(node.Head.nType & NODE_REFERENCE) nReturn += NODE_SIZE_REFERENCE;
    if(nHeader == NODE_MESH) return nReturn;
    else if(node.Head.nType & NODE_MESH){
        nReturn += NODE_SIZE_MESH;
        if(bXbox) nReturn -= 4;
        if(!bK2) nReturn -= 8;
    }
    if(nHeader == NODE_SKIN) return nReturn;
    else if(node.Head.nType & NODE_SKIN) nReturn += NODE_SIZE_SKIN;
    if(nHeader == NODE_DANGLY) return nReturn;
    else if(node.Head.nType & NODE_DANGLY) nReturn += NODE_SIZE_DANGLY;
    if(nHeader == NODE_AABB) return nReturn;
    else if(node.Head.nType & NODE_AABB) nReturn += NODE_SIZE_AABB;
    if(nHeader == NODE_SABER) return nReturn;
    else if(node.Head.nType & NODE_SABER) nReturn += NODE_SIZE_SABER;
}

bool IsMaterialWalkable(int nMat){
    if(nMat == MATERIAL_NONWALK ||
       nMat == MATERIAL_OBSCURING ||
       nMat == MATERIAL_TRANSPARENT ||
       nMat == MATERIAL_LAVA ||
       //nMat == MATERIAL_BOTTOMLESSPIT ||
       nMat == MATERIAL_DEEPWATER ||
       nMat == MATERIAL_SNOW) return false;
    return true;
}

//This function together with the next one, checks the currently loaded data in MDL for any special properties
void MDL::CheckPeculiarities(){
    ReportObject ReportMdl(*this);
    FileHeader & Data = *FH;
    std::stringstream ssReturn;
    bool bUpdate = false;
    Report("Checking for peculiarities...");
    ssReturn << "Lucky you! Your model has some peculiarities:";
    if(!Data.MH.GH.RuntimeArray1.empty()){
        ssReturn << "\r\n - First empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(!Data.MH.GH.RuntimeArray2.empty()){
        ssReturn << "\r\n - Second empty runtime array in the GH has a some nonzero value!";
        bUpdate = true;
    }
    if(Data.MH.GH.nRefCount != 0){
        ssReturn << "\r\n - RefCount has a value!";
        bUpdate = true;
    }
    if(Data.MH.GH.nModelType != 2){
        ssReturn << "\r\n - Header ModelType different than 2!";
        bUpdate = true;
    }
    if(Data.MH.nUnknown != 0 && !bWriteSmoothing){
        ssReturn << "\r\n - Second classification padding number different than 0!";
        bUpdate = true;
    }
    if(Data.MH.nChildModelCount != 0){
        ssReturn << "\r\n - ChildModelCount has a value!";
        bUpdate = true;
    }
    if(Data.MH.AnimationArray.GetDoCountsDiffer()){
        ssReturn << "\r\n - AnimationArray counts differ!";
        bUpdate = true;
    }
    if(Data.MH.nPadding != 0){
        ssReturn << "\r\n - Unknown int32 after the Root Head Node Offset in the MH has a value!";
        bUpdate = true;
    }
    if(Data.MH.nMdxLength2 != Data.nMdxLength){
        ssReturn << "\r\n - MdxLength in FH and MdxLength2 in MH don't have the same value!";
        bUpdate = true;
    }
    if(Data.MH.nOffsetIntoMdx != 0){
        ssReturn << "\r\n - OffsetIntoMdx after the MdxLength2 in the MH has a value!";
        bUpdate = true;
    }
    if(Data.MH.NameArray.GetDoCountsDiffer()){
        ssReturn << "\r\n - NameArray counts differ!";
        bUpdate = true;
    }
    for(int a = 0; a < Data.MH.AnimationArray.nCount; a++){
        if(!Data.MH.GH.RuntimeArray1.empty()){
            ssReturn << "\r\n   - First empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(!Data.MH.GH.RuntimeArray2.empty()){
            ssReturn << "\r\n   - Second empty runtime array in the Animation GH has a some nonzero value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nRefCount != 0){
            ssReturn << "\r\n   - Animation counterpart to RefCount has a value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).EventArray.GetDoCountsDiffer()){
            ssReturn << "\r\n   - EventArray counts differ!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nPadding2 != 0){
            ssReturn << "\r\n   - Unknown int32 after EventArrayHead has a value!";
            bUpdate = true;
        }
        if(Data.MH.Animations.at(a).nModelType != 5){
            ssReturn << "\r\n   - Animation ModelType is not 5!";
            bUpdate = true;
        }
        if(CheckNodes(Data.MH.Animations.at(a).ArrayOfNodes, ssReturn, a)) bUpdate = true;
    }
    if(CheckNodes(Data.MH.ArrayOfNodes, ssReturn, -1)) bUpdate = true;
    if(!bUpdate){
        ReportMdl << "Checked for peculiarities, nothing to report.\n";
        return;
    }
    MessageBox(NULL, ssReturn.str().c_str(), "Notification", MB_OK);
}

bool MDL::CheckNodes(std::vector<Node> & NodeArray, std::stringstream & ssReturn, MdlInteger<unsigned int> nAnimation){
    bool bMasterUpdate = false;
    for(int b = 0; b < NodeArray.size(); b++){
        if(NodeArray.at(b).Head.nType == 0){
            //Ghost node
        }
        else if(!(NodeArray.at(b).Head.nType == NODE_HEADER ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_LIGHT ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_EMITTER ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_REFERENCE ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_MESH ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_MESH | NODE_SKIN ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_MESH | NODE_DANGLY ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_MESH | NODE_AABB ||
                  NodeArray.at(b).Head.nType == NODE_HEADER | NODE_MESH | NODE_SABER ) )
        {
            ssReturn << "\r\n     - Unknown node type: " << NodeArray.at(b).Head.nType << "!";
            bMasterUpdate = true;
        }
        else{
            bool bUpdate = false;
            std::stringstream ssAdd;
            std::string sCont;
            if(!nAnimation.Valid()) sCont = "geometry";
            else sCont = FH->MH.Animations.at(nAnimation).sName.c_str();
            ssAdd << "\r\n - " << FH->MH.Names.at(NodeArray.at(b).Head.nNameIndex).sName << " (" << sCont << ")";
            if(NodeArray.at(b).Head.nType & NODE_HEADER){
                if(NodeArray.at(b).Head.nPadding1 != 0){
                    ssAdd << "\r\n     - Header: Unknown short after NameIndex has a value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ChildrenArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Header: ChildArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ControllerArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Header: ControllerArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Head.ControllerDataArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Header: ControllerDataArray counts differ!";
                    bUpdate = true;
                }
                for(int c = 0; c < NodeArray.at(b).Head.Controllers.size(); c++){
                    Controller & ctrl = NodeArray.at(b).Head.Controllers.at(c);

                    if( (ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                         ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION) &&
                        ctrl.nAnimation.Valid() &&
                        ctrl.nUnknown2 == ctrl.nControllerType + 8){}
                    else if(!ctrl.nUnknown2.Valid() &&
                            ( (ctrl.nControllerType != CONTROLLER_HEADER_POSITION &&
                               ctrl.nControllerType != CONTROLLER_HEADER_ORIENTATION) ||
                              !ctrl.nAnimation.Valid()) ){}
                    else{
                        std::string sLoc;
                        if(!ctrl.nAnimation.Valid()) sLoc = "geometry";
                        else sLoc = FH->MH.Animations.at(ctrl.nAnimation).sName.c_str();
                        ssAdd << "\r\n     - Header: New controller unknown2 value! (" << ctrl.nUnknown2 << " - " << ReturnControllerName(ctrl.nControllerType, FH->MH.ArrayOfNodes.at(ctrl.nNameIndex).Head.nType) << " controller in node " << FH->MH.Names.at(ctrl.nNameIndex).sName << " in " << sLoc << ")";
                        bUpdate = true;
                    }
                    /***
                        This if for checking controller "padding" values. These numbers are in no way random.
                        Header and light controllers always have 0 for the third number, while emitter and mesh controllers have it greater than 0.
                        In keyed controllers, light and emitter seem to group together against header and mesh.
                        Selfillumcolor usually has the same padding values as scale, but they are different
                        in for example: n_admrlsaulkar or 003ebof
                    /** /
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
                              GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_MESH) && ctrl.nAnimation != -1 &&
                             (ctrl.nPadding[0] == 50 &&
                              ctrl.nPadding[1] == 18 &&
                              ctrl.nPadding[2] == 0   )){}
                    else if(  ctrl.nAnimation != -1 &&
                             (ctrl.nPadding[0] == 51 &&
                              ctrl.nPadding[1] == 18 &&
                              ctrl.nPadding[2] == 0   )){}
                    /// the following are all emitter and mesh single controllers (as long as the last value is non-0)
                    else if( (GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_EMITTER ||
                              GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_MESH) &&
                              ctrl.nPadding[2] > 0 && ctrl.nAnimation == -1 ){}
                    else{
                        //ssAdd << "\r\n     - Header: Previously unseen controller padding! (" << (int)ctrl.nPadding[0] << ", " << (int)ctrl.nPadding[1] << ", " << (int)ctrl.nPadding[2] << ")";
                        //bUpdate = true;
                    }
                    /**/
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_LIGHT){
                if(NodeArray.at(b).Light.UnknownArray.nCount != 0){
                    ssAdd << "\r\n     - Light: Unknown array not empty!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareSizeArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Light: FlareSizeArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlarePositionArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Light: FlarePositionArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareColorShiftArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Light: FlareColorShiftArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Light.FlareTextureNameArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Light: FlareTextureNameArray counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_EMITTER){
                /*
                if(NodeArray.at(b).Emitter.nUnknown1 != 0){
                    ssAdd << "\r\n     - Emitter: Unknown short after Loop has a value!";
                    bUpdate = true;
                }
                */
            }
            if(NodeArray.at(b).Head.nType & NODE_MESH){
                if(NodeArray.at(b).Mesh.nUnknown3[1] != -1 || NodeArray.at(b).Mesh.nUnknown3[2] != 0){
                    ssAdd << "\r\n     - Mesh: The unknown -1 -1 0 array has a different value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.FaceArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Mesh: FaceArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.IndexCounterArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Mesh: IndexCounterArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.IndexLocationArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Mesh: IndexLocationArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Mesh.MeshInvertedCounterArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Mesh: MeshInvertedCounterArray counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_SKIN){
                if(!NodeArray.at(b).Skin.UnknownArray.empty()){
                    ssAdd << "\r\n     - Skin: Unknown empty array has some nonzero value!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.QBoneArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Skin: QBoneArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.TBoneArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Skin: TBoneArray counts differ!";
                    bUpdate = true;
                }
                if(NodeArray.at(b).Skin.Array8Array.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Skin: Array8Array counts differ!";
                    bUpdate = true;
                }
            }
            if(NodeArray.at(b).Head.nType & NODE_DANGLY){
                if(NodeArray.at(b).Dangly.ConstraintArray.GetDoCountsDiffer()){
                    ssAdd << "\r\n     - Dangly: ConstraintArray counts differ!";
                    bUpdate = true;
                }
            }
            if(bUpdate){
                bMasterUpdate = true;
                ssReturn << ssAdd.str();
            }
        }
    }
    return bMasterUpdate;
}


std::string ReturnClassificationName(int nClassification){
    switch(nClassification){
        case CLASS_OTHER: return "other";
        case CLASS_EFFECT: return "effect";
        case CLASS_TILE: return "tile";
        case CLASS_CHARACTER: return "character";
        case CLASS_DOOR: return "door";
        case CLASS_LIGHTSABER: return "lightsaber";
        case CLASS_PLACEABLE: return "placeable";
        case CLASS_FLYER: return "flyer";
    }
    std::cout << "ReturnClassification() ERROR: Unknown classification " << nClassification << ".\n";
    return "unknown";
}

int ReturnController(std::string sController, int nType){
    if(sController == "position") return CONTROLLER_HEADER_POSITION;
    else if(sController == "orientation") return CONTROLLER_HEADER_ORIENTATION;
    else if(sController == "scale") return CONTROLLER_HEADER_SCALING;
    else if(nType & NODE_LIGHT){
        if(sController == "color") return CONTROLLER_LIGHT_COLOR;
        else if(sController == "radius") return CONTROLLER_LIGHT_RADIUS;
        else if(sController == "shadowradius") return CONTROLLER_LIGHT_SHADOWRADIUS;          //Missing from NWmax
        else if(sController == "verticaldisplacement") return CONTROLLER_LIGHT_VERTICALDISPLACEMENT;  //Missing from NWmax
        else if(sController == "multiplier") return CONTROLLER_LIGHT_MULTIPLIER;
    }
    else if(nType & NODE_EMITTER){
        if(sController == "alphaend") return CONTROLLER_EMITTER_ALPHAEND;
        else if(sController == "alphastart") return CONTROLLER_EMITTER_ALPHASTART;
        else if(sController == "birthrate") return CONTROLLER_EMITTER_BIRTHRATE;
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
    }
    else if(nType & NODE_MESH){
        if(sController == "selfillumcolor") return CONTROLLER_MESH_SELFILLUMCOLOR;
        else if(sController == "alpha") return CONTROLLER_MESH_ALPHA;
    }
    return 0;
}

std::string ReturnControllerName(int nController, int nType){
    switch(nController){
        case CONTROLLER_HEADER_POSITION:            return "position";
        case CONTROLLER_HEADER_ORIENTATION:         return "orientation";
        case CONTROLLER_HEADER_SCALING:             return "scale";
    }

    if(nType & NODE_LIGHT){
        switch(nController){
        case CONTROLLER_LIGHT_COLOR:                return "color";
        case CONTROLLER_LIGHT_RADIUS:               return "radius";
        case CONTROLLER_LIGHT_SHADOWRADIUS:         return "shadowradius";          //Missing from NWmax
        case CONTROLLER_LIGHT_VERTICALDISPLACEMENT: return "verticaldisplacement";  //Missing from NWmax
        case CONTROLLER_LIGHT_MULTIPLIER:           return "multiplier";
        }
    }
    else if(nType & NODE_EMITTER){
        switch(nController){
        case CONTROLLER_EMITTER_ALPHAEND:           return "alphaEnd";
        case CONTROLLER_EMITTER_ALPHASTART:         return "alphaStart";
        case CONTROLLER_EMITTER_BIRTHRATE:          return "birthrate";
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
    std::cout << "ReturnController() ERROR: Unknown controller " << nController << " (type " << nType << ").\n";
    return "unknown";
}
