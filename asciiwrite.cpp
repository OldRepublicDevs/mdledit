#include "MDL.h"
#include <algorithm>
#include <iomanip>

/**
    Functions:
    MDL::ExportAscii()
    MDL::ExportPwkAscii()
    MDL::ExportDwkAscii()
    MDL::ExportWokAscii()
    RecursiveAabb() //Helper
    MDL::ConvertToAscii()
/**/

void MDL::ExportAscii(std::string &sExport){
    if(!FH) return;
    std::stringstream ss;
    try{
        ConvertToAscii(CONVERT_MODEL, ss, (void*) &(FH->MH));
    }
    catch(const std::exception & e){
        Error("An exception occurred while creating model ascii:\n\n" + std::string(e.what()) + "\n\nThe exported ascii will be broken.");
    }
    sExport = ss.str();
}
void MDL::ExportPwkAscii(std::string &sExport){
    if(!Pwk) return;
    if(!Pwk->GetData()) return;
    std::stringstream ss;
    try{
        ConvertToAscii(CONVERT_PWK, ss, (void*) Pwk.get());
    }
    catch(const std::exception & e){
        Error("An exception occurred while creating pwk ascii:\n\n" + std::string(e.what()) + "\n\nThe exported ascii will be broken.");
    }
    sExport = ss.str();
}
void MDL::ExportDwkAscii(std::string &sExport){
    std::stringstream ss;
    try{
        ConvertToAscii(CONVERT_DWK, ss, (void*) this);
    }
    catch(const std::exception & e){
        Error("An exception occurred while creating dwk ascii:\n\n" + std::string(e.what()) + "\n\nThe exported ascii will be broken.");
    }
    sExport = ss.str();
}
void MDL::ExportWokAscii(std::string &sExport){
    if(!Wok) return;
    if(!Wok->GetData()) return;
    std::stringstream ss;
    try{
        ConvertToAscii(CONVERT_WOK, ss, (void*) Wok.get());
    }
    catch(const std::exception & e){
        Error("An exception occurred while creating wok ascii:\n\n" + std::string(e.what()) + "\n\nThe exported ascii will be broken.");
    }
    sExport = ss.str();
}

std::string nl = "\r\n";
void RecursiveAabb(Aabb * AABB, std::stringstream &str){
    str << nl << "    " << PrepareFloat(AABB->vBBmin.fX) <<
                " " << PrepareFloat(AABB->vBBmin.fY) <<
                " " << PrepareFloat(AABB->vBBmin.fZ) <<
                " " << PrepareFloat(AABB->vBBmax.fX) <<
                " " << PrepareFloat(AABB->vBBmax.fY) <<
                " " << PrepareFloat(AABB->vBBmax.fZ) <<
                " " << AABB->nID.Print();
    if(AABB->nChild1 > 0){
        RecursiveAabb(&AABB->Child1[0], str);
    }
    if(AABB->nChild2 > 0){
        RecursiveAabb(&AABB->Child2[0], str);
    }
}



/// Here we need another pass at the meshes to weld the vertices if the option is turned on
void BuildWeldedArrays(Node & node){
    if(!(node.Head.nType & NODE_MESH) || (node.Head.nType & NODE_SABER) || (node.Head.nType & NODE_AABB)) return;

    /// Get temp vertex and constraint arrays
    std::vector<Vertex> & vertices = node.Mesh.TempVertices;
    std::vector<double> * p_constraints = nullptr;
    if(node.Head.nType & NODE_DANGLY) p_constraints = &node.Dangly.TempConstraints;

    /// Clear the target arrays
    vertices.clear();
    if(p_constraints) p_constraints->clear();

    /// Reset all the processed flags to false
    for(Face & face : node.Mesh.Faces){
            face.bProcessed = {false, false, false};
    }

    for(int f = 0; f < node.Mesh.Faces.size(); f++){
        Face & face = node.Mesh.Faces.at(f);
        for(int i = 0; i < 3; i++){
            if(face.bProcessed.at(i)) continue;

            /// Copy the vertex struct
            Vertex vert = node.Mesh.Vertices.at(face.nIndexVertex.at(i));

            for(int f2 = f; f2 < node.Mesh.Faces.size(); f2++){
                Face & face2 = node.Mesh.Faces.at(f2);
                for(int i2 = 0; i2 < 3; i2++){
                    //Make sure that we're only changing what's past our current position if we are in the same face.
                    if(f2 == f && i2 <= i) continue;

                    Vertex & vert2 = node.Mesh.Vertices.at(face2.nIndexVertex.at(i2));
                    if(vert.fX == vert2.fX &&
                       vert.fY == vert2.fY &&
                       vert.fZ == vert2.fZ &&
                       (!(node.Head.nType & NODE_SKIN) || vert.MDXData.Weights == vert2.MDXData.Weights) &&
                       (!p_constraints || (node.Dangly.Constraints.at(face.nIndexVertex.at(i)) == node.Dangly.Constraints.at(face2.nIndexVertex.at(i2))))
                       ){
                        face2.nTempIndexVertex.at(i2) = node.Mesh.TempVertices.size();
                        face2.bProcessed.at(i2) = true;
                    }
                }
            }
            face.nTempIndexVertex.at(i) = node.Mesh.TempVertices.size();
            face.bProcessed.at(i) = true;
            if(p_constraints) node.Dangly.TempConstraints.push_back(node.Dangly.Constraints.at(face.nIndexVertex.at(i)));
            node.Mesh.TempVertices.push_back(std::move(vert));
        }
    }
}

void MDL::ConvertToAscii(int nDataType, std::stringstream & sReturn, void * Data){
    ReportObject ReportMdl(*this);
    //ReportMdl << "Converting to ascii, data type: " << nDataType << "\n";
    std::string nl = "\r\n";
    std::stringstream sTimestamp;
    static unsigned nExportedNodes = 0;
    SYSTEMTIME st;
    GetLocalTime(&st);
    sTimestamp << (st.wDay<10? "0" : "") << st.wDay << "/" << (st.wMonth<10? "0" : "") << st.wMonth << "/" << st.wYear;
    sTimestamp << " " << (st.wHour<10? "0" : "") << st.wHour << ":" << (st.wMinute<10? "0" : "") << st.wMinute << ":" << (st.wSecond<10? "0" : "") << st.wSecond;
    sTimestamp << " (Local Time)";

    if(nDataType == 0) return;
    else if(nDataType == CONVERT_MODEL){
        ModelHeader * mh = (ModelHeader*) Data;
        nExportedNodes = 0;
        sReturn << "# Exported with MDLedit " << version.Print() << " ";
        if(src == AsciiSource) sReturn << "from ascii source ";
        else if(src == BinarySource) sReturn << "from binary source ";
        else std::cout << "Error: Source neither ascii nor binary!\n";
        sReturn << "at " << sTimestamp.str();
        sReturn << nl << "# MODEL ASCII";
        sReturn << nl << "newmodel " << mh->GH.sName.c_str();
        sReturn << nl << "setsupermodel " << mh->GH.sName.c_str() << " " << mh->cSupermodelName.c_str();
        sReturn << nl << "classification " << ReturnClassificationName(mh->nClassification).c_str();
        sReturn << nl << "classification_unk1 " << (int) mh->nSubclassification;
        sReturn << nl << "ignorefog " << (mh->nAffectedByFog ? 0 : 1);
        sReturn << nl << "setanimationscale " << PrepareFloat(mh->fScale);
        sReturn << nl << "compress_quaternions " << (mh->bCompressQuaternions ? 1 : 0);
        sReturn << nl << "headlink " << (HeadLinked() ? 1 : 0);
        sReturn << nl;
        sReturn << nl << "# GEOM ASCII";
        sReturn << nl << "beginmodelgeom " << mh->GH.sName.c_str();
        if(Wok)
            sReturn << nl << "  layoutposition " << PrepareFloat(mh->vLytPosition.fX) << " " << PrepareFloat(mh->vLytPosition.fY) << " " << PrepareFloat(mh->vLytPosition.fZ);
        sReturn     << nl << "  bmin " << PrepareFloat(mh->vBBmin.fX) << " " << PrepareFloat(mh->vBBmin.fY) << " " << PrepareFloat(mh->vBBmin.fZ);
        sReturn     << nl << "  bmax " << PrepareFloat(mh->vBBmax.fX) << " " << PrepareFloat(mh->vBBmax.fY) << " " << PrepareFloat(mh->vBBmax.fZ);
        sReturn     << nl << "  radius " << PrepareFloat(mh->fRadius);
        for(int n = 0; n < mh->Names.size(); n++){
            ConvertToAscii(CONVERT_NODE, sReturn, (void*) &n);
        }
        sReturn << nl << "endmodelgeom " << mh->GH.sName.c_str() << nl;

        if(bWriteAnimations){
            for(int n = 0; n < mh->Animations.size(); n++){
                ConvertToAscii(CONVERT_ANIMATION, sReturn, (void*) &mh->Animations.at(n));
            }
        }

        sReturn << nl << nl << "donemodel " << mh->GH.sName.c_str() << nl;

        ReportMdl << "Exported nodes: " << nExportedNodes << "\n";
    }
    else if(nDataType == CONVERT_ANIMATION){
        Animation * anim = (Animation*) Data;
        sReturn << nl << "newanim " << anim->sName.c_str() << " " << FH->MH.GH.sName.c_str();
        sReturn << nl << "  length " << PrepareFloat(anim->fLength);
        sReturn << nl << "  transtime " << PrepareFloat(anim->fTransition);
        sReturn << nl << "  animroot " << anim->sAnimRoot.c_str();
        if(anim->Events.size() > 0){
            for(int s = 0; s < anim->Events.size(); s++){
                sReturn << nl << "  event " << anim->Events.at(s).fTime << " " << anim->Events.at(s).sName.c_str();
            }
        }
        for(int n = 0; n < anim->ArrayOfNodes.size(); n++){
            Node & node = anim->ArrayOfNodes.at(n);
            ConvertToAscii(CONVERT_ANIMATION_NODE, sReturn, (void*) &node);
        }
        sReturn << nl << "doneanim " << anim->sName.c_str() << " " << FH->MH.GH.sName.c_str();
    }
    else if(nDataType == CONVERT_ANIMATION_NODE){
        Node * node = (Node*) Data;
        sReturn << nl << "    node dummy ";
        sReturn << MakeUniqueName(node->Head.nNameIndex);
        //ReportMdl << "Animation node: " << MakeUniqueName(node->Head.nNameIndex) << nl;
        sReturn << nl << "      parent " << (node->Head.nParentIndex.Valid() ? MakeUniqueName(node->Head.nParentIndex) : "NULL");
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_KEYED, sReturn, (void*) &(node->Head.Controllers.at(n)));
            }
        }
        else if(node->Head.ControllerData.size() > 0){
            sReturn << nl << "      extra_data " << node->Head.ControllerData.size();
            //ConvertToAscii(CONVERT_CONTROLLERLESS_DATA, sReturn, (void*) node);
        }
        sReturn << nl << "    endnode";
    }
    else if(nDataType == CONVERT_NODE){
        if(!FH) return;
        ModelHeader * mh = &FH->MH;
        int n = *((int*) Data);
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(n);

        if(!nNodeIndex.Valid()){
            sReturn << nl << "name " << mh->Names.at(n).sName;
        }
        else{
            Node & node = mh->ArrayOfNodes.at(nNodeIndex);

            //std::cout << "Exporting node " << nExportedNodes << " (" << GetNodeName(node) << ")\n";
            nExportedNodes++;

            if(node.Head.nType & NODE_HEADER){
                ConvertToAscii(CONVERT_HEADER, sReturn, (void*) &node);
            }
            else{
            }
            if(node.Head.nType & NODE_AABB){
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_AABB, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_SABER){
                ConvertToAscii(CONVERT_SABER, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_DANGLY){
                if(bMinimizeVerts2) BuildWeldedArrays(node);
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_DANGLY, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_SKIN && !bSkinToTrimesh){
                if(bMinimizeVerts2) BuildWeldedArrays(node);
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_SKIN, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_MESH){
                if(bMinimizeVerts2) BuildWeldedArrays(node);
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_REFERENCE){
                ConvertToAscii(CONVERT_REFERENCE, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_EMITTER){
                ConvertToAscii(CONVERT_EMITTER, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_LIGHT){
                ConvertToAscii(CONVERT_LIGHT, sReturn, (void*) &node);
            }
            if(node.Head.nType != 0) sReturn << nl << "endnode";
        }
    }
    else if(nDataType == CONVERT_HEADER){
        Node * node = (Node*) Data;
        sReturn << nl << "node ";
        if(     node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_AABB)) sReturn << "aabb";
        else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_DANGLY)) sReturn << "danglymesh";
        else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_SKIN)) sReturn << (!bSkinToTrimesh ? "skin" : "trimesh");
        else if(node->Head.nType == (NODE_HEADER | NODE_MESH | NODE_SABER)) sReturn << (!bLightsaberToTrimesh ? "lightsaber" : "trimesh");
        else if(node->Head.nType == (NODE_HEADER | NODE_MESH)) sReturn << "trimesh";
        else if(node->Head.nType == (NODE_HEADER | NODE_REFERENCE)) sReturn << "reference";
        else if(node->Head.nType == (NODE_HEADER | NODE_EMITTER)) sReturn << "emitter";
        else if(node->Head.nType == (NODE_HEADER | NODE_LIGHT)) sReturn << "light";
        else if(node->Head.nType == (NODE_HEADER)) sReturn << "dummy";
        else sReturn << "dummy"; /// A catch-all
        sReturn << " ";
        sReturn << MakeUniqueName(node->Head.nNameIndex);
        sReturn << nl << "  parent " << (node->Head.nParentIndex.Valid() ? MakeUniqueName(node->Head.nParentIndex) : "NULL");
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sReturn, (void*) &(node->Head.Controllers.at(n)));
            }
        }
    }
    else if(nDataType == CONVERT_LIGHT){
        Node * node = (Node*) Data;
        sReturn << nl << "  lightpriority " << node->Light.nLightPriority;
        sReturn << nl << "  ndynamictype " << node->Light.nDynamicType;
        sReturn << nl << "  ambientonly " << node->Light.nAmbientOnly;
        sReturn << nl << "  affectdynamic " << node->Light.nAffectDynamic;
        sReturn << nl << "  shadow " << node->Light.nShadow;
        sReturn << nl << "  flare " << node->Light.nFlare;
        sReturn << nl << "  fadinglight " << node->Light.nFadingLight;
        sReturn << nl << "  flareradius " << PrepareFloat(node->Light.fFlareRadius);
        sReturn << nl << "  texturenames " << node->Light.FlareTextureNames.size();
        for(int n = 0; n < node->Light.FlareTextureNames.size(); n++){
            sReturn << nl << "    " << node->Light.FlareTextureNames.at(n).sName;
        }
        sReturn << nl << "  flaresizes " << node->Light.FlareSizes.size();
        for(int n = 0; n < node->Light.FlareSizes.size(); n++){
            sReturn << nl << "    " << node->Light.FlareSizes.at(n);
        }
        sReturn << nl << "  flarepositions " << node->Light.FlarePositions.size();
        for(int n = 0; n < node->Light.FlarePositions.size(); n++){
            sReturn << nl << "    " << node->Light.FlarePositions.at(n);
        }
        sReturn << nl << "  flarecolorshifts " << node->Light.FlareColorShifts.size();
        for(int n = 0; n < node->Light.FlareColorShifts.size(); n++){
            sReturn << nl << "    " << node->Light.FlareColorShifts.at(n).fR << " " << node->Light.FlareColorShifts.at(n).fG << " " << node->Light.FlareColorShifts.at(n).fB;
        }
    }
    else if(nDataType == CONVERT_EMITTER){
        Node * node = (Node*) Data;
        sReturn << nl << "  deadspace " << PrepareFloat(node->Emitter.fDeadSpace);
        sReturn << nl << "  blastRadius " << PrepareFloat(node->Emitter.fBlastRadius);
        sReturn << nl << "  blastLength " << PrepareFloat(node->Emitter.fBlastLength);
        sReturn << nl << "  numBranches " << node->Emitter.nBranchCount;
        sReturn << nl << "  controlptsmoothing " << node->Emitter.fControlPointSmoothing;
        sReturn << nl << "  xgrid " << node->Emitter.nxGrid;
        sReturn << nl << "  ygrid " << node->Emitter.nyGrid;
        sReturn << nl << "  spawntype " << node->Emitter.nSpawnType;
        sReturn << nl << "  update " << node->Emitter.cUpdate.c_str();
        sReturn << nl << "  render " << node->Emitter.cRender.c_str();
        sReturn << nl << "  blend " << node->Emitter.cBlend.c_str();
        sReturn << nl << "  texture " << node->Emitter.cTexture.c_str();
        sReturn << nl << "  chunkname " << node->Emitter.cChunkName.c_str();
        sReturn << nl << "  twosidedtex " << node->Emitter.nTwosidedTex;
        sReturn << nl << "  loop " << node->Emitter.nLoop;
        sReturn << nl << "  renderorder " << node->Emitter.nRenderOrder;
        sReturn << nl << "  m_bFrameBlending " << (int) node->Emitter.nFrameBlending;
        sReturn << nl << "  m_sDepthTextureName " << node->Emitter.cDepthTextureName.c_str();

        sReturn << nl << "  p2p " << (node->Emitter.nFlags & EMITTER_FLAG_P2P ? 1 : 0);
        sReturn << nl << "  p2p_sel " << (node->Emitter.nFlags & EMITTER_FLAG_P2P_SEL ? 1 : 0);
        sReturn << nl << "  affectedByWind " << (node->Emitter.nFlags & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0);
        sReturn << nl << "  m_isTinted " << (node->Emitter.nFlags & EMITTER_FLAG_TINTED ? 1 : 0);
        sReturn << nl << "  bounce " << (node->Emitter.nFlags & EMITTER_FLAG_BOUNCE ? 1 : 0);
        sReturn << nl << "  random " << (node->Emitter.nFlags & EMITTER_FLAG_RANDOM ? 1 : 0);
        sReturn << nl << "  inherit " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT ? 1 : 0);
        sReturn << nl << "  inheritvel " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_VEL ? 1 : 0);
        sReturn << nl << "  inherit_local " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0);
        sReturn << nl << "  splat " << (node->Emitter.nFlags & EMITTER_FLAG_SPLAT ? 1 : 0);
        sReturn << nl << "  inherit_part " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_PART ? 1 : 0);
        sReturn << nl << "  depth_texture " << (node->Emitter.nFlags & EMITTER_FLAG_DEPTH_TEXTURE ? 1 : 0);
        sReturn << nl << "  emitterflag13 " << (node->Emitter.nFlags & EMITTER_FLAG_13 ? 1 : 0);
    }
    else if(nDataType == CONVERT_REFERENCE){
        Node * node = (Node*) Data;
        sReturn << nl << "  refModel " << node->Reference.sRefModel.c_str();
        sReturn << nl << "  reattachable " << node->Reference.nReattachable;
    }
    else if(nDataType == CONVERT_MESH){
        Node * node = (Node*) Data;
        sReturn << nl << "  diffuse " << PrepareFloat(node->Mesh.fDiffuse.fR) << " " << PrepareFloat(node->Mesh.fDiffuse.fG) << " " << PrepareFloat(node->Mesh.fDiffuse.fB);
        sReturn << nl << "  ambient " << PrepareFloat(node->Mesh.fAmbient.fR) << " " << PrepareFloat(node->Mesh.fAmbient.fG) << " " << PrepareFloat(node->Mesh.fAmbient.fB);
        sReturn << nl << "  transparencyhint " << node->Mesh.nTransparencyHint;
        sReturn << nl << "  animateuv " << node->Mesh.nAnimateUV;
        if(node->Mesh.nAnimateUV == 0){
            sReturn << nl << "  uvdirectionx 0.0";
            sReturn << nl << "  uvdirectiony 0.0";
            sReturn << nl << "  uvjitter 0.0";
            sReturn << nl << "  uvjitterspeed 0.0";
        }
        else{
            sReturn << nl << "  uvdirectionx " << PrepareFloat(node->Mesh.fUVDirectionX);
            sReturn << nl << "  uvdirectiony " << PrepareFloat(node->Mesh.fUVDirectionY);
            sReturn << nl << "  uvjitter " << PrepareFloat(node->Mesh.fUVJitter);
            sReturn << nl << "  uvjitterspeed " << PrepareFloat(node->Mesh.fUVJitterSpeed);
        }
        sReturn << nl << "  lightmapped " << (int) node->Mesh.nHasLightmap;
        sReturn << nl << "  rotatetexture " << (int) node->Mesh.nRotateTexture;
        sReturn << nl << "  m_bIsBackgroundGeometry " << (int) node->Mesh.nBackgroundGeometry;
        sReturn << nl << "  shadow " << (int) node->Mesh.nShadow;
        sReturn << nl << "  beaming " << (int) node->Mesh.nBeaming;
        sReturn << nl << "  render " << (int) node->Mesh.nRender;
        sReturn << nl << "  dirt_enabled " << (int) node->Mesh.nDirtEnabled;
        sReturn << nl << "  dirt_texture " << node->Mesh.nDirtTexture;
        sReturn << nl << "  dirt_worldspace " << node->Mesh.nDirtCoordSpace;
        sReturn << nl << "  hologram_donotdraw " << (int) node->Mesh.nHideInHolograms;
        sReturn << nl << "  tangentspace " << (node->Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1 ? 1 : 0);
        sReturn << nl << "  inv_count " << node->Mesh.nMeshInvertedCounter;
        if(node->Mesh.cTexture1.c_str() != std::string()) sReturn << nl << "  bitmap " << node->Mesh.GetTexture(1);
        if(node->Mesh.cTexture2.c_str() != std::string()) sReturn << nl << "  bitmap2 " << node->Mesh.GetTexture(2);
        if(node->Mesh.cTexture3.c_str() != std::string()) sReturn << nl << "  texture0 " << node->Mesh.GetTexture(3);
        if(node->Mesh.cTexture4.c_str() != std::string()) sReturn << nl << "  texture1 " << node->Mesh.GetTexture(4);

        /// If the Wok is present, we may want to put out the wok verts instead.
        std::vector<Vertex> * ptr_verts = &node->Mesh.Vertices;
        std::vector<Vertex> WokVerts;
        if(bUseWokData && (node->Head.nType & NODE_AABB) && Wok && Wok->GetData()){
            WokVerts = GetWokVertData(*node);
            ptr_verts = &WokVerts;
        }
        else if(bMinimizeVerts2 && !(node->Head.nType & NODE_AABB) && !(node->Head.nType & NODE_SABER)){
            ptr_verts = &node->Mesh.TempVertices;
        }

        sReturn << nl << "  verts " << ptr_verts->size();
        for(int n = 0; n < ptr_verts->size(); n++){
            //Two possibilities - I put MDX if MDX is present, otherwise MDL
            if(!Mdx) sReturn << nl << "    " << PrepareFloat(ptr_verts->at(n).fX) << " " << PrepareFloat(ptr_verts->at(n).fY) << " " << PrepareFloat(ptr_verts->at(n).fZ);
            else if(!bXbox) sReturn << nl << "    " << PrepareFloat(ptr_verts->at(n).MDXData.vVertex.fX) << " " << PrepareFloat(ptr_verts->at(n).MDXData.vVertex.fY) << " " << PrepareFloat(ptr_verts->at(n).MDXData.vVertex.fZ);
        }
        sReturn << nl << "  faces " << node->Mesh.Faces.size();
        for(int n = 0; n < node->Mesh.Faces.size(); n++){
            sReturn << nl << "    ";
            if(bMinimizeVerts2 && !(node->Head.nType & NODE_AABB) && !(node->Head.nType & NODE_SABER)){
                sReturn << node->Mesh.Faces.at(n).nTempIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nTempIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nTempIndexVertex.at(2);
            }
            else{
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << "  " << std::to_string((node->Mesh.Faces.at(n).nSmoothingGroup == 0) ? 1 : node->Mesh.Faces.at(n).nSmoothingGroup);
            if(node->Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                sReturn << "  " << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            else sReturn << "  0 0 0";
            sReturn << "  " << node->Mesh.Faces.at(n).nMaterialID;
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
            sReturn << nl << "  tverts " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << nl << "    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV1.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV1.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
            sReturn << nl << "  texindices1 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << nl << "    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << nl << "  tverts1 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << nl << "    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV2.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV2.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
            sReturn << nl << "  texindices2 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << nl << "    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << nl << "  tverts2 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << nl << "    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV3.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV3.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
            sReturn << nl << "  texindices3 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << nl << "    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << nl << "  tverts3 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << nl << "    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV4.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV4.fY);
            }
        }
    }
    else if(nDataType == CONVERT_SKIN){
        Node * node = (Node*) Data;
        ModelHeader & data = FH->MH;
        if(!Mdx->sBuffer.empty()){
            std::vector<Vertex> * ptr_verts = &node->Mesh.Vertices;
            if(bMinimizeVerts2 && !(node->Head.nType & NODE_AABB) && !(node->Head.nType & NODE_SABER)){
                ptr_verts = &node->Mesh.TempVertices;
            }
            sReturn << nl << "  weights " << ptr_verts->size();
            for(int n = 0; n < ptr_verts->size(); n++){
                sReturn << nl << "    ";
                int i = 0;
                MdlInteger<unsigned short> nBoneNumber; // = (int) round(ptr_verts->at(n).MDXData.Weights.fWeightIndex.at(i));
                //ReportMdl << "Bone name index array size: " << node->Skin.BoneNameIndices.size() << nl;
                bool bDependentVert = false;
                while(i < 4){
                    nBoneNumber = ptr_verts->at(n).MDXData.Weights.nWeightIndex.at(i);
                    //ReportMdl << "Reading bone number " << nBoneNumber << nl;
                    if(nBoneNumber.Valid() && nBoneNumber < node->Skin.BoneNameIndices.size()){
                        MdlInteger<unsigned short> nNameIndex = node->Skin.BoneNameIndices.at(nBoneNumber);
                        //if (nNameIndex > -1) nNameIndex = data.NameIndicesInOffsetOrder.at(nNameIndex);
                        //ReportMdl << "Reading bone number " << nBoneNumber;
                        //ReportMdl << ", representing bone " << FH->MH.Names.at(node->Skin.BoneNameIndices.at(nBoneNumber)).sName.c_str() << ".\n";
                        //sReturn << " " << FH->MH.Names.at(nNameIndex).sName.c_str() << " " << PrepareFloat(ptr_verts->at(n).MDXData.Weights.fWeightValue.at(i));
                        if(nNameIndex.Valid() && nNameIndex < FH->MH.Names.size()){
                            if(!bDependentVert) bDependentVert = true;
                            sReturn << " " << MakeUniqueName(nNameIndex) << " " << PrepareFloat(ptr_verts->at(n).MDXData.Weights.fWeightValue.at(i));
                        }
                    }
                    i++;
                    //if(i < 4) nBoneNumber = (int) round(ptr_verts->at(n).MDXData.Weights.fWeightIndex.at(i));
                }
                if(!bDependentVert){
                    sReturn << " root 1.0";
                }
            }
        }
    }
    else if(nDataType == CONVERT_DANGLY){
        Node * node = (Node*) Data;
        sReturn << nl << "  displacement " << PrepareFloat(node->Dangly.fDisplacement);
        sReturn << nl << "  tightness " << PrepareFloat(node->Dangly.fTightness);
        sReturn << nl << "  period " << PrepareFloat(node->Dangly.fPeriod);

        std::vector<double> * ptr_constraints = &node->Dangly.Constraints;
        if(bMinimizeVerts2 && !(node->Head.nType & NODE_AABB) && !(node->Head.nType & NODE_SABER)){
            ptr_constraints = &node->Dangly.TempConstraints;
        }

        sReturn << nl << "  constraints " << ptr_constraints->size();
        for(int n = 0; n < ptr_constraints->size(); n++){
            sReturn << nl << "    " << PrepareFloat(ptr_constraints->at(n));
        }
    }
    else if(nDataType == CONVERT_AABB){
        Node * node = (Node*) Data;

        /// Write out aabb tree
        sReturn << nl << "  aabb";
        RecursiveAabb(&node->Walkmesh.RootAabb, sReturn);

        /// Write out room links
        if(Wok){
            if(Wok->GetData()){
                BWMHeader & data = *Wok->GetData();
                sReturn << nl << "  roomlinks";
                Vector vLyt;
                if(FH) vLyt = FH->MH.vLytPosition;
                for(int l = 0; l < data.edges.size(); l++){
                    if(data.edges.at(l).nTransition.Valid()){
                        int nIndex = data.edges.at(l).nIndex;
                        Face & wokface = data.faces.at(nIndex/3);
                        Vector & v1 = data.verts.at(wokface.nIndexVertex.at(0));
                        Vector & v2 = data.verts.at(wokface.nIndexVertex.at(1));
                        Vector & v3 = data.verts.at(wokface.nIndexVertex.at(2));
                        for(int f = 0; f < node->Mesh.Faces.size(); f++){
                            Face & mdlface = node->Mesh.Faces.at(f);
                            Vector v4 (node->Mesh.Vertices.at(mdlface.nIndexVertex.at(0)).vFromRoot + vLyt);
                            Vector v5 (node->Mesh.Vertices.at(mdlface.nIndexVertex.at(1)).vFromRoot + vLyt);
                            Vector v6 (node->Mesh.Vertices.at(mdlface.nIndexVertex.at(2)).vFromRoot + vLyt);
                            if(v1.Compare(v4, 0.1) && v2.Compare(v5, 0.1) && v3.Compare(v6, 0.1)){
                                mdlface.nEdgeTransitions.at(nIndex%3) = data.edges.at(l).nTransition;
                                sReturn << nl << "    " << (3*f + nIndex%3) << " " << data.edges.at(l).nTransition;
                                break;
                            }
                        }
                    }
                }
                sReturn << nl << "  endlist";
            }
        }
    }
    else if(nDataType == CONVERT_SABER){
        Node * node = (Node*) Data;
        sReturn << nl << "  inv_count " << node->Saber.nInvCount1 << " " << node->Saber.nInvCount2;
        if(node->Mesh.cTexture1.c_str() != std::string()) sReturn << nl << "  bitmap " << node->Mesh.GetTexture(1);

        if(node->Saber.SaberData.size() == 176){
            Vector vDiff;
            Vector vOut;
            vDiff = node->Saber.SaberData.at(4).vVertex - node->Saber.SaberData.at(0).vVertex;

            sReturn << nl << "  verts 16";
            vOut = node->Saber.SaberData.at(0).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(1).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(2).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(3).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(0).vVertex + vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(1).vVertex + vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(2).vVertex + vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(3).vVertex + vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(88).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(89).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(90).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(91).vVertex;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(88).vVertex - vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(89).vVertex - vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(90).vVertex - vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(91).vVertex - vDiff;
            sReturn << nl << "    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            sReturn << nl << "  faces 12";
            /// SIDE 1
            sReturn << nl << "    5 4 0  1  5 4 0  0";
            sReturn << nl << "    0 1 5  1  0 1 5  0";
            sReturn << nl << "    13 8 12  1  13 8 12  0";
            sReturn << nl << "    8 13 9  1  8 13 9  0";
            sReturn << nl << "    6 5 1  1  6 5 1  0";
            //sReturn << nl << "    2 6 5  1  2 6 5  0"; //Correct faces, but not what they are in the game
            sReturn << nl << "    1 2 6  1  1 2 6  0";
            //sReturn << nl << "    1 2 5  1  1 2 5  0"; //Correct faces, but not what they are in the game
            sReturn << nl << "    10 9 13  1  10 9 13  0";
            sReturn << nl << "    13 14 10  1  13 14 10  0";
            sReturn << nl << "    3 6 2  1  3 6 2  0";
            sReturn << nl << "    6 3 7  1  6 3 7  0";
            sReturn << nl << "    15 11 14  1  15 11 14  0";
            sReturn << nl << "    10 14 11  1  10 14 11  0";
            /// SIDE 2
            /*
            sReturn << nl << "    4 5 0  1  4 5 0  0";
            sReturn << nl << "    1 0 5  1  1 0 5  0";
            sReturn << nl << "    8 13 12  1  8 13 12  0";
            sReturn << nl << "    13 8 9  1  13 8 9  0";
            sReturn << nl << "    5 6 1  1  5 6 1  0";
            //sReturn << nl << "    6 2 5  1  6 2 5  0"; //Correct faces, but not what they are in the game
            sReturn << nl << "    2 1 6  1  2 1 6  0";
            //sReturn << nl << "    2 1 5  1  2 1 5  0"; //Correct faces, but not what they are in the game
            sReturn << nl << "    9 10 13  1  9 10 13  0";
            sReturn << nl << "    14 13 10  1  14 13 10  0";
            sReturn << nl << "    6 3 2  1  6 3 2  0";
            sReturn << nl << "    3 6 7  1  3 6 7  0";
            sReturn << nl << "    11 15 14  1  11 15 14  0";
            sReturn << nl << "    14 10 11  1  14 10 11  0";
            */

            sReturn << nl << "  tverts 16";
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(0).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(0).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(1).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(1).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(2).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(2).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(3).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(3).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(4).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(4).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(5).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(5).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(6).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(6).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(7).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(7).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(88).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(88).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(89).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(89).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(90).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(90).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(91).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(91).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(92).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(92).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(93).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(93).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(94).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(94).vUV1.fY);
            sReturn << nl << "    " << PrepareFloat(node->Saber.SaberData.at(95).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(95).vUV1.fY);
        }
    }
    /// TODO: cases where num(controllers) == 0 but num(controller data) > 0
    else if(nDataType == CONVERT_CONTROLLERLESS_DATA){
        Node & node = * (Node*) Data;
        ModelHeader & data = FH->MH;
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(node.Head.nNameIndex);
        if(!nNodeIndex.Valid()) throw mdlexception("converting controllerless data to ascii error: dealing with a name index that does not have a node in geometry.");
        Node & geonode = data.ArrayOfNodes.at(nNodeIndex);
        Location loc = geonode.GetLocation();

        std::cout << "Converting controllerless data " << node.Head.ControllerData.size() << ".\n";
        if(node.Head.ControllerData.size() == 4){
            sReturn << nl << "      controllerless_orientationkey";
            //Compressed orientation
            Quaternion qCurrent;
            AxisAngle aaCurrent;
            for(int n = 0; n < 2; n++){
                sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(n)) << " ";
                float fCompressed = node.Head.ControllerData.at(2 + n);
                qCurrent = DecompressQuaternion(*(unsigned*)&fCompressed);
                aaCurrent = AxisAngle(qCurrent);
                sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
            }
            sReturn << nl << "      endlist";
        }
        else if(node.Head.ControllerData.size() == 8){
            sReturn << nl << "      controllerless_positionkey";
            //normal position
            for(int n = 0; n < 2; n++){
                sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(n)) << " ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(2 + n*3 + 0));
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(2 + n*3 + 1));
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(2 + n*3 + 2));
            }
            sReturn << nl << "      endlist";
        }
        else if(node.Head.ControllerData.size() == 12){
            sReturn << nl << "      controllerless_positionkey";
            //normal position
            for(int n = 0; n < 2; n++){
                sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(n)) << " ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(2 + n*3 + 0));
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(2 + n*3 + 1));
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(2 + n*3 + 2));
            }
            sReturn << nl << "      endlist";

            sReturn << nl << "      controllerless_orientationkey";
            //Compressed orientation
            Quaternion qCurrent;
            AxisAngle aaCurrent;
            for(int n = 0; n < 2; n++){
                sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(8 + n)) << " ";
                float fCompressed = node.Head.ControllerData.at(8 + 2 + n);
                qCurrent = DecompressQuaternion(*(unsigned*)&fCompressed);
                aaCurrent = AxisAngle(qCurrent);
                sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
            }
            sReturn << nl << "      endlist";
        }
        else std::cout << "Found a new type of controllerless animation data: " << node.Head.ControllerData.size() << "floats!\n";
    }
    else if(nDataType == CONVERT_CONTROLLER_KEYED){
        Controller * ctrl = (Controller*) Data;
        ModelHeader & data = FH->MH;
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(ctrl->nNameIndex);
        if(!nNodeIndex.Valid()) throw mdlexception("converting keyed controller to ascii error: dealing with a name index that does not have a node in geometry.");
        Node & geonode = data.ArrayOfNodes.at(nNodeIndex);
        Location loc = geonode.GetLocation();
        Node * p_node = nullptr;
        if(ctrl->nAnimation.Valid()){
            Animation & anim = data.Animations.at(ctrl->nAnimation);
            /// The advantage of these loops is that they will find ALL nodes with the same node index, even if there are several.
            for(Node & animnode : anim.ArrayOfNodes){
                if(ctrl->nNameIndex == animnode.Head.nNameIndex){
                    for(Controller & search_ctrl : animnode.Head.Controllers){
                        if(&search_ctrl == ctrl){
                            p_node = &animnode;
                            break;
                        }
                    }
                    if(p_node) break;
                }
            }
        }
        if(!p_node) throw mdlexception("ConvertToAscii() ERROR: Couldn't find the animation node to which a keyed controller belongs.");
        Node & node = *p_node; /// This could crash
        //ReportMdl << "Node does not crash\n";

        sReturn << nl << "      ";
        sReturn << ReturnControllerName(ctrl->nControllerType, geonode.Head.nType);
        if(ctrl->nColumnCount & 16 && !bBezierToLinear) sReturn << "bezier";
        sReturn << "key";
        try{
            if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
                //Compressed orientation
                Quaternion qCurrent, qPrevious;
                AxisAngle aaCurrent, aaDiff;
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                    float fCompressed = node.Head.ControllerData.at(ctrl->nDataStart + n);
                    qCurrent = DecompressQuaternion(*(unsigned*)&fCompressed);
                    aaCurrent = AxisAngle(qCurrent);

                    sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
                }
            }
            else if(ctrl->nColumnCount == 4 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
                //Uncompressed orientation
                Quaternion qCurrent, qPrevious;
                AxisAngle aaCurrent, aaDiff;
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                    qCurrent = Quaternion(node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 0),
                                          node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 1),
                                          node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 2),
                                          node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 3));
                    aaCurrent = AxisAngle(qCurrent);

                    sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
                }
            }
            /// positionbezierkey
            else if((ctrl->nColumnCount & 16) && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
                //positionbezierkey
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n));
                    sReturn << " " << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (0)));
                    sReturn << " " << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (1)));
                    sReturn << " " << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (2)));
                    if(!bBezierToLinear){
                        sReturn << "  " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (3)));
                        sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (4)));
                        sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (5)));
                        sReturn << "  " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (6)));
                        sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (7)));
                        sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (8)));
                    }
                }
            }
            /// regular bezierkey
            else if((ctrl->nColumnCount & 16)){
                //bezierkey
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                    for(int i = 0; i < (ctrl->nColumnCount & 15) * (!bBezierToLinear ? 3 : 1); i++){
                        sReturn << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*((ctrl->nColumnCount & 15) * 3) + i));
                        if(i < (ctrl->nColumnCount & 15) * 3 - 1) sReturn << " ";
                        if((i+1) % (ctrl->nColumnCount & 15) == 0) sReturn << " ";
                    }
                }
            }
            /// positionkey
            else if(ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
                //normal position
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                    sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 0));
                    sReturn << " ";
                    sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 1));
                    sReturn << " ";
                    sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + 2));
                }
            }
            /// regular key
            else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){
                //default parser
                for(int n = 0; n < ctrl->nValueCount; n++){
                    sReturn << nl << "        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                    for(int i = 0; i < ctrl->nColumnCount; i++){
                        sReturn << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + i));
                        if(i < ctrl->nColumnCount - 1) sReturn << " ";
                    }
                }
            }
            else{
                std::string sLocation;
                if(!ctrl->nAnimation.Valid()) sLocation = "geometry";
                else sLocation = FH->MH.Animations.at(ctrl->nAnimation).sName;
                ReportMdl << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << FH->MH.Names.at(ctrl->nNameIndex).sName << " (" << sLocation.c_str() << ")!\n";
                Error("A controller type is not being handled! Check the console and add the necessary code!");
            }
        }
        catch(const std::out_of_range & e){
            Warning("Missing controller data on animation controller '" + ReturnControllerName(ctrl->nControllerType, geonode.Head.nType) +
                               "' on node '" + data.Names.at(node.Head.nNameIndex).sName + "' in animation '" + data.Animations.at(node.nAnimation).sName.c_str() + "'.\n" + e.what());
        }
        catch(const std::exception & e){
            Error("An exception occurred on animation controller '" + ReturnControllerName(ctrl->nControllerType, geonode.Head.nType) +
                               "' on node '" + data.Names.at(node.Head.nNameIndex).sName + "' in animation '" + data.Animations.at(node.nAnimation).sName.c_str() + "':\n" + e.what());
        }
        catch(...){
            Error("An unknown exception occurred on animation controller '" + ReturnControllerName(ctrl->nControllerType, geonode.Head.nType) +
                               "' on node '" + data.Names.at(node.Head.nNameIndex).sName + "' in animation '" + data.Animations.at(node.nAnimation).sName.c_str() + "'.");
        }
        sReturn << nl << "      endlist";
    }
    else if(nDataType == CONVERT_CONTROLLER_SINGLE){
        Controller * ctrl = (Controller*) Data;
        if(ctrl->nValueCount > 1){
            Error("Error! Single controller has more than one value. Skipping.");
            return;
        }
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(ctrl->nNameIndex);
        if(!nNodeIndex.Valid()) throw mdlexception("converting single controller to ascii error: dealing with a name index that does not have a node in geometry.");
        Node & node = FH->MH.ArrayOfNodes.at(nNodeIndex);
        sReturn << nl << "  " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " ";
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            float fCompressed = node.Head.ControllerData.at(ctrl->nDataStart);
            Orientation CtrlOrient(DecompressQuaternion(*(unsigned*)&fCompressed));
            //sReturn << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fX) << " " << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fY) << " " << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fZ) << " " << PrepareFloat(CtrlOrient.GetQuaternion().fW);
            sReturn << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fX) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fY) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fZ) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().fAngle);
        }
        else if(ctrl->nColumnCount == 4 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Uncompressed orientation
            Orientation CtrlOrient;
            double fQX = node.Head.ControllerData.at(ctrl->nDataStart + 0);
            double fQY = node.Head.ControllerData.at(ctrl->nDataStart + 1);
            double fQZ = node.Head.ControllerData.at(ctrl->nDataStart + 2);
            double fQW = node.Head.ControllerData.at(ctrl->nDataStart + 3);
            CtrlOrient.SetQuaternion(fQX, fQY, fQZ, fQW);
            //sReturn << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fX) << " " << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fY) << " " << PrepareFloat(CtrlOrient.GetQuaternion().vAxis.fZ) << " " << PrepareFloat(CtrlOrient.GetQuaternion().fW);
            sReturn << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fX) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fY) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().vAxis.fZ) << " " << PrepareFloat(CtrlOrient.GetAxisAngle().fAngle);
        }
        else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){
            //default parser
            for(int i = 0; i < ctrl->nColumnCount; i++){
                sReturn << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + i));
                if(i < ctrl->nColumnCount - 1) sReturn << " ";
            }

        }
        else{
            std::string sLocation;
            if(!ctrl->nAnimation.Valid()) sLocation = "geometry";
            else sLocation = FH->MH.Animations.at(ctrl->nAnimation).sName;
            ReportMdl << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << FH->MH.Names.at(ctrl->nNameIndex).sName << " (" << sLocation.c_str() << ")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
    }
    else if(nDataType == CONVERT_WOK){
        BWM * bwm = (BWM*) Data;
        if(!bwm->GetData()) return;
        BWMHeader & data = *bwm->GetData();

        std::string sModel ("unknown");
        if(FH) sModel = FH->MH.GH.sName.c_str();

        sReturn << "# Exported with MDLedit " << version.Print() << " ";
        if(src == AsciiSource) sReturn << "from ascii source ";
        else if(src == BinarySource) sReturn << "from binary source ";
        else std::cout << "Error: Source neither ascii nor binary!\n";
        sReturn << "at " << sTimestamp.str();
        sReturn << nl << "# WOKMESH  ASCII";
        sReturn << nl << "node trimesh WALKMESH";
        sReturn << nl << "  parent " << sModel;
        sReturn << nl << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
        sReturn << nl << "  orientation 1.0 0.0 0.0 0.0";

        sReturn << nl << "  verts " << data.verts.size();
        for(int n = 0; n < data.verts.size(); n++){
            sReturn << nl << "    " << PrepareFloat(data.verts.at(n).fX) << " " << PrepareFloat(data.verts.at(n).fY) << " " << PrepareFloat(data.verts.at(n).fZ);
        }

        sReturn << nl << "  faces " << data.faces.size();
        for(int n = 0; n < data.faces.size(); n++){
            sReturn << nl << "    ";
            sReturn << data.faces.at(n).nIndexVertex.at(0);
            sReturn << " " << data.faces.at(n).nIndexVertex.at(1);
            sReturn << " " << data.faces.at(n).nIndexVertex.at(2);
            sReturn << "  1";
            sReturn << "  0 0 0";
            sReturn << "  " << data.faces.at(n).nMaterialID;
        }

        int nRoomLinkSize = 0;
        std::stringstream ssRoomlinks;
        for(int n = 0; n < data.edges.size(); n++){
            if(data.edges.at(n).nTransition.Valid()){
                ssRoomlinks << nl << "    " << data.edges.at(n).nIndex << " " << data.edges.at(n).nTransition;
                nRoomLinkSize++;
            }
        }
        sReturn << nl << "  roomlinks " << data.edges.size() << ssRoomlinks.str();

        sReturn << nl << "endnode";
    }
    else if(nDataType == CONVERT_PWK){
        BWM * bwm = (BWM*) Data;
        if(!bwm->GetData()) return;
        BWMHeader & data = *bwm->GetData();
        /// DETERMINE NAMES
        std::string sUse1, sUse2, sMesh, sRoot, sModel;
        if(FH){
            FileHeader & Data = *FH;

            sModel = Data.MH.GH.sName.c_str();
            for(int n = 0; n < Data.MH.Names.size(); n++){
                std::string & sName = Data.MH.Names.at(n).sName;
                if(sName.find("pwk_use01")!=std::string::npos || sName.find("pwk_dp_use_01")!=std::string::npos) sUse1 = sName;
                else if(sName.find("pwk_use02")!=std::string::npos || sName.find("pwk_dp_use_02")!=std::string::npos) sUse2 = sName;
                else if(safesubstr(sName, strlen(sName.c_str()) - 3, 3) == "_wg") sMesh = sName;
                else if(safesubstr(sName, strlen(sName.c_str()) - 4, 4) == "_pwk") sRoot = sName;
            }
            std::string sPrefix = safesubstr(Data.MH.GH.sName, 0, 4);
            std::transform(sPrefix.begin(), sPrefix.end(), sPrefix.begin(), ::tolower);
            if(sPrefix == "plc_") sPrefix = safesubstr(Data.MH.GH.sName, 4, 3);
            else sPrefix = safesubstr(Data.MH.GH.sName, 0, 3);
            if(sUse1.empty()) sUse1 = sPrefix + "_pwk_use01";
            if(sUse2.empty()) sUse2 = sPrefix + "_pwk_use02";
            if(sMesh.empty()) sMesh = std::string(Data.MH.GH.sName.c_str()) + "_wg";
            if(sRoot.empty()) sRoot = std::string(Data.MH.GH.sName.c_str()) + "_pwk";
        }
        if(sUse1.empty()) sUse1 = "pwk_use01";
        if(sUse2.empty()) sUse2 = "pwk_use02";
        if(sMesh.empty()) sMesh = "wg";
        if(sRoot.empty()) sRoot = "pwk";

        /// WRITE PWK
        sReturn << "# Exported with MDLedit " << version.Print() << " ";
        if(src == AsciiSource) sReturn << "from ascii source ";
        else if(src == BinarySource) sReturn << "from binary source ";
        else std::cout << "Error: Source neither ascii nor binary!\n";
        sReturn << "at " << sTimestamp.str();
        sReturn << nl << "# PWKMESH  ASCII";
        sReturn << nl << "node dummy " << sRoot;
        sReturn << nl << "  parent " << sModel;
        sReturn << nl << "endnode";
        sReturn << nl << "node trimesh " << sMesh;
        sReturn << nl << "  parent " << sRoot;
        sReturn << nl << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
        sReturn << nl << "  orientation 1.0 0.0 0.0 0.0";
        sReturn << nl << "  verts " << data.verts.size();
        for(int v = 0; v < data.verts.size(); v++){
            Vector & vert = data.verts.at(v);
            sReturn << nl << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
        }
        sReturn << nl << "  faces " << data.faces.size();
        for(int v = 0; v < data.faces.size(); v++){
            Face & face = data.faces.at(v);
            sReturn << nl << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
        }
        sReturn << nl << "endnode";
        sReturn << nl << "node dummy " << sUse1;
        sReturn << nl << "  parent " << sMesh;
        sReturn << nl << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
        sReturn << nl << "endnode";
        sReturn << nl << "node dummy " << sUse2;
        sReturn << nl << "  parent " << sMesh;
        sReturn << nl << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
        sReturn << nl << "endnode";
    }
    else if(nDataType == CONVERT_DWK){
        if(!Dwk0 && !Dwk1 && !Dwk2) return;
        if(Dwk0) if(!Dwk0->GetData()) return;
        if(Dwk1) if(!Dwk1->GetData()) return;
        if(Dwk2) if(!Dwk2->GetData()) return;

        /// DETERMINE NAMES
        std::string sClosedUse1, sClosedUse2, sOpen1Use1, sOpen1Use2, sOpen2Use1, sOpen2Use2, sClosedMesh, sOpen1Mesh, sOpen2Mesh, sRoot, sModel;
        bool bClosedUse1 = true, bClosedUse2 = true, bOpen1Use1 = true, bOpen1Use2 = true, bOpen2Use1 = true, bOpen2Use2 = true;
        bool bClosedMesh = true, bOpen1Mesh = true, bOpen2Mesh = true, bRoot = true;
        if(FH){
            FileHeader & Data = *FH;

            sModel = Data.MH.GH.sName.c_str();
            for(int n = 0; n < Data.MH.Names.size(); n++){
                std::string & sName = Data.MH.Names.at(n).sName;
                if(sName.find("DWK_dp_closed_01")!=std::string::npos) sClosedUse1 = sName;
                else if(sName.find("DWK_dp_closed_02")!=std::string::npos) sClosedUse2 = sName;
                else if(sName.find("DWK_dp_open1_01")!=std::string::npos) sOpen1Use1 = sName;
                else if(sName.find("DWK_dp_open1_02")!=std::string::npos) sOpen1Use2 = sName;
                else if(sName.find("DWK_dp_open2_01")!=std::string::npos) sOpen2Use1 = sName;
                else if(sName.find("DWK_dp_open2_02")!=std::string::npos) sOpen2Use2 = sName;
                else if(sName.find("DWK_wg_closed")!=std::string::npos) sClosedMesh = sName;
                else if(sName.find("DWK_wg_open1")!=std::string::npos) sOpen1Mesh = sName;
                else if(sName.find("DWK_wg_open2")!=std::string::npos) sOpen2Mesh = sName;
                else if(safesubstr(sName, strlen(sName.c_str()) - 4, 4) == "_DWK") sRoot = sName;
            }
            if(sClosedUse1.empty()) bClosedUse1 = false;
            if(sClosedUse2.empty()) bClosedUse2 = false;
            if(sOpen1Use1.empty()) bOpen1Use1 = false;
            if(sOpen1Use2.empty()) bOpen1Use2 = false;
            if(sOpen2Use1.empty()) bOpen2Use1 = false;
            if(sOpen2Use2.empty()) bOpen2Use2 = false;
            if(sClosedMesh.empty()) bClosedMesh = false;
            if(sOpen1Mesh.empty()) bOpen1Mesh = false;
            if(sOpen2Mesh.empty()) bOpen2Mesh = false;
            if(sRoot.empty()) bRoot = false;
            if(sRoot.empty()) sRoot = std::string(Data.MH.GH.sName.c_str()) + "_DWK";
        }
        if(sClosedUse1.empty()) sClosedUse1 = "md_DWK_dp_closed_01";
        if(sClosedUse2.empty()) sClosedUse2 = "md_DWK_dp_closed_02";
        if(sOpen1Use1.empty()) sOpen1Use1 = "md_DWK_dp_open1_01";
        if(sOpen1Use2.empty()) sOpen1Use2 = "md_DWK_dp_open1_02";
        if(sOpen2Use1.empty()) sOpen2Use1 = "md_DWK_dp_open2_01";
        if(sOpen2Use2.empty()) sOpen2Use2 = "md_DWK_dp_open2_02";
        if(sClosedMesh.empty()) sClosedMesh = "md_DWK_wg_closed";
        if(sOpen1Mesh.empty()) sOpen1Mesh = "md_DWK_wg_open1";
        if(sOpen2Mesh.empty()) sOpen2Mesh = "md_DWK_wg_open2";
        if(sRoot.empty()) sRoot = "DWK";

        /// WRITE DWK
        sReturn << "# Exported with MDLedit " << version.Print() << " ";
        if(src == AsciiSource) sReturn << "from ascii source ";
        else if(src == BinarySource) sReturn << "from binary source ";
        else std::cout << "Error: Source neither ascii nor binary!\n";
        sReturn << "at " << sTimestamp.str();
        sReturn << nl << "# DWKMESH  ASCII";
        sReturn << nl << "node dummy " << sRoot;
        sReturn << nl << "  parent " << sModel;
        sReturn << nl << "endnode";
        if(Dwk0){
            BWMHeader & data = *Dwk0->GetData();
            sReturn << nl << "node trimesh " << sClosedMesh;
            sReturn << nl << "  parent " << sRoot;
            sReturn << nl << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << nl << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << nl << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << nl << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << nl << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << nl << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << nl << "endnode";
            sReturn << nl << "node dummy " << sClosedUse1;
            sReturn << nl << "  parent " << sClosedMesh;
            sReturn << nl << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << nl << "endnode";
            sReturn << nl << "node dummy " << sClosedUse2;
            sReturn << nl << "  parent " << sClosedMesh;
            sReturn << nl << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
            sReturn << nl << "endnode";
        }
        if(Dwk1){
            BWMHeader & data = *Dwk1->GetData();
            sReturn << nl << "node trimesh " << sOpen1Mesh;
            sReturn << nl << "  parent " << sRoot;
            sReturn << nl << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << nl << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << nl << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << nl << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << nl << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << nl << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << nl << "endnode";
            sReturn << nl << "node dummy " << sOpen1Use1;
            sReturn << nl << "  parent " << sOpen1Mesh;
            sReturn << nl << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << nl << "endnode";
            if(bOpen1Use2){
                sReturn << nl << "node dummy " << sOpen1Use2;
                sReturn << nl << "  parent " << sOpen1Mesh;
                sReturn << nl << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
                sReturn << nl << "endnode";
            }
        }
        if(Dwk2){
            BWMHeader & data = *Dwk2->GetData();
            sReturn << nl << "node trimesh " << sOpen2Mesh;
            sReturn << nl << "  parent " << sRoot;
            sReturn << nl << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << nl << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << nl << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << nl << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << nl << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << nl << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << nl << "endnode";
            sReturn << nl << "node dummy " << sOpen2Use1;
            sReturn << nl << "  parent " << sOpen2Mesh;
            sReturn << nl << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << nl << "endnode";
            if(bOpen2Use2){
                sReturn << nl << "node dummy " << sOpen2Use2;
                sReturn << nl << "  parent " << sOpen2Mesh;
                sReturn << nl << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
                sReturn << nl << "endnode";
            }
        }
    }
}
