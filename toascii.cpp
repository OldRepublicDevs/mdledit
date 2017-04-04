#include "MDL.h"

/**
    Functions:
    MDL::ExportAscii()
    RecursiveAabb() //Helper
    PrepareFloat() //Helper
    MDL::ConvertToAscii()
/**/

void MDL::ExportAscii(std::string &sExport){
    std::stringstream ss;
    ConvertToAscii(CONVERT_MODEL, ss, (void*) &FH->MH);
    sExport = ss.str();
}

void RecursiveAabb(Aabb * AABB, std::stringstream &str){
    str << string_format("\n    %f %f %f %f %f %f %i", AABB->vBBmin.fX, AABB->vBBmin.fY, AABB->vBBmin.fZ, AABB->vBBmax.fX, AABB->vBBmax.fY, AABB->vBBmax.fZ, AABB->nID);
    if(AABB->nChild1 > 0){
        RecursiveAabb(&AABB->Child1[0], str);
    }
    if(AABB->nChild2 > 0){
        RecursiveAabb(&AABB->Child2[0], str);
    }
}

char cReturn[4][255];
char * PrepareFloat(double fFloat, unsigned int n){
    sprintf(cReturn[n], "%#.7f", RoundDec(fFloat, 8));
    TruncateDec(cReturn[n]);
    return cReturn[n];
}

void MDL::ConvertToAscii(int nDataType, std::stringstream & sReturn, void * Data){
    if(nDataType == 0) return;
    else if(nDataType == CONVERT_MODEL){
        ModelHeader * mh = (ModelHeader*) Data;
        sReturn << string_format("# MDLedit from KOTOR binary source");
        sReturn << "\n# model " << mh->GH.sName.c_str();
        sReturn << "\nnewmodel " << mh->GH.sName.c_str();
        sReturn << string_format("\nsetsupermodel %s %s", mh->GH.sName.c_str(), mh->cSupermodelName.c_str());
        sReturn << "\nclassification " << ReturnClassificationName(mh->nClassification).c_str();
        sReturn << "\nsetanimationscale " << PrepareFloat(mh->fScale, 0);
        sReturn << "\n\nbeginmodelgeom " << mh->GH.sName.c_str();
        sReturn << string_format("\n  bmin %s %s %s", PrepareFloat(mh->vBBmin.fX, 0), PrepareFloat(mh->vBBmin.fY, 1), PrepareFloat(mh->vBBmin.fZ, 2));
        sReturn << string_format("\n  bmax %s %s %s", PrepareFloat(mh->vBBmax.fX, 0), PrepareFloat(mh->vBBmax.fY, 1), PrepareFloat(mh->vBBmax.fZ, 2));
        sReturn << "\n  radius " << PrepareFloat(mh->fRadius, 0);
        for(int n = 0; n < mh->ArrayOfNodes.size(); n++){
            Node & node = mh->ArrayOfNodes.at(n);
            if(node.Head.nType & NODE_HAS_HEADER){
                ConvertToAscii(CONVERT_HEADER, sReturn, (void*) &node);
            }
            else{
                std::cout<<"Writing ASCII WARNING: Headerless (ghost?) node! Offset: "<<node.nOffset<<"\n";
                sReturn << "\nnode dummy " << mh->Names.at(n).sName;
            }
            if(node.Head.nType & NODE_HAS_AABB){
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_AABB, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_SABER){
                ConvertToAscii(CONVERT_SABER, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_DANGLY){
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_DANGLY, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_SKIN){
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
                ConvertToAscii(CONVERT_SKIN, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_MESH){
                ConvertToAscii(CONVERT_MESH, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_EMITTER){
                ConvertToAscii(CONVERT_EMITTER, sReturn, (void*) &node);
            }
            else if(node.Head.nType & NODE_HAS_LIGHT){
                ConvertToAscii(CONVERT_LIGHT, sReturn, (void*) &node);
            }
            ConvertToAscii(CONVERT_ENDNODE, sReturn, (void*) &node);
        }
        sReturn << string_format("\nendmodelgeom %s\n", mh->GH.sName.c_str());

        for(int n = 0; n < mh->Animations.size(); n++){
            ConvertToAscii(CONVERT_ANIMATION, sReturn, (void*) &mh->Animations[n]);
        }
        sReturn << string_format("\ndonemodel %s\n", mh->GH.sName.c_str());
    }
    else if(nDataType == CONVERT_ANIMATION){
        Animation * anim = (Animation*) Data;
        sReturn << "\nnewanim " << anim->sName.c_str() <<" "<< FH->MH.GH.sName.c_str();
        sReturn << "\n  length " << PrepareFloat(anim->fLength, 0);
        sReturn << "\n  transtime " << PrepareFloat(anim->fTransition, 0);
        sReturn << "\n  animroot " << anim->sAnimRoot.c_str();
        if(anim->Sounds.size() > 0){
            sReturn << "\n  eventlist " << anim->sName.c_str();
            for(int s = 0; s < anim->Sounds.size(); s++){
                sReturn << "\n    " << anim->Sounds.at(s).fTime << " " << anim->Sounds.at(s).sName.c_str();
            }
            sReturn << "\n  endlist " << anim->sName.c_str();
        }
        for(int n = 0; n < anim->ArrayOfNodes.size(); n++){
            Node & node = anim->ArrayOfNodes.at(n);
            ConvertToAscii(CONVERT_ANIMATION_NODE, sReturn, (void*) &node);
            ConvertToAscii(CONVERT_ENDNODE, sReturn, (void*) &node);
        }
        sReturn << string_format("\ndoneanim %s %s", anim->sName.c_str(), FH->MH.GH.sName.c_str());
    }
    else if(nDataType == CONVERT_ANIMATION_NODE){
        Node * node = (Node*) Data;
        sReturn << "\nnode ";
        if(node->Head.nType & NODE_HAS_AABB) sReturn << "aabb ";
        else if(node->Head.nType & NODE_HAS_DANGLY) sReturn << "danglymesh ";
        else if(node->Head.nType & NODE_HAS_SKIN) sReturn << "skin ";
        else if(node->Head.nType & NODE_HAS_SABER) sReturn << "trimesh 2081__"; /// Official keyword: lightsaber
        else if(node->Head.nType & NODE_HAS_MESH) sReturn << "trimesh ";
        else if(node->Head.nType & NODE_HAS_EMITTER) sReturn << "emitter ";
        else if(node->Head.nType & NODE_HAS_LIGHT) sReturn << "light ";
        else if(node->Head.nType & NODE_HAS_HEADER) sReturn << "dummy ";
        sReturn << MakeUniqueName(node->Head.nNameIndex);//FH->MH.Names[node->Head.nNameIndex].sName.c_str();
        sReturn << "\n  parent " << (node->Head.nParentIndex != -1 ? MakeUniqueName(node->Head.nParentIndex) : "NULL"); //FH->MH.Names[node->Head.nParentIndex].sName.c_str() : "NULL");
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_KEYED, sReturn, (void*) &(node->Head.Controllers[n]));
            }
        }
    }
    else if(nDataType == CONVERT_HEADER){
        Node * node = (Node*) Data;
        sReturn << "\nnode ";
        if(node->Head.nType & NODE_HAS_AABB) sReturn << "aabb ";
        else if(node->Head.nType & NODE_HAS_DANGLY) sReturn << "danglymesh ";
        else if(node->Head.nType & NODE_HAS_SKIN) sReturn << "skin ";
        else if(node->Head.nType & NODE_HAS_SABER) sReturn << "trimesh 2081__"; /// Official keyword: lightsaber
        else if(node->Head.nType & NODE_HAS_MESH) sReturn << "trimesh ";
        else if(node->Head.nType & NODE_HAS_EMITTER) sReturn << "emitter ";
        else if(node->Head.nType & NODE_HAS_LIGHT) sReturn << "light ";
        else if(node->Head.nType & NODE_HAS_HEADER) sReturn << "dummy ";
        sReturn << MakeUniqueName(node->Head.nNameIndex);//FH->MH.Names[node->Head.nNameIndex].sName.c_str();
        sReturn << "\n  parent " << (node->Head.nParentIndex != -1 ? MakeUniqueName(node->Head.nParentIndex) : "NULL"); //FH->MH.Names[node->Head.nParentIndex].sName.c_str() : "NULL");
        //sReturn << FH->MH.Names[node->Head.nNameIndex].sName.c_str();
        //sReturn << "\n  parent " << (node->Head.nParentIndex != -1 ? FH->MH.Names[node->Head.nParentIndex].sName.c_str() : "NULL");
        if(node->Head.Controllers.size() > 0){
            /*if(node->Head.Controllers[0].nControllerType != CONTROLLER_HEADER_POSITION){
                sReturn << string_format("\n  position %s %s %s", PrepareFloat(node->Head.Pos.fX, 0), PrepareFloat(node->Head.Pos.fY, 1), PrepareFloat(node->Head.Pos.fZ, 2));
                if(node->Head.Controllers[0].nControllerType != CONTROLLER_HEADER_ORIENTATION){
                    sReturn << string_format("\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
                }
            }
            else if(node->Head.Controllers[1].nControllerType != CONTROLLER_HEADER_ORIENTATION){
                sReturn << string_format("\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
            }*/
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sReturn, (void*) &(node->Head.Controllers[n]));
            }
        }/*
        else{
            sReturn << string_format("\n  position %s %s %s", PrepareFloat(node->Head.Pos.fX, 0), PrepareFloat(node->Head.Pos.fY, 1), PrepareFloat(node->Head.Pos.fZ, 2));
            sReturn << string_format("\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
        }*/
    }
    else if(nDataType == CONVERT_ENDNODE){
        sReturn << "\nendnode";
    }
    else if(nDataType == CONVERT_LIGHT){
        Node * node = (Node*) Data;
        sReturn << "\n  lightpriority " << node->Light.nLightPriority;
        sReturn << "\n  ndynamictype " << node->Light.nDynamicType;
        sReturn << "\n  ambientonly " << node->Light.nAmbientOnly;
        sReturn << "\n  affectdynamic " << node->Light.nAffectDynamic;
        sReturn << "\n  shadow " << node->Light.nShadow;
        sReturn << "\n  flare " << node->Light.nFlare; /// !!! Make sure that NWMax isn't expecting lensflares
        sReturn << "\n  fadinglight " << node->Light.nFadingLight;
        sReturn << "\n  flareradius " << PrepareFloat(node->Light.fFlareRadius, 0); //NWmax reads this as an int
        sReturn << "\n  texturenames " << node->Light.FlareTextureNames.size();
        for(int n = 0; n < node->Light.FlareTextureNames.size(); n++){
            sReturn<<"\n    "<<node->Light.FlareTextureNames[n].sName;
        }
        sReturn << "\n  flaresizes " << node->Light.FlareSizes.size();
        for(int n = 0; n < node->Light.FlareSizes.size(); n++){
            sReturn<<"\n    "<<node->Light.FlareSizes[n];
        }
        sReturn << "\n  flarepositions " << node->Light.FlarePositions.size();
        for(int n = 0; n < node->Light.FlarePositions.size(); n++){
            sReturn<<"\n    "<<node->Light.FlarePositions[n];
        }
        sReturn << "\n  flarecolorshifts " << node->Light.FlareColorShifts.size();
        for(int n = 0; n < node->Light.FlareColorShifts.size(); n++){
            sReturn<<"\n    "<<node->Light.FlareColorShifts[n].fR<<" "<<node->Light.FlareColorShifts[n].fG<<" "<<node->Light.FlareColorShifts[n].fB;
        }
    }
    else if(nDataType == CONVERT_EMITTER){
        Node * node = (Node*) Data;
        sReturn << "\n  deadspace " << PrepareFloat(node->Emitter.fDeadSpace, 0);
        sReturn << "\n  blastRadius " << PrepareFloat(node->Emitter.fBlastRadius, 0);
        sReturn << "\n  blastLength " << PrepareFloat(node->Emitter.fBlastLength, 0);
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
        sReturn << "\n  m_bFrameBlending " << node->Emitter.nFrameBlending;
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
        sReturn << "\n  renderorder " << (node->Emitter.nFlags & EMITTER_FLAG_RENDER_ORDER ? 1 : 0);
    }
    else if(nDataType == CONVERT_MESH){
        Node * node = (Node*) Data;
        sReturn << "\n  diffuse " << PrepareFloat(node->Mesh.fDiffuse.fR, 0) << " " << PrepareFloat(node->Mesh.fDiffuse.fG, 1) <<" "<< PrepareFloat(node->Mesh.fDiffuse.fB, 2);
        sReturn << "\n  ambient " << PrepareFloat(node->Mesh.fAmbient.fR, 0) << " " << PrepareFloat(node->Mesh.fAmbient.fG, 1) <<" "<< PrepareFloat(node->Mesh.fAmbient.fB, 2);
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
        //sReturn << "\n  specular 0.0 0.0 0.0";
        sReturn << "\n  transparencyhint " << node->Mesh.nTransparencyHint;
        sReturn << "\n  animateuv " << node->Mesh.nAnimateUV;
        //if(node->Mesh.nAnimateUV){
            sReturn << "\n  uvdirectionx " << PrepareFloat(node->Mesh.fUVDirectionX, 0);
            sReturn << "\n  uvdirectiony " << PrepareFloat(node->Mesh.fUVDirectionY, 0);
            sReturn << "\n  uvjitter " << PrepareFloat(node->Mesh.fUVJitter, 0);
            sReturn << "\n  uvjitterspeed " << PrepareFloat(node->Mesh.fUVJitterSpeed, 0);
        /*}
        else{
            sReturn << "\n  uvdirectionx 0.0";
            sReturn << "\n  uvdirectiony 0.0";
            sReturn << "\n  uvjitter 0.0";
            sReturn << "\n  uvjitterspeed 0.0";
        }*/
        sReturn << "\n  tangentspace " << (node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1 ? 1 : 0);
        //sReturn << string_format("\n  wirecolor 1 1 1");
        if(node->Mesh.cTexture1 != "" && node->Mesh.nTextureNumber >= 1) sReturn << "\n  bitmap " << node->Mesh.GetTexture(1);
        if(node->Mesh.cTexture2 != "" && node->Mesh.nTextureNumber >= 2) sReturn << "\n  bitmap2 " << node->Mesh.GetTexture(2);
        if(node->Mesh.cTexture3 != "" && node->Mesh.nTextureNumber >= 3) sReturn << "\n  texture0 " << node->Mesh.GetTexture(3);
        if(node->Mesh.cTexture4 != "" && node->Mesh.nTextureNumber >= 4) sReturn << "\n  texture1 " << node->Mesh.GetTexture(4);
        //if(node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2) sReturn << string_format("\n  lightmap %s", node->Mesh.GetTexture(2));
        sReturn << "\n  verts " << node->Mesh.Vertices.size();
        for(int n = 0; n < node->Mesh.Vertices.size(); n++){
            //Two possibilities - I put MDX if MDX is present, otherwise MDL
            if(!Mdx) sReturn << "\n    "<<PrepareFloat(node->Mesh.Vertices[n].fX, 0)<<" "<<PrepareFloat(node->Mesh.Vertices[n].fY, 1)<<" "<<PrepareFloat(node->Mesh.Vertices[n].fZ, 2);
            else sReturn << "\n    "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fX, 0)<<" "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fY, 1)<<" "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fZ, 2);
        }
        sReturn << "\n  faces " << node->Mesh.Faces.size();
        for(int n = 0; n < node->Mesh.Faces.size(); n++){
            sReturn << "\n    ";
            sReturn << node->Mesh.Faces[n].nIndexVertex[0];
            sReturn << " " << node->Mesh.Faces[n].nIndexVertex[1];
            sReturn << " " << node->Mesh.Faces[n].nIndexVertex[2];
            sReturn << " " << node->Mesh.Faces[n].nSmoothingGroup;
            sReturn << " " << node->Mesh.Faces[n].nIndexVertex[0];
            sReturn << " " << node->Mesh.Faces[n].nIndexVertex[1];
            sReturn << " " << node->Mesh.Faces[n].nIndexVertex[2];
            sReturn << " " << node->Mesh.Faces[n].nMaterialID;
        }
        if(!Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            sReturn << "\n  tverts "<<node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   " << PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV1.fX, 0);
                sReturn << " "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV1.fY, 1);
            }
        }
        if(!Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            sReturn << "\n  tverts1 "<<node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   "<< PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV2.fX, 0);
                sReturn << " "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV2.fY, 1);
            }
        }
        if(!Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            sReturn << "\n  tverts2 "<<node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   "<< PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV3.fX, 0);
                sReturn << " "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV3.fY, 1);
            }
        }
        if(!Mdx->sBuffer.empty() && node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            sReturn << "\n  tverts3 "<<node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   "<< PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV4.fX, 0);
                sReturn << " "<<PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV4.fY, 1);
            }
        }
    }
    else if(nDataType == CONVERT_SKIN){
        Node * node = (Node*) Data;
        if(!Mdx->sBuffer.empty()){
            sReturn << "\n  weights "<<node->Mesh.Vertices.size();
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\n   ";
                int i = 0;
                int nBoneNumber = (int) round(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightIndex[i]);
                //std::cout<<"Bone name index array size: "<<node->Skin.BoneNameIndexes.size()<<"\n";
                while(nBoneNumber != -1 && i < 4){
                    //std::cout<<"Reading bone number "<<nBoneNumber;
                    //std::cout<<", representing bone "<<FH->MH.Names.at(node->Skin.BoneNameIndexes.at(nBoneNumber)).sName.c_str()<<".\n";
                    int nNameIndex = node->Skin.BoneNameIndexes.at(nBoneNumber);
                    //sReturn << " "<<FH->MH.Names.at(nNameIndex).sName.c_str()<<" "<<PrepareFloat(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightValue[i], 0);
                    sReturn << " "<<MakeUniqueName(nNameIndex)<<" "<<PrepareFloat(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightValue[i], 0);
                    i++;
                    nBoneNumber = (int) round(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightIndex[i]);
                }
                if(i == 0){
                    sReturn << " root 1.0";
                }
            }
        }
    }
    else if(nDataType == CONVERT_DANGLY){
        Node * node = (Node*) Data;
        sReturn << "\n  displacement " << PrepareFloat(node->Dangly.fDisplacement, 0);
        sReturn << "\n  tightness " << PrepareFloat(node->Dangly.fTightness, 0);
        sReturn << "\n  period " << PrepareFloat(node->Dangly.fPeriod, 0);
        sReturn << "\n  constraints " << node->Dangly.Constraints.size();
        for(int n = 0; n < node->Dangly.Constraints.size(); n++){
            sReturn << "\n    "<<PrepareFloat(node->Dangly.Constraints[n], 0);
        }
    }
    else if(nDataType == CONVERT_AABB){
        Node * node = (Node*) Data;
        sReturn << string_format("\n  aabb");
        RecursiveAabb(&node->Walkmesh.RootAabb, sReturn);
    }
    else if(nDataType == CONVERT_SABER){
        Node * node = (Node*) Data;
        sReturn << "\n  diffuse " << PrepareFloat(node->Mesh.fDiffuse.fR, 0) << " " << PrepareFloat(node->Mesh.fDiffuse.fG, 1) <<" "<< PrepareFloat(node->Mesh.fDiffuse.fB, 2);
        sReturn << "\n  ambient " << PrepareFloat(node->Mesh.fAmbient.fR, 0) << " " << PrepareFloat(node->Mesh.fAmbient.fG, 1) <<" "<< PrepareFloat(node->Mesh.fAmbient.fB, 2);
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
        sReturn << "\n  uvdirectionx " << PrepareFloat(node->Mesh.fUVDirectionX, 0);
        sReturn << "\n  uvdirectiony " << PrepareFloat(node->Mesh.fUVDirectionY, 0);
        sReturn << "\n  uvjitter " << PrepareFloat(node->Mesh.fUVJitter, 0);
        sReturn << "\n  uvjitterspeed " << PrepareFloat(node->Mesh.fUVJitterSpeed, 0);
        sReturn << "\n  tangentspace 0";
        //sReturn << "\n  specular 0.0 0.0 0.0";
        //sReturn << "\n  wirecolor 1 1 1";
        if(node->Mesh.cTexture1 != "" && node->Mesh.nTextureNumber >= 1) sReturn << "\n  bitmap " << node->Mesh.GetTexture(1);

        if(node->Saber.SaberData.size() == 176){
            Vector vDiff;
            Vector vOut;
            vDiff = node->Saber.SaberData.at(4).vVertex - node->Saber.SaberData.at(0).vVertex;

            sReturn << "\n  verts 16";
            vOut = node->Saber.SaberData.at(0).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(1).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(2).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(3).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);

            vOut = node->Saber.SaberData.at(0).vVertex + vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(1).vVertex + vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(2).vVertex + vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(3).vVertex + vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);

            vOut = node->Saber.SaberData.at(88).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(89).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(90).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(91).vVertex;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);

            vOut = node->Saber.SaberData.at(88).vVertex - vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(89).vVertex - vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(90).vVertex - vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);
            vOut = node->Saber.SaberData.at(91).vVertex - vDiff;
            sReturn << "\n    "<<PrepareFloat(vOut.fX, 0)<<" "<<PrepareFloat(vOut.fY, 1)<<" "<<PrepareFloat(vOut.fZ, 2);

            sReturn << "\n  faces 12";
            sReturn << "\n    5 4 0 1 5 4 0 0";
            sReturn << "\n    0 1 5 1 0 1 5 0";
            sReturn << "\n    13 8 12 1 13 8 12 0";
            sReturn << "\n    8 13 9 1 8 13 9 0";
            sReturn << "\n    6 5 1 1 6 5 1 0";
            //sReturn << "\n    2 6 5 1 2 6 5 0"; //Correct faces, but not what they are in the game
            sReturn << "\n    1 2 6 1 1 2 6 0";
            //sReturn << "\n    1 2 5 1 1 2 5 0"; //Correct faces, but not what they are in the game
            sReturn << "\n    10 9 13 1 10 9 13 0";
            sReturn << "\n    13 14 10 1 13 14 10 0";
            sReturn << "\n    3 6 2 1 3 6 2 0";
            sReturn << "\n    6 3 7 1 6 3 7 0";
            sReturn << "\n    15 11 14 1 15 11 14 0";
            sReturn << "\n    10 14 11 1 10 14 11 0";

            sReturn << "\n  tverts 16";
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(0).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(0).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(0).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(1).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(1).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(1).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(2).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(2).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(2).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(3).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(3).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(3).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(4).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(4).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(4).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(5).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(5).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(5).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(6).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(6).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(6).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(7).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(7).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(7).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(88).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(88).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(88).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(89).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(89).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(89).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(90).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(90).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(90).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(91).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(91).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(91).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(92).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(92).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(92).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(93).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(93).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(93).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(94).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(94).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(94).vUV.fZ, 2);
            sReturn << "\n    "<<PrepareFloat(node->Saber.SaberData.at(95).vUV.fX, 0)<<" "<<PrepareFloat(node->Saber.SaberData.at(95).vUV.fY, 1)<<" "<<PrepareFloat(node->Saber.SaberData.at(95).vUV.fZ, 2);
        }
    }
    /// TODO: cases where num(controllers) == 0 but num(controller data) > 0
    else if(nDataType == CONVERT_CONTROLLER_KEYED){
        Controller * ctrl = (Controller*) Data;
        Node & geonode = GetNodeByNameIndex(ctrl->nNameIndex);
        Location loc = geonode.GetLocation();
        Node & node = GetNodeByNameIndex(ctrl->nNameIndex, ctrl->nAnimation);
        sReturn<<"\n"<<ReturnControllerName(ctrl->nControllerType, geonode.Head.nType);
        if(ctrl->nColumnCount > 16) sReturn<<"bezierkey";
        else sReturn<<"key";
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            ///TO-DO: add the geometry value
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                Orientation CtrlOrient;
                ByteBlock4.f = node.Head.ControllerData[ctrl->nDataStart + n];
                CtrlOrient.Decompress(ByteBlock4.ui);
                CtrlOrient.ConvertToAA();
                sReturn << PrepareFloat(CtrlOrient.Get(AA_X), 0) << " " << PrepareFloat(CtrlOrient.Get(AA_Y), 1) << " " << PrepareFloat(CtrlOrient.Get(AA_Z), 2) << " " << PrepareFloat(CtrlOrient.Get(AA_A), 3);
            }
        }
        else if(ctrl->nColumnCount == 4 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Uncompressed orientation
            ///TO-DO: add the geometry value
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                Orientation CtrlOrient;
                double fQX = node.Head.ControllerData[ctrl->nDataStart + n*4 + 0];
                double fQY = node.Head.ControllerData[ctrl->nDataStart + n*4 + 1];
                double fQZ = node.Head.ControllerData[ctrl->nDataStart + n*4 + 2];
                double fQW = node.Head.ControllerData[ctrl->nDataStart + n*4 + 3];
                CtrlOrient.Quaternion(fQX, fQY, fQZ, fQW);
                CtrlOrient.ConvertToAA();
                sReturn << PrepareFloat(CtrlOrient.Get(AA_X), 0) << " " << PrepareFloat(CtrlOrient.Get(AA_Y), 1) << " " << PrepareFloat(CtrlOrient.Get(AA_Z), 2) << " " << PrepareFloat(CtrlOrient.Get(AA_A), 3);
            }
        }/*
        /// positionbezierkey
        else if(ctrl->nColumnCount > 16 && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //positionbezierkey
            std::cout<<"Warning! Converting positionbezierkey to ascii positionkey. The bezier data is lost.\n";
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData[ctrl->nDataStart + n*9 + (0 + 2)], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData[ctrl->nDataStart + n*9 + (3 + 2)], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData[ctrl->nDataStart + n*9 + (6 + 2)], 0);
            }
        }*/
        /// regular bezierkey
        else if(ctrl->nColumnCount > 16){
            //bezierkey
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                for(int i = 0; i < (ctrl->nColumnCount - 16) * 3; i++){
                    sReturn << PrepareFloat(node.Head.ControllerData[ctrl->nDataStart + n*((ctrl->nColumnCount - 16) * 3) + i], 0);
                    if(i < (ctrl->nColumnCount - 16) * 3 - 1) sReturn << " ";
                }
            }
        }
        /// positionkey
        else if(ctrl->nColumnCount == 3 && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //normal position
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 0], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 1], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 2], 0);
            }

        }
        /// regular key
        else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){
            //default parser
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                for(int i = 0; i < ctrl->nColumnCount; i++){
                    sReturn << PrepareFloat(node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + i], 0);
                    if(i < ctrl->nColumnCount - 1) sReturn << " ";
                }
            }

        }
        else{
            std::string sLocation;
            if(ctrl->nAnimation == -1) sLocation = "geometry";
            else sLocation = FH->MH.Animations[ctrl->nAnimation].sName;
            std::cout<<"Controller data error for "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<" in "<<FH->MH.Names[ctrl->nNameIndex].sName<<" ("<<sLocation.c_str()<<")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
        sReturn << "\nendlist";
    }
    else if(nDataType == CONVERT_CONTROLLER_SINGLE){
        Controller * ctrl = (Controller*) Data;
        if(ctrl->nValueCount > 1){
            Error("Error! Single controller has more than one value. Skipping.");
            return;
        }
        Node & node = GetNodeByNameIndex(ctrl->nNameIndex);
        sReturn<<"\n  "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<"  ";
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            Orientation CtrlOrient;
            ByteBlock4.f = node.Head.ControllerData[ctrl->nDataStart];
            CtrlOrient.Decompress(ByteBlock4.ui);
            CtrlOrient.ConvertToAA();
            sReturn << PrepareFloat(CtrlOrient.Get(AA_X), 0) << " " << PrepareFloat(CtrlOrient.Get(AA_Y), 1) << " " << PrepareFloat(CtrlOrient.Get(AA_Z), 2) << " " << PrepareFloat(CtrlOrient.Get(AA_A), 3);
        }
        else if(ctrl->nColumnCount == 4 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Uncompressed orientation
            Orientation CtrlOrient;
            double fQX = node.Head.ControllerData[ctrl->nDataStart + 0];
            double fQY = node.Head.ControllerData[ctrl->nDataStart + 1];
            double fQZ = node.Head.ControllerData[ctrl->nDataStart + 2];
            double fQW = node.Head.ControllerData[ctrl->nDataStart + 3];
            CtrlOrient.Quaternion(fQX, fQY, fQZ, fQW);
            CtrlOrient.ConvertToAA();
            sReturn << PrepareFloat(CtrlOrient.Get(AA_X), 0) << " " << PrepareFloat(CtrlOrient.Get(AA_Y), 1) << " " << PrepareFloat(CtrlOrient.Get(AA_Z), 2) << " " << PrepareFloat(CtrlOrient.Get(AA_A), 3);
        }
    /*  else if(ctrl->nColumnCount == 19 && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //positionbezierkey
            // WE SHOULDNT BE ABLE TO PARSE THIS IN SINGLE CONTROLLERS
        }
    */  else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){
            //default parser
            for(int i = 0; i < ctrl->nColumnCount; i++){
                sReturn << PrepareFloat(node.Head.ControllerData[ctrl->nDataStart + i], 0);
                if(i < ctrl->nColumnCount - 1) sReturn << " ";
            }

        }
        else{
            std::string sLocation;
            if(ctrl->nAnimation == -1) sLocation = "geometry";
            else sLocation = FH->MH.Animations[ctrl->nAnimation].sName;
            std::cout<<"Controller data error for "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<" in "<<FH->MH.Names[ctrl->nNameIndex].sName<<" ("<<sLocation.c_str()<<")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
    }
}
