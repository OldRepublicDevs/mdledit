#include "MDL.h"

void MDL::AddMenuLines(std::vector<std::string>cItem, LPARAM lParam, MenuLineAdder * pmla){
    if((cItem[0] == "")) return;

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
    else if((cItem[1] == "Animations")){
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
    else return;
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
    std::cout<<string_format("ReturnClassification() ERROR: Unknown classification %i.\n", nClassification);
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
    else if(sController == "randombirthrate") return CONTROLLER_EMITTER_RANDOMBIRTHRATE;    //Missing from NWmax
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
        case CONTROLLER_LIGHT_MULTIPLIER:           return "multiplier";             //NWmax reads 'multipler', not 'multiplier'
        }
    }
    else if(nType & NODE_HAS_EMITTER){
        switch(nController){
        case CONTROLLER_EMITTER_ALPHAEND:           return "alphaend";
        case CONTROLLER_EMITTER_ALPHASTART:         return "alphastart";
        case CONTROLLER_EMITTER_BRITHRATE:          return "birthrate";
        case CONTROLLER_EMITTER_BOUNCE_CO:          return "bounce_co";
        case CONTROLLER_EMITTER_COMBINETIME:        return "combinetime";
        case CONTROLLER_EMITTER_DRAG:               return "drag";
        case CONTROLLER_EMITTER_FPS:                return "fps";
        case CONTROLLER_EMITTER_FRAMEEND:           return "frameend";
        case CONTROLLER_EMITTER_FRAMESTART:         return "framestart";
        case CONTROLLER_EMITTER_GRAV:               return "grav";
        case CONTROLLER_EMITTER_LIFEEXP:            return "lifeexp";
        case CONTROLLER_EMITTER_MASS:               return "mass";
        case CONTROLLER_EMITTER_P2P_BEZIER2:        return "p2p_bezier2";
        case CONTROLLER_EMITTER_P2P_BEZIER3:        return "p2p_bezier3";
        case CONTROLLER_EMITTER_PARTICLEROT:        return "particlerot";
        case CONTROLLER_EMITTER_RANDVEL:            return "randvel";
        case CONTROLLER_EMITTER_SIZESTART:          return "sizestart";
        case CONTROLLER_EMITTER_SIZEEND:            return "sizeend";
        case CONTROLLER_EMITTER_SIZESTART_Y:        return "sizestart_y";
        case CONTROLLER_EMITTER_SIZEEND_Y:          return "sizeend_y";
        case CONTROLLER_EMITTER_SPREAD:             return "spread";
        case CONTROLLER_EMITTER_THRESHOLD:          return "threshold";
        case CONTROLLER_EMITTER_VELOCITY:           return "velocity";
        case CONTROLLER_EMITTER_XSIZE:              return "xsize";
        case CONTROLLER_EMITTER_YSIZE:              return "ysize";
        case CONTROLLER_EMITTER_BLURLENGTH:         return "blurlength";
        case CONTROLLER_EMITTER_LIGHTNINGDELAY:     return "lightningdelay";
        case CONTROLLER_EMITTER_LIGHTNINGRADIUS:    return "lightningradius";
        case CONTROLLER_EMITTER_LIGHTNINGSCALE:     return "lightningscale";
        case CONTROLLER_EMITTER_LIGHTNINGSUBDIV:    return "lightningsubdiv";
        case CONTROLLER_EMITTER_LIGHTNINGZIGZAG:    return "lightningzigzag";    //Missing from NWmax
        case CONTROLLER_EMITTER_ALPHAMID:           return "alphamid";           //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTSTART:       return "percentstart";       //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTMID:         return "percentmid";         //Missing from NWmax
        case CONTROLLER_EMITTER_PERCENTEND:         return "percentend";         //Missing from NWmax
        case CONTROLLER_EMITTER_SIZEMID:            return "sizemid";            //Missing from NWmax
        case CONTROLLER_EMITTER_SIZEMID_Y:          return "sizemid_y";          //Missing from NWmax
        case CONTROLLER_EMITTER_RANDOMBIRTHRATE:    return "randombirthrate";    //Missing from NWmax
        case CONTROLLER_EMITTER_TARGETSIZE:         return "targetsize";         //Missing from NWmax
        case CONTROLLER_EMITTER_NUMCONTROLPTS:      return "numcontrolpts";      //Missing from NWmax
        case CONTROLLER_EMITTER_CONTROLPTRADIUS:    return "controlptradius";    //Missing from NWmax
        case CONTROLLER_EMITTER_CONTROLPTDELAY:     return "controlptdelay";     //Missing from NWmax
        case CONTROLLER_EMITTER_TANGENTSPREAD:      return "tangentspread";      //Missing from NWmax
        case CONTROLLER_EMITTER_TANGENTLENGTH:      return "tangentlength";      //Missing from NWmax
        case CONTROLLER_EMITTER_COLORMID:           return "colormid";           //Missing from NWmax
        case CONTROLLER_EMITTER_COLOREND:           return "colorend";
        case CONTROLLER_EMITTER_COLORSTART:         return "colorstart";
        case CONTROLLER_EMITTER_DETONATE:           return "detonate";           //Missing from NWmax
        }
    }
    else{
        switch(nController){
        case CONTROLLER_MESH_SELFILLUMCOLOR:        return "selfillumcolor";
        case CONTROLLER_MESH_ALPHA:                 return "alpha";
        }
    }
    std::cout<<string_format("ReturnController() ERROR: Unknown controller %i (type %i).\n", nController, nType);
    return "unknown";
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

HTREEITEM AppendChildren(Node & node, HTREEITEM Prev, std::vector<Name> & Names){
        if(node.Head.nType & NODE_HAS_LIGHT){
            HTREEITEM Light = Append("Light", (LPARAM) &node.Light, Prev);
            Append("Flare Radius", (LPARAM) &node.Light.fFlareRadius, Light);
            Append("Unknown Array", (LPARAM) &node.Light);
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
            if(node.Mesh.nNumberOfVerts > 0){
                Append("MDX Data Pointers", (LPARAM) &node.Mesh, Vertices);
                char cVert [255];
                HTREEITEM Vert;
                for(int n = 0; n < node.Mesh.nNumberOfVerts; n++){
                    sprintf(cVert, "Vertex %i", n);
                    Vert = Append(cVert, (LPARAM) &(node.Mesh.Vertices[n]), Vertices);
                    //if(node.Mesh.nMdxDataSize > 0) Append("MDX Data", (LPARAM) &(node.Mesh.Vertices[n].MDXData), Vert);
                }
                if(node.Mesh.nMdxDataSize > 0 && !Mdx.empty()) Append("Extra MDX Data", (LPARAM) &(node.Mesh.MDXData), Vertices);
            }
            HTREEITEM Faces = Append("Faces", (LPARAM) &node.Mesh, Mesh);
            if(node.Mesh.Faces.size() > 0){
                //Append("Number of Vertex Indices 2 Array", (LPARAM) &node.Mesh, Faces);
                //Append("Location of Vertex Indices 2 Array", (LPARAM) &node.Mesh, Faces);
                char cFace [255];
                HTREEITEM Face;
                for(int n = 0; n < node.Mesh.FaceArray.nCount; n++){
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
            Append("Shininess", (LPARAM) &node.Mesh.nShininess);
            Append("Textures", (LPARAM) &node.Mesh);
            Append("Trimesh Flags", (LPARAM) &node.Mesh);
            Append("Animated UV", (LPARAM) &node.Mesh);
            Append("Unknown Array of 3 Integers", (LPARAM) node.Mesh.nUnknown3);
            Append("Unknown Lightsaber Bytes", (LPARAM) &node.Mesh);
            Append("Dirt", (LPARAM) &node.Mesh);
            Append("Total Area", (LPARAM) &node.Mesh.fTotalArea);
            //Append("Padding", (LPARAM) &node.Mesh.nPadding);
        }
        if(node.Head.nType & NODE_HAS_SKIN){
            char cBone [255];
            HTREEITEM Skin = Append("Skin", (LPARAM) &node.Skin, Prev);
            HTREEITEM Bones = Append("Bones", (LPARAM) &node.Skin, Skin);
            if(node.Skin.Bones.size() > 0){
                for(int n = 0; n < node.Skin.Bones.size(); n++){
                    std::string sBone = "Bone " + Names[n].cName;
                    Append(sBone, (LPARAM) &(node.Skin.Bones.at(n)), Bones);
                }
            }
            Append("Bone Indexes", (LPARAM) &node.Skin.nBoneIndexes, Skin);
            Append("MDX Data Pointers", (LPARAM) &node.Skin, Skin);
            Append("Unknown Int32 1", (LPARAM) &node.Skin.nUnknown1, Skin);
            Append("Unknown Int32 2", (LPARAM) &node.Skin.nUnknown2);
            Append("Unknown Int32 3", (LPARAM) &node.Skin.nUnknown3);
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

void MDL::BuildTree(){
    if(FH.empty()){
        std::cout<<"No data. Do not build tree.\n";
        return;
    }
    FileHeader & Data = FH[0];

    HTREEITEM Root = Append(sFile, NULL, TVI_ROOT);
    HTREEITEM Header = Append("Header", (LPARAM) &(Data.MH), Root);
    Append("Bounding Box Min", (LPARAM) (&Data.MH.vBBmin), Header);
    Append("Bounding Box Max", (LPARAM) (&Data.MH.vBBmax));
    Append("Radius", (LPARAM) &(Data.MH.fRadius));
    Append("Scale", (LPARAM) &(Data.MH.fScale));

    HTREEITEM Animations = Append("Animations", NULL, Root);
    HTREEITEM Nodes, Animation, Sounds, CurrentNode;
    for(int n = 0; n < Data.MH.AnimationArray.nCount; n++){
        Animation = Append(Data.MH.Animations[n].cName, (LPARAM) &(Data.MH.Animations[n]), Animations);
        Append("Length", (LPARAM) &(Data.MH.Animations[n].fLength), Animation);
        Append("Transition", (LPARAM) &(Data.MH.Animations[n].fTransition));
        Sounds = Append("Sounds", (LPARAM) &(Data.MH.Animations[n]));
        for(int i = 0; i < Data.MH.Animations[n].SoundArray.nCount; i++){
            Append(Data.MH.Animations[n].Sounds[i].cName.c_str(), (LPARAM) &(Data.MH.Animations[n].Sounds[i]), Sounds);
        }
        Nodes = Append("Animated Nodes", NULL, Animation);
        for(int a = 0; a < Data.MH.Animations[n].ArrayOfNodes.size(); a++){
            Node & node = Data.MH.Animations[n].ArrayOfNodes[a];

            CurrentNode = Append(Data.MH.Names[node.Head.nNameIndex].cName, (LPARAM) &node, Nodes);

            HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
            Append("Controller Data", (LPARAM) &node.Head, Controllers);
            for(int n = 0; n < node.Head.Controllers.size(); n++){
                int nCtrlIndex = node.Head.Controllers[n].nNameIndex;
                int nCtrlType = node.Head.Controllers[n].nControllerType;
                Node & ctrlnode = GetNodeByNameIndex(nCtrlIndex);
                std::string sName = ReturnControllerName(nCtrlType, ctrlnode.Head.nType);
                if(node.Head.Controllers[n].nColumnCount == 19) sName+="bezierkey";
                else sName+="key";
                Append(sName, (LPARAM) &(node.Head.Controllers[n]), Controllers);
            }

            HTREEITEM Parent = Append("Parent", NULL, CurrentNode);
            if(node.Head.nParentIndex != -1){
                Append(Data.MH.Names[node.Head.nParentIndex].cName, (LPARAM) &GetNodeByNameIndex(node.Head.nParentIndex, n), Parent);
            }

            HTREEITEM Children = Append("Children", (LPARAM) &node.Head, CurrentNode);
            for(int g = 0; g < Data.MH.Animations[n].ArrayOfNodes.size(); g++){
                Node & curnode = Data.MH.Animations[n].ArrayOfNodes[g];
                if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                    Append(Data.MH.Names[curnode.Head.nNameIndex].cName, (LPARAM) &curnode, Children);
                }
            }
        }
    }

    Nodes = Append("Geometry", NULL, Root);
    for(int a = 0; a < Data.MH.ArrayOfNodes.size(); a++){
        Node & node = Data.MH.ArrayOfNodes[a];

        std::string sType;
        if(node.Head.nType & NODE_HAS_SABER) sType = "(saber) ";
        else if(node.Head.nType & NODE_HAS_AABB) sType = "(walkmesh) ";
        else if(node.Head.nType & NODE_HAS_DANGLY) sType = "(danglymesh) ";
        else if(node.Head.nType & NODE_HAS_SKIN) sType = "(skin) ";
        else if(node.Head.nType & NODE_HAS_MESH) sType = "(mesh) ";
        else if(node.Head.nType & NODE_HAS_EMITTER) sType = "(emitter) ";
        else if(node.Head.nType & NODE_HAS_LIGHT) sType = "(light) ";
        else if(node.Head.nType & NODE_HAS_HEADER) sType = "(basic) ";
        else sType = "(error) ";
        CurrentNode = Append(sType + Data.MH.Names[node.Head.nNameIndex].cName, (LPARAM) &node, Nodes);

        AppendChildren(node, CurrentNode, Data.MH.Names);

        //Append("Position", (LPARAM) &(node.Head.vPos), CurrentNode);

        //Append("Orientation", (LPARAM) &(node.Head.oOrient));

        HTREEITEM Controllers = Append("Controllers", (LPARAM) &node.Head, CurrentNode);
        Append("Controller Data", (LPARAM) &node.Head, Controllers);
        for(int n = 0; n < node.Head.Controllers.size(); n++){
            Append(ReturnControllerName(node.Head.Controllers[n].nControllerType, node.Head.nType), (LPARAM) &(node.Head.Controllers[n]), Controllers);
        }

        HTREEITEM Parent = Append("Parent", NULL, CurrentNode);
        if(node.Head.nParentIndex != -1){
            Append(Data.MH.Names[node.Head.nParentIndex].cName, (LPARAM) &GetNodeByNameIndex(node.Head.nParentIndex, -1), Parent);
        }

        HTREEITEM Children = Append("Children", (LPARAM) &node.Head, CurrentNode);
        for(int g = 0; g < Data.MH.ArrayOfNodes.size(); g++){
            Node & curnode = Data.MH.ArrayOfNodes[g];
            if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                Append(Data.MH.Names[curnode.Head.nNameIndex].cName, (LPARAM) &curnode, Children);
            }
        }
    }

    TreeView_Expand(hTree, Root, TVE_EXPAND);
    std::cout<<"Tree building done!\n";
}


extern char cReturn[4][255];
char * PrepareFloat(double fFloat, unsigned int n);

void MDL::DetermineDisplayText(std::vector<std::string>cItem, std::stringstream & sPrint, LPARAM lParam){
    bool bMdl = false;
    bool bWok = false;
    for(int j = 0; !bMdl && !bWok; j++){
        if((cItem[j] == Model.GetFilename())) bMdl = true;
        else if((cItem[j] == Walkmesh.GetFilename())) bWok = true;
    }

    if((cItem[0] == "")) sPrint.flush();

    /// Header ///
    else if((cItem[0] == "Header")){
            sPrint << string_format("Header\r\nModel Name: %s\r\nModel Type: %i\r\nClassification: %s\r\nSupermodel: %s",
                    FH[0].MH.GH.cName.c_str(), FH[0].MH.GH.nModelType, ReturnClassificationName(FH[0].MH.nClassification).c_str(),
                    FH[0].MH.cSupermodelName.c_str()/*, FH[0].MH.nChildModelCount*/);
            sPrint << "\r\nUnknown Supermodel uint32?: "<<FH[0].MH.nUnknown2;
            sPrint << "\r\n\r\nPadding? (Model Type): "<<(int)FH[0].MH.GH.nPadding[0]<<" "<<(int)FH[0].MH.GH.nPadding[1]<<" "<<(int)FH[0].MH.GH.nPadding[2];
            sPrint << "\r\nPadding? (Classification): "<<(int)FH[0].MH.nUnknown1[0]<<" "<<(int)FH[0].MH.nUnknown1[1]<<" "<<(int)FH[0].MH.nUnknown1[2];
            sPrint << string_format("\r\n\r\nMDL Length: %i\r\nMDX Length: %i\r\nFunction Pointer 0: %i\r\nFunction Pointer 1: %i",
                    FH[0].nMdlLength, FH[0].nMdxLength, FH[0].MH.GH.nFunctionPointer0, FH[0].MH.GH.nFunctionPointer1);
    }
    else if((cItem[0] == "Bounding Box Min")) sPrint << "Bounding Box Min:"<<"\r\nx: "<<((Vector*) lParam)->fX<<"\r\ny: "<<((Vector*) lParam)->fY<<"\r\nz: "<<((Vector*) lParam)->fZ;
    else if((cItem[0] == "Bounding Box Max")) sPrint << "Bounding Box Max:"<<"\r\nx: "<<((Vector*) lParam)->fX<<"\r\ny: "<<((Vector*) lParam)->fY<<"\r\nz: "<<((Vector*) lParam)->fZ;
    else if((cItem[0] == "Radius")) sPrint << string_format("Radius:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Scale")) sPrint << string_format("Scale:\r\n%f", *((double*) lParam));

    /// Animations ///
    else if((cItem[0] == "Animations")){
            sPrint << string_format("Animations\r\nOffset to Animation Array: %i\r\nAnimation Count: %i",
                    FH[0].MH.AnimationArray.nOffset, FH[0].MH.AnimationArray.nCount);
    }
    else if((cItem[1] == "Animations")){
        Animation * anim = (Animation * ) lParam;
        sPrint << string_format("Animation %s\r\nOffset: %u\r\nOffset to Root: %u\r\nOwner: %s\r\nNumber of Objects: %u",
                anim->cName.c_str(), anim->nOffset, anim->nOffsetToRootAnimationNode, anim->cName2.c_str(), anim->nNumberOfObjects);
        sPrint << "\r\n\r\nModel Type: "<<(int)anim->nModelType;
        sPrint << "\r\nPadding?: "<<(int)anim->nPadding[0]<<" "<<(int)anim->nPadding[1]<<" "<<(int)anim->nPadding[2];
        sPrint << string_format("\r\n\r\nFunction Pointer 0: %u\r\nFunction Pointer 1: %u\r\n",
                anim->nFunctionPointer0, anim->nFunctionPointer1);
    }
    else if((cItem[0] == "Length")) sPrint << string_format("Length:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Transition")) sPrint << string_format("Transition:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Sounds")){
        Animation * anim = (Animation * ) lParam;
        sPrint << string_format("Sounds\r\nOffset: %u\r\nCount: %u", anim->SoundArray.nOffset, anim->SoundArray.nCount);
    }
    else if((cItem[1] == "Sounds")){
        Sound * snd = (Sound*) lParam;
        sPrint << string_format("Sound:\r\n%s\r\n\r\nTime:\r\n%f", snd->cName.c_str(), snd->fTime);
    }

    /// Geometry ///
    else if((cItem[0] == "Geometry")){
            sPrint << string_format("Geometry\r\nOffset to Name Array: %i\r\nOffset to Root Node: %i\r\nOffset to Head Root Node: %i\r\nNode Count: %i\r\nTotal Node Count (with Supermodel): %i",
                    FH[0].MH.NameArray.nOffset, FH[0].MH.GH.nOffsetToRootNode, FH[0].MH.nOffsetToHeadRootNode, FH[0].MH.NameArray.nCount, FH[0].MH.GH.nTotalNumberOfNodes);
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
            sPrint << ") "<<FH[0].MH.Names[node->Head.nNameIndex].cName.c_str();
            sPrint << string_format("\r\nOffset: %i\r\nOffset to Root: %i\r\nOffset to Parent: %i\r\nID: %i", node->nOffset, node->Head.nOffsetToRoot, node->Head.nOffsetToParent, node->Head.nID1);
            sPrint << "\r\nPosition: "<<PrepareFloat(node->Head.vPos.fX, 0);
            sPrint << "\r\n          "<<PrepareFloat(node->Head.vPos.fY, 0);
            sPrint << "\r\n          "<<PrepareFloat(node->Head.vPos.fZ, 0);
            sPrint << "\r\nOrientation: "<<PrepareFloat(node->Head.oOrient.qX, 0)<<" (AA "<<PrepareFloat(node->Head.oOrient.fX, 1)<<")";
            sPrint << "\r\n             "<<PrepareFloat(node->Head.oOrient.qY, 0)<<" (AA "<<PrepareFloat(node->Head.oOrient.fY, 1)<<")";
            sPrint << "\r\n             "<<PrepareFloat(node->Head.oOrient.qZ, 0)<<" (AA "<<PrepareFloat(node->Head.oOrient.fZ, 1)<<")";
            sPrint << "\r\n             "<<PrepareFloat(node->Head.oOrient.qW, 0)<<" (AA "<<PrepareFloat(node->Head.oOrient.fAngle, 1)<<")";
    }/*
    else if((cItem[0] == "Position")) sPrint << string_format("Position: \r\nx: %f\r\ny: %f\r\nz: %f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if((cItem[0] == "Orientation")){
        Orientation * orient = (Orientation*) lParam;
        sPrint << "Orientation: \r\nx: "<<orient->qX<<" (aa "<<orient->fX<<")\r\ny: "<<orient->qY<<" (aa "<<orient->fY<<")\r\nz: "<<orient->qZ<<" (aa "<<orient->fZ<<")\r\nw: "<<orient->qW<<" (aa "<<orient->fAngle<<")";
    }*/
    else if((cItem[0] == "Controllers")){
        Header * head = (Header * ) lParam;
        sPrint << string_format("Controllers\r\nOffset: %u\r\nCount: %u", head->ControllerArray.nOffset, head->ControllerArray.nCount);
    }
    else if((cItem[0] == "Controller Data")){
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
    else if((cItem[1] == "Controllers")){
        Controller * ctrl = (Controller*) lParam;
        sPrint << string_format("%s\r\n\r\nController type:  %i\r\nUnknown int16:   %hi\r\nValue Count:      %hi\r\nTimekey Start:    %hi\r\nData Start:       %hi\r\nColumn Count:     %hhi\r\nPadding?:         %hhi, %hhi, %hhi",
                cItem[0].c_str(), ctrl->nControllerType, ctrl->nUnknown2, ctrl->nValueCount, ctrl->nTimekeyStart, ctrl->nDataStart, ctrl->nColumnCount, ctrl->nPadding[0], ctrl->nPadding[1], ctrl->nPadding[2]);
    }
    else if((cItem[0] == "Children")){
        Header * head = (Header * ) lParam;
        sPrint << string_format("Children\r\nOffset: %u\r\nCount: %u", head->ChildrenArray.nOffset, head->ChildrenArray.nCount);
    }

    /// Light ///
    else if((cItem[0] == "Flare Radius")) sPrint << string_format("Flare Radius:\r\n%f", *((float*) lParam));
    else if((cItem[0] == "Unknown Array")){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Unknown Array\r\nOffset: %u\r\nCount: %u", light->UnknownArray.nOffset, light->UnknownArray.nCount);
    }
    else if((cItem[0] == "Flare Sizes")){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Sizes\r\nOffset: %u\r\nCount: %u", light->FlareSizeArray.nOffset, light->FlareSizeArray.nCount);
        if(light->FlareSizes.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareSizeArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlareSizes[n], 0);
            }
        }
    }
    else if((cItem[0] == "Flare Positions")){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Positions\r\nOffset: %u\r\nCount: %u", light->FlarePositionArray.nOffset, light->FlareSizeArray.nCount);
        if(light->FlarePositions.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlarePositionArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlarePositions[n], 0);
            }
        }
    }
    else if((cItem[0] == "Flare Color Shifts")){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Color Shifts\r\nOffset: %u\r\nCount: %u", light->FlareColorShiftArray.nOffset, light->FlareColorShiftArray.nCount);
        if(light->FlareColorShifts.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareColorShiftArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << PrepareFloat(light->FlareColorShifts[n].fR, 0) << ", " << PrepareFloat(light->FlareColorShifts[n].fG, 1) << ", " << PrepareFloat(light->FlareColorShifts[n].fB, 2);
            }
        }
    }
    else if((cItem[0] == "Flare Textures")){
        LightHeader * light = (LightHeader * ) lParam;
        sPrint << string_format("Flare Textures\r\nOffset: %u\r\nCount: %u", light->FlareTextureNameArray.nOffset, light->FlareTextureNameArray.nCount);
        if(light->FlareTextureNames.size() > 0){
            sPrint << "\r\nData:";
            for(int n = 0; n < light->FlareTextureNameArray.nCount; n++){
                sPrint << "\r\n"<<n+1<<". " << light->FlareTextureNames[n].cName;
            }
        }
    }
    else if((cItem[0] == "Light Priority")) sPrint << string_format("Light Priority:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Ambient Only")) sPrint << string_format("Ambient Only:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Dynamic Type")) sPrint << string_format("Dynamic Type:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Affect Dynamic")) sPrint << string_format("Affect Dynamic:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Shadow")) sPrint << string_format("Shadow:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Flare")) sPrint << string_format("Flare:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Fading Light")) sPrint << string_format("Fading Light:\r\n%u", *((int*) lParam));

    /// Emitter ///
    else if((cItem[0] == "Dead Space")) sPrint << string_format("Dead Space:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Blast Radius")) sPrint << string_format("Blast Radius:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Blast Length")) sPrint << string_format("Blast Length:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Branch Count")) sPrint << string_format("Branch Count:\r\n%u", *((unsigned int*) lParam));
    else if((cItem[0] == "Control Point Smoothing")) sPrint << string_format("Control Point Smoothing:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "X Grid")) sPrint << string_format("X Grid:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Y Grid")) sPrint << string_format("Y Grid:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Spawn Type")) sPrint << string_format("Spawn Type:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Update")) sPrint << string_format("Update:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Render")) sPrint << string_format("Render:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Blend")) sPrint << string_format("Blend:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Texture")) sPrint << string_format("Texture:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Chunk Name")) sPrint << string_format("Chunk Name:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Twosided Texture")) sPrint << string_format("Twosided Texture:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Loop")) sPrint << string_format("Loop:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Unknown Int16 1")) sPrint << string_format("Unknown Int16 1:\r\n%u", *((short*) lParam));
    //else if((cItem[0] == "Render Order")) sPrint << string_format("Render Order:\r\n%u", *((short*) lParam));
    else if((cItem[0] == "Frame Blending")) sPrint << string_format("Frame Blending:\r\n%u", *((unsigned char*) lParam));
    else if((cItem[0] == "Depth Texture Name")) sPrint << string_format("Depth Texture Name:\r\n%s", ((char*) lParam));
    else if((cItem[0] == "Unknown Byte 1")) sPrint << string_format("Unknown Byte 1:\r\n%u", *((unsigned char*) lParam));
    else if((cItem[0] == "Emitter Flags?"))
        sPrint << string_format("Emitter Flags??\r\np2p:           %i\r\np2p_sel:       %i\r\naffected_wind: %i\r\ntinted:        %i\r\nbounce:        %i\r\nrandom:        %i\r\ninherit:       %i\r\ninherit_vel:   %i\r\ninherit_local: %i\r\nsplat:         %i\r\ninherit_part:  %i",
                *((int*) lParam) & EMITTER_FLAG_P2P ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_P2P_SEL ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_TINTED ? 1 : 0,
                *((int*) lParam) & EMITTER_FLAG_BOUNCE ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_RANDOM ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_INHERIT ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_INHERIT_VEL ? 1 : 0,
                *((int*) lParam) & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_SPLAT ? 1 : 0, *((int*) lParam) & EMITTER_FLAG_INHERIT_PART ? 1 : 0);

    /// Mesh ///
    else if((cItem[0] == "Mesh")){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Mesh\r\nFunction Pointer 0: %u\r\nFunction Pointer 1: %u",
                mesh->nFunctionPointer0, mesh->nFunctionPointer1);
        sPrint << "\r\n\r\nInverted Counter: "<<mesh->nMeshInvertedCounter<<"\r\n(Offset: "<<mesh->MeshInvertedCounterArray.nOffset<<", Count: "<<mesh->MeshInvertedCounterArray.GetCount()<<")";
    }
    else if((cItem[0] == "Mesh Inverted Counters Array")){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Mesh Inverted Counters Array\r\nOffset: %u\r\nCount: %u",
                mesh->MeshInvertedCounterArray.nOffset, mesh->MeshInvertedCounterArray.nCount, mesh->nMeshInvertedCounter);
        if(mesh->MeshInvertedCounterArray.nCount > 0){
            sPrint << string_format("\r\nValues: %i", mesh->nMeshInvertedCounter);
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
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1 ? 1 : 0, mesh->nOffsetToUnknownStructInMDX,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2 ? 1 : 0, mesh->nOffsetToUnusedMDXStructure1,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3 ? 1 : 0, mesh->nOffsetToUnusedMDXStructure2,
                mesh->nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4 ? 1 : 0, mesh->nOffsetToUnusedMDXStructure3);
    }
    else if((cItem[0] == "Vertices") && !bWok){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Vertices\r\nOffset: %u\r\nCount: %u", mesh->nOffsetToVertArray, mesh->nNumberOfVerts);
    }
    else if((cItem[0] == "Extra MDX Data")){
        MDXDataStruct * mdx = (MDXDataStruct * ) lParam;
        sPrint << cItem[0].c_str();
        Node & node = GetNodeByNameIndex(mdx->nNameIndex);
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
            sPrint << string_format("\r\n\r\nWeight Value: %f\r\n              %f\r\n              %f\r\n              %f", mdx->fSkin1[0], mdx->fSkin1[1], mdx->fSkin1[2], mdx->fSkin1[3]);
            sPrint << string_format("\r\n\r\nWeight Index: %f\r\n              %f\r\n              %f\r\n              %f", mdx->fSkin2[0], mdx->fSkin2[1], mdx->fSkin2[2], mdx->fSkin2[3]);
        }
    }
    else if((cItem[1] == "Vertices")){
        Vertex * vert = (Vertex * ) lParam;
        sPrint << string_format("%s\r\nx: %f\r\ny: %f\r\nz: %f",
                cItem[0].c_str(), vert->fX, vert->fY, vert->fZ);
        MDXDataStruct * mdx = &vert->MDXData;
        Node & node = GetNodeByNameIndex(mdx->nNameIndex);
        if(node.Mesh.nMdxDataSize > 0 && !Mdx.empty()){
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
                sPrint << string_format("\r\n\r\nWeight Value: %f\r\n              %f\r\n              %f\r\n              %f", mdx->fSkin1[0], mdx->fSkin1[1], mdx->fSkin1[2], mdx->fSkin1[3]);
                sPrint << string_format("\r\n\r\nWeight Index: %f\r\n              %f\r\n              %f\r\n              %f", mdx->fSkin2[0], mdx->fSkin2[1], mdx->fSkin2[2], mdx->fSkin2[3]);
            }
        }
    }
    else if((cItem[0] == "Faces") && !bWok){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Faces\r\nOffset: %u\r\nCount: %u", mesh->FaceArray.nOffset, mesh->FaceArray.nCount);
        sPrint << "\r\n\r\nIndexes Count: "<<mesh->nVertIndicesCount<<"\r\n(Offset: "<<mesh->IndexCounterArray.nOffset<<", Count: "<<mesh->IndexCounterArray.GetCount()<<")";
        sPrint << "\r\nIndexes Offset: "<<mesh->nVertIndicesLocation<<"\r\n(Offset: "<<mesh->IndexLocationArray.nOffset<<", Count: "<<mesh->IndexLocationArray.GetCount()<<")";
    }
    else if((cItem[1] == "Faces")){
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
    else if((cItem[0] == "Vertex Indices 2")){
        VertIndicesStruct * vert = (VertIndicesStruct * ) lParam;
        sPrint << string_format("Vertex Indices 2\r\nValues: %u, %u, %u", vert->nValues[0], vert->nValues[1], vert->nValues[2]);
    }
    else if((cItem[0] == "Average")) sPrint << string_format("Average: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if((cItem[0] == "Ambient Color")) sPrint << string_format("Ambient Color: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if((cItem[0] == "Diffuse Color")) sPrint << string_format("Diffuse Color: \r\n%f\r\n%f\r\n%f", ((double*) lParam)[0], ((double*) lParam)[1], ((double*) lParam)[2]);
    else if((cItem[0] == "Shininess")) sPrint << string_format("Shininess:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Textures")){
        MeshHeader * mesh = (MeshHeader * ) lParam;
        sPrint << string_format("Textures\r\nTexture Count: %u\r\nTexture 1: %s\r\nTexture 2: %s\r\nTexture 3: %s\r\nTexture 4: %s",
                mesh->nTextureNumber, mesh->GetTexture(1), mesh->GetTexture(2), mesh->GetTexture(3), mesh->GetTexture(4));
    }
    else if((cItem[0] == "Trimesh Flags"))
        sPrint << string_format("Trimesh Flags\r\nHasLightmap: %i\r\nRotateTexture: %i\r\nBackgroundGeometry: %i\r\nShadow: %i\r\nBeaming: %i\r\nRender: %i\r\nDirt Enabled: %i\r\nHide in Holograms: %i",
                ((MeshHeader*) lParam)->nHasLightmap, ((MeshHeader*) lParam)->nRotateTexture, ((MeshHeader*) lParam)->nBackgroundGeometry, ((MeshHeader*) lParam)->nShadow,
                ((MeshHeader*) lParam)->nBeaming, ((MeshHeader*) lParam)->nRender, ((MeshHeader*) lParam)->nDirtEnabled, ((MeshHeader*) lParam)->nHideInHolograms);
    else if((cItem[0] == "Animated UV"))
        sPrint << string_format("Animated UV\r\nAnimate UV:      %i\r\nUV Direction X:  %f\r\nUV Direction Y:  %f\r\nUV Jitter:       %f\r\nUV Jitter Speed: %f",
                ((MeshHeader*) lParam)->nAnimateUV, ((MeshHeader*) lParam)->fUVDirectionX, ((MeshHeader*) lParam)->fUVDirectionY, ((MeshHeader*) lParam)->fUVJitter, ((MeshHeader*) lParam)->fUVJitterSpeed);
    else if((cItem[0] == "Unknown Lightsaber Bytes"))
        sPrint << string_format("Unknown Lightsaber Bytes\r\nUnknown 1: %i\r\nUnknown 2: %i\r\nUnknown 3: %i\r\nUnknown 4: %i\r\nUnknown 5: %i\r\nUnknown 6: %i\r\nUnknown 7: %i\r\nUnknown 8: %i",
                ((MeshHeader*) lParam)->nSaberUnknown1, ((MeshHeader*) lParam)->nSaberUnknown2, ((MeshHeader*) lParam)->nSaberUnknown3, ((MeshHeader*) lParam)->nSaberUnknown4,
                ((MeshHeader*) lParam)->nSaberUnknown5, ((MeshHeader*) lParam)->nSaberUnknown6, ((MeshHeader*) lParam)->nSaberUnknown7, ((MeshHeader*) lParam)->nSaberUnknown8);
    else if((cItem[0] == "Unknown Array of 3 Integers")) sPrint << string_format("Unknown Array of 3 Integers: \r\n%i\r\n%i\r\n%i", ((int*) lParam)[0], ((int*) lParam)[1], ((int*) lParam)[2]);
    else if((cItem[0] == "Dirt")){
        MeshHeader * mesh = (MeshHeader *) lParam;
        sPrint << "Dirt Texture: " << mesh->nDirtTexture;
        sPrint << "\r\nDirt Coord Space: " << mesh->nDirtCoordSpace;
    }
    else if((cItem[0] == "Total Area")) sPrint << string_format("Total Area:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Padding")) sPrint << string_format("Padding:\r\n%i", *((int*) lParam));

    /// Skin ///
    else if((cItem[0] == "Bones")){
        SkinHeader * skin = (SkinHeader * ) lParam;
        sPrint << string_format("Bonemap\r\nOffset: %u\r\nCount: %u", skin->nOffsetToBonemap, skin->nNumberOfBonemap);
        sPrint << string_format("\r\n\r\nQ Bones\r\nOffset: %u\r\nCount: %u", skin->QBoneArray.nOffset, skin->QBoneArray.nCount);
        sPrint << string_format("\r\n\r\nT Bones\r\nOffset: %u\r\nCount: %u", skin->TBoneArray.nOffset, skin->TBoneArray.nCount);
        sPrint << string_format("\r\n\r\nArray8\r\nOffset: %u\r\nCount: %u", skin->Array8Array.nOffset, skin->Array8Array.nCount);
    }
    else if((cItem[1] == "Bones")){
        Bone * bone = (Bone * ) lParam;
        sPrint << cItem[0].c_str();
        sPrint << "\r\n\r\nBonemap: "<<PrepareFloat(bone->fBonemap, 0);
        sPrint << "\r\n\r\nTBone: "<<PrepareFloat(bone->TBone.fX, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->TBone.fY, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->TBone.fZ, 0);
        sPrint << "\r\n\r\nQBone: "<<PrepareFloat(bone->QBone.qX, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.qY, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.qZ, 0);
        sPrint << "\r\n       "<<PrepareFloat(bone->QBone.qW, 0);
        sPrint << "\r\n\r\nArray8: "<<PrepareFloat(bone->fArray8, 0);
    }
    else if((cItem[0] == "MDX Data Pointers") && (cItem[1] == "Skin")){
        SkinHeader * skin = (SkinHeader *) lParam;
        sPrint << string_format("MDX Data Pointers\r\nTo Weight Value: %i\r\nTo Weight Index: %i",
            skin->nPointerToStruct1InMDX, skin->nPointerToStruct2InMDX);
    }
    else if((cItem[0] == "Bone Indexes")){
        short * sarray = (short * ) lParam;
        sPrint<<"Bone Indexes";
        for(int n = 0; n < 18; n++) sPrint<<"\r\nIndex "<<n+1<<": "<<sarray[n];
    }

    /// Danglymesh ///
    else if((cItem[0] == "Constraints")){
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
    else if((cItem[0] == "Displacement")) sPrint << string_format("Displacement:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Tightness")) sPrint << string_format("Tightness:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Period")) sPrint << string_format("Period:\r\n%f", *((double*) lParam));
    else if((cItem[0] == "Data2")){
        DanglymeshHeader * dangly = (DanglymeshHeader * ) lParam;
        sPrint << string_format("Data2\r\nOffset: %u\r\nCount: %u", dangly->nOffsetToData2, dangly->ConstraintArray.nCount);
    }
    else if((cItem[1] == "Data2")){
        Vector * data = (Vector * ) lParam;
        sPrint << cItem[0].c_str();
        sPrint << "\r\nx: "<<data->fX;
        sPrint << "\r\ny: "<<data->fY;
        sPrint << "\r\nz: "<<data->fZ;
    }

    /// Walkmesh ///
    else if((cItem[0] == "Walkmesh")){
        WalkmeshHeader * walk = (WalkmeshHeader * ) lParam;
        sPrint << string_format("Walkmesh\r\nOffset to aabb tree: %u\r\nCount: ?", walk->nOffsetToAabb);
    }
    else if((cItem[1] == "Walkmesh")){
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
    else if((cItem[0] == "Saber Data")){
        SaberHeader * saber = (SaberHeader * ) lParam;
        sPrint << string_format("Saber Data\r\nOffset 1: %u\r\nOffset 2: %u\r\nOffset 3: %u\r\nCount: %u", saber->nOffsetToSaberData1, saber->nOffsetToSaberData2, saber->nOffsetToSaberData3, saber->nNumberOfSaberData);
    }
    else if((cItem[1] == "Saber Data")){
        SaberDataStruct * saber = (SaberDataStruct *) lParam;
        sPrint << string_format("%s", cItem[0].c_str());
        sPrint << string_format("\r\n1. Vertex Coordinates:\r\n   %f\r\n   %f\r\n   %f", saber->vVertex.fX, saber->vVertex.fY, saber->vVertex.fZ);
        sPrint << string_format("\r\n\r\n2. UV:\r\n   %f\r\n   %f", saber->vUV.fX, saber->vUV.fY);
        sPrint << string_format("\r\n\r\n3. Normal?:\r\n   %f\r\n   %f\r\n   %f", saber->vNormal.fX, saber->vNormal.fY, saber->vNormal.fZ);
    }
    else if((cItem[0] == "Mesh Inverted Counter 1")) sPrint << string_format("Mesh Inverted Counter 1:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Mesh Inverted Counter 2")) sPrint << string_format("Mesh Inverted Counter 2:\r\n%u", *((int*) lParam));

    /// WOK ///
    else if((cItem[1] == "Aabb")){
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
    else if((cItem[1] == "Array 1")){
        sPrint << string_format("Values:\r\n%i\r\n%i\r\n%i", ((Triples*) lParam)->n1, ((Triples*) lParam)->n2, ((Triples*) lParam)->n3);
    }
    else if((cItem[1] == "Array 2")){
        sPrint << string_format("Values:\r\n%i\r\n%i", ((Triples*) lParam)->n1, ((Triples*) lParam)->n2);
    }
    else if((cItem[1] == "Array 3")) sPrint << string_format("Value:\r\n%i", *((int*) lParam));

    /// Unknowns ///
    else if((cItem[0] == "Unknown Int32 1")) sPrint << string_format("Unknown Int32 1:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Unknown Int32 2")) sPrint << string_format("Unknown Int32 2:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Unknown Int32 3")) sPrint << string_format("Unknown Int32 3:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Unknown Int32 4")) sPrint << string_format("Unknown Int32 4:\r\n%u", *((int*) lParam));
    else if((cItem[0] == "Unknown Int32 5")) sPrint << string_format("Unknown Int32 5:\r\n%u", *((int*) lParam));
    else sPrint.flush();
}
