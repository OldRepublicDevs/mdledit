#include "MDL.h"
#include <iomanip>

/**
    Functions:
    AddMenuLines() //frame.h
    Append() //Helper
    AppendAabb() //Helper
    AppendChildren() //Helper
    BuildTree() //frame.h
    DetermineDisplayText //frame.h
/**/

void AddMenuLines(std::vector<std::string>cItem, LPARAM lParam, MenuLineAdder * pmla, int nFile){

    if(cItem.at(0) == "") return;

    /// Header
    else if(cItem.at(0) == "Header" && nFile == 0){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    /// Geometry Node
    else if((cItem.at(1) == "Geometry") || ((cItem.at(3) == "Geometry") && ((cItem.at(1) == "Children") || (cItem.at(1) == "Parent")))){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_VIEW_ASCII, "View ASCII");
        pmla->nIndex++;
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    /// Animation
    else if(cItem.at(1) == "Animations"){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_VIEW_ASCII, "View ASCII");
        pmla->nIndex++;
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    /// Animation Node
    else if((cItem.at(2) == "Animations") || ((cItem.at(4) == "Animations") && ((cItem.at(1) == "Children") || (cItem.at(1) == "Parent")))){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_VIEW_ASCII, "View ASCII");
        pmla->nIndex++;
    }
    /// Controllers
    else if(cItem.at(1) == "Controllers"){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_VIEW_ASCII, "View ASCII");
        pmla->nIndex++;
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    /// Node subtypes and arrays
    else if(cItem.at(0) == "Light" ||
            cItem.at(1) == "Lens Flares" ||
            cItem.at(0) == "Emitter" ||
            cItem.at(0) == "Reference" ||
            cItem.at(0) == "Mesh" ||
            cItem.at(1) == "Vertices" && nFile == 0 ||
            cItem.at(1) == "Faces" && nFile == 0 ||
            cItem.at(1) == "Bones" ||
            cItem.at(0) == "Danglymesh" ||
            cItem.at(1) == "Danglymesh" ||
            cItem.at(0) == "Lightsaber" ||
            cItem.at(1) == "Lightsaber" ){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    else return;
}

HTREEITEM Append(const std::string & sString, LPARAM lParam = NULL, HTREEITEM hParentNew = NULL, HTREEITEM hAfterNew = NULL, UINT Flags = NULL){
    static HTREEITEM hPrev;
    if(sString.empty()) return hPrev;
    static HTREEITEM hParent;
    HTREEITEM hAfter;

    //Determine hParent
    if(hParentNew == NULL) {}
    else hParent = hParentNew;

    //Determine hAfter
    if(hAfterNew == NULL) hAfter = hPrev;
    else hAfter = hAfterNew;

    //Add item
    TVINSERTSTRUCT tvis;
    TVITEMEX * item = &tvis.itemex;
    item->mask = TVIF_TEXT | TVIF_PARAM;
    item->pszText = (char*) sString.c_str();
    item->cchTextMax = sString.size();
    item->lParam = lParam;
    if(Flags != NULL){
        item->mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
        item->state = Flags;
    }
    tvis.hParent = hParent;
    tvis.hInsertAfter = hAfter;
    hPrev = TreeView_InsertItem(hTree, &tvis);
    return hPrev;
}

void AppendAabb(Aabb * AABB, HTREEITEM TopLevel, int & nCount){
    //std::cout << "AppendAabb() DEBUG: the values are: " << AABB->f1 << ", " << AABB->f2 << ", " << AABB->f3 << ".\n";
    char cAabb [255];
    sprintf(cAabb, "aabb %i", nCount);
    Append(cAabb, (LPARAM) AABB, TopLevel);
    nCount++;
    if(AABB->Child1.size() > 0) AppendAabb(&(AABB->Child1.front()), TopLevel, nCount);
    if(AABB->Child2.size() > 0) AppendAabb(&(AABB->Child2.front()), TopLevel, nCount);
}

HTREEITEM AppendChildren(Node & node, HTREEITEM Prev, std::vector<Name> & Names, MDL & Mdl){
        if(node.Head.nType & NODE_LIGHT){
            HTREEITEM Light = Append("Light", (LPARAM) &node, Prev);
            HTREEITEM LensFlares = Append("Lens Flares", (LPARAM) &node.Light, Light);
            int nMaxSize = std::max(node.Light.FlareSizes.size(),
                           std::max(node.Light.FlarePositions.size(),
                           std::max(node.Light.FlareColorShifts.size(),
                                    node.Light.FlareTextureNames.size())));
            for(int n = 0; n < nMaxSize; n++)
                Append("Lens Flare " + std::to_string(n), (LPARAM) &node, LensFlares);
        }
        if(node.Head.nType & NODE_EMITTER){
            HTREEITEM Emitter = Append("Emitter", (LPARAM) &node, Prev);
        }
        if(node.Head.nType & NODE_REFERENCE){
            HTREEITEM Reference = Append("Reference", (LPARAM) &node, Prev);
        }
        if(node.Head.nType & NODE_MESH){
            HTREEITEM Mesh = Append("Mesh", (LPARAM) &node, Prev);
            HTREEITEM Vertices = Append("Vertices", (LPARAM) &node, Mesh);
            if(node.Mesh.Vertices.size() > 0){
                for(int n = 0; n < node.Mesh.Vertices.size(); n++)
                    Append("Vertex " + std::to_string(n), (LPARAM) &(node.Mesh.Vertices[n]), Vertices);
            }
            HTREEITEM Faces = Append("Faces", (LPARAM) &node.Mesh, Mesh);
            if(node.Mesh.Faces.size() > 0){
                for(int n = 0; n < node.Mesh.Faces.size(); n++){
                    Append("Face " + std::to_string(n), (LPARAM) &(node.Mesh.Faces[n]), Faces);
                }
            }
        }
        if(node.Head.nType & NODE_SKIN){
            char cBone [255];
            HTREEITEM Skin = Append("Skin", (LPARAM) &node, Prev);
            HTREEITEM Bones = Append("Bones", (LPARAM) &node.Skin, Skin);
            if(node.Skin.Bones.size() > 0){
                for(int n = 0; n < node.Skin.Bones.size(); n++){
                    std::string sBone = "Bone " + Names[n].sName;
                    Append(sBone, (LPARAM) &(node.Skin.Bones.at(n)), Bones);
                }
            }
        }
        if(node.Head.nType & NODE_DANGLY){
            HTREEITEM Danglymesh = Append("Danglymesh", (LPARAM) &node, Prev);
            for(int i = 0; i < node.Dangly.Constraints.size(); i++)
                Append("Dangly Vertex " + std::to_string(i), (LPARAM) &(node), Danglymesh);
        }
        if(node.Head.nType & NODE_AABB){
            HTREEITEM Walkmesh = Append("Aabb", (LPARAM) &node.Walkmesh, Prev);
            int nCounter = 0;
            if(node.Walkmesh.nOffsetToAabb > 0) AppendAabb(&(node.Walkmesh.RootAabb), Walkmesh, nCounter);
        }
        if(node.Head.nType & NODE_SABER){
            HTREEITEM Saber = Append("Lightsaber", (LPARAM) &node, Prev);
            if(node.Saber.SaberData.size() > 0){
                for(int i = 0; i < node.Saber.SaberData.size(); i++){
                    Append("Lightsaber Vertex " + std::to_string(i), (LPARAM) &(node.Saber.SaberData[i]), Saber);
                }

            }
        }
}

void BuildTree(MDL & Mdl){
    if(!Mdl.GetFileData()){
        std::cout << "No data. Do not build tree.\n";
        return;
    }
    FileHeader & Data = *Mdl.GetFileData();

    HTREEITEM Root = Append(Mdl.GetFilename(), NULL, TVI_ROOT);
    HTREEITEM Header = Append("Header", (LPARAM) &(Data.MH), Root);

    HTREEITEM Animations = Append("Animations", NULL, Root);
    HTREEITEM Nodes, Animation, Events, CurrentNode;
    for(int n = 0; n < Data.MH.Animations.size(); n++){
        Animation = Append(Data.MH.Animations[n].sName, (LPARAM) &(Data.MH.Animations[n]), Animations);
        for(int a = 0; a < Data.MH.Animations[n].ArrayOfNodes.size(); a++){
            Node & node = Data.MH.Animations[n].ArrayOfNodes[a];

            CurrentNode = Append(Data.MH.Names[node.Head.nNodeNumber].sName, (LPARAM) &node, Animation);

            HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
            for(int n = 0; n < node.Head.Controllers.size(); n++){
                int nCtrlIndex = node.Head.Controllers[n].nNodeNumber;
                int nCtrlType = node.Head.Controllers[n].nControllerType;
                Node & ctrlnode = Mdl.GetNodeByNameIndex(nCtrlIndex);
                std::string sName = ReturnControllerName(nCtrlType, ctrlnode.Head.nType);
                if(node.Head.Controllers[n].nColumnCount == 19) sName+="bezierkey";
                else sName+="key";
                Append(sName, (LPARAM) &(node.Head.Controllers[n]), Controllers);
            }

            HTREEITEM Parent = Append("Parent", NULL, CurrentNode);
            if(node.Head.nParentIndex != -1){
                Append(Data.MH.Names[node.Head.nParentIndex].sName, (LPARAM) &Mdl.GetNodeByNameIndex(node.Head.nParentIndex, n), Parent);
            }

            HTREEITEM Children = Append("Children", (LPARAM) &node.Head, CurrentNode);
            for(int g = 0; g < Data.MH.Animations[n].ArrayOfNodes.size(); g++){
                Node & curnode = Data.MH.Animations[n].ArrayOfNodes[g];
                if(curnode.Head.nParentIndex == node.Head.nNodeNumber){
                    Append(Data.MH.Names[curnode.Head.nNodeNumber].sName, (LPARAM) &curnode, Children);
                }
            }
        }
    }

    Nodes = Append("Geometry", NULL, Root);
    for(int a = 0; a < Data.MH.ArrayOfNodes.size(); a++){
        Node & node = Data.MH.ArrayOfNodes[a];
        if(node.Head.nType != 0){
            std::string sType;
            if(     node.Head.nType == (NODE_HEADER | NODE_MESH | NODE_SABER)) sType = "(saber) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_MESH | NODE_AABB)) sType = "(aabb) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_MESH | NODE_DANGLY)) sType = "(dangly) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_MESH | NODE_SKIN)) sType = "(skin) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_MESH)) sType = "(mesh) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_REFERENCE)) sType = "(reference) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_EMITTER)) sType = "(emitter) ";
            else if(node.Head.nType == (NODE_HEADER | NODE_LIGHT)) sType = "(light) ";
            else if(node.Head.nType == NODE_HEADER) sType = "(basic) ";
            else sType = "(unknown) ";
            CurrentNode = Append(sType + Data.MH.Names[node.Head.nNodeNumber].sName, (LPARAM) &node, Nodes);

            AppendChildren(node, CurrentNode, Data.MH.Names, Mdl);

            HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
            for(int n = 0; n < node.Head.Controllers.size(); n++){
                Append(ReturnControllerName(node.Head.Controllers[n].nControllerType, node.Head.nType), (LPARAM) &(node.Head.Controllers[n]), Controllers);
            }

            HTREEITEM Parent = Append("Parent", NULL, CurrentNode);
            if(node.Head.nParentIndex != -1){
                Append(Data.MH.Names[node.Head.nParentIndex].sName, (LPARAM) &Mdl.GetNodeByNameIndex(node.Head.nParentIndex, -1), Parent);
            }

            HTREEITEM Children = Append("Children", (LPARAM) &node.Head, CurrentNode);
            for(int g = 0; g < Data.MH.ArrayOfNodes.size(); g++){
                Node & curnode = Data.MH.ArrayOfNodes[g];
                if(curnode.Head.nParentIndex == node.Head.nNodeNumber){
                    Append(Data.MH.Names[curnode.Head.nNodeNumber].sName, (LPARAM) &curnode, Children);
                }
            }
        }
    }

    TreeView_Expand(hTree, Root, TVE_EXPAND);
    std::cout << "Model tree building done!\n";
}

void BuildTree(BWM & Bwm){
    if(!Bwm.GetData()) return;
    BWMHeader & Walkmesh = *Bwm.GetData();

    HTREEITEM Root = Append(Bwm.GetFilename(), NULL, TVI_ROOT);
    Append("Header", NULL, Root);
    HTREEITEM Verts = Append("Vertices", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.verts.size(); n++){
        Append("Vertex " + std::to_string(n), (LPARAM) &Walkmesh.verts[n], Verts);
    }
    HTREEITEM Faces = Append("Faces", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.faces.size(); n++){
        Append("Face " + std::to_string(n), (LPARAM) &Walkmesh.faces[n], Faces);
    }
    HTREEITEM Aabb = Append("Aabb", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.aabb.size(); n++){
        Append("aabb " + std::to_string(n), (LPARAM) &Walkmesh.aabb[n], Aabb);
    }
    HTREEITEM Array2 = Append("Edges", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.edges.size(); n++){
        Append("Edge " + std::to_string(n), (LPARAM) &Walkmesh.edges[n], Array2);
    }
    HTREEITEM Array3 = Append("Perimeters", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.perimeters.size(); n++){
        Append("Perimeter " + std::to_string(n), (LPARAM) &Walkmesh.perimeters[n], Array3);
    }
    InvalidateRect(hTree, nullptr, true);
    std::cout << "Walkmesh tree building done!\n";
}

extern MDL Model;

void DetermineDisplayText(std::vector<std::string>cItem, std::stringstream & sPrint, LPARAM lParam){
    if(DEBUG_LEVEL > 1000) std::cout << "Updating Display!";
    bool bMdl = false, bWok = false, bPwk = false, bDwk0 = false, bDwk1 = false, bDwk2 = false;
    for(int j = 0; !bMdl && !bWok && !bPwk && !bDwk0 && !bDwk1 && !bDwk2; j++){
        if(cItem.at(j) == Model.GetFilename()) bMdl = true;
        if(!Model.Wok) {}
        else if(cItem.at(j) == Model.Wok->GetFilename()) bWok = true;
        if(!Model.Pwk) {}
        else if(cItem.at(j) == Model.Pwk->GetFilename()) bPwk = true;
        if(!Model.Dwk0) {}
        else if(cItem.at(j) == Model.Dwk0->GetFilename()) bDwk0 = true;
        if(!Model.Dwk1) {}
        else if(cItem.at(j) == Model.Dwk1->GetFilename()) bDwk1 = true;
        if(!Model.Dwk2) {}
        else if(cItem.at(j) == Model.Dwk2->GetFilename()) bDwk2 = true;
    }

    if(bMdl){
        FileHeader & Data = *Model.GetFileData();

        if(cItem.at(0) == "") sPrint.flush();

        /// Header ///
        else if(cItem.at(0) == "Header"){
            std::string sModelType;
            if(Data.MH.GH.nModelType == 1) sModelType = "geometry";
            else if(Data.MH.GH.nModelType == 2) sModelType = "model";
            else if(Data.MH.GH.nModelType == 5) sModelType = "animation";
            else sModelType = "unknown";
            sPrint <<           "== Header ==";
            sPrint << "\r\n" << "Model Name: " << Data.MH.GH.sName.c_str();
            sPrint << "\r\n" << "Model Type: " << (int)Data.MH.GH.nModelType << " (" << sModelType << ")";
            sPrint << "\r\n" << "Supermodel: " << Data.MH.cSupermodelName.c_str();
            //sPrint << "\r\n" << " Supermodel Reference: " << Data.MH.nSupermodelReference;
            sPrint << "\r\n" << "Classification: " << (short) Data.MH.nClassification << " (" << ReturnClassificationName(Data.MH.nClassification).c_str() << ")";
            sPrint << "\r\n" << "Unknown1: " << (short) Data.MH.nSubclassification;
            //sPrint << "\r\n" << "Unknown2: " << (short) Data.MH.nUnknown;
            sPrint << "\r\n" << "Affected By Fog: " << (short) Data.MH.nAffectedByFog;

            sPrint << "\r\n";
            sPrint << "\r\n" << "Bounding Box Min: " << PrepareFloat(Data.MH.vBBmin.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(Data.MH.vBBmin.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(Data.MH.vBBmin.fZ);
            sPrint << "\r\n" << "Bounding Box Max: " << PrepareFloat(Data.MH.vBBmax.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(Data.MH.vBBmax.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(Data.MH.vBBmax.fZ);
            sPrint << "\r\n" << "Radius: " << PrepareFloat(Data.MH.fRadius);
            sPrint << "\r\n" << "Animation Scale: " << PrepareFloat(Data.MH.fScale);
            sPrint << "\r\n";
            sPrint << "\r\n" << "MDL Length: " << Data.nMdlLength;
            sPrint << "\r\n" << "MDX Length: " << Data.nMdxLength;
            sPrint << "\r\n" << "Function Pointer 0: " << Data.MH.GH.nFunctionPointer0;
            sPrint << "\r\n" << "Function Pointer 1: " << Data.MH.GH.nFunctionPointer1;
        }

        /// Animations ///
        else if(cItem.at(0) == "Animations"){
            sPrint << "== Animations ==";
            sPrint << "\r\n" << "Offset to Animation Array: " << Data.MH.AnimationArray.nOffset;
            sPrint << "\r\n" << "Animation Count: " << Data.MH.Animations.size();
        }
        else if(cItem.at(1) == "Animations"){
            Animation * anim = (Animation * ) lParam;
            std::string sModelType;
            if(anim->nModelType == 1) sModelType = "geometry";
            else if(anim->nModelType == 2) sModelType = "model";
            else if(anim->nModelType == 5) sModelType = "animation";
            else sModelType = "unknown";
            sPrint <<           "== Animation '" << anim->sName.c_str() << "' ==";
            sPrint << "\r\n" << "Length:     " << PrepareFloat(anim->fLength);
            sPrint << "\r\n" << "Transition: " << PrepareFloat(anim->fTransition);
            sPrint << "\r\n" << "Anim Root:  " << anim->sAnimRoot.c_str();
            sPrint << "\r\n";
            sPrint << "\r\n" << "Offset to Anim: " << anim->nOffset;
            sPrint << "\r\n" << "Offset to Root: " << anim->nOffsetToRootAnimationNode;
            sPrint << "\r\n" << "Number of Names: " << anim->nNumberOfNames;
            sPrint << "\r\n" << "Model Type: " << (int) anim->nModelType << " (" << sModelType << ")";
            sPrint << "\r\n" << "Function Pointer 0: " << anim->nFunctionPointer0;
            sPrint << "\r\n" << "Function Pointer 1: " << anim->nFunctionPointer1;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Events --";
            sPrint << "\r\n" << "Offset: " << anim->EventArray.nOffset;
            sPrint << "\r\n" << "Count: " << anim->EventArray.nCount;
            for(int e = 0; e < anim->Events.size(); e++){
                sPrint << "\r\n" << "Event " << e << ":  " << anim->Events.at(e).sName.c_str() << " at " << PrepareFloat(anim->Events.at(e).fTime);
            }
        }

        /// Geometry ///
        else if(cItem.at(0) == "Geometry"){
            sPrint << "== Geometry ==";
            sPrint << "\r\n" << "Offset to Name Array: " << Data.MH.NameArray.nOffset;
            sPrint << "\r\n" << "Offset to Root Node:  " << Data.MH.GH.nOffsetToRootNode;
            sPrint << "\r\n" << "Offset to Head Root:  " << Data.MH.nOffsetToHeadRootNode;
            sPrint << "\r\n";
            sPrint << "\r\n" << "Name Count: " << Data.MH.Names.size();
            sPrint << "\r\n" << "Node Count: " << Data.MH.nNodeCount;
            sPrint << "\r\n" << "Total Nodes: " << Data.MH.GH.nTotalNumberOfNodes << " (with Supermodel)";
            sPrint << "\r\n";
            sPrint << "\r\n" << "Layout Position: " << PrepareFloat(Data.MH.vLytPosition.fX) << " " << PrepareFloat(Data.MH.vLytPosition.fY) << " " << PrepareFloat(Data.MH.vLytPosition.fZ);
        }

        /// Node ///
        else if(((cItem.at(1) == "Geometry") || (cItem.at(2) == "Animations") || (cItem.at(1) == "Children") || (cItem.at(1) == "Parent")) && !bWok){
            Node * node = (Node * ) lParam;
            //std::cout << "Current name in problematic position: " << cItem.at(0).c_str() << "\n";
            sPrint << "== " << Data.MH.Names[node->Head.nNodeNumber].sName.c_str() << " ==";
            sPrint << "\r\n" << "Type: " << node->Head.nType << " (";
                if(     node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_DANGLY)) sPrint << "dangly";
                else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_SKIN)) sPrint << "skin";
                else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_SABER)) sPrint << "saber";
                else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_AABB)) sPrint << "aabb";
                else if(node->Head.nType == (NODE_HEADER | NODE_MESH)) sPrint << "mesh";
                else if(node->Head.nType == (NODE_HEADER | NODE_REFERENCE)) sPrint << "reference";
                else if(node->Head.nType == (NODE_HEADER | NODE_EMITTER)) sPrint << "emitter";
                else if(node->Head.nType == (NODE_HEADER | NODE_LIGHT)) sPrint << "light";
                else if(node->Head.nType == (NODE_HEADER)) sPrint << "basic";
                else sPrint << "unknown";
                sPrint << ")";
            sPrint << "\r\n" << "Node: " << node->Head.nSupernodeNumber;
            sPrint << "\r\n" << "Name: " << node->Head.nNodeNumber << " (" << Data.MH.Names[node->Head.nNodeNumber].sName.c_str() << ")";
            sPrint << "\r\n" << "Offset to Root:   " << node->Head.nOffsetToRoot;
            sPrint << "\r\n" << "Offset to Node:   " << node->nOffset;
            sPrint << "\r\n" << "Offset to Parent: " << node->Head.nOffsetToParent;
            if(node->nAnimation == -1){
                sPrint << "\r\n";
                sPrint << "\r\n" << "Position: " << PrepareFloat(node->Head.vPos.fX);
                sPrint << "\r\n" << "          " << PrepareFloat(node->Head.vPos.fY);
                sPrint << "\r\n" << "          " << PrepareFloat(node->Head.vPos.fZ);
                sPrint << "\r\n" << "Orientation: " << PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fX);// << " (AA " << PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fX) << ")";
                sPrint << "\r\n" << "             " << PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fY);// << " (AA " << PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fY) << ")";
                sPrint << "\r\n" << "             " << PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fZ);// << " (AA " << PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fZ) << ")";
                sPrint << "\r\n" << "             " << PrepareFloat(node->Head.oOrient.GetQuaternion().fW);// << " (AA " << PrepareFloat(node->Head.oOrient.GetAxisAngle().fAngle) << ")";
            }
        }
        else if(cItem.at(0) == "Controllers"){
            Header * head = (Header * ) lParam;
            sPrint << "== Controllers ==";
            sPrint << "\r\n" << "Offset: " << head->ControllerArray.nOffset;
            sPrint << "\r\n" << "Count:  " << head->ControllerArray.nCount;

            std::vector<double> & fFloats = head->ControllerData;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Controller Data --";
            sPrint << "\r\n" << "Offset: " << head->ControllerDataArray.nOffset;
            sPrint << "\r\n" << "Count:  " << head->ControllerDataArray.nCount;
            if(fFloats.size() > 0){
                //sPrint << "\r\n" << "Data:";
                char cSpaces [10];
                int i = 0;
                while(i < fFloats.size()){
                    //std::cout << string_format("Printing Controller Data float %i\n", i);
                    if(fFloats[i] >= 0.0 || (!std::isfinite(fFloats[i]) && !(fFloats[i] < 0.0))) sprintf(cSpaces, " ");
                    else sprintf(cSpaces, "");
                    if(i < 10) strcat(cSpaces, " ");
                    sPrint << "\r\n" << "Data " << i << ": " << cSpaces << " " << PrepareFloat(fFloats[i], false);
                    i++;
                }
            }
        }
        else if(cItem.at(1) == "Controllers"){
            Controller * ctrl = (Controller*) lParam;
            Node & geonode = Data.MH.ArrayOfNodes.at(ctrl->nNodeNumber);
            sPrint << "== Controller '" << cItem.at(0).c_str() << "' ==";
            sPrint << "\r\n" << "Controller Type: " << ctrl->nControllerType << " (" << ReturnControllerName(ctrl->nControllerType, geonode.Head.nType) << ")";
            sPrint << "\r\n" << "Unknown: " << ctrl->nUnknown2;
            sPrint << "\r\n";
            sPrint << "\r\n" << "Value Count:   " << ctrl->nValueCount;
            sPrint << "\r\n" << "Timekey Start: " << ctrl->nTimekeyStart;
            sPrint << "\r\n" << "Data Start:    " << ctrl->nDataStart;
            sPrint << "\r\n" << "Column Count:  " << (int) (ctrl->nColumnCount & 15);
            sPrint << "\r\n" << "Bezier: " << (ctrl->nColumnCount & 16 ? 1 : 0);
            sPrint << "\r\n";
            sPrint << "\r\n" << "Value" << (ctrl->nValueCount > 1 ? "s" : "") << ":";

            Node * tempNode = nullptr;
            if(ctrl->nAnimation >= 0){
                Animation & anim = Data.MH.Animations.at(ctrl->nAnimation);
                for(int an = 0; an < anim.ArrayOfNodes.size() && tempNode == nullptr; an++){
                    Node & animNode = anim.ArrayOfNodes.at(an);
                    if(ctrl->nNodeNumber == animNode.Head.nNodeNumber){
                        for(int ac = 0; ac < animNode.Head.Controllers.size() && tempNode == nullptr; ac++){
                            if(&animNode.Head.Controllers.at(ac) == ctrl) tempNode = &animNode;
                        }
                    }
                }
                if(tempNode == nullptr) throw mdlexception("Could not find animation node with the currently selected controller!");
            }
            else tempNode = &geonode;
            Node & node = *tempNode;
            Location loc = geonode.GetLocation();

            for(int n = 0; n < ctrl->nValueCount; n++){
                if(ctrl->nAnimation >= 0) sPrint << "\r\n" << "(" << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << ") ";
                else sPrint << " ";

                /// Orientation controller
                if(ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){

                    /// Compressed orientation
                    if(ctrl->nColumnCount == 2){
                        ByteBlock4.f = node.Head.ControllerData.at(ctrl->nDataStart + n);
                        Quaternion qCurrent = DecompressQuaternion(ByteBlock4.ui);

                        sPrint << PrepareFloat(qCurrent.vAxis.fX) << " " << PrepareFloat(qCurrent.vAxis.fY) << " " << PrepareFloat(qCurrent.vAxis.fZ) << " " << PrepareFloat(qCurrent.fW);
                    }

                    /// Uncompressed orientation
                    else if(ctrl->nColumnCount == 4){
                        Quaternion qCurrent = Quaternion(node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 0),
                                              node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 1),
                                              node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 2),
                                              node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 3));

                        sPrint << PrepareFloat(qCurrent.vAxis.fX) << " " << PrepareFloat(qCurrent.vAxis.fY) << " " << PrepareFloat(qCurrent.vAxis.fZ) << " " << PrepareFloat(qCurrent.fW);
                    }

                    /// unknown orientation controller type
                    else{
                        std::cout << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << Data.MH.Names.at(ctrl->nNodeNumber).sName << " (" << (ctrl->nAnimation == -1 ? "geometry" : Data.MH.Animations.at(ctrl->nAnimation).sName.c_str()) << ")!\n";
                        Error("A controller type is not being handled! Check the console and add the necessary code!");
                    }
                }

                /// bezier controller
                else if(ctrl->nColumnCount & 16){

                    /// Position controller
                    if(ctrl->nControllerType == CONTROLLER_HEADER_POSITION && ctrl->nAnimation >= 0){
                        sPrint << " " << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (0)));
                        sPrint << " " << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (1)));
                        sPrint << " " << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (2)));
                        sPrint << " | " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (3)));
                        sPrint << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (4)));
                        sPrint << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (5)));
                        sPrint << " | " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (6)));
                        sPrint << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (7)));
                        sPrint << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (8)));
                    }

                    /// Other controllers
                    else{
                        for(int i = 0; i < (ctrl->nColumnCount & 15) * 3; i++){
                            sPrint << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n * ((ctrl->nColumnCount & 15) * 3) + i));
                            if(i < (ctrl->nColumnCount & 15) * 3 - 1) sPrint << " ";
                            if(i % (ctrl->nColumnCount & 15) == 0 && i > 0) sPrint << "| ";
                        }
                    }
                }

                /// regular controller
                else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){

                    /// Position controller
                    if(ctrl->nControllerType == CONTROLLER_HEADER_POSITION && ctrl->nAnimation >= 0){
                        sPrint << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 0));
                        sPrint << " ";
                        sPrint << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 1));
                        sPrint << " ";
                        sPrint << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 2));
                    }

                    /// Other controllers
                    else{
                        for(int i = 0; i < ctrl->nColumnCount; i++){
                            sPrint << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + i));
                            if(i < ctrl->nColumnCount - 1) sPrint << " ";
                        }
                    }
                }

                /// unknown controller type
                else{
                    std::cout << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << Data.MH.Names.at(ctrl->nNodeNumber).sName << " (" << (ctrl->nAnimation == -1 ? "geometry" : Data.MH.Animations.at(ctrl->nAnimation).sName.c_str()) << ")!\n";
                    Error("A controller type is not being handled! Check the console and add the necessary code!");
                }
            }
        }
        else if(cItem.at(0) == "Children"){
            Header * head = (Header * ) lParam;
            sPrint << "== Children ==";
            sPrint << "\r\n" << "Offset: " << head->ChildrenArray.nOffset;
            sPrint << "\r\n" << "Count:  " << head->ChildrenArray.nCount;
        }

        /// Light ///
        else if(cItem.at(0) == "Light"){
            LightHeader * light = &((Node*) lParam)->Light;
            sPrint << "== Light ==";
            sPrint << "\r\n" << "LightPriority:  " << light->nLightPriority;
            sPrint << "\r\n" << "Ambient Only:   " << light->nAmbientOnly;
            sPrint << "\r\n" << "Dynamic Type:   " << light->nDynamicType;
            sPrint << "\r\n" << "Affect Dynamic: " << light->nAffectDynamic;
            sPrint << "\r\n" << "Shadow:         " << light->nShadow;
            sPrint << "\r\n" << "Flare:          " << light->nFlare;
            sPrint << "\r\n" << "Fading Light:   " << light->nFadingLight;
        }
        else if(cItem.at(0) == "Lens Flares"){
            LightHeader * light = (LightHeader * ) lParam;
            sPrint << "== Lens Flares ==";
            sPrint << "\r\n" << "Sizes Offset: " << light->FlareSizeArray.nOffset;
            sPrint << "\r\n" << "Sizes Count:  " << light->FlareSizes.size();
            sPrint << "\r\n";
            sPrint << "\r\n" << "Positions Offset: " << light->FlarePositionArray.nOffset;
            sPrint << "\r\n" << "Positions Count:  " << light->FlarePositions.size();
            sPrint << "\r\n";
            sPrint << "\r\n" << "Color Shifts Offset: " << light->FlareColorShiftArray.nOffset;
            sPrint << "\r\n" << "Color Shifts Count:  " << light->FlareColorShifts.size();
            sPrint << "\r\n";
            sPrint << "\r\n" << "Textures Offset: " << light->FlareTextureNameArray.nOffset;
            sPrint << "\r\n" << "Textures Count:  " << light->FlareTextureNames.size();
            sPrint << "\r\n";
            sPrint << "\r\n" << "Flare Radius: " << PrepareFloat(light->fFlareRadius);
        }
        else if(cItem.at(1) == "Lens Flares"){
            LightHeader * light = & ((Node * ) lParam)->Light;

            std::string sName = cItem.at(0);
            int nIndex = -1;
            int nPos = 0;
            if(sName.length() < 1) return;
            for(nPos = sName.length() - 1; nPos >= 0 && sName.at(nPos) != ' '; nPos--){}
            if(nPos == 0) return;
            try{ nIndex = stoi(sName.substr(nPos+1),(size_t*) NULL); }
            catch(std::invalid_argument){
                std::cout << "DetermineTreeText - Lens Flare: There was an error converting the string: " << sName.substr(nPos) << ".\n";
                return;
            }

            sPrint << "== " << sName << " ==";
            try{ sPrint << "\r\n" << "Size:        " << PrepareFloat(light->FlareSizes.at(nIndex)); }
            catch(...){}
            try{ sPrint << "\r\n" << "Texture:     " << light->FlareTextureNames.at(nIndex).sName.c_str(); }
            catch(...){}
            try{ sPrint << "\r\n" << "Position:    " << PrepareFloat(light->FlarePositions.at(nIndex)); }
            catch(...){}
            try{
                 sPrint << "\r\n" << "Color Shift: " << PrepareFloat(light->FlareColorShifts.at(nIndex).fR);
                 sPrint << "\r\n" << "             " << PrepareFloat(light->FlareColorShifts.at(nIndex).fG);
                 sPrint << "\r\n" << "             " << PrepareFloat(light->FlareColorShifts.at(nIndex).fB);
            }
            catch(...){}
        }

        /// Emitter ///
        else if(cItem.at(0) == "Emitter"){
            EmitterHeader * emitter = &((Node*) lParam)->Emitter;
            sPrint <<           "== Emitter ==";
            sPrint << "\r\n" << "Dead Space:         " << PrepareFloat(emitter->fDeadSpace);
            sPrint << "\r\n" << "Blast Radius:       " << PrepareFloat(emitter->fBlastRadius);
            sPrint << "\r\n" << "Blast Length:       " << PrepareFloat(emitter->fBlastLength);
            sPrint << "\r\n" << "Branch Count:       " << emitter->nBranchCount;
            sPrint << "\r\n" << "Ctrl Pt Smoothing:  " << PrepareFloat(emitter->fControlPointSmoothing);
            sPrint << "\r\n" << "X Grid:             " << emitter->nxGrid;
            sPrint << "\r\n" << "Y Grid:             " << emitter->nyGrid;
            sPrint << "\r\n" << "Spawn Type:         " << emitter->nSpawnType;
            sPrint << "\r\n" << "Update:             " << emitter->cUpdate.c_str();
            sPrint << "\r\n" << "Render:             " << emitter->cRender.c_str();
            sPrint << "\r\n" << "Blend:              " << emitter->cBlend.c_str();
            sPrint << "\r\n" << "Texture:            " << emitter->cTexture.c_str();
            sPrint << "\r\n" << "Chunk Name:         " << emitter->cChunkName.c_str();
            sPrint << "\r\n" << "Twosided Texture:   " << emitter->nTwosidedTex;
            sPrint << "\r\n" << "Loop:               " << emitter->nLoop;
            sPrint << "\r\n" << "Render Order:       " << emitter->nRenderOrder;
            sPrint << "\r\n" << "Frame Blending:     " << (short) emitter->nFrameBlending;
            sPrint << "\r\n" << "Depth Texture Name: " << emitter->cDepthTextureName.c_str();
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Emitter Flags --";
            sPrint << "\r\n" << "  p2p:            " << (emitter->nFlags & EMITTER_FLAG_P2P ? 1 : 0);
            sPrint << "\r\n" << "  p2p_sel:        " << (emitter->nFlags & EMITTER_FLAG_P2P_SEL ? 1 : 0);
            sPrint << "\r\n" << "  affected_wind:  " << (emitter->nFlags & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0);
            sPrint << "\r\n" << "  tinted:         " << (emitter->nFlags & EMITTER_FLAG_TINTED ? 1 : 0);
            sPrint << "\r\n" << "  bounce:         " << (emitter->nFlags & EMITTER_FLAG_BOUNCE ? 1 : 0);
            sPrint << "\r\n" << "  random:         " << (emitter->nFlags & EMITTER_FLAG_RANDOM ? 1 : 0);
            sPrint << "\r\n" << "  inherit:        " << (emitter->nFlags & EMITTER_FLAG_INHERIT ? 1 : 0);
            sPrint << "\r\n" << "  inherit_vel:    " << (emitter->nFlags & EMITTER_FLAG_INHERIT_VEL ? 1 : 0);
            sPrint << "\r\n" << "  inherit_local:  " << (emitter->nFlags & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0);
            sPrint << "\r\n" << "  splat:          " << (emitter->nFlags & EMITTER_FLAG_SPLAT ? 1 : 0);
            sPrint << "\r\n" << "  inherit_part:   " << (emitter->nFlags & EMITTER_FLAG_INHERIT_PART ? 1 : 0);
            sPrint << "\r\n" << "  depth_texture?: " << (emitter->nFlags & EMITTER_FLAG_DEPTH_TEXTURE ? 1 : 0);
            sPrint << "\r\n" << "  renderorder?:   " << (emitter->nFlags & EMITTER_FLAG_13 ? 1 : 0);
        }

        /// Reference ///
        else if(cItem.at(0) == "Reference"){
            ReferenceHeader * ref = &((Node*) lParam)->Reference;
            sPrint <<           "== Reference ==";
            sPrint << "\r\n" << "Reference Model: " << ref->sRefModel.c_str();
            sPrint << "\r\n" << "Reattachable: " << ref->nReattachable;
        }

        /// Mesh ///
        else if(cItem.at(0) == "Mesh"){
            MeshHeader * mesh = &((Node*) lParam)->Mesh;
            sPrint << "== Mesh ==";
            sPrint << "\r\n" << "-- Textures --";
            sPrint << "\r\n" << "Texture Count: " << mesh->nTextureNumber;
            sPrint << "\r\n" << "Texture 1: " << mesh->GetTexture(1);
            sPrint << "\r\n" << "Texture 2: " << mesh->GetTexture(2);
            sPrint << "\r\n" << "Texture 3: " << mesh->GetTexture(3);
            sPrint << "\r\n" << "Texture 4: " << mesh->GetTexture(4);
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Trimesh Flags --";
            sPrint << "\r\n" << "Render:              " << (int) mesh->nRender;
            sPrint << "\r\n" << "Shadow:              " << (int) mesh->nShadow;
            sPrint << "\r\n" << "Beaming:             " << (int) mesh->nBeaming;
            sPrint << "\r\n" << "Lightmapped:         " << (int) mesh->nHasLightmap;
            sPrint << "\r\n" << "Rotate Texture:      " << (int) mesh->nRotateTexture;
            sPrint << "\r\n" << "Background Geometry: " << (int) mesh->nBackgroundGeometry;
            if(Model.bK2) sPrint << "\r\n" << "Hide in Holograms:   " << (int) mesh->nHideInHolograms;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Animated UV --";
            sPrint << "\r\n" << "Animate UV:      " << mesh->nAnimateUV;
            sPrint << "\r\n" << "UV Direction X:  " << PrepareFloat(mesh->fUVDirectionX);
            sPrint << "\r\n" << "UV Direction Y:  " << PrepareFloat(mesh->fUVDirectionY);
            sPrint << "\r\n" << "UV Jitter:       " << PrepareFloat(mesh->fUVJitter);
            sPrint << "\r\n" << "UV Jitter Speed: " << PrepareFloat(mesh->fUVJitterSpeed);
            if(Model.bK2) sPrint << "\r\n";
            if(Model.bK2) sPrint << "\r\n" << "-- Dirt --";
            if(Model.bK2) sPrint << "\r\n" << "Dirt Enabled:     " << (int) mesh->nDirtEnabled;
            if(Model.bK2) sPrint << "\r\n" << "Dirt Texture:     " << mesh->nDirtTexture;
            if(Model.bK2) sPrint << "\r\n" << "Dirt Coord Space: " << mesh->nDirtCoordSpace;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Other --";
            sPrint << "\r\n" << "Transparency Hint: " << mesh->nTransparencyHint;
            sPrint << "\r\n" << "Ambient Color: " << PrepareFloat(mesh->fAmbient.fR);
            sPrint << "\r\n" << "               " << PrepareFloat(mesh->fAmbient.fG);
            sPrint << "\r\n" << "               " << PrepareFloat(mesh->fAmbient.fB);
            sPrint << "\r\n" << "Diffuse Color: " << PrepareFloat(mesh->fDiffuse.fR);
            sPrint << "\r\n" << "               " << PrepareFloat(mesh->fDiffuse.fG);
            sPrint << "\r\n" << "               " << PrepareFloat(mesh->fDiffuse.fB);
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Unknown Lightsaber Bytes --";
            sPrint << "\r\n" << "Unknown 1: " << (int) mesh->nSaberUnknown1;
            sPrint << "\r\n" << "Unknown 2: " << (int) mesh->nSaberUnknown2;
            sPrint << "\r\n" << "Unknown 3: " << (int) mesh->nSaberUnknown3;
            sPrint << "\r\n" << "Unknown 4: " << (int) mesh->nSaberUnknown4;
            sPrint << "\r\n" << "Unknown 5: " << (int) mesh->nSaberUnknown5;
            sPrint << "\r\n" << "Unknown 6: " << (int) mesh->nSaberUnknown6;
            sPrint << "\r\n" << "Unknown 7: " << (int) mesh->nSaberUnknown7;
            sPrint << "\r\n" << "Unknown 8: " << (int) mesh->nSaberUnknown8;
            sPrint << "\r\n";
            sPrint << "\r\n" << "Bounding Box Min: " << PrepareFloat(mesh->vBBmin.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(mesh->vBBmin.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(mesh->vBBmin.fZ);
            sPrint << "\r\n" << "Bounding Box Max: " << PrepareFloat(mesh->vBBmax.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(mesh->vBBmax.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(mesh->vBBmax.fZ);
            sPrint << "\r\n" << "Average: " << PrepareFloat(mesh->vAverage.fX);
            sPrint << "\r\n" << "         " << PrepareFloat(mesh->vAverage.fY);
            sPrint << "\r\n" << "         " << PrepareFloat(mesh->vAverage.fZ);
            sPrint << "\r\n" << "Radius: " << PrepareFloat(mesh->fRadius);
            sPrint << "\r\n" << "Total Area: " << PrepareFloat(mesh->fTotalArea);
            sPrint << "\r\n";
            sPrint << "\r\n" << "Mesh Inverted Counter: " << mesh->nMeshInvertedCounter;
            sPrint << "\r\n" << "(Offset: " << mesh->MeshInvertedCounterArray.nOffset << ", Count: " << mesh->MeshInvertedCounterArray.nCount << ")";
            sPrint << "\r\n";
            sPrint << "\r\n" << "Function Pointer 0: " << mesh->nFunctionPointer0;
            sPrint << "\r\n" << "Function Pointer 1: " << mesh->nFunctionPointer1;
        }
        else if(cItem.at(0) == "Vertices"){
            Node & node = * (Node * ) lParam;
            MeshHeader * mesh = &node.Mesh;
            sPrint <<           "== Vertices ==";
            sPrint << "\r\n" << "Offset: " << mesh->nOffsetToVertArray;
            sPrint << "\r\n" << "Count:  " << mesh->nNumberOfVerts;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- MDX Data --";
            sPrint << "\r\n" << "Offset: " << mesh->nOffsetIntoMdx;
            sPrint << "\r\n" << "Size:   " << mesh->nMdxDataSize;
            sPrint << "\r\n" << "Bitflags:";
            sPrint << "\r\n" << "  Vertex:   " << (mesh->nMdxDataBitmap & MDX_FLAG_VERTEX ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxVertex << ")";
            sPrint << "\r\n" << "  Normal:   " << (mesh->nMdxDataBitmap & MDX_FLAG_NORMAL ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxNormal << ")";
            sPrint << "\r\n" << "  UV1:      " << (mesh->nMdxDataBitmap & MDX_FLAG_UV1 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxUV1 << ")";
            sPrint << "\r\n" << "  UV2:      " << (mesh->nMdxDataBitmap & MDX_FLAG_UV2 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxUV2 << ")";
            sPrint << "\r\n" << "  UV3:      " << (mesh->nMdxDataBitmap & MDX_FLAG_UV3 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxUV3 << ")";
            sPrint << "\r\n" << "  UV4:      " << (mesh->nMdxDataBitmap & MDX_FLAG_UV4 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxUV4 << ")";
            sPrint << "\r\n" << "  Colors:   " << (mesh->nMdxDataBitmap & MDX_FLAG_COLOR ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxColor << ")";
            sPrint << "\r\n" << "  Tangent1: " << (mesh->nMdxDataBitmap & MDX_FLAG_TANGENT1 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxTangent1 << ")";
            sPrint << "\r\n" << "  Tangent2: " << (mesh->nMdxDataBitmap & MDX_FLAG_TANGENT2 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxTangent2 << ")";
            sPrint << "\r\n" << "  Tangent3: " << (mesh->nMdxDataBitmap & MDX_FLAG_TANGENT3 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxTangent3 << ")";
            sPrint << "\r\n" << "  Tangent4: " << (mesh->nMdxDataBitmap & MDX_FLAG_TANGENT4 ? 1 : 0) << " (Offset " << mesh->nOffsetToMdxTangent4 << ")";

            if(Model.Mdx){
                sPrint << "\r\n";
                sPrint << "\r\n" << "-- Extra MDX Data --";
                VertexData * mdx = &mesh->MDXData;
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                    sPrint << "\r\n" << "Vertex: " << PrepareFloat(mdx->vVertex.fX, false);
                    sPrint << "\r\n" << "        " << PrepareFloat(mdx->vVertex.fY, false);
                    sPrint << "\r\n" << "        " << PrepareFloat(mdx->vVertex.fZ, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
                    sPrint << "\r\n" << "Normal: " << PrepareFloat(mdx->vNormal.fX, false);
                    sPrint << "\r\n" << "        " << PrepareFloat(mdx->vNormal.fY, false);
                    sPrint << "\r\n" << "        " << PrepareFloat(mdx->vNormal.fZ, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                    sPrint << "\r\n" << "UV1: " << PrepareFloat(mdx->vUV1.fX, false);
                    sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV1.fY, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
                    sPrint << "\r\n" << "UV2: " << PrepareFloat(mdx->vUV2.fX, false);
                    sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV2.fY, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
                    sPrint << "\r\n" << "UV3: " << PrepareFloat(mdx->vUV3.fX, false);
                    sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV3.fY, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
                    sPrint << "\r\n" << "UV4: " << PrepareFloat(mdx->vUV4.fX, false);
                    sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV4.fY, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                    sPrint << "\r\n" << "Tangent 1:   " << PrepareFloat(mdx->vTangent1[0].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[0].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[0].fZ, false);
                    sPrint << "\r\n" << "Bitangent 1: " << PrepareFloat(mdx->vTangent1[1].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[1].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[1].fZ, false);
                    sPrint << "\r\n" << "Normal 1:    " << PrepareFloat(mdx->vTangent1[2].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[2].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[2].fZ, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
                    sPrint << "\r\n" << "Tangent 2:   " << PrepareFloat(mdx->vTangent2[0].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[0].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[0].fZ, false);
                    sPrint << "\r\n" << "Bitangent 2: " << PrepareFloat(mdx->vTangent2[1].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[1].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[1].fZ, false);
                    sPrint << "\r\n" << "Normal 2:    " << PrepareFloat(mdx->vTangent2[2].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[2].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[2].fZ, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
                    sPrint << "\r\n" << "Tangent 3:   " << PrepareFloat(mdx->vTangent3[0].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[0].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[0].fZ, false);
                    sPrint << "\r\n" << "Bitangent 3: " << PrepareFloat(mdx->vTangent3[1].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[1].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[1].fZ, false);
                    sPrint << "\r\n" << "Normal 3:    " << PrepareFloat(mdx->vTangent3[2].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[2].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[2].fZ, false);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
                    sPrint << "\r\n" << "Tangent 4:   " << PrepareFloat(mdx->vTangent4[0].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[0].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[0].fZ, false);
                    sPrint << "\r\n" << "Bitangent 4: " << PrepareFloat(mdx->vTangent4[1].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[1].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[1].fZ, false);
                    sPrint << "\r\n" << "Normal 4:    " << PrepareFloat(mdx->vTangent4[2].fX, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[2].fY, false);
                    sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[2].fZ, false);
                }
                if(node.Head.nType & NODE_SKIN){
                    sPrint << "\r\n" << "Weight Value: " << PrepareFloat(mdx->Weights.fWeightValue[0], false);
                    sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[1], false);
                    sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[2], false);
                    sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[3], false);
                    sPrint << "\r\n" << "Weight Index: " << mdx->Weights.nWeightIndex[0];
                    sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[1];
                    sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[2];
                    sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[3];
                }
            }
        }
        else if(cItem.at(1) == "Vertices"){
            Vertex * vert = (Vertex * ) lParam;
            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            if(!Model.bXbox){
                sPrint << "\r\n" << "-- MDL Data --";
                sPrint << "\r\n" << "Vertex: " << PrepareFloat(vert->fX, false);
                sPrint << "\r\n" << "        " << PrepareFloat(vert->fY, false);
                sPrint << "\r\n" << "        " << PrepareFloat(vert->fZ, false);
            }

            VertexData * mdx = &vert->MDXData;
            Node & node = Model.GetNodeByNameIndex(mdx->nNodeNumber);
            if(!(node.Head.nType & NODE_SABER)){
                if(node.Mesh.nMdxDataSize > 0 && Model.Mdx){
                    if(!Model.bXbox) sPrint << "\r\n";
                    sPrint << "\r\n" << "-- MDX Data --";
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                        sPrint << "\r\n" << "Vertex: " << PrepareFloat(mdx->vVertex.fX, false);
                        sPrint << "\r\n" << "        " << PrepareFloat(mdx->vVertex.fY, false);
                        sPrint << "\r\n" << "        " << PrepareFloat(mdx->vVertex.fZ, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
                        sPrint << "\r\n" << "Normal: " << PrepareFloat(mdx->vNormal.fX, false);
                        sPrint << "\r\n" << "        " << PrepareFloat(mdx->vNormal.fY, false);
                        sPrint << "\r\n" << "        " << PrepareFloat(mdx->vNormal.fZ, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                        sPrint << "\r\n" << "UV1: " << PrepareFloat(mdx->vUV1.fX, false);
                        sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV1.fY, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
                        sPrint << "\r\n" << "UV2: " << PrepareFloat(mdx->vUV2.fX, false);
                        sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV2.fY, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
                        sPrint << "\r\n" << "UV3: " << PrepareFloat(mdx->vUV3.fX, false);
                        sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV3.fY, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
                        sPrint << "\r\n" << "UV4: " << PrepareFloat(mdx->vUV4.fX, false);
                        sPrint << "\r\n" << "     " << PrepareFloat(mdx->vUV4.fY, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                        sPrint << "\r\n" << "Tangent 1:   " << PrepareFloat(mdx->vTangent1[0].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[0].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[0].fZ, false);
                        sPrint << "\r\n" << "Bitangent 1: " << PrepareFloat(mdx->vTangent1[1].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[1].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[1].fZ, false);
                        sPrint << "\r\n" << "Normal 1:    " << PrepareFloat(mdx->vTangent1[2].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[2].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent1[2].fZ, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
                        sPrint << "\r\n" << "Tangent 2:   " << PrepareFloat(mdx->vTangent2[0].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[0].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[0].fZ, false);
                        sPrint << "\r\n" << "Bitangent 2: " << PrepareFloat(mdx->vTangent2[1].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[1].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[1].fZ, false);
                        sPrint << "\r\n" << "Normal 2:    " << PrepareFloat(mdx->vTangent2[2].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[2].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent2[2].fZ, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
                        sPrint << "\r\n" << "Tangent 3:   " << PrepareFloat(mdx->vTangent3[0].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[0].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[0].fZ, false);
                        sPrint << "\r\n" << "Bitangent 3: " << PrepareFloat(mdx->vTangent3[1].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[1].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[1].fZ, false);
                        sPrint << "\r\n" << "Normal 3:    " << PrepareFloat(mdx->vTangent3[2].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[2].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent3[2].fZ, false);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
                        sPrint << "\r\n" << "Tangent 4:   " << PrepareFloat(mdx->vTangent4[0].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[0].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[0].fZ, false);
                        sPrint << "\r\n" << "Bitangent 4: " << PrepareFloat(mdx->vTangent4[1].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[1].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[1].fZ, false);
                        sPrint << "\r\n" << "Normal 4:    " << PrepareFloat(mdx->vTangent4[2].fX, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[2].fY, false);
                        sPrint << "\r\n" << "             " << PrepareFloat(mdx->vTangent4[2].fZ, false);
                    }
                    if(node.Head.nType & NODE_SKIN){
                        sPrint << "\r\n" << "Weight Value: " << PrepareFloat(mdx->Weights.fWeightValue[0], false);
                        sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[1], false);
                        sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[2], false);
                        sPrint << "\r\n" << "              " << PrepareFloat(mdx->Weights.fWeightValue[3], false);
                        sPrint << "\r\n" << "Weight Index: " << mdx->Weights.nWeightIndex[0];
                        sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[1];
                        sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[2];
                        sPrint << "\r\n" << "              " << mdx->Weights.nWeightIndex[3];
                    }
                }
            }
        }
        else if((cItem.at(0) == "Faces") && !bWok){
            MeshHeader * mesh = (MeshHeader * ) lParam;
            sPrint << "== Faces ==";
            sPrint << "\r\n" << "Offset: " << mesh->FaceArray.nOffset;
            sPrint << "\r\n" << "Count:  " << mesh->FaceArray.nCount;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Vertex Indices --";
            sPrint << "\r\n" << "Indices Count: " << mesh->nVertIndicesCount << " (Offset: " << mesh->IndexCounterArray.nOffset << ")";
            sPrint << "\r\n" << "Indices Offset: " << mesh->nVertIndicesLocation << " (Offset: " << mesh->IndexLocationArray.nOffset << ")";
            //sPrint << "\r\n";
            for(int n = 0; n < mesh->VertIndices.size(); n++){
                sPrint << "\r\n" << "Face " << n << ":  " << mesh->VertIndices.at(n).at(0) << ", " << mesh->VertIndices.at(n).at(1) << ", " << mesh->VertIndices.at(n).at(2);
            }
        }
        else if(cItem.at(1) == "Faces"){
            Face * face = (Face * ) lParam;
            sPrint << "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Face Normal: " << PrepareFloat(face->vNormal.fX, false);
            sPrint << "\r\n" << "             " << PrepareFloat(face->vNormal.fY, false);
            sPrint << "\r\n" << "             " << PrepareFloat(face->vNormal.fZ, false);
            sPrint << "\r\n" << "Plane Distance: " << PrepareFloat(face->fDistance, false);
            sPrint << "\r\n" << "Material ID: " << face->nMaterialID;
            sPrint << "\r\n" << "Adjacent Faces: " << face->nAdjacentFaces[0] << ", " << face->nAdjacentFaces[1] << ", " << face->nAdjacentFaces[2];
            sPrint << "\r\n" << "Vertex Indices: " << face->nIndexVertex[0] << ", " << face->nIndexVertex[1] << ", " << face->nIndexVertex[2];
            sPrint << "\r\n";
            sPrint << "\r\n" << "Area: " << PrepareFloat(face->fArea);
            sPrint << "\r\n" << "Smoothing groups: ";
            for(int n = 0; n < 32; n++){
                if(pown(2, n) & face->nSmoothingGroup) sPrint << n+1 << " ";
            }
            /*
            sPrint << "\r\n";
            sPrint << "\r\nBounding Box Min: " << std::setprecision(5) << face->vBBmin.fX;
            sPrint << "\r\n                  " << std::setprecision(5) << face->vBBmin.fY;
            sPrint << "\r\n                  " << std::setprecision(5) << face->vBBmin.fZ;
            sPrint << "\r\nBounding Box Max: " << std::setprecision(5) << face->vBBmax.fX;
            sPrint << "\r\n                  " << std::setprecision(5) << face->vBBmax.fY;
            sPrint << "\r\n                  " << std::setprecision(5) << face->vBBmax.fZ;
            */
        }

        /// Skin ///
        else if(cItem.at(0) == "Skin"){
            SkinHeader * skin = &((Node*) lParam)->Skin;
            sPrint <<           "== Skin ==";
            //sPrint << "\r\n";
            sPrint << "\r\n" << "-- MDX Data Pointers --";
            sPrint << "\r\n" << "To Weight Value: " << skin->nOffsetToMdxWeightValues;
            sPrint << "\r\n" << "To Weight Index: " << skin->nOffsetToMdxBoneIndices;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Bone Indices --";
            for(int n = 0; n < 16; n++) sPrint << "\r\n" << "Index " << n+1 << ": " << skin->nBoneIndices[n];
        }
        else if(cItem.at(0) == "Bones"){
            SkinHeader * skin = (SkinHeader * ) lParam;
            sPrint <<           "== Bonemap ==";
            sPrint << "\r\n" << "Offset: " << skin->nOffsetToBonemap;
            sPrint << "\r\n" << "Count:  " << skin->nNumberOfBonemap;
            sPrint << "\r\n";
            sPrint << "\r\n" << "== Q Bones ==";
            sPrint << "\r\n" << "Offset: " << skin->QBoneArray.nOffset;
            sPrint << "\r\n" << "Count:  " << skin->QBoneArray.nCount;
            sPrint << "\r\n";
            sPrint << "\r\n" << "== T Bones ==";
            sPrint << "\r\n" << "Offset: " << skin->TBoneArray.nOffset;
            sPrint << "\r\n" << "Count:  " << skin->TBoneArray.nCount;
            sPrint << "\r\n";
            sPrint << "\r\n" << "== Array8 ==";
            sPrint << "\r\n" << "Offset: " << skin->Array8Array.nOffset;
            sPrint << "\r\n" << "Count:  " << skin->Array8Array.nCount;
        }
        else if(cItem.at(1) == "Bones"){
            Bone * bone = (Bone * ) lParam;
            sPrint << "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Bonemap: " << bone->nBonemap;
            sPrint << "\r\n" << "TBone: " << PrepareFloat(bone->TBone.fX);
            sPrint << "\r\n" << "       " << PrepareFloat(bone->TBone.fY);
            sPrint << "\r\n" << "       " << PrepareFloat(bone->TBone.fZ);
            sPrint << "\r\n" << "QBone: " << PrepareFloat(bone->QBone.GetQuaternion().vAxis.fX);
            sPrint << "\r\n" << "       " << PrepareFloat(bone->QBone.GetQuaternion().vAxis.fY);
            sPrint << "\r\n" << "       " << PrepareFloat(bone->QBone.GetQuaternion().vAxis.fZ);
            sPrint << "\r\n" << "       " << PrepareFloat(bone->QBone.GetQuaternion().fW);
        }

        /// Danglymesh ///
        else if(cItem.at(0) == "Danglymesh"){
            DanglymeshHeader * dangly = &((Node*) lParam)->Dangly;
            sPrint <<           "== Danglymesh ==";
            sPrint << "\r\n" << "Displacement: " << PrepareFloat(dangly->fDisplacement);
            sPrint << "\r\n" << "Tightness:    " << PrepareFloat(dangly->fTightness);
            sPrint << "\r\n" << "Period:       " << PrepareFloat(dangly->fPeriod);
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Constraints --";
            sPrint << "\r\n" << "Offset: " << dangly->ConstraintArray.nOffset;
            sPrint << "\r\n" << "Count:  " << dangly->ConstraintArray.nCount;
            sPrint << "\r\n";
            sPrint << "\r\n" << "-- Data2 --";
            sPrint << "\r\n" << "Offset: " << dangly->nOffsetToData2;
        }
        else if(cItem.at(1) == "Danglymesh"){
            DanglymeshHeader * dangly = &((Node * ) lParam)->Dangly;

            std::string sName = cItem.at(0);
            int nIndex = -1;
            int nPos = 0;
            if(sName.length() < 1) return;
            for(nPos = sName.length() - 1; nPos >= 0 && sName.at(nPos) != ' '; nPos--){}
            if(nPos == 0) return;
            try{ nIndex = stoi(sName.substr(nPos+1),(size_t*) NULL); }
            catch(std::invalid_argument){
                std::cout << "DetermineTreeText - Dangly Vertices: There was an error converting the string: " << sName.substr(nPos) << ".\n";
                return;
            }

            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Constraint: " << PrepareFloat(dangly->Constraints.at(nIndex));
            sPrint << "\r\n" << "Data2: " << PrepareFloat(dangly->Data2.at(nIndex).fX);
            sPrint << "\r\n" << "       " << PrepareFloat(dangly->Data2.at(nIndex).fY);
            sPrint << "\r\n" << "       " << PrepareFloat(dangly->Data2.at(nIndex).fZ);
        }

        /// Walkmesh ///
        else if(cItem.at(0) == "Aabb"){
            WalkmeshHeader * walk = (WalkmeshHeader * ) lParam;
            sPrint <<           "== AABB Tree ==";
            sPrint << "\r\n" << "Offset to AABB Tree: " << walk->nOffsetToAabb;
        }
        else if(cItem.at(1) == "Aabb"){
            Aabb * aabb = (Aabb * ) lParam;
            std::string sProperty;
            if(aabb->nProperty == 1) sProperty = "Positive X";
            else if(aabb->nProperty == 2) sProperty = "Positive Y";
            else if(aabb->nProperty == 4) sProperty = "Positive Z";
            else if(aabb->nProperty == 8) sProperty = "Negative X";
            else if(aabb->nProperty == 16) sProperty = "Negative Y";
            else if(aabb->nProperty == 32) sProperty = "Negative Z";
            else sProperty = "None";
            sPrint <<           "== " << cItem.at(0) << " ==";
            sPrint << "\r\n" << "Offset: " << aabb->nOffset;
            sPrint << "\r\n" << "Bounding Box Min: " << PrepareFloat(aabb->vBBmin.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmin.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmin.fZ);
            sPrint << "\r\n" << "Bounding Box Max: " << PrepareFloat(aabb->vBBmax.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmax.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmax.fZ);
            sPrint << "\r\n" << "Face Index: " << aabb->nID;
            sPrint << "\r\n" << "2nd Child Property: " << aabb->nProperty << " (" << sProperty << ")";
            sPrint << "\r\n" << "Offset to Child 1: " << aabb->nChild1;
            sPrint << "\r\n" << "Offset to Child 2: " << aabb->nChild2;
        }

        /// Saber ///
        else if(cItem.at(0) == "Lightsaber"){
            SaberHeader * saber = &((Node*) lParam)->Saber;
            sPrint <<           "== Lightsaber ==";
            sPrint << "\r\n" << "Offset to Verts: " << saber->nOffsetToSaberVerts;
            sPrint << "\r\n" << "Offset to UVs: " << saber->nOffsetToSaberUVs;
            sPrint << "\r\n" << "Offset to Normals: " << saber->nOffsetToSaberNormals;
            sPrint << "\r\n";
            sPrint << "\r\n" << "Mesh Inverted Counters: " << saber->nInvCount1 << ", " << saber->nInvCount2;
        }
        else if(cItem.at(1) == "Lightsaber"){
            VertexData * saber = (VertexData *) lParam;
            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Vertex:  " << PrepareFloat(saber->vVertex.fX);
            sPrint << "\r\n" << "         " << PrepareFloat(saber->vVertex.fY);
            sPrint << "\r\n" << "         " << PrepareFloat(saber->vVertex.fZ);
            sPrint << "\r\n" << "Normal:  " << PrepareFloat(saber->vNormal.fX);
            sPrint << "\r\n" << "         " << PrepareFloat(saber->vNormal.fY);
            sPrint << "\r\n" << "         " << PrepareFloat(saber->vNormal.fZ);
            sPrint << "\r\n" << "UV: " << PrepareFloat(saber->vUV1.fX);
            sPrint << "\r\n" << "    " << PrepareFloat(saber->vUV1.fY);
        }

        /// ELSE ///
        else sPrint.flush();
    }
    else if(bWok || bPwk || bDwk0 || bDwk1 || bDwk2){
        BWMHeader & Data = (bWok? *Model.Wok->GetData() : (bPwk? *Model.Pwk->GetData() : (bDwk0? *Model.Dwk0->GetData() : (bDwk1? *Model.Dwk1->GetData() : *Model.Dwk2->GetData()))));

        if(cItem.at(0) == "") sPrint.flush();
        else if(cItem.at(0) == "Header"){
            sPrint << "Walkmesh Type: " << Data.nType;
            if(Data.nType == 0) sPrint << " (pwk/dwk)";
            else if(Data.nType == 1) sPrint << " (wok)";
            sPrint << "\r\n";
            sPrint << "\r\n" << "Position: " << PrepareFloat(Data.vPosition.fX);
            sPrint << "\r\n" << "          " << PrepareFloat(Data.vPosition.fY);
            sPrint << "\r\n" << "          " << PrepareFloat(Data.vPosition.fZ);
            sPrint << "\r\n";
            sPrint << "\r\n" << "Relative Use Hook 1: " << PrepareFloat(Data.vUse1.fX);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vUse1.fY);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vUse1.fZ);
            sPrint << "\r\n" << "Relative Use Hook 2: " << PrepareFloat(Data.vUse2.fX);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vUse2.fY);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vUse2.fZ);
            sPrint << "\r\n" << "Absolute Use Hook 1: " << PrepareFloat(Data.vDwk1.fX);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vDwk1.fY);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vDwk1.fZ);
            sPrint << "\r\n" << "Absolute Use Hook 2: " << PrepareFloat(Data.vDwk2.fX);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vDwk2.fY);
            sPrint << "\r\n" << "                     " << PrepareFloat(Data.vDwk2.fZ);
        }
        else if(cItem.at(0) == "Aabb"){
            sPrint <<           "== Aabb ==";
            sPrint << "\r\n" << "Offset: " << Data.nOffsetToAabb;
            sPrint << "\r\n" << "Count: " << Data.nNumberOfAabb;
        }
        else if(cItem.at(1) == "Aabb"){
            Aabb * aabb = (Aabb * ) lParam;
            std::string sProperty;
            if(aabb->nProperty == 1) sProperty = "Positive X";
            else if(aabb->nProperty == 2) sProperty = "Positive Y";
            else if(aabb->nProperty == 4) sProperty = "Positive Z";
            else if(aabb->nProperty == 8) sProperty = "Negative X";
            else if(aabb->nProperty == 16) sProperty = "Negative Y";
            else if(aabb->nProperty == 32) sProperty = "Negative Z";
            else sProperty = "None";
            sPrint <<           "== " << cItem.at(0) << " ==";
            sPrint << "\r\n" << "Bounding Box Min: " << PrepareFloat(aabb->vBBmin.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmin.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmin.fZ);
            sPrint << "\r\n" << "Bounding Box Max: " << PrepareFloat(aabb->vBBmax.fX);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmax.fY);
            sPrint << "\r\n" << "                  " << PrepareFloat(aabb->vBBmax.fZ);
            sPrint << "\r\n" << "Face Index: " << aabb->nID;
            sPrint << "\r\n" << "2nd Child Property: " << aabb->nProperty << " (" << sProperty << ")";
            sPrint << "\r\n" << "Child 1 Index: " << (signed int) aabb->nChild1;
            sPrint << "\r\n" << "Child 2 Index: " << (signed int) aabb->nChild2;
            sPrint << "\r\n" << "Unknown: " << aabb->nExtra;
        }
        else if((cItem.at(0) == "Vertices")){
            sPrint <<           "== Vertices ==";
            sPrint << "\r\n" << "Offset: " << Data.nOffsetToVerts;
            sPrint << "\r\n" << "Count: " << Data.nNumberOfVerts;
        }
        else if(cItem.at(1) == "Vertices"){
            Vector * vert = (Vector * ) lParam;
            sPrint <<           "== " << cItem.at(0).c_str() << "==";
            sPrint << "\r\n" << "x: " << PrepareFloat(vert->fX);
            sPrint << "\r\n" << "y: " << PrepareFloat(vert->fY);
            sPrint << "\r\n" << "z: " << PrepareFloat(vert->fZ);
        }
        else if((cItem.at(0) == "Faces")){
            sPrint <<           "== Faces ==";
            sPrint << "\r\n" << "Offset to Indices: " << Data.nOffsetToIndices;
            sPrint << "\r\n" << "Offset to Material IDs: " << Data.nOffsetToMaterials;
            sPrint << "\r\n" << "Offset to Face Normals: " << Data.nOffsetToNormals;
            sPrint << "\r\n" << "Offset to Face Distances: " << Data.nOffsetToDistances;
            sPrint << "\r\n" << "Count: " << Data.nNumberOfFaces;
            sPrint << "\r\n" << "Offset to Adjacent Edges: " << Data.nOffsetToAdjacentFaces;
            sPrint << "\r\n" << "Adjacent Edge Count: " << Data.nNumberOfAdjacentFaces;
        }
        else if(cItem.at(1) == "Faces"){
            Face * face = (Face * ) lParam;
            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Face Normal: " << PrepareFloat(face->vNormal.fX);
            sPrint << "\r\n" << "             " << PrepareFloat(face->vNormal.fY);
            sPrint << "\r\n" << "             " << PrepareFloat(face->vNormal.fZ);
            sPrint << "\r\n" << "Plane Distance: " << PrepareFloat(face->fDistance);
            sPrint << "\r\n" << "Material ID: " << face->nMaterialID;
            if(face->nMaterialID != 7)
                sPrint << "\r\n" << "Adjacent Edges: " << face->nAdjacentFaces[0] << ", " << face->nAdjacentFaces[1] << ", " << face->nAdjacentFaces[2];
            sPrint << "\r\n" << "Vertex Indices: " << face->nIndexVertex[0] << ", " << face->nIndexVertex[1] << ", " << face->nIndexVertex[2];
            /*
            sPrint << "\r\n";
            sPrint << "\r\n" << "Bounding Box Min: " << std::setprecision(5) << face->vBBmin.fX;
            sPrint << "\r\n" << "                  " << std::setprecision(5) << face->vBBmin.fY;
            sPrint << "\r\n" << "                  " << std::setprecision(5) << face->vBBmin.fZ;
            sPrint << "\r\n" << "Bounding Box Max: " << std::setprecision(5) << face->vBBmax.fX;
            sPrint << "\r\n" << "                  " << std::setprecision(5) << face->vBBmax.fY;
            sPrint << "\r\n" << "                  " << std::setprecision(5) << face->vBBmax.fZ;
            */
        }
        else if(cItem.at(0) == "Edges"){
            sPrint <<           "== Edges ==";
            sPrint << "\r\n" << "Offset: " << Data.nOffsetToEdges;
            sPrint << "\r\n" << "Count: " << Data.nNumberOfEdges;
        }
        else if(cItem.at(1) == "Edges"){
            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Index: " << ((Edge*) lParam)->nIndex;
            sPrint << "\r\n" << "Transition: " << ((Edge*) lParam)->nTransition;
        }
        else if(cItem.at(0) == "Perimeters"){
            sPrint <<           "== Perimeters ==";
            sPrint << "\r\n" << "Offset: " << Data.nOffsetToPerimeters;
            sPrint << "\r\n" << "Count: " << Data.nNumberOfPerimeters;
        }
        else if(cItem.at(1) == "Perimeters"){
            sPrint <<           "== " << cItem.at(0).c_str() << " ==";
            sPrint << "\r\n" << "Final Edge: " << *((int*) lParam);
        }
        else sPrint.flush();
    }
    else sPrint.flush();
}
