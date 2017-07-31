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
    ConvertToAscii(CONVERT_MODEL, ss, (void*) &(FH->MH));
    sExport = ss.str();
}
void MDL::ExportPwkAscii(std::string &sExport){
    if(!Pwk) return;
    if(!Pwk->GetData()) return;
    std::stringstream ss;
    ConvertToAscii(CONVERT_PWK, ss, (void*) Pwk.get());
    sExport = ss.str();
}
void MDL::ExportDwkAscii(std::string &sExport){
    std::stringstream ss;
    ConvertToAscii(CONVERT_DWK, ss, (void*) this);
    sExport = ss.str();
}
void MDL::ExportWokAscii(std::string &sExport){
    if(!Wok) return;
    if(!Wok->GetData()) return;
    std::stringstream ss;
    ConvertToAscii(CONVERT_WOK, ss, (void*) Wok.get());
    sExport = ss.str();
}

void RecursiveAabb(Aabb * AABB, std::stringstream &str){
    str << "\n    " << PrepareFloat(AABB->vBBmin.fX) <<
                " " << PrepareFloat(AABB->vBBmin.fY) <<
                " " << PrepareFloat(AABB->vBBmin.fZ) <<
                " " << PrepareFloat(AABB->vBBmax.fX) <<
                " " << PrepareFloat(AABB->vBBmax.fY) <<
                " " << PrepareFloat(AABB->vBBmax.fZ) <<
                " " << AABB->nID;
    if(AABB->nChild1 > 0){
        RecursiveAabb(&AABB->Child1[0], str);
    }
    if(AABB->nChild2 > 0){
        RecursiveAabb(&AABB->Child2[0], str);
    }
}

void MDL::ConvertToAscii(int nDataType, std::stringstream & sReturn, void * Data){
    //std::cout << "Converting to ascii, data type: " << nDataType << "\n";
    std::stringstream sTimestamp;
    SYSTEMTIME st;
    GetLocalTime(&st);
    sTimestamp << "at " << (st.wDay<10? "0" : "") << st.wDay << "/" << (st.wMonth<10? "0" : "") << st.wMonth << "/" << st.wYear;
    sTimestamp << " " << (st.wHour<10? "0" : "") << st.wHour << ":" << (st.wMinute<10? "0" : "") << st.wMinute << ":" << (st.wSecond<10? "0" : "") << st.wSecond;
    sTimestamp << " (Local Time)";

    if(nDataType == 0) return;
    else if(nDataType == CONVERT_MODEL){
        ModelHeader * mh = (ModelHeader*) Data;
        sReturn << "# MDLedit from KOTOR binary source " << sTimestamp.str();
        sReturn << "\n# MODEL ASCII";
        sReturn << "\nnewmodel " << mh->GH.sName.c_str();
        sReturn << "\nsetsupermodel " << mh->GH.sName.c_str() << " " << mh->cSupermodelName.c_str();
        sReturn << "\nclassification " << ReturnClassificationName(mh->nClassification).c_str();
        sReturn << "\nclassification_unk1 " << (int) mh->nSubclassification;
        sReturn << "\nignorefog " << (mh->nAffectedByFog ? 0 : 1);
        sReturn << "\nsetanimationscale " << PrepareFloat(mh->fScale);
        sReturn << "\n";
        sReturn << "\n# GEOM ASCII";
        sReturn << "\nbeginmodelgeom " << mh->GH.sName.c_str();
        if(Wok)
            sReturn << "\n  layoutposition " << PrepareFloat(mh->vLytPosition.fX) << " " << PrepareFloat(mh->vLytPosition.fY) << " " << PrepareFloat(mh->vLytPosition.fZ);
        sReturn     << "\n  bmin " << PrepareFloat(mh->vBBmin.fX) << " " << PrepareFloat(mh->vBBmin.fY) << " " << PrepareFloat(mh->vBBmin.fZ);
        sReturn     << "\n  bmax " << PrepareFloat(mh->vBBmax.fX) << " " << PrepareFloat(mh->vBBmax.fY) << " " << PrepareFloat(mh->vBBmax.fZ);
        sReturn     << "\n  radius " << PrepareFloat(mh->fRadius);
        for(int n = 0; n < mh->ArrayOfNodes.size(); n++){
            ConvertToAscii(CONVERT_NODE, sReturn, (void*) &n);
        }
        sReturn << "\nendmodelgeom " << mh->GH.sName.c_str() << "\n";

        if(bWriteAnimations){
            for(int n = 0; n < mh->Animations.size(); n++){
                ConvertToAscii(CONVERT_ANIMATION, sReturn, (void*) &mh->Animations.at(n));
            }
        }

        sReturn << "\n\ndonemodel " << mh->GH.sName.c_str() << "\n";
    }
    else if(nDataType == CONVERT_ANIMATION){
        Animation * anim = (Animation*) Data;
        sReturn << "\nnewanim " << anim->sName.c_str() << " " << FH->MH.GH.sName.c_str();
        sReturn << "\n  length " << PrepareFloat(anim->fLength);
        sReturn << "\n  transtime " << PrepareFloat(anim->fTransition);
        sReturn << "\n  animroot " << anim->sAnimRoot.c_str();
        if(anim->Events.size() > 0){
            for(int s = 0; s < anim->Events.size(); s++){
                sReturn << "\n  event " << anim->Events.at(s).fTime << " " << anim->Events.at(s).sName.c_str();
            }
        }
        for(int n = 0; n < anim->ArrayOfNodes.size(); n++){
            Node & node = anim->ArrayOfNodes.at(n);
            ConvertToAscii(CONVERT_ANIMATION_NODE, sReturn, (void*) &node);
        }
        sReturn << "\ndoneanim " << anim->sName.c_str() << " " << FH->MH.GH.sName.c_str();
    }
    else if(nDataType == CONVERT_ANIMATION_NODE){
        Node * node = (Node*) Data;
        sReturn << "\n    node dummy ";
        sReturn << MakeUniqueName(node->Head.nNodeNumber);
        //std::cout << "Animation node: " << MakeUniqueName(node->Head.nNodeNumber) << "\n";
        sReturn << "\n      parent " << (node->Head.nParentIndex != -1 ? MakeUniqueName(node->Head.nParentIndex) : "NULL");
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_KEYED, sReturn, (void*) &(node->Head.Controllers.at(n)));
            }
        }
        sReturn << "\n    endnode";
    }
    else if(nDataType == CONVERT_NODE){
        if(!FH) return;
        ModelHeader * mh = &FH->MH;
        int n = *((int*) Data);
        Node & node = mh->ArrayOfNodes.at(n);

        if(node.Head.nType & NODE_HEADER){
            ConvertToAscii(CONVERT_HEADER, sReturn, (void*) &node);
        }
        else{
            //std::cout << "Writing ASCII WARNING: Headerless (ghost?) node! Offset: " << node.nOffset << "\n";
            sReturn << "\nname " << mh->Names.at(n).sName;
        }
        if(node.Head.nType & NODE_AABB){
            ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
            ConvertToAscii(CONVERT_AABB, sReturn, (void*) &node);
        }
        else if(node.Head.nType & NODE_SABER){
            ConvertToAscii(CONVERT_SABER, sReturn, (void*) &node);
        }
        else if(node.Head.nType & NODE_DANGLY){
            ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
            ConvertToAscii(CONVERT_DANGLY, sReturn, (void*) &node);
        }
        else if(node.Head.nType & NODE_SKIN && !bSkinToTrimesh){
            ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
            ConvertToAscii(CONVERT_SKIN, sReturn, (void*) &node);
        }
        else if(node.Head.nType & NODE_MESH){
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
        if(node.Head.nType != 0) sReturn << "\nendnode";
    }
    else if(nDataType == CONVERT_HEADER){
        Node * node = (Node*) Data;
        sReturn << "\nnode ";
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
        sReturn << MakeUniqueName(node->Head.nNodeNumber);
        sReturn << "\n  parent " << (node->Head.nParentIndex != -1 ? MakeUniqueName(node->Head.nParentIndex) : "NULL");
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sReturn, (void*) &(node->Head.Controllers.at(n)));
            }
        }
    }
    else if(nDataType == CONVERT_LIGHT){
        Node * node = (Node*) Data;
        sReturn << "\n  lightpriority " << node->Light.nLightPriority;
        sReturn << "\n  ndynamictype " << node->Light.nDynamicType;
        sReturn << "\n  ambientonly " << node->Light.nAmbientOnly;
        sReturn << "\n  affectdynamic " << node->Light.nAffectDynamic;
        sReturn << "\n  shadow " << node->Light.nShadow;
        sReturn << "\n  flare " << node->Light.nFlare;
        sReturn << "\n  fadinglight " << node->Light.nFadingLight;
        sReturn << "\n  flareradius " << PrepareFloat(node->Light.fFlareRadius);
        sReturn << "\n  texturenames " << node->Light.FlareTextureNames.size();
        for(int n = 0; n < node->Light.FlareTextureNames.size(); n++){
            sReturn << "\n    " << node->Light.FlareTextureNames.at(n).sName;
        }
        sReturn << "\n  flaresizes " << node->Light.FlareSizes.size();
        for(int n = 0; n < node->Light.FlareSizes.size(); n++){
            sReturn << "\n    " << node->Light.FlareSizes.at(n);
        }
        sReturn << "\n  flarepositions " << node->Light.FlarePositions.size();
        for(int n = 0; n < node->Light.FlarePositions.size(); n++){
            sReturn << "\n    " << node->Light.FlarePositions.at(n);
        }
        sReturn << "\n  flarecolorshifts " << node->Light.FlareColorShifts.size();
        for(int n = 0; n < node->Light.FlareColorShifts.size(); n++){
            sReturn << "\n    " << node->Light.FlareColorShifts.at(n).fR << " " << node->Light.FlareColorShifts.at(n).fG << " " << node->Light.FlareColorShifts.at(n).fB;
        }
    }
    else if(nDataType == CONVERT_EMITTER){
        Node * node = (Node*) Data;
        sReturn << "\n  deadspace " << PrepareFloat(node->Emitter.fDeadSpace);
        sReturn << "\n  blastRadius " << PrepareFloat(node->Emitter.fBlastRadius);
        sReturn << "\n  blastLength " << PrepareFloat(node->Emitter.fBlastLength);
        sReturn << "\n  numBranches " << node->Emitter.nBranchCount;
        sReturn << "\n  controlptsmoothing " << node->Emitter.fControlPointSmoothing;
        sReturn << "\n  xgrid " << node->Emitter.nxGrid;
        sReturn << "\n  ygrid " << node->Emitter.nyGrid;
        sReturn << "\n  spawntype " << node->Emitter.nSpawnType;
        sReturn << "\n  update " << node->Emitter.cUpdate.c_str();
        sReturn << "\n  render " << node->Emitter.cRender.c_str();
        sReturn << "\n  blend " << node->Emitter.cBlend.c_str();
        sReturn << "\n  texture " << node->Emitter.cTexture.c_str();
        sReturn << "\n  chunkname " << node->Emitter.cChunkName.c_str();
        sReturn << "\n  twosidedtex " << node->Emitter.nTwosidedTex;
        sReturn << "\n  loop " << node->Emitter.nLoop;
        sReturn << "\n  renderorder " << node->Emitter.nRenderOrder;
        sReturn << "\n  m_bFrameBlending " << (int) node->Emitter.nFrameBlending;
        sReturn << "\n  m_sDepthTextureName " << node->Emitter.cDepthTextureName.c_str();

        sReturn << "\n  p2p " << (node->Emitter.nFlags & EMITTER_FLAG_P2P ? 1 : 0);
        sReturn << "\n  p2p_sel " << (node->Emitter.nFlags & EMITTER_FLAG_P2P_SEL ? 1 : 0);
        sReturn << "\n  affectedByWind " << (node->Emitter.nFlags & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0);
        sReturn << "\n  m_isTinted " << (node->Emitter.nFlags & EMITTER_FLAG_TINTED ? 1 : 0);
        sReturn << "\n  bounce " << (node->Emitter.nFlags & EMITTER_FLAG_BOUNCE ? 1 : 0);
        sReturn << "\n  random " << (node->Emitter.nFlags & EMITTER_FLAG_RANDOM ? 1 : 0);
        sReturn << "\n  inherit " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT ? 1 : 0);
        sReturn << "\n  inheritvel " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_VEL ? 1 : 0);
        sReturn << "\n  inherit_local " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0);
        sReturn << "\n  splat " << (node->Emitter.nFlags & EMITTER_FLAG_SPLAT ? 1 : 0);
        sReturn << "\n  inherit_part " << (node->Emitter.nFlags & EMITTER_FLAG_INHERIT_PART ? 1 : 0);
        sReturn << "\n  depth_texture " << (node->Emitter.nFlags & EMITTER_FLAG_DEPTH_TEXTURE ? 1 : 0);
        sReturn << "\n  emitterflag13 " << (node->Emitter.nFlags & EMITTER_FLAG_13 ? 1 : 0);
    }
    else if(nDataType == CONVERT_REFERENCE){
        Node * node = (Node*) Data;
        sReturn << "\n  refModel " << node->Reference.sRefModel.c_str();
        sReturn << "\n  reattachable " << node->Reference.nReattachable;
    }
    else if(nDataType == CONVERT_MESH){
        Node * node = (Node*) Data;
        sReturn << "\n  diffuse " << PrepareFloat(node->Mesh.fDiffuse.fR) << " " << PrepareFloat(node->Mesh.fDiffuse.fG) << " " << PrepareFloat(node->Mesh.fDiffuse.fB);
        sReturn << "\n  ambient " << PrepareFloat(node->Mesh.fAmbient.fR) << " " << PrepareFloat(node->Mesh.fAmbient.fG) << " " << PrepareFloat(node->Mesh.fAmbient.fB);
        sReturn << "\n  rotatetexture " << (int) node->Mesh.nRotateTexture;
        sReturn << "\n  shadow " << (int) node->Mesh.nShadow;
        sReturn << "\n  render " << (int) node->Mesh.nRender;
        sReturn << "\n  beaming " << (int) node->Mesh.nBeaming;
        sReturn << "\n  lightmapped " << (int) node->Mesh.nHasLightmap;
        sReturn << "\n  m_blsBackgroundGeometry " << (int) node->Mesh.nBackgroundGeometry;
        sReturn << "\n  dirt_enabled " << (int) node->Mesh.nDirtEnabled;
        sReturn << "\n  dirt_texture " << node->Mesh.nDirtTexture;
        sReturn << "\n  dirt_worldspace " << node->Mesh.nDirtCoordSpace;
        sReturn << "\n  hologram_donotdraw " << (int) node->Mesh.nHideInHolograms;
        sReturn << "\n  transparencyhint " << node->Mesh.nTransparencyHint;
        sReturn << "\n  animateuv " << node->Mesh.nAnimateUV;
        if(node->Mesh.nAnimateUV == 0){
            sReturn << "\n  uvdirectionx 0.0";
            sReturn << "\n  uvdirectiony 0.0";
            sReturn << "\n  uvjitter 0.0";
            sReturn << "\n  uvjitterspeed 0.0";
        }
        else{
            sReturn << "\n  uvdirectionx " << PrepareFloat(node->Mesh.fUVDirectionX);
            sReturn << "\n  uvdirectiony " << PrepareFloat(node->Mesh.fUVDirectionY);
            sReturn << "\n  uvjitter " << PrepareFloat(node->Mesh.fUVJitter);
            sReturn << "\n  uvjitterspeed " << PrepareFloat(node->Mesh.fUVJitterSpeed);
        }
        sReturn << "\n  tangentspace " << (node->Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1 ? 1 : 0);
        sReturn << "\n" << "  inv_count " << node->Mesh.nMeshInvertedCounter;
        if(node->Mesh.cTexture1.c_str() != std::string()) sReturn << "\n  bitmap " << node->Mesh.GetTexture(1);
        if(node->Mesh.cTexture2.c_str() != std::string()) sReturn << "\n  bitmap2 " << node->Mesh.GetTexture(2);
        if(node->Mesh.cTexture3.c_str() != std::string()) sReturn << "\n  texture0 " << node->Mesh.GetTexture(3);
        if(node->Mesh.cTexture4.c_str() != std::string()) sReturn << "\n  texture1 " << node->Mesh.GetTexture(4);
        sReturn << "\n  verts " << node->Mesh.Vertices.size();
        for(int n = 0; n < node->Mesh.Vertices.size(); n++){
            //Two possibilities - I put MDX if MDX is present, otherwise MDL
            if(!Mdx) sReturn << "\n    " << PrepareFloat(node->Mesh.Vertices.at(n).fX) << " " << PrepareFloat(node->Mesh.Vertices.at(n).fY) << " " << PrepareFloat(node->Mesh.Vertices.at(n).fZ);
            else sReturn << "\n    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vVertex.fX) << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vVertex.fY) << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vVertex.fZ);
        }
        sReturn << "\n  faces " << node->Mesh.Faces.size();
        for(int n = 0; n < node->Mesh.Faces.size(); n++){
            sReturn << "\n    ";
            sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
            sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
            sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
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
            sReturn << "\n  tverts " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV1.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV1.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
            sReturn << "\n  texindices1 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << "\n    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << "\n  tverts1 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV2.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV2.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
            sReturn << "\n  texindices2 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << "\n    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << "\n  tverts2 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV3.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV3.fY);
            }
        }
        if(Mdx && !Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
            sReturn << "\n  texindices3 " << node->Mesh.Faces.size();
            for(int n = 0; n < node->Mesh.Faces.size(); n++){
                sReturn << "\n    ";
                sReturn << node->Mesh.Faces.at(n).nIndexVertex.at(0);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(1);
                sReturn << " " << node->Mesh.Faces.at(n).nIndexVertex.at(2);
            }
            sReturn << "\n  tverts3 " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n    " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV4.fX);
                sReturn << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.vUV4.fY);
            }
        }
    }
    else if(nDataType == CONVERT_SKIN){
        Node * node = (Node*) Data;
        if(!Mdx->sBuffer.empty()){
            sReturn << "\n  weights " << node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   ";
                int i = 0;
                signed short nBoneNumber; // = (int) round(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightIndex.at(i));
                //std::cout << "Bone name index array size: " << node->Skin.BoneNameIndices.size() << "\n";
                bool bDependentVert = false;
                while(i < 4){
                    nBoneNumber = node->Mesh.Vertices.at(n).MDXData.Weights.nWeightIndex.at(i);
                    //std::cout << "Reading bone number " << nBoneNumber << "\n";
                    if(nBoneNumber > -1 && nBoneNumber < node->Skin.BoneNameIndices.size()){
                        int nNodeNumber = node->Skin.BoneNameIndices.at(nBoneNumber);
                        //std::cout << "Reading bone number " << nBoneNumber;
                        //std::cout << ", representing bone " << FH->MH.Names.at(node->Skin.BoneNameIndices.at(nBoneNumber)).sName.c_str() << ".\n";
                        //sReturn << " " << FH->MH.Names.at(nNodeNumber).sName.c_str() << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightValue.at(i));
                        if(nNodeNumber > -1 && nNodeNumber < FH->MH.ArrayOfNodes.size()){
                            if(!bDependentVert) bDependentVert = true;
                            sReturn << " " << MakeUniqueName(nNodeNumber) << " " << PrepareFloat(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightValue.at(i));
                        }
                    }
                    i++;
                    //if(i < 4) nBoneNumber = (int) round(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightIndex.at(i));
                }
                if(!bDependentVert){
                    sReturn << " root 1.0";
                }
            }
        }
    }
    else if(nDataType == CONVERT_DANGLY){
        Node * node = (Node*) Data;
        sReturn << "\n  displacement " << PrepareFloat(node->Dangly.fDisplacement);
        sReturn << "\n  tightness " << PrepareFloat(node->Dangly.fTightness);
        sReturn << "\n  period " << PrepareFloat(node->Dangly.fPeriod);
        sReturn << "\n  constraints " << node->Dangly.Constraints.size();
        for(int n = 0; n < node->Dangly.Constraints.size(); n++){
            sReturn << "\n    " << PrepareFloat(node->Dangly.Constraints.at(n));
        }
    }
    else if(nDataType == CONVERT_AABB){
        Node * node = (Node*) Data;

        /// Write out aabb tree
        sReturn << "\n  aabb";
        RecursiveAabb(&node->Walkmesh.RootAabb, sReturn);

        /// Write out room links
        if(Wok){
            if(Wok->GetData()){
                BWMHeader & data = *Wok->GetData();
                sReturn << "\n  roomlinks";
                Vector vLyt;
                if(FH) vLyt = FH->MH.vLytPosition;
                for(int l = 0; l < data.edges.size(); l++){
                    if(data.edges.at(l).nTransition != -1){
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
                            if(v1.Compare(v4, 0.01) && v2.Compare(v5, 0.01) && v3.Compare(v6, 0.01)){
                                mdlface.nEdgeTransitions.at(nIndex%3) = data.edges.at(l).nTransition;
                                sReturn << "\n    " << (3*f + nIndex%3) << " " << data.edges.at(l).nTransition;
                                f = node->Mesh.Faces.size();
                            }
                        }
                    }
                }
                sReturn << "\n  endlist";
            }
        }
    }
    else if(nDataType == CONVERT_SABER){
        Node * node = (Node*) Data;
        sReturn << "\n" << "  inv_count " << node->Saber.nInvCount1 << " " << node->Saber.nInvCount2;
        if(node->Mesh.cTexture1.c_str() != std::string()) sReturn << "\n" << "  bitmap " << node->Mesh.GetTexture(1);

        if(node->Saber.SaberData.size() == 176){
            Vector vDiff;
            Vector vOut;
            vDiff = node->Saber.SaberData.at(4).vVertex - node->Saber.SaberData.at(0).vVertex;

            sReturn << "\n  verts 16";
            vOut = node->Saber.SaberData.at(0).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(1).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(2).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(3).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(0).vVertex + vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(1).vVertex + vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(2).vVertex + vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(3).vVertex + vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(88).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(89).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(90).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(91).vVertex;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            vOut = node->Saber.SaberData.at(88).vVertex - vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(89).vVertex - vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(90).vVertex - vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);
            vOut = node->Saber.SaberData.at(91).vVertex - vDiff;
            sReturn << "\n    " << PrepareFloat(vOut.fX) << " " << PrepareFloat(vOut.fY) << " " << PrepareFloat(vOut.fZ);

            sReturn << "\n  faces 12";
            /// SIDE 1
            sReturn << "\n    5 4 0  1  5 4 0  0";
            sReturn << "\n    0 1 5  1  0 1 5  0";
            sReturn << "\n    13 8 12  1  13 8 12  0";
            sReturn << "\n    8 13 9  1  8 13 9  0";
            sReturn << "\n    6 5 1  1  6 5 1  0";
            //sReturn << "\n    2 6 5  1  2 6 5  0"; //Correct faces, but not what they are in the game
            sReturn << "\n    1 2 6  1  1 2 6  0";
            //sReturn << "\n    1 2 5  1  1 2 5  0"; //Correct faces, but not what they are in the game
            sReturn << "\n    10 9 13  1  10 9 13  0";
            sReturn << "\n    13 14 10  1  13 14 10  0";
            sReturn << "\n    3 6 2  1  3 6 2  0";
            sReturn << "\n    6 3 7  1  6 3 7  0";
            sReturn << "\n    15 11 14  1  15 11 14  0";
            sReturn << "\n    10 14 11  1  10 14 11  0";
            /// SIDE 2
            /*
            sReturn << "\n    4 5 0  1  4 5 0  0";
            sReturn << "\n    1 0 5  1  1 0 5  0";
            sReturn << "\n    8 13 12  1  8 13 12  0";
            sReturn << "\n    13 8 9  1  13 8 9  0";
            sReturn << "\n    5 6 1  1  5 6 1  0";
            //sReturn << "\n    6 2 5  1  6 2 5  0"; //Correct faces, but not what they are in the game
            sReturn << "\n    2 1 6  1  2 1 6  0";
            //sReturn << "\n    2 1 5  1  2 1 5  0"; //Correct faces, but not what they are in the game
            sReturn << "\n    9 10 13  1  9 10 13  0";
            sReturn << "\n    14 13 10  1  14 13 10  0";
            sReturn << "\n    6 3 2  1  6 3 2  0";
            sReturn << "\n    3 6 7  1  3 6 7  0";
            sReturn << "\n    11 15 14  1  11 15 14  0";
            sReturn << "\n    14 10 11  1  14 10 11  0";
            */

            sReturn << "\n  tverts 16";
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(0).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(0).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(1).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(1).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(2).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(2).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(3).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(3).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(4).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(4).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(5).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(5).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(6).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(6).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(7).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(7).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(88).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(88).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(89).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(89).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(90).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(90).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(91).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(91).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(92).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(92).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(93).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(93).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(94).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(94).vUV1.fY);
            sReturn << "\n    " << PrepareFloat(node->Saber.SaberData.at(95).vUV1.fX) << " " << PrepareFloat(node->Saber.SaberData.at(95).vUV1.fY);
        }
    }
    /// TODO: cases where num(controllers) == 0 but num(controller data) > 0
    else if(nDataType == CONVERT_CONTROLLER_KEYED){
        Controller * ctrl = (Controller*) Data;
        ModelHeader & data = FH->MH;
        Node & geonode = GetNodeByNameIndex(ctrl->nNodeNumber);
        Location loc = geonode.GetLocation();
        Node * tempNode = nullptr;
        if(ctrl->nAnimation >= 0){
            Animation & anim = data.Animations.at(ctrl->nAnimation);
            for(int an = 0; an < anim.ArrayOfNodes.size() && tempNode == nullptr; an++){
                Node & animNode = anim.ArrayOfNodes.at(an);
                if(ctrl->nNodeNumber == animNode.Head.nNodeNumber){
                    for(int ac = 0; ac < animNode.Head.Controllers.size() && tempNode == nullptr; ac++){
                        if(&animNode.Head.Controllers.at(ac) == ctrl) tempNode = &animNode;
                    }
                }
            }
        }
        Node & node = *tempNode;
        //std::cout << "Node does not crash\n";

        sReturn << "\n      " << ReturnControllerName(ctrl->nControllerType, geonode.Head.nType);
        if(ctrl->nColumnCount > 16 && !bBezierToLinear) sReturn << "bezier";
        sReturn << "key";
        double PI = 3.14159;
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            Quaternion qCurrent, qPrevious;
            AxisAngle aaCurrent, aaDiff;
            //std::cout << "Printing compressed orientation keys. Timekey: " << ctrl->nTimekeyStart << ", Datakey: " << ctrl->nDataStart << ", Values: " << ctrl->nValueCount << "\n";
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                ByteBlock4.f = node.Head.ControllerData.at(ctrl->nDataStart + n);
                qCurrent = DecompressQuaternion(ByteBlock4.ui);
                aaCurrent = AxisAngle(qCurrent);
                /*
                if(n > 0){
                    aaDiff = AxisAngle(qCurrent * qPrevious.inverse());
                    //std::cout << "Theta is " << aaDiff.fAngle << ".\n";
                    if(abs(aaDiff.fAngle) - PI > 0.0001){
                        std::cout << "Correcting orientation on node '" << FH->MH.Names.at(ctrl->nNodeNumber).sName.c_str() << "' in animation '" << FH->MH.Animations.at(ctrl->nAnimation).sName.c_str() << "'.\n";
                        //std::cout << "Changing " << aaCurrent.Print() << "... ";
                        if(abs(aaCurrent.fAngle) == 0.0) aaCurrent.fAngle = 2.0 * PI;
                        else aaCurrent.fAngle = (aaCurrent.fAngle / abs(aaCurrent.fAngle)) * -2.0 * PI + aaCurrent.fAngle;
                        //std::cout << "to " << aaCurrent.Print() << ".\n";
                    }
                }
                /*
                if(n > 0){
                    aaDiff = AxisAngle(qCurrent * qPrevious.inverse());
                    //std::cout << "Theta is " << aaDiff.fAngle << ".\n";
                    int znj = 0;
                    while(abs(aaDiff.fAngle) > PI && znj < 10){
                        //std::cout << "Changing " << aaCurrent.Print() << "... ";
                        aaCurrent.fAngle = pow(-1.0, (double) znj) * (double) (znj+1) * 2.0 * PI + aaCurrent.fAngle;
                        znj++;
                        aaDiff = AxisAngle(Quaternion(aaCurrent) * qPrevious.inverse());
                        //std::cout << "to " << aaCurrent.Print() << ".\n";
                    }
                }
                qCurrent = Quaternion(aaCurrent);
                qPrevious = qCurrent;
                */

                //sReturn << PrepareFloat(qCurrent.vAxis.fX) << " " << PrepareFloat(qCurrent.vAxis.fY) << " " << PrepareFloat(qCurrent.vAxis.fZ) << " " << PrepareFloat(qCurrent.fW);
                sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
            }
        }
        else if(ctrl->nColumnCount == 4 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Uncompressed orientation
            //std::cout << "Printing uncompressed orientation keys. Timekey: " << ctrl->nTimekeyStart << ", Datakey: " << ctrl->nDataStart << ", Values: " << ctrl->nValueCount << "\n";
            Quaternion qCurrent, qPrevious;
            AxisAngle aaCurrent, aaDiff;
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                qCurrent = Quaternion(node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 0),
                                      node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 1),
                                      node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 2),
                                      node.Head.ControllerData.at(ctrl->nDataStart + n*4 + 3));
                aaCurrent = AxisAngle(qCurrent);
                /*
                if(n > 0){
                    aaDiff = AxisAngle(qCurrent * qPrevious.inverse());
                    //std::cout << "Theta is " << aaDiff.fAngle << ".\n";
                    if(abs(aaDiff.fAngle) - PI > 0.0001){
                        //std::cout << "Changing " << aaCurrent.Print() << "... ";
                        if(abs(aaCurrent.fAngle) == 0.0) aaCurrent.fAngle = 2.0 * PI;
                        else aaCurrent.fAngle = (aaCurrent.fAngle / abs(aaCurrent.fAngle)) * -2.0 * PI + aaCurrent.fAngle;
                        //std::cout << "to " << aaCurrent.Print() << ".\n";
                    }
                }
                /*
                if(n > 0){
                    aaDiff = AxisAngle(qCurrent * qPrevious.inverse());
                    //std::cout << "Theta is " << aaDiff.fAngle << ".\n";
                    int znj = 0;
                    while(abs(aaDiff.fAngle) > PI && znj < 10){
                        //std::cout << "Changing " << aaCurrent.Print() << "... ";
                        aaCurrent.fAngle = pow(-1.0, (double) znj) * (double) (znj+1) * 2.0 * PI + aaCurrent.fAngle;
                        znj++;
                        aaDiff = AxisAngle(Quaternion(aaCurrent) * qPrevious.inverse());
                        //std::cout << "to " << aaCurrent.Print() << ".\n";
                    }
                }
                qCurrent = Quaternion(aaCurrent);
                qPrevious = qCurrent;
                */

                //sReturn << PrepareFloat(qCurrent.vAxis.fX) << " " << PrepareFloat(qCurrent.vAxis.fY) << " " << PrepareFloat(qCurrent.vAxis.fZ) << " " << PrepareFloat(qCurrent.fW);
                sReturn << PrepareFloat(aaCurrent.vAxis.fX) << " " << PrepareFloat(aaCurrent.vAxis.fY) << " " << PrepareFloat(aaCurrent.vAxis.fZ) << " " << PrepareFloat(aaCurrent.fAngle);
                //std::cout << "Done printing key " << n << "\n";
            }
        }
        /// positionbezierkey
        else if(ctrl->nColumnCount & 16 && !bBezierToLinear && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //positionbezierkey
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n));
                sReturn << " " << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (0)));
                sReturn << " " << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (1)));
                sReturn << " " << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (2)));
                sReturn << "  " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (3)));
                sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (4)));
                sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (5)));
                sReturn << "  " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (6)));
                sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (7)));
                sReturn << " " << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*9 + (8)));
            }
        }
        /// regular bezierkey
        else if(ctrl->nColumnCount & 16 && !bBezierToLinear){
            //bezierkey
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                for(int i = 0; i < (ctrl->nColumnCount & 15) * 3; i++){
                    sReturn << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*((ctrl->nColumnCount & 15) * 3) + i));
                    if(i < (ctrl->nColumnCount & 15) * 3 - 1) sReturn << " ";
                    if(i % (ctrl->nColumnCount & 15) == 0 && i > 0) sReturn << " ";
                }
            }
        }
        /// positionkey
        else if(ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //normal position
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
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
                sReturn << "\n        " << PrepareFloat(node.Head.ControllerData.at(ctrl->nTimekeyStart + n)) << " ";
                for(int i = 0; i < ctrl->nColumnCount; i++){
                    sReturn << PrepareFloat(node.Head.ControllerData.at(ctrl->nDataStart + n*ctrl->nColumnCount + i));
                    if(i < ctrl->nColumnCount - 1) sReturn << " ";
                }
            }

        }
        else{
            std::string sLocation;
            if(ctrl->nAnimation == -1) sLocation = "geometry";
            else sLocation = FH->MH.Animations.at(ctrl->nAnimation).sName;
            std::cout << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << FH->MH.Names.at(ctrl->nNodeNumber).sName << " (" << sLocation.c_str() << ")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
        sReturn << "\n      endlist";
    }
    else if(nDataType == CONVERT_CONTROLLER_SINGLE){
        Controller * ctrl = (Controller*) Data;
        if(ctrl->nValueCount > 1){
            Error("Error! Single controller has more than one value. Skipping.");
            return;
        }
        Node & node = GetNodeByNameIndex(ctrl->nNodeNumber);
        sReturn << "\n  " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " ";
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            ByteBlock4.f = node.Head.ControllerData.at(ctrl->nDataStart);
            Orientation CtrlOrient(DecompressQuaternion(ByteBlock4.ui));
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
            if(ctrl->nAnimation == -1) sLocation = "geometry";
            else sLocation = FH->MH.Animations.at(ctrl->nAnimation).sName;
            std::cout << "Controller data error for " << ReturnControllerName(ctrl->nControllerType, node.Head.nType) << " in " << FH->MH.Names.at(ctrl->nNodeNumber).sName << " (" << sLocation.c_str() << ")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
    }
    else if(nDataType == CONVERT_WOK){
        BWM * bwm = (BWM*) Data;
        if(!bwm->GetData()) return;
        BWMHeader & data = *bwm->GetData();

        std::string sModel ("unknown");
        if(FH) sModel = FH->MH.GH.sName.c_str();

        sReturn         << "# Exported from MDLedit " << sTimestamp.str();
        sReturn << "\n" << "# WOKMESH  ASCII";
        sReturn << "\n" << "node trimesh WALKMESH";
        sReturn << "\n" << "  parent " << sModel;
        sReturn << "\n" << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
        sReturn << "\n" << "  orientation 1.0 0.0 0.0 0.0";

        sReturn << "\n  verts " << data.verts.size();
        for(int n = 0; n < data.verts.size(); n++){
            sReturn << "\n    " << PrepareFloat(data.verts.at(n).fX) << " " << PrepareFloat(data.verts.at(n).fY) << " " << PrepareFloat(data.verts.at(n).fZ);
        }

        sReturn << "\n  faces " << data.faces.size();
        for(int n = 0; n < data.faces.size(); n++){
            sReturn << "\n    ";
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
            if(data.edges.at(n).nTransition != -1){
                ssRoomlinks << "\n    " << data.edges.at(n).nIndex << " " << data.edges.at(n).nTransition;
                nRoomLinkSize++;
            }
        }
        sReturn << "\n  roomlinks " << data.edges.size() << ssRoomlinks.str();

        sReturn << "\nendnode";
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
        sReturn      << "# Exported from MDLedit " << sTimestamp.str();
        sReturn << "\n" << "# PWKMESH  ASCII";
        sReturn << "\n" << "node dummy " << sRoot;
        sReturn << "\n" << "  parent " << sModel;
        sReturn << "\n" << "endnode";
        sReturn << "\n" << "node trimesh " << sMesh;
        sReturn << "\n" << "  parent " << sRoot;
        sReturn << "\n" << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
        sReturn << "\n" << "  orientation 1.0 0.0 0.0 0.0";
        sReturn << "\n" << "  verts " << data.verts.size();
        for(int v = 0; v < data.verts.size(); v++){
            Vector & vert = data.verts.at(v);
            sReturn << "\n" << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
        }
        sReturn << "\n" << "  faces " << data.faces.size();
        for(int v = 0; v < data.faces.size(); v++){
            Face & face = data.faces.at(v);
            sReturn << "\n" << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
        }
        sReturn << "\n" << "endnode";
        sReturn << "\n" << "node dummy " << sUse1;
        sReturn << "\n" << "  parent " << sMesh;
        sReturn << "\n" << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
        sReturn << "\n" << "endnode";
        sReturn << "\n" << "node dummy " << sUse2;
        sReturn << "\n" << "  parent " << sMesh;
        sReturn << "\n" << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
        sReturn << "\n" << "endnode";
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
        sReturn      << "# Exported from MDLedit " << sTimestamp.str();
        sReturn << "\n" << "# DWKMESH  ASCII";
        sReturn << "\n" << "node dummy " << sRoot;
        sReturn << "\n" << "  parent " << sModel;
        sReturn << "\n" << "endnode";
        if(Dwk0){
            BWMHeader & data = *Dwk0->GetData();
            sReturn << "\n" << "node trimesh " << sClosedMesh;
            sReturn << "\n" << "  parent " << sRoot;
            sReturn << "\n" << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << "\n" << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << "\n" << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << "\n" << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << "\n" << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << "\n" << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << "\n" << "endnode";
            sReturn << "\n" << "node dummy " << sClosedUse1;
            sReturn << "\n" << "  parent " << sClosedMesh;
            sReturn << "\n" << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << "\n" << "endnode";
            sReturn << "\n" << "node dummy " << sClosedUse2;
            sReturn << "\n" << "  parent " << sClosedMesh;
            sReturn << "\n" << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
            sReturn << "\n" << "endnode";
        }
        if(Dwk1){
            BWMHeader & data = *Dwk1->GetData();
            sReturn << "\n" << "node trimesh " << sOpen1Mesh;
            sReturn << "\n" << "  parent " << sRoot;
            sReturn << "\n" << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << "\n" << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << "\n" << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << "\n" << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << "\n" << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << "\n" << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << "\n" << "endnode";
            sReturn << "\n" << "node dummy " << sOpen1Use1;
            sReturn << "\n" << "  parent " << sOpen1Mesh;
            sReturn << "\n" << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << "\n" << "endnode";
            if(bOpen1Use2){
                sReturn << "\n" << "node dummy " << sOpen1Use2;
                sReturn << "\n" << "  parent " << sOpen1Mesh;
                sReturn << "\n" << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
                sReturn << "\n" << "endnode";
            }
        }
        if(Dwk2){
            BWMHeader & data = *Dwk2->GetData();
            sReturn << "\n" << "node trimesh " << sOpen2Mesh;
            sReturn << "\n" << "  parent " << sRoot;
            sReturn << "\n" << "  position " << PrepareFloat(data.vPosition.fX) << " " << PrepareFloat(data.vPosition.fY) << " " << PrepareFloat(data.vPosition.fZ);
            sReturn << "\n" << "  orientation 1.0 0.0 0.0 0.0";
            sReturn << "\n" << "  verts " << data.verts.size();
            for(int v = 0; v < data.verts.size(); v++){
                Vector & vert = data.verts.at(v);
                sReturn << "\n" << "    " << PrepareFloat(vert.fX) << " " << PrepareFloat(vert.fY) << " " << PrepareFloat(vert.fZ);
            }
            sReturn << "\n" << "  faces " << data.faces.size();
            for(int v = 0; v < data.faces.size(); v++){
                Face & face = data.faces.at(v);
                sReturn << "\n" << "    " << face.nIndexVertex.at(0) << " " << face.nIndexVertex.at(1) << " " << face.nIndexVertex.at(2) << "  1  0 0 0  " << face.nMaterialID;
            }
            sReturn << "\n" << "endnode";
            sReturn << "\n" << "node dummy " << sOpen2Use1;
            sReturn << "\n" << "  parent " << sOpen2Mesh;
            sReturn << "\n" << "  position " << PrepareFloat(data.vUse1.fX) << " " << PrepareFloat(data.vUse1.fY) << " " << PrepareFloat(data.vUse1.fZ);
            sReturn << "\n" << "endnode";
            if(bOpen2Use2){
                sReturn << "\n" << "node dummy " << sOpen2Use2;
                sReturn << "\n" << "  parent " << sOpen2Mesh;
                sReturn << "\n" << "  position " << PrepareFloat(data.vUse2.fX) << " " << PrepareFloat(data.vUse2.fY) << " " << PrepareFloat(data.vUse2.fZ);
                sReturn << "\n" << "endnode";
            }
        }
    }
}
