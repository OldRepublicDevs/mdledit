#include "MDL.h"

/**
    Functions:
    AddMenuLines() //frame.h
    Append() //Helper
    AppendAabb() //Helper
    AppendChildren() //Helper
    BuildTree() //frame.h
    DetermineDisplayText //frame.h
/**/

void AddMenuLines(std::vector<std::string>cItem, LPARAM lParam, MenuLineAdder * pmla){
    bool bVertex = false;
    if(cItem[0].length() > 6){
        if(cItem[0].substr(0, 6) == "Vertex") bVertex = true;
    }

    if(cItem[0] == "") return;

    /// Node ///
    else if((cItem[1] == "Geometry") || ((cItem[3] == "Geometry") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node* node = (Node*) lParam;
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_CONTROLLER_DATA, "View ascii");
        pmla->nIndex++;
        if(node->Head.nType & NODE_HAS_MESH){
            InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_GEO_VIEWER, "View geometry");
            pmla->nIndex++;
        }
    }
    else if(cItem[1] == "Animations"){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_CONTROLLER_DATA, "View ascii");
        pmla->nIndex++;
    }
    else if((cItem[1] == "Animated Nodes") || ((cItem[3] == "Animated Nodes") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_CONTROLLER_DATA, "View ascii");
        pmla->nIndex++;
    }
    else if((cItem[1] == "Controllers") && cItem.at(0) != "Controller Data"){
        //Controller * ctrl = (Controller*) lParam;
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_CONTROLLER_DATA, "View ascii");
        pmla->nIndex++;
    }
    else if(cItem[1] == "Saber Data" && bVertex){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    else if(cItem[1] == "Vertices" && bVertex){
        InsertMenu(pmla->hMenu, pmla->nIndex, MF_BYPOSITION | MF_STRING, IDPM_OPEN_EDITOR, "Edit");
        pmla->nIndex++;
    }
    else if(cItem[0] == "Trimesh Flags"){
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
    //std::cout<<"AppendAabb() DEBUG: the values are: "<<AABB->f1<<", "<<AABB->f2<<", "<<AABB->f3<<".\n";
    char cAabb [255];
    sprintf(cAabb, "aabb %i", nCount);
    Append(cAabb, (LPARAM) AABB, TopLevel);
    nCount++;
    if(AABB->nChild1 > 0) AppendAabb(&(AABB->Child1[0]), TopLevel, nCount);
    if(AABB->nChild2 > 0) AppendAabb(&(AABB->Child2[0]), TopLevel, nCount);
}

HTREEITEM AppendChildren(Node & node, HTREEITEM Prev, std::vector<Name> & Names, MDL & Mdl){
        if(node.Head.nType & NODE_HAS_LIGHT){
            HTREEITEM Light = Append("Light", (LPARAM) &node.Light, Prev);
            Append("Flare Radius", (LPARAM) &node.Light.fFlareRadius, Light);
            //Append("Unknown Array", (LPARAM) &node.Light);
            Append("Flare Sizes", (LPARAM) &node.Light);
            Append("Flare Positions", (LPARAM) &node.Light);
            Append("Flare Color Shifts", (LPARAM) &node.Light);
            Append("Flare Textures", (LPARAM) &node.Light);
            Append("Light Priority", (LPARAM) &node.Light.nLightPriority);
            Append("Ambient Only", (LPARAM) &node.Light.nAmbientOnly);
            Append("Dynamic Type", (LPARAM) &node.Light.nDynamicType);
            Append("Affect Dynamic", (LPARAM) &node.Light.nAffectDynamic);
            Append("Shadow", (LPARAM) &node.Light.nShadow);
            Append("Flare", (LPARAM) &node.Light.nFlare);
            Append("Fading Light", (LPARAM) &node.Light.nFadingLight);
        }
        if(node.Head.nType & NODE_HAS_EMITTER){
            HTREEITEM Emitter = Append("Emitter", (LPARAM) &node.Emitter, Prev);
            Append("Dead Space", (LPARAM) &node.Emitter.fDeadSpace, Emitter);
            Append("Blast Radius", (LPARAM) &node.Emitter.fBlastRadius);
            Append("Blast Length", (LPARAM) &node.Emitter.fBlastLength);
            Append("Branch Count", (LPARAM) &node.Emitter.nBranchCount);
            Append("Control Point Smoothing", (LPARAM) &node.Emitter.fControlPointSmoothing);
            Append("X Grid", (LPARAM) &node.Emitter.nxGrid);
            Append("Y Grid", (LPARAM) &node.Emitter.nyGrid);
            Append("Spawn Type", (LPARAM) &node.Emitter.nSpawnType);
            Append("Update", (LPARAM) node.Emitter.cUpdate.c_str());
            Append("Render", (LPARAM) node.Emitter.cRender.c_str());
            Append("Blend", (LPARAM) node.Emitter.cBlend.c_str());
            Append("Texture", (LPARAM) node.Emitter.cTexture.c_str());
            Append("Chunk Name", (LPARAM) node.Emitter.cChunkName.c_str());
            Append("Twosided Texture", (LPARAM) &node.Emitter.nTwosidedTex);
            Append("Loop", (LPARAM) &node.Emitter.nLoop);
            //Append("Render Order", (LPARAM) &node.Emitter.nRenderOrder);
            Append("Unknown Int16 1", (LPARAM) &node.Emitter.nUnknown1);
            Append("Frame Blending", (LPARAM) &node.Emitter.nFrameBlending);
            Append("Depth Texture Name", (LPARAM) node.Emitter.cDepthTextureName.c_str());
            Append("Unknown Byte 1", (LPARAM) &node.Emitter.nUnknown2);

            Append("Emitter Flags?", (LPARAM) &node.Emitter.nFlags);
        }
        if(node.Head.nType & NODE_HAS_MESH){
            HTREEITEM Mesh = Append("Mesh", (LPARAM) &node.Mesh, Prev);
            //Append("Mesh Inverted Counters Array", (LPARAM) &node.Mesh, Mesh);
            HTREEITEM Vertices = Append("Vertices", (LPARAM) &node.Mesh, Mesh);
            if(node.Mesh.Vertices.size() > 0){
                Append("MDX Data Pointers", (LPARAM) &node.Mesh, Vertices);
                char cVert [255];
                HTREEITEM Vert;
                for(int n = 0; n < node.Mesh.Vertices.size(); n++){
                    sprintf(cVert, "Vertex %i", n);
                    Vert = Append(cVert, (LPARAM) &(node.Mesh.Vertices[n]), Vertices);
                    //if(node.Mesh.nMdxDataSize > 0) Append("MDX Data", (LPARAM) &(node.Mesh.Vertices[n].MDXData), Vert);
                }
                if(node.Mesh.nMdxDataSize > 0 && Mdl.Mdx) Append("Extra MDX Data", (LPARAM) &(node.Mesh.MDXData), Vertices);
            }
            HTREEITEM Faces = Append("Faces", (LPARAM) &node.Mesh, Mesh);
            if(node.Mesh.Faces.size() > 0){
                //Append("Number of Vertex Indices 2 Array", (LPARAM) &node.Mesh, Faces);
                //Append("Location of Vertex Indices 2 Array", (LPARAM) &node.Mesh, Faces);
                char cFace [255];
                HTREEITEM Face;
                for(int n = 0; n < node.Mesh.Faces.size(); n++){
                    sprintf(cFace, "Face %i", n);
                    Face = Append(cFace, (LPARAM) &(node.Mesh.Faces[n]), Faces);
                    if(node.Mesh.IndexLocationArray.nCount > 0) Append("Vertex Indices 2", (LPARAM) &(node.Mesh.VertIndices[n]), Face);
                }
            }
            Append("Bounding Box Min", (LPARAM) &node.Mesh.vBBmin, Mesh);
            Append("Bounding Box Max", (LPARAM) &node.Mesh.vBBmax, Mesh);
            Append("Radius", (LPARAM) &node.Mesh.fRadius, Mesh);
            Append("Average", (LPARAM) &node.Mesh.vAverage);
            Append("Diffuse Color", (LPARAM) &node.Mesh.fDiffuse);
            Append("Ambient Color", (LPARAM) &node.Mesh.fAmbient);
            Append("Transparency Hint", (LPARAM) &node.Mesh.nTransparencyHint);
            Append("Textures", (LPARAM) &node.Mesh);
            Append("Trimesh Flags", (LPARAM) &node);
            Append("Animated UV", (LPARAM) &node.Mesh);
            //Append("Unknown Array of 3 Integers", (LPARAM) node.Mesh.nUnknown3);
            if(Mdl.bK2) Append("Dirt", (LPARAM) &node.Mesh);
            Append("Total Area", (LPARAM) &node.Mesh.fTotalArea);
            Append("Unknown Lightsaber Bytes", (LPARAM) &node.Mesh);
            //Append("Padding", (LPARAM) &node.Mesh.nPadding);
        }
        if(node.Head.nType & NODE_HAS_SKIN){
            char cBone [255];
            HTREEITEM Skin = Append("Skin", (LPARAM) &node.Skin, Prev);
            HTREEITEM Bones = Append("Bones", (LPARAM) &node.Skin, Skin);
            if(node.Skin.Bones.size() > 0){
                for(int n = 0; n < node.Skin.Bones.size(); n++){
                    std::string sBone = "Bone " + Names[n].sName;
                    Append(sBone, (LPARAM) &(node.Skin.Bones.at(n)), Bones);
                }
            }
            Append("Bone Indexes", (LPARAM) &node.Skin.nBoneIndexes, Skin);
            Append("MDX Data Pointers", (LPARAM) &node.Skin, Skin);
        }
        if(node.Head.nType & NODE_HAS_DANGLY){
            HTREEITEM Danglymesh = Append("Danglymesh", (LPARAM) &node.Dangly, Prev);
            Append("Constraints", (LPARAM) &node.Dangly, Danglymesh);
            Append("Displacement", (LPARAM) &node.Dangly.fDisplacement);
            Append("Tightness", (LPARAM) &node.Dangly.fTightness);
            Append("Period", (LPARAM) &node.Dangly.fPeriod);
            HTREEITEM Data2 = Append("Data2", (LPARAM) &node.Dangly);
            if(node.Dangly.Constraints.size() > 0){
                std::stringstream ssVert;
                for(int i = 0; i < node.Dangly.ConstraintArray.nCount; i++){
                    ssVert.str(std::string());
                    ssVert << "Vertex " << i;
                    Append(ssVert.str(), (LPARAM) &(node.Dangly.Data2.at(i)), Data2);
                }
            }
        }
        if(node.Head.nType & NODE_HAS_AABB){
            HTREEITEM Walkmesh = Append("Walkmesh", (LPARAM) &node.Walkmesh, Prev);
            int nCounter = 0;
            if(node.Walkmesh.nOffsetToAabb > 0) AppendAabb(&(node.Walkmesh.RootAabb), Walkmesh, nCounter);
        }
        if(node.Head.nType & NODE_HAS_SABER){
            HTREEITEM Saber = Append("Saber", (LPARAM) &node.Saber, Prev);
            HTREEITEM Data = Append("Saber Data", (LPARAM) &node.Saber, Saber);
            if(node.Mesh.nNumberOfVerts > 0){
                char cSaber [255];
                for(int i = 0; i < node.Mesh.nNumberOfVerts; i++){
                    sprintf(cSaber, "Vertex %i", i);
                    Append(cSaber, (LPARAM) &(node.Saber.SaberData[i]), Data);
                }

            }
            Append("Mesh Inverted Counter 1", (LPARAM) &node.Saber.nInvCount1, Saber);
            Append("Mesh Inverted Counter 2", (LPARAM) &node.Saber.nInvCount2, Saber);
        }
}

void BuildTree(MDL & Mdl){
    if(!Mdl.GetFileData()){
        std::cout<<"No data. Do not build tree.\n";
        return;
    }
    FileHeader & Data = *Mdl.GetFileData();

    HTREEITEM Root = Append(Mdl.GetFilename(), NULL, TVI_ROOT);
    HTREEITEM Header = Append("Header", (LPARAM) &(Data.MH), Root);
    Append("Bounding Box Min", (LPARAM) (&Data.MH.vBBmin), Header);
    Append("Bounding Box Max", (LPARAM) (&Data.MH.vBBmax));
    Append("Radius", (LPARAM) &(Data.MH.fRadius));
    Append("Scale", (LPARAM) &(Data.MH.fScale));

    HTREEITEM Animations = Append("Animations", NULL, Root);
    HTREEITEM Nodes, Animation, Sounds, CurrentNode;
    for(int n = 0; n < Data.MH.Animations.size(); n++){
        Animation = Append(Data.MH.Animations[n].sName, (LPARAM) &(Data.MH.Animations[n]), Animations);
        Append("Length", (LPARAM) &(Data.MH.Animations[n].fLength), Animation);
        Append("Transition", (LPARAM) &(Data.MH.Animations[n].fTransition));
        Sounds = Append("Sounds", (LPARAM) &(Data.MH.Animations[n]));
        for(int i = 0; i < Data.MH.Animations[n].Sounds.size(); i++){
            Append(Data.MH.Animations[n].Sounds[i].sName.c_str(), (LPARAM) &(Data.MH.Animations[n].Sounds[i]), Sounds);
        }
        Nodes = Append("Animated Nodes", NULL, Animation);
        for(int a = 0; a < Data.MH.Animations[n].ArrayOfNodes.size(); a++){
            Node & node = Data.MH.Animations[n].ArrayOfNodes[a];

            CurrentNode = Append(Data.MH.Names[node.Head.nNameIndex].sName, (LPARAM) &node, Nodes);

            HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
            if(!node.Head.ControllerData.empty()) Append("Controller Data", (LPARAM) &node.Head, Controllers);
            for(int n = 0; n < node.Head.Controllers.size(); n++){
                int nCtrlIndex = node.Head.Controllers[n].nNameIndex;
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
                if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                    Append(Data.MH.Names[curnode.Head.nNameIndex].sName, (LPARAM) &curnode, Children);
                }
            }
        }
    }

    Nodes = Append("Geometry", NULL, Root);
    for(int a = 0; a < Data.MH.ArrayOfNodes.size(); a++){
        Node & node = Data.MH.ArrayOfNodes[a];
        if(node.Head.nType != 0){
            std::string sType;
            if(node.Head.nType & NODE_HAS_SABER) sType = "(saber) ";
            else if(node.Head.nType & NODE_HAS_AABB) sType = "(walkmesh) ";
            else if(node.Head.nType & NODE_HAS_DANGLY) sType = "(danglymesh) ";
            else if(node.Head.nType & NODE_HAS_SKIN) sType = "(skin) ";
            else if(node.Head.nType & NODE_HAS_MESH) sType = "(mesh) ";
            else if(node.Head.nType & NODE_HAS_EMITTER) sType = "(emitter) ";
            else if(node.Head.nType & NODE_HAS_LIGHT) sType = "(light) ";
            else if(node.Head.nType & NODE_HAS_HEADER) sType = "(basic) ";
            else sType = "(unknown) ";
            CurrentNode = Append(sType + Data.MH.Names[node.Head.nNameIndex].sName, (LPARAM) &node, Nodes);

            AppendChildren(node, CurrentNode, Data.MH.Names, Mdl);

            HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
            if(!node.Head.ControllerData.empty()) Append("Controller Data", (LPARAM) &node.Head, Controllers);
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
                if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                    Append(Data.MH.Names[curnode.Head.nNameIndex].sName, (LPARAM) &curnode, Children);
                }
            }
        }
    }

    TreeView_Expand(hTree, Root, TVE_EXPAND);
    std::cout<<"Tree building done!\n";
}


extern char cReturn[4][255];
char * PrepareFloat(double fFloat, unsigned int n);

extern MDL Model;
extern WOK Walkmesh;

void DetermineDisplayText(std::vector<std::string>cItem, std::stringstream & sPrint, LPARAM lParam){
    bool bMdl = false;
    bool bWok = false;
    for(int j = 0; !bMdl && !bWok; j++){
        if(cItem[j] == Model.GetFilename()) bMdl = true;
        else if(cItem[j] == Walkmesh.GetFilename()) bWok = true;
    }

    FileHeader & Data = *Model.GetFileData();

    if(cItem[0] == "") sPrint.flush();

    /// Header ///
    else if(cItem[0] == "Header"){
            std::string sModelType;
            if(Data.MH.GH.nModelType == 1) sModelType = "geometry";
            else if(Data.MH.GH.nModelType == 2) sModelType = "model";
            else if(Data.MH.GH.nModelType == 5) sModelType = "animation";
            else sModelType = "unknown";
            sPrint << "Header";
            sPrint << "\r\nModel Name: "<<Data.MH.GH.sName.c_str();
            sPrint << "\r\nModel Type: "<<Data.MH.GH.nModelType<<" ("<<sModelType<<")";
            sPrint << "\r\n   Padding: "<<(int)Data.MH.GH.nPadding[0]<<" "<<(int)Data.MH.GH.nPadding[1]<<" "<<(int)Data.MH.GH.nPadding[2];
            sPrint << "\r\nClassification: "<<ReturnClassificationName(Data.MH.nClassification).c_str();
            sPrint << "\r\n   Classification numbers: "<<(int)Data.MH.nUnknown1[0]<<" "<<(int)Data.MH.nUnknown1[1]<<" "<<(int)Data.MH.nUnknown1[2];
            sPrint << "\r\nSupermodel: "<<Data.MH.cSupermodelName.c_str();
            sPrint << "\r\nSupermodel Reference: "<<Data.MH.nSupermodelReference;
            sPrint << "\r\n";
            sPrint << "\r\nMDL Length: "<<Data.nMdlLength;
            sPrint << "\r\nMDX Length: "<<Data.nMdxLength;
            sPrint << "\r\nFunction Pointer 0: "<<Data.MH.GH.nFunctionPointer0;
            sPrint << "\r\nFunction Pointer 1: "<<Data.MH.GH.nFunctionPointer1;
    }
    else if(cItem[0] == "Bounding Box Min") sPrint << "Bounding Box Min:"<<"\r\nx: "<<((Vector*) lParam)->fX<<"\r\ny: "<<((Vector*) lParam)->fY<<"\r\nz: "<<((Vector*) lParam)->fZ;
    else if(cItem[0] == "Bounding Box Max") sPrint << "Bounding Box Max:"<<"\r\nx: "<<((Vector*) lParam)->fX<<"\r\ny: "<<((Vector*) lParam)->fY<<"\r\nz: "<<((Vector*) lParam)->fZ;
    else if(cItem[0] == "Radius") sPrint << "Radius:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Scale") sPrint << "Scale:\r\n" << *((double*) lParam);

    /// Animations ///
    else if(cItem[0] == "Animations"){
        sPrint << "Animations";
        sPrint << "\r\nOffset to Animation Array: "<<Data.MH.AnimationArray.nOffset;
        sPrint << "\r\nAnimation Count: "<<Data.MH.Animations.size();
    }
    else if(cItem[1] == "Animations"){
        Animation * anim = (Animation * ) lParam;
        std::string sModelType;
        if(anim->nModelType == 1) sModelType = "geometry";
        else if(anim->nModelType == 2) sModelType = "model";
        else if(anim->nModelType == 5) sModelType = "animation";
        else sModelType = "unknown";
        sPrint << "Animation "<<anim->sName.c_str();
        sPrint << "\r\nOffset: "<<anim->nOffset;
        sPrint << "\r\nOffset to Root: "<<anim->nOffsetToRootAnimationNode;
        sPrint << "\r\nAnimation Root: "<<anim->sAnimRoot.c_str();
        sPrint << "\r\nNumber of Names: "<<anim->nNumberOfNames;
        sPrint << "\r\n";
        sPrint << "\r\nModel Type: "<<(int)anim->nModelType<<" ("<<sModelType<<")";
        sPrint << "\r\nPadding: "<<(int)anim->nPadding[0]<<" "<<(int)anim->nPadding[1]<<" "<<(int)anim->nPadding[2];
        sPrint << "\r\n";
        sPrint << "\r\nFunction Pointer 0: "<<anim->nFunctionPointer0;
        sPrint << "\r\nFunction Pointer 1: "<<anim->nFunctionPointer1;
    }
    else if(cItem[0] == "Length") sPrint << "Length:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Transition") sPrint << "Transition:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Sounds"){
        Animation * anim = (Animation * ) lParam;
        sPrint << string_format("Sounds\r\nOffset: %u\r\nCount: %u", anim->SoundArray.nOffset, anim->SoundArray.nCount);
    }
    else if(cItem[1] == "Sounds"){
        Sound * snd = (Sound*) lParam;
        sPrint << string_format("Sound:\r\n%s\r\n\r\nTime:\r\n%f", snd->sName.c_str(), snd->fTime);
    }

    /// Geometry ///
    else if(cItem[0] == "Geometry"){
        sPrint << "Geometry";
        sPrint << "\r\nOffset to Name Array: "<<Data.MH.NameArray.nOffset;
        sPrint << "\r\nOffset to Root Node: "<<Data.MH.GH.nOffsetToRootNode;
        sPrint << "\r\nOffset to Head Root Node: "<<Data.MH.nOffsetToHeadRootNode;
        sPrint << "\r\nName Count: "<<Data.MH.Names.size();
        sPrint << "\r\nNode Count: "<<Data.MH.nNodeCount;
        sPrint << "\r\nTotal Node Count (with Supermodel): "<<Data.MH.GH.nTotalNumberOfNodes;
    }

    /// Node ///
    else if(((cItem[1] == "Geometry") || (cItem[1] == "Animated Nodes") || (cItem[1] == "Children") || (cItem[1] == "Parent")) && !bWok){
        Node * node = (Node * ) lParam;
        //std::cout<<"Current name in problematic position: "<<cItem[0].c_str()<<"\n";
        sPrint<<node->Head.nType<<" (";
        if(node->Head.nType & NODE_HAS_DANGLY) sPrint << "danglymesh";
        else if(node->Head.nType & NODE_HAS_SKIN) sPrint << "skin";
        else if(node->Head.nType & NODE_HAS_SABER) sPrint << "saber";
        else if(node->Head.nType & NODE_HAS_AABB) sPrint << "walkmesh";
        else if(node->Head.nType & NODE_HAS_MESH) sPrint << "mesh";
        else if(node->Head.nType & NODE_HAS_EMITTER) sPrint << "emitter";
        else if(node->Head.nType & NODE_HAS_LIGHT) sPrint << "light";
        else if(node->Head.nType & NODE_HAS_HEADER) sPrint << "basic";
        else sPrint << "unknown - file likely faulty!";
        sPrint << ") "<<Data.MH.Names[node->Head.nNameIndex].sName.c_str();
        sPrint << "\r\nOffset: "<<node->nOffset;
        sPrint << "\r\nOffset to Root: "<<node->Head.nOffsetToRoot;
        sPrint << "\r\nOffset to Parent: "<<node->Head.nOffsetToParent;
        sPrint << "\r\nName Index: "<<node->Head.nNameIndex;
        sPrint << "\r\nID: "<<node->Head.nID1;
        if(node->nAnimation == -1){
            sPrint << "\r\nPosition:";
            sPrint << "\r\n  x: "<<PrepareFloat(node->Head.vPos.fX, 0);
            sPrint << "\r\n  y: "<<PrepareFloat(node->Head.vPos.fY, 0);
            sPrint << "\r\n  z: "<<PrepareFloat(node->Head.vPos.fZ, 0);
            sPrint << "\r\nOrientation:";
            sPrint << "\r\n  x: "<<PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fX, 0);//<<" (AA "<<PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fX, 1)<<")";
            sPrint << "\r\n  y: "<<PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fY, 0);//<<" (AA "<<PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fY, 1)<<")";
            sPrint << "\r\n  z: "<<PrepareFloat(node->Head.oOrient.GetQuaternion().vAxis.fZ, 0);//<<" (AA "<<PrepareFloat(node->Head.oOrient.GetAxisAngle().vAxis.fZ, 1)<<")";
            sPrint << "\r\n  w: "<<PrepareFloat(node->Head.oOrient.GetQuaternion().fW, 0);//<<" (AA "<<PrepareFloat(node->Head.oOrient.GetAxisAngle().fAngle, 1)<<")";
        }
    }/*
    else if(cItem[0] == "Position") sPrint << string_format("Position: \r\nx: %f\r\ny: %f\r\nz: %f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if(cItem[0] == "Orientation"){
        Orientation * orient = (Orientation*) lParam;
        sPrint << "Orientation: \r\nx: "<<orient->qX<<" (aa "<<orient->fX<<")\r\ny: "<<orient->qY<<" (aa "<<orient->fY<<")\r\nz: "<<orient->qZ<<" (aa "<<orient->fZ<<")\r\nw: "<<orient->qW<<" (aa "<<orient->fAngle<<")";
    }*/
    else if(cItem[0] == "Controllers"){
        Header * head = (Header * ) lParam;
        sPrint << string_format("Controllers\r\nOffset: %u\r\nCount: %u", head->ControllerArray.nOffset, head->ControllerArray.nCount);
    }
    else if(cItem[0] == "Controller Data"){
        Header * head = (Header * ) lParam;
        std::vector<double> & fFloats = head->ControllerData;
        sPrint << string_format("Controller Data\r\nOffset: %u\r\nCount: %u", head->ControllerDataArray.nOffset, head->ControllerDataArray.nCount);
        if(fFloats.size() > 0) sPrint << "\r\n\r\nData:";
        char cSpaces [10];
        int i = 0;
        while(i < fFloats.size()){
            //std::cout<<string_format("Printing Controller Data float %i\n", i);
            if(fFloats[i] >= 0.0) sprintf(cSpaces, " ");
            else sprintf(cSpaces, "");
            if(i+1 < 10) strcat(cSpaces, " ");
            sPrint << string_format("\r\n%i.%s %f", i, cSpaces, fFloats[i]);
            i++;
        }
    }
    else if(cItem[1] == "Controllers"){
        Controller * ctrl = (Controller*) lParam;
        sPrint << string_format("%s\r\n\r\nController type:  %i\r\nUnknown int16:   %hi\r\nValue Count:      %hi\r\nTimekey Start:    %hi\r\nData Start:       %hi\r\nColumn Count:     %hhi\r\nPadding?:         %hhi, %hhi, %hhi",
                cItem[0].c_str(), ctrl->nControllerType, ctrl->nUnknown2, ctrl->nValueCount, ctrl->nTimekeyStart, ctrl->nDataStart, ctrl->nColumnCount, ctrl->nPadding[0], ctrl->nPadding[1], ctrl->nPadding[2]);
    }
    else if(cItem[0] == "Children"){
        Header * head = (Header * ) lParam;
        sPrint << string_format("Children\r\nOffset: %u\r\nCount: %u", head->ChildrenArray.nOffset, head->ChildrenArray.nCount);
    }

    /// Light ///
    else if(cItem[0] == "Flare Radius") sPrint << "Flare Radius:\r\n" << *((float*) lParam);
    else if(cItem[0] == "Unknown Array"){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Unknown Array\r\nOffset: %u\r\nCount: %u", light->UnknownArray.nOffset, light->UnknownArray.nCount);
    }
    else if(cItem[0] == "Flare Sizes"){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Sizes\r\nOffset: %u\r\nCount: %u", light->FlareSizeArray.nOffset, light->FlareSizeArray.nCount);
        if(light->FlareSizes.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareSizeArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlareSizes[n], 0);
            }
        }
    }
    else if(cItem[0] == "Flare Positions"){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Positions\r\nOffset: %u\r\nCount: %u", light->FlarePositionArray.nOffset, light->FlareSizeArray.nCount);
        if(light->FlarePositions.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlarePositionArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlarePositions[n], 0);
            }
        }
    }
    else if(cItem[0] == "Flare Color Shifts"){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Color Shifts\r\nOffset: %u\r\nCount: %u", light->FlareColorShiftArray.nOffset, light->FlareColorShiftArray.nCount);
        if(light->FlareColorShifts.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareColorShiftArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlareColorShifts[n].fR, 0) << ", " << PrepareFloat(light->FlareColorShifts[n].fG, 1) << ", " << PrepareFloat(light->FlareColorShifts[n].fB, 2);
            }
        }
    }
    else if(cItem[0] == "Flare Textures"){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Textures\r\nOffset: %u\r\nCount: %u", light->FlareTextureNameArray.nOffset, light->FlareTextureNameArray.nCount);
        if(light->FlareTextureNames.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareTextureNameArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << light->FlareTextureNames[n].sName;
            }
        }
    }
    else if(cItem[0] == "Light Priority") sPrint << "Light Priority:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Ambient Only") sPrint << "Ambient Only:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Dynamic Type") sPrint << "Dynamic Type:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Affect Dynamic") sPrint << "Affect Dynamic:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Shadow") sPrint << "Shadow:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Flare") sPrint << "Flare:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Fading Light") sPrint << "Fading Light:\r\n" << *((int*) lParam);

    /// Emitter ///
    else if(cItem[0] == "Dead Space") sPrint << "Dead Space:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Blast Radius") sPrint << "Blast Radius:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Blast Length") sPrint << "Blast Length:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Branch Count") sPrint << "Branch Count:\r\n" << *((unsigned int*) lParam);
    else if(cItem[0] == "Control Point Smoothing") sPrint << "Control Point Smoothing:\r\n" << *((double*) lParam);
    else if(cItem[0] == "X Grid") sPrint << "X Grid:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Y Grid") sPrint << "Y Grid:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Spawn Type") sPrint << "Spawn Type:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Update") sPrint << "Update:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Render") sPrint << "Render:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Blend") sPrint << "Blend:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Texture") sPrint << "Texture:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Chunk Name") sPrint << "Chunk Name:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Twosided Texture") sPrint << "Twosided Texture:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Loop") sPrint << "Loop:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Unknown Int16 1") sPrint << "Unknown Int16 1:\r\n" << *((short*) lParam);
    //else if(cItem[0] == "Render Order") sPrint << string_format("Render Order:\r\n%u", *((short*) lParam));
    else if(cItem[0] == "Frame Blending") sPrint << "Frame Blending:\r\n" << (int) *((unsigned char*) lParam);
    else if(cItem[0] == "Depth Texture Name") sPrint << "Depth Texture Name:\r\n" << ((char*) lParam);
    else if(cItem[0] == "Unknown Byte 1") sPrint << "Unknown Byte 1:\r\n" << (int) *((unsigned char*) lParam);
    else if(cItem[0] == "Emitter Flags?"){
        sPrint << "Emitter Flags??";
        sPrint << "\r\np2p:            "<<(*((int*) lParam) & EMITTER_FLAG_P2P ? 1 : 0);
        sPrint << "\r\np2p_sel:        "<<(*((int*) lParam) & EMITTER_FLAG_P2P_SEL ? 1 : 0);
        sPrint << "\r\naffected_wind:  "<<(*((int*) lParam) & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0);
        sPrint << "\r\ntinted:         "<<(*((int*) lParam) & EMITTER_FLAG_TINTED ? 1 : 0);
        sPrint << "\r\nbounce:         "<<(*((int*) lParam) & EMITTER_FLAG_BOUNCE ? 1 : 0);
        sPrint << "\r\nrandom:         "<<(*((int*) lParam) & EMITTER_FLAG_RANDOM ? 1 : 0);
        sPrint << "\r\ninherit:        "<<(*((int*) lParam) & EMITTER_FLAG_INHERIT ? 1 : 0);
        sPrint << "\r\ninherit_vel:    "<<(*((int*) lParam) & EMITTER_FLAG_INHERIT_VEL ? 1 : 0);
        sPrint << "\r\ninherit_local:  "<<(*((int*) lParam) & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0);
        sPrint << "\r\nsplat:          "<<(*((int*) lParam) & EMITTER_FLAG_SPLAT ? 1 : 0);
        sPrint << "\r\ninherit_part:   "<<(*((int*) lParam) & EMITTER_FLAG_INHERIT_PART ? 1 : 0);
        sPrint << "\r\ndepth_texture?: "<<(*((int*) lParam) & EMITTER_FLAG_DEPTH_TEXTURE ? 1 : 0);
        sPrint << "\r\nrenderorder?:   "<<(*((int*) lParam) & EMITTER_FLAG_RENDER_ORDER ? 1 : 0);
    }

    /// Mesh ///
    else if(cItem[0] == "Mesh"){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Mesh\r\nFunction Pointer 0: %u\r\nFunction Pointer 1: %u",
                mesh->nFunctionPointer0, mesh->nFunctionPointer1);
        sPrint << "\r\n\r\nInverted Counter: "<<mesh->nMeshInvertedCounter<<"\r\n(Offset: "<<mesh->MeshInvertedCounterArray.nOffset<<", Count: "<<mesh->MeshInvertedCounterArray.nCount<<")";
    }
    else if(cItem[0] == "Mesh Inverted Counters Array"){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Mesh Inverted Counters Array\r\nOffset: %u\r\nCount: %u",
                mesh->MeshInvertedCounterArray.nOffset, mesh->MeshInvertedCounterArray.nCount, mesh->nMeshInvertedCounter);
        if(mesh->MeshInvertedCounterArray.nCount > 0){
            sPrint << "\r\nValues: " << mesh->nMeshInvertedCounter;
        }
    }
    else if((cItem[0] == "MDX Data Pointers") && (cItem[2] == "Mesh")){
        MeshHeader * mesh = (MeshHeader *) lParam;
        sPrint << string_format("MDX Data Pointers\r\nOffset: %i\r\nSize: %i\r\nBitmap:\r\n  Vertex: %i (Offset %i)\r\n  Normal: %i (Offset %i)\r\n  UV1:    %i (Offset %i)\r\n  UV2:    %i (Offset %i)\r\n  UV3:    %i (Offset %i)\r\n  UV4:    %i (Offset %i)\r\n  Unknown: %i (Offset %i)\r\n  Tangent1: %i (Offset %i)\r\n  Tangent2?: %i (Offset %i)\r\n  Tangent3?: %i (Offset %i)\r\n  Tangent4?: %i (Offset %i)",
                mesh->nOffsetIntoMdx, mesh->nMdxDataSize,
                mesh->nMdxDataBitmap & MDX_FLAG_VERTEX ? 1 : 0, mesh->nOffsetToVerticesInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_NORMAL ? 1 : 0, mesh->nOffsetToNormalsInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_UV1 ? 1 : 0, mesh->nOffsetToUVsInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_UV2 ? 1 : 0, mesh->nOffsetToUV2sInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_UV3 ? 1 : 0, mesh->nOffsetToUV3sInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_UV4 ? 1 : 0, mesh->nOffsetToUV4sInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_0040 ? 1 : 0, mesh->nOffsetToUnknownInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1 ? 1 : 0, mesh->nOffsetToTangentSpaceInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2 ? 1 : 0, mesh->nOffsetToTangentSpace2InMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3 ? 1 : 0, mesh->nOffsetToTangentSpace3InMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4 ? 1 : 0, mesh->nOffsetToTangentSpace4InMDX);
    }
    else if((cItem[0] == "Vertices") && !bWok){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << "Vertices\r\nOffset: "<<mesh->nOffsetToVertArray;
        sPrint<<"\r\nCount: "<<mesh->nNumberOfVerts;
    }
    else if(cItem[0] == "Extra MDX Data"){
        MDXDataStruct * mdx = (MDXDataStruct * ) lParam;
        sPrint << cItem[0].c_str();
        Node & node = Model.GetNodeByNameIndex(mdx->nNameIndex);
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            sPrint << string_format("\r\n\r\nVertex: %f\r\n        %f\r\n        %f", mdx->vVertex.fX, mdx->vVertex.fY, mdx->vVertex.fZ);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
            sPrint << string_format("\r\n\r\nNormal: %f\r\n        %f\r\n        %f", mdx->vNormal.fX, mdx->vNormal.fY, mdx->vNormal.fZ);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            sPrint << string_format("\r\n\r\nUV1:    %f\r\n        %f", mdx->vUV1.fX, mdx->vUV1.fY);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            sPrint << string_format("\r\n\r\nUV2:    %f\r\n        %f", mdx->vUV2.fX, mdx->vUV2.fY);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            sPrint << string_format("\r\n\r\nUV3:    %f\r\n        %f", mdx->vUV3.fX, mdx->vUV3.fY);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            sPrint << string_format("\r\n\r\nUV4:    %f\r\n        %f", mdx->vUV4.fX, mdx->vUV4.fY);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
            sPrint << string_format("\r\n\r\nTangent Space 1");
            sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent1[0].fX, mdx->vTangent1[0].fY, mdx->vTangent1[0].fZ);
            sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent1[1].fX, mdx->vTangent1[1].fY, mdx->vTangent1[1].fZ);
            sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent1[2].fX, mdx->vTangent1[2].fY, mdx->vTangent1[2].fZ);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
            sPrint << string_format("\r\n\r\nTangent Space 2");
            sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent2[0].fX, mdx->vTangent2[0].fY, mdx->vTangent2[0].fZ);
            sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent2[1].fX, mdx->vTangent2[1].fY, mdx->vTangent2[1].fZ);
            sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent2[2].fX, mdx->vTangent2[2].fY, mdx->vTangent2[2].fZ);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
            sPrint << string_format("\r\n\r\nTangent Space 3");
            sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent3[0].fX, mdx->vTangent3[0].fY, mdx->vTangent3[0].fZ);
            sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent3[1].fX, mdx->vTangent3[1].fY, mdx->vTangent3[1].fZ);
            sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent3[2].fX, mdx->vTangent3[2].fY, mdx->vTangent3[2].fZ);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
            sPrint << string_format("\r\n\r\nTangent Space 4");
            sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent4[0].fX, mdx->vTangent4[0].fY, mdx->vTangent4[0].fZ);
            sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent4[1].fX, mdx->vTangent4[1].fY, mdx->vTangent4[1].fZ);
            sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent4[2].fX, mdx->vTangent4[2].fY, mdx->vTangent4[2].fZ);
        }
        if(node.Head.nType & NODE_HAS_SKIN){
            sPrint << string_format("\r\n\r\nWeight Value: %f\r\n              %f\r\n              %f\r\n              %f", mdx->Weights.fWeightValue[0], mdx->Weights.fWeightValue[1], mdx->Weights.fWeightValue[2], mdx->Weights.fWeightValue[3]);
            sPrint << string_format("\r\n\r\nWeight Index: %f\r\n              %f\r\n              %f\r\n              %f", mdx->Weights.fWeightIndex[0], mdx->Weights.fWeightIndex[1], mdx->Weights.fWeightIndex[2], mdx->Weights.fWeightIndex[3]);
        }
    }
    else if(cItem[1] == "Vertices"){
        Vertex * vert = (Vertex * ) lParam;
        sPrint << string_format("%s\r\nx: %f\r\ny: %f\r\nz: %f",
                cItem[0].c_str(), vert->fX, vert->fY, vert->fZ);

        MDXDataStruct * mdx = &vert->MDXData;
        Node & node = Model.GetNodeByNameIndex(mdx->nNameIndex);
        if(!(node.Head.nType & NODE_HAS_SABER)){
            if(node.Mesh.nMdxDataSize > 0 && Model.Mdx){
                sPrint << "\r\n\r\nMDX Data";
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                    sPrint << string_format("\r\nVertex: %f\r\n        %f\r\n        %f", mdx->vVertex.fX, mdx->vVertex.fY, mdx->vVertex.fZ);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
                    sPrint << string_format("\r\n\r\nNormal: %f\r\n        %f\r\n        %f", mdx->vNormal.fX, mdx->vNormal.fY, mdx->vNormal.fZ);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
                    sPrint << string_format("\r\n\r\nUV1:    %f\r\n        %f", mdx->vUV1.fX, mdx->vUV1.fY);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
                    sPrint << string_format("\r\n\r\nUV2:    %f\r\n        %f", mdx->vUV2.fX, mdx->vUV2.fY);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
                    sPrint << string_format("\r\n\r\nUV3:    %f\r\n        %f", mdx->vUV3.fX, mdx->vUV3.fY);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
                    sPrint << string_format("\r\n\r\nUV4:    %f\r\n        %f", mdx->vUV4.fX, mdx->vUV4.fY);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
                    sPrint << string_format("\r\n\r\nTangent Space 1");
                    sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent1[0].fX, mdx->vTangent1[0].fY, mdx->vTangent1[0].fZ);
                    sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent1[1].fX, mdx->vTangent1[1].fY, mdx->vTangent1[1].fZ);
                    sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent1[2].fX, mdx->vTangent1[2].fY, mdx->vTangent1[2].fZ);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
                    sPrint << string_format("\r\n\r\nTangent Space 2");
                    sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent2[0].fX, mdx->vTangent2[0].fY, mdx->vTangent2[0].fZ);
                    sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent2[1].fX, mdx->vTangent2[1].fY, mdx->vTangent2[1].fZ);
                    sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent2[2].fX, mdx->vTangent2[2].fY, mdx->vTangent2[2].fZ);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
                    sPrint << string_format("\r\n\r\nTangent Space 3");
                    sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent3[0].fX, mdx->vTangent3[0].fY, mdx->vTangent3[0].fZ);
                    sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent3[1].fX, mdx->vTangent3[1].fY, mdx->vTangent3[1].fZ);
                    sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent3[2].fX, mdx->vTangent3[2].fY, mdx->vTangent3[2].fZ);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
                    sPrint << string_format("\r\n\r\nTangent Space 4");
                    sPrint << string_format("\r\nTangent:   %f\r\n           %f\r\n           %f", mdx->vTangent4[0].fX, mdx->vTangent4[0].fY, mdx->vTangent4[0].fZ);
                    sPrint << string_format("\r\nBitangent: %f\r\n           %f\r\n           %f", mdx->vTangent4[1].fX, mdx->vTangent4[1].fY, mdx->vTangent4[1].fZ);
                    sPrint << string_format("\r\nNormal:    %f\r\n           %f\r\n           %f", mdx->vTangent4[2].fX, mdx->vTangent4[2].fY, mdx->vTangent4[2].fZ);
                }
                if(node.Head.nType & NODE_HAS_SKIN){
                    sPrint << string_format("\r\n\r\nWeight Value: %f\r\n              %f\r\n              %f\r\n              %f", mdx->Weights.fWeightValue[0], mdx->Weights.fWeightValue[1], mdx->Weights.fWeightValue[2], mdx->Weights.fWeightValue[3]);
                    sPrint << string_format("\r\n\r\nWeight Index: %f\r\n              %f\r\n              %f\r\n              %f", mdx->Weights.fWeightIndex[0], mdx->Weights.fWeightIndex[1], mdx->Weights.fWeightIndex[2], mdx->Weights.fWeightIndex[3]);
                }
            }
        }
    }
    else if((cItem[0] == "Faces") && !bWok){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Faces\r\nOffset: %u\r\nCount: %u", mesh->FaceArray.nOffset, mesh->FaceArray.nCount);
        sPrint << "\r\n\r\nIndexes Count: "<<mesh->nVertIndicesCount<<"\r\n(Offset: "<<mesh->IndexCounterArray.nOffset<<")";
        sPrint << "\r\nIndexes Offset: "<<mesh->nVertIndicesLocation<<"\r\n(Offset: "<<mesh->IndexLocationArray.nOffset<<")";
    }
    else if(cItem[1] == "Faces"){
        Face * face = (Face * ) lParam;
        sPrint << string_format("%s\r\nNormal: %f\r\n        %f\r\n        %f\r\nDistance: %f\r\nMaterial ID: %i\r\nAdjacent Faces: %i, %i, %i\r\nVertex Indices: %i, %i, %i",
                cItem[0].c_str(), face->vNormal.fX, face->vNormal.fY, face->vNormal.fZ, face->fDistance, face->nMaterialID,
                face->nAdjacentFaces[0], face->nAdjacentFaces[1], face->nAdjacentFaces[2], face->nIndexVertex[0], face->nIndexVertex[1], face->nIndexVertex[2]);
        sPrint << "\r\n\r\nArea: "<<face->fArea;
        sPrint << "\r\nSmoothing groups: ";
        for(int n = 0; n < 32; n++){
            if(pown(2, n) & face->nSmoothingGroup) sPrint << n+1 <<" ";
        }
    }
    else if(cItem[0] == "Vertex Indices 2"){
        VertIndicesStruct * vert = (VertIndicesStruct * ) lParam;
        sPrint << string_format("Vertex Indices 2\r\nValues: %u, %u, %u", vert->nValues[0], vert->nValues[1], vert->nValues[2]);
    }
    else if(cItem[0] == "Average") sPrint << string_format("Average: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if(cItem[0] == "Ambient Color") sPrint << string_format("Ambient Color: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if(cItem[0] == "Diffuse Color") sPrint << string_format("Diffuse Color: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if(cItem[0] == "Transparency Hint") sPrint << "Transparency Hint:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Textures"){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Textures\r\nTexture Count: %u\r\nTexture 1: %s\r\nTexture 2: %s\r\nTexture 3: %s\r\nTexture 4: %s",
                mesh->nTextureNumber, mesh->GetTexture(1), mesh->GetTexture(2), mesh->GetTexture(3), mesh->GetTexture(4));
    }
    else if(cItem[0] == "Trimesh Flags"){
        Node * node = (Node*) lParam;
        sPrint << "Trimesh Flags";
        sPrint << "\r\nHas Lightmap: " << (int) node->Mesh.nHasLightmap;
        sPrint << "\r\nRotate Texture: " << (int) node->Mesh.nRotateTexture;
        sPrint << "\r\nBackground Geometry: " << (int) node->Mesh.nBackgroundGeometry;
        sPrint << "\r\nShadow: " << (int) node->Mesh.nShadow;
        sPrint << "\r\nBeaming: " << (int) node->Mesh.nBeaming;
        sPrint << "\r\nRender: " << (int) node->Mesh.nRender;
        if(Model.bK2) sPrint << "\r\nDirt Enabled: " << (int) node->Mesh.nDirtEnabled;
        if(Model.bK2) sPrint << "\r\nHide in Holograms: " << (int) node->Mesh.nHideInHolograms;
    }
    else if(cItem[0] == "Animated UV")
        sPrint << string_format("Animated UV\r\nAnimate UV:      %i\r\nUV Direction X:  %f\r\nUV Direction Y:  %f\r\nUV Jitter:       %f\r\nUV Jitter Speed: %f",
                ((MeshHeader*) lParam)->nAnimateUV, ((MeshHeader*) lParam)->fUVDirectionX, ((MeshHeader*) lParam)->fUVDirectionY, ((MeshHeader*) lParam)->fUVJitter, ((MeshHeader*) lParam)->fUVJitterSpeed);
    else if(cItem[0] == "Unknown Lightsaber Bytes")
        sPrint << string_format("Unknown Lightsaber Bytes\r\nUnknown 1: %i\r\nUnknown 2: %i\r\nUnknown 3: %i\r\nUnknown 4: %i\r\nUnknown 5: %i\r\nUnknown 6: %i\r\nUnknown 7: %i\r\nUnknown 8: %i",
                ((MeshHeader*) lParam)->nSaberUnknown1, ((MeshHeader*) lParam)->nSaberUnknown2, ((MeshHeader*) lParam)->nSaberUnknown3, ((MeshHeader*) lParam)->nSaberUnknown4,
                ((MeshHeader*) lParam)->nSaberUnknown5, ((MeshHeader*) lParam)->nSaberUnknown6, ((MeshHeader*) lParam)->nSaberUnknown7, ((MeshHeader*) lParam)->nSaberUnknown8);
    else if(cItem[0] == "Unknown Array of 3 Integers") sPrint << string_format("Unknown Array of 3 Integers: \r\n%i\r\n%i\r\n%i", ((int*) lParam)[0], ((int*) lParam)[1], ((int*) lParam)[2]);
    else if(cItem[0] == "Dirt"){
        MeshHeader * mesh = (MeshHeader *) lParam;
        sPrint << "Dirt Texture: " << mesh->nDirtTexture;
        sPrint << "\r\nDirt Coord Space: " << mesh->nDirtCoordSpace;
    }
    else if(cItem[0] == "Total Area") sPrint << "Total Area:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Padding") sPrint << "Padding:\r\n" << *((int*) lParam);

    /// Skin ///
    else if(cItem[0] == "Bones"){
        SkinHeader * skin = (SkinHeader * ) lParam;
        sPrint << string_format("Bonemap\r\nOffset: %u\r\nCount: %u", skin->nOffsetToBonemap, skin->nNumberOfBonemap);
        sPrint << string_format("\r\n\r\nQ Bones\r\nOffset: %u\r\nCount: %u", skin->QBoneArray.nOffset, skin->QBoneArray.nCount);
        sPrint << string_format("\r\n\r\nT Bones\r\nOffset: %u\r\nCount: %u", skin->TBoneArray.nOffset, skin->TBoneArray.nCount);
        sPrint << string_format("\r\n\r\nArray8\r\nOffset: %u\r\nCount: %u", skin->Array8Array.nOffset, skin->Array8Array.nCount);
    }
    else if(cItem[1] == "Bones"){
        Bone * bone = (Bone * ) lParam;
        sPrint << cItem[0].c_str();
        sPrint << "\r\n\r\nBonemap: "<<PrepareFloat(bone->fBonemap, 0);
        sPrint << "\r\n\r\nTBone: "<<PrepareFloat(bone->TBone.fX, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->TBone.fY, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->TBone.fZ, 0);
        sPrint << "\r\n\r\nQBone: "<<PrepareFloat(bone->QBone.GetQuaternion().vAxis.fX, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.GetQuaternion().vAxis.fY, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.GetQuaternion().vAxis.fZ, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.GetQuaternion().fW, 0);
        //sPrint << "\r\n\r\nArray8: "<<PrepareFloat(bone->fArray8, 0);
    }
    else if((cItem[0] == "MDX Data Pointers") && (cItem[1] == "Skin")){
        SkinHeader * skin = (SkinHeader *) lParam;
        sPrint << string_format("MDX Data Pointers\r\nTo Weight Value: %i\r\nTo Weight Index: %i",
            skin->nOffsetToWeightValuesInMDX, skin->nOffsetToBoneIndexInMDX);
    }
    else if(cItem[0] == "Bone Indexes"){
        short * sarray = (short * ) lParam;
        sPrint<<"Bone Indexes";
        for(int n = 0; n < 18; n++) sPrint<<"\r\nIndex "<<n+1<<": "<<sarray[n];
    }

    /// Danglymesh ///
    else if(cItem[0] == "Constraints"){
        DanglymeshHeader * dangly = (DanglymeshHeader * ) lParam;
        sPrint << string_format("Constraints\r\nOffset: %u\r\nCount: %u", dangly->ConstraintArray.nOffset, dangly->ConstraintArray.nCount);
        if(dangly->Constraints.size() > 0){
            char cSpaces [10];
            sPrint << string_format("\r\n\r\nData:");
            int n = 0;
            while(n < dangly->ConstraintArray.nCount){
                if(dangly->Constraints[n] >= 0.0) sprintf(cSpaces, " ");
                else sprintf(cSpaces, "");
                if(n+1 < 10) strcat(cSpaces, " ");
                sPrint << string_format("\r\n%i.%s %f", n+1, cSpaces, dangly->Constraints[n]);
                n++;
            }
        }
    }
    else if(cItem[0] == "Displacement") sPrint << "Displacement:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Tightness") sPrint << "Tightness:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Period") sPrint << "Period:\r\n" << *((double*) lParam);
    else if(cItem[0] == "Data2"){
        DanglymeshHeader * dangly = (DanglymeshHeader * ) lParam;
        sPrint << string_format("Data2\r\nOffset: %u\r\nCount: %u", dangly->nOffsetToData2, dangly->ConstraintArray.nCount);
    }
    else if(cItem[1] == "Data2"){
        Vector * data = (Vector * ) lParam;
        sPrint << cItem[0].c_str();
        sPrint << "\r\nx: "<<data->fX;
        sPrint << "\r\ny: "<<data->fY;
        sPrint << "\r\nz: "<<data->fZ;
    }

    /// Walkmesh ///
    else if(cItem[0] == "Walkmesh"){
        WalkmeshHeader * walk = (WalkmeshHeader * ) lParam;
        sPrint << string_format("Walkmesh\r\nOffset to aabb tree: %u\r\nCount: ?", walk->nOffsetToAabb);
    }
    else if(cItem[1] == "Walkmesh"){
        Aabb * aabb = (Aabb * ) lParam;
        std::string sProperty;
        if(aabb->nProperty == 1) sProperty = "Positive X";
        else if(aabb->nProperty == 2) sProperty = "Positive Y";
        else if(aabb->nProperty == 4) sProperty = "Positive Z";
        else if(aabb->nProperty == 8) sProperty = "Negative X";
        else if(aabb->nProperty == 16) sProperty = "Negative Y";
        else if(aabb->nProperty == 32) sProperty = "Negative Z";
        else sProperty = "None";
        sPrint << cItem[0];
        sPrint << "\r\nOffset: "<<aabb->nOffset;
        sPrint << "\r\nBounding Box Min: "<<aabb->vBBmin.fX;
        sPrint << "\r\n                  "<<aabb->vBBmin.fY;
        sPrint << "\r\n                  "<<aabb->vBBmin.fZ;
        sPrint << "\r\nBounding Box Max: "<<aabb->vBBmax.fX;
        sPrint << "\r\n                  "<<aabb->vBBmax.fY;
        sPrint << "\r\n                  "<<aabb->vBBmax.fZ;
        sPrint << "\r\nFace Index: "<<aabb->nID;
        sPrint << "\r\n2nd Child Property: "<<sProperty;
        sPrint << "\r\nOffset to Child 1: "<<aabb->nChild1;
        sPrint << "\r\nOffset to Child 2: "<<aabb->nChild2;
    }

    /// Saber ///
    else if(cItem[0] == "Saber Data"){
        SaberHeader * saber = (SaberHeader * ) lParam;
        sPrint << string_format("Saber Data\r\nOffset 1: %u\r\nOffset 2: %u\r\nOffset 3: %u\r\nCount: %u", saber->nOffsetToSaberData1, saber->nOffsetToSaberData2, saber->nOffsetToSaberData3, saber->nNumberOfSaberData);
    }
    else if(cItem[1] == "Saber Data"){
        SaberDataStruct * saber = (SaberDataStruct *) lParam;
        sPrint << cItem[0].c_str();
        sPrint << string_format("\r\n1. Vertex Coordinates:\r\n   %f\r\n   %f\r\n   %f", saber->vVertex.fX, saber->vVertex.fY, saber->vVertex.fZ);
        sPrint << string_format("\r\n\r\n2. UV:\r\n   %f\r\n   %f", saber->vUV.fX, saber->vUV.fY);
        sPrint << string_format("\r\n\r\n3. Normal?:\r\n   %f\r\n   %f\r\n   %f", saber->vNormal.fX, saber->vNormal.fY, saber->vNormal.fZ);
    }
    else if(cItem[0] == "Mesh Inverted Counter 1") sPrint << "Mesh Inverted Counter 1:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Mesh Inverted Counter 2") sPrint << "Mesh Inverted Counter 2:\r\n" << *((int*) lParam);

    /// WOK ///
    else if(cItem[1] == "Aabb"){
        Aabb * aabb = (Aabb * ) lParam;
        std::string sProperty;
        if(aabb->nProperty == 1) sProperty = "Positive X";
        else if(aabb->nProperty == 2) sProperty = "Positive Y";
        else if(aabb->nProperty == 4) sProperty = "Positive Z";
        else if(aabb->nProperty == 8) sProperty = "Negative X";
        else if(aabb->nProperty == 16) sProperty = "Negative Y";
        else if(aabb->nProperty == 32) sProperty = "Negative Z";
        else sProperty = "None";
        sPrint << cItem[0];
        sPrint << "\r\nOffset: "<<aabb->nOffset;
        sPrint << "\r\nBounding Box Min: "<<aabb->vBBmin.fX;
        sPrint << "\r\n                  "<<aabb->vBBmin.fY;
        sPrint << "\r\n                  "<<aabb->vBBmin.fZ;
        sPrint << "\r\nBounding Box Max: "<<aabb->vBBmax.fX;
        sPrint << "\r\n                  "<<aabb->vBBmax.fY;
        sPrint << "\r\n                  "<<aabb->vBBmax.fZ;
        sPrint << "\r\nFace Index: "<<aabb->nID;
        sPrint << "\r\n2nd Child Property: "<<sProperty;
        sPrint << "\r\nChild 1 Index: "<<aabb->nChild1;
        sPrint << "\r\nChild 2 Index: "<<aabb->nChild2;
        sPrint << "\r\nExtra: "<<aabb->nExtra;
    }
    else if(cItem[1] == "Array 1"){
        sPrint << string_format("Values:\r\n%i\r\n%i\r\n%i", ((Triples*) lParam)->n1, ((Triples*) lParam)->n2, ((Triples*) lParam)->n3);
    }
    else if(cItem[1] == "Array 2"){
        sPrint << string_format("Values:\r\n%i\r\n%i", ((Triples*) lParam)->n1, ((Triples*) lParam)->n2);
    }
    else if(cItem[1] == "Array 3") sPrint << "Value:\r\n" << *((int*) lParam);

    /// Unknowns ///
    else if(cItem[0] == "Unknown Int32 1") sPrint << "Unknown Int32 1:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Unknown Int32 2") sPrint << "Unknown Int32 2:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Unknown Int32 3") sPrint << "Unknown Int32 3:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Unknown Int32 4") sPrint << "Unknown Int32 4:\r\n" << *((int*) lParam);
    else if(cItem[0] == "Unknown Int32 5") sPrint << "Unknown Int32 5:\r\n" << *((int*) lParam);
    else sPrint.flush();
}
