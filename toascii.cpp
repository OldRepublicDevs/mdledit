#include "MDL.h"

void MDL::ExportAscii(std::string &sExport){
    std::stringstream ss;
    ConvertToAscii(CONVERT_MODEL, ss, (void*) &FH[0].MH);
    sExport = ss.str();
}

void RecursiveAabb(Aabb * AABB, std::stringstream &str){
    str << string_format("\r\n    %f %f %f %f %f %f %i", AABB->vBBmin.fX, AABB->vBBmin.fY, AABB->vBBmin.fZ, AABB->vBBmax.fX, AABB->vBBmax.fY, AABB->vBBmax.fZ, AABB->nID);
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
        sReturn << string_format("\r\n# model %s", mh->GH.sName.c_str());
        sReturn << string_format("\r\nnewmodel %s", mh->GH.sName.c_str());
        sReturn << string_format("\r\nsetsupermodel %s %s", mh->GH.sName.c_str(), mh->cSupermodelName.c_str());
        sReturn << string_format("\r\nclassification %s", ReturnClassificationName(mh->nClassification).c_str());
        sReturn << string_format("\r\nsetanimationscale %s", PrepareFloat(mh->fScale, 0));
        sReturn << string_format("\r\n\r\nbeginmodelgeom %s", mh->GH.sName.c_str());
        sReturn << string_format("\r\n  bmin %s %s %s", PrepareFloat(mh->vBBmin.fX, 0), PrepareFloat(mh->vBBmin.fY, 1), PrepareFloat(mh->vBBmin.fZ, 2));
        sReturn << string_format("\r\n  bmax %s %s %s", PrepareFloat(mh->vBBmax.fX, 0), PrepareFloat(mh->vBBmax.fY, 1), PrepareFloat(mh->vBBmax.fZ, 2));
        sReturn << string_format("\r\n  radius %s", PrepareFloat(mh->fRadius, 0));
        for(int n = 0; n < mh->ArrayOfNodes.size(); n++){
            Node & node = mh->ArrayOfNodes.at(n);
            if(node.Head.nType & NODE_HAS_HEADER){
                ConvertToAscii(CONVERT_HEADER, sReturn, (void*) &node);
            }
            else{
                std::cout<<string_format("Writing ASCII ERROR!!! Headerless node! Offset: %u\n", node.nOffset);
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
        sReturn << string_format("\r\nendmodelgeom %s\r\n", mh->GH.sName.c_str());

        for(int n = 0; n < mh->Animations.size(); n++){
            ConvertToAscii(CONVERT_ANIMATION, sReturn, (void*) &mh->Animations[n]);
        }
        sReturn << string_format("\r\ndonemodel %s\r\n", mh->GH.sName.c_str());
    }
    else if(nDataType == CONVERT_ANIMATION){
        Animation * anim = (Animation*) Data;
        sReturn << string_format("\r\nnewanim %s %s", anim->sName.c_str(), FH[0].MH.GH.sName.c_str());
        sReturn << string_format("\r\n  length %s", PrepareFloat(anim->fLength, 0));
        sReturn << string_format("\r\n  transtime %s", PrepareFloat(anim->fTransition, 0));
        sReturn << string_format("\r\n  animroot %s", anim->sAnimRoot.c_str());
        if(anim->Sounds.size() > 0){
            sReturn << string_format("\r\n  eventlist %s", anim->sAnimRoot.c_str());
            for(int s = 0; s < anim->Sounds.size(); s++){
                sReturn << "\r\n    " << anim->Sounds.at(s).fTime << " " << anim->Sounds.at(s).sName.c_str();
            }
            sReturn << string_format("\r\n  endlist %s", anim->sAnimRoot.c_str());
        }
        for(int n = 0; n < anim->ArrayOfNodes.size(); n++){
            Node & node = anim->ArrayOfNodes.at(n);
            ConvertToAscii(CONVERT_ANIMATION_NODE, sReturn, (void*) &node);
            ConvertToAscii(CONVERT_ENDNODE, sReturn, (void*) &node);
        }
        sReturn << string_format("\r\ndoneanim %s %s", anim->sName.c_str(), FH[0].MH.GH.sName.c_str());
    }
    else if(nDataType == CONVERT_ANIMATION_NODE){
        Node * node = (Node*) Data;
        char cType [255];
        if(node->Head.nType & NODE_HAS_AABB) sprintf(cType, "aabb");
        else if(node->Head.nType & NODE_HAS_DANGLY) sprintf(cType, "danglymesh");
        else if(node->Head.nType & NODE_HAS_SKIN) sprintf(cType, "skin");
        else if(node->Head.nType & NODE_HAS_MESH) sprintf(cType, "trimesh");
        else if(node->Head.nType & NODE_HAS_EMITTER) sprintf(cType, "emitter");
        else if(node->Head.nType & NODE_HAS_LIGHT) sprintf(cType, "light");
        else if(node->Head.nType & NODE_HAS_HEADER) sprintf(cType, "dummy");
        sReturn << string_format("\r\nnode %s %s", cType, FH[0].MH.Names[node->Head.nNameIndex].sName.c_str());
        if(node->Head.nParentIndex != -1){
            sReturn << string_format("\r\n  parent %s", FH[0].MH.Names[node->Head.nParentIndex].sName.c_str());
        }
        else{
            sReturn << "\r\n  parent NULL";
        }
        if(node->Head.Controllers.size() > 0){
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_KEYED, sReturn, (void*) &(node->Head.Controllers[n]));
            }
        }
    }
    else if(nDataType == CONVERT_HEADER){
        Node * node = (Node*) Data;
        char cType [255];
        if(node->Head.nType & NODE_HAS_AABB) sprintf(cType, "aabb");
        else if(node->Head.nType & NODE_HAS_DANGLY) sprintf(cType, "danglymesh");
        else if(node->Head.nType & NODE_HAS_SKIN) sprintf(cType, "skin");
        else if(node->Head.nType & NODE_HAS_MESH) sprintf(cType, "trimesh");
        else if(node->Head.nType & NODE_HAS_EMITTER) sprintf(cType, "emitter");
        else if(node->Head.nType & NODE_HAS_LIGHT) sprintf(cType, "light");
        else if(node->Head.nType & NODE_HAS_HEADER) sprintf(cType, "dummy");
        sReturn << string_format("\r\nnode %s %s", cType, FH[0].MH.Names[node->Head.nNameIndex].sName.c_str());
        if(node->Head.nParentIndex != -1){
            sReturn << string_format("\r\n  parent %s", FH[0].MH.Names[node->Head.nParentIndex].sName.c_str());
        }
        else{
            sReturn << string_format("\r\n  parent NULL");
        }
        if(node->Head.Controllers.size() > 0){
            /*if(node->Head.Controllers[0].nControllerType != CONTROLLER_HEADER_POSITION){
                sReturn << string_format("\r\n  position %s %s %s", PrepareFloat(node->Head.Pos.fX, 0), PrepareFloat(node->Head.Pos.fY, 1), PrepareFloat(node->Head.Pos.fZ, 2));
                if(node->Head.Controllers[0].nControllerType != CONTROLLER_HEADER_ORIENTATION){
                    sReturn << string_format("\r\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
                }
            }
            else if(node->Head.Controllers[1].nControllerType != CONTROLLER_HEADER_ORIENTATION){
                sReturn << string_format("\r\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
            }*/
            for(int n = 0; n < node->Head.Controllers.size(); n++){
                ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sReturn, (void*) &(node->Head.Controllers[n]));
            }
        }/*
        else{
            sReturn << string_format("\r\n  position %s %s %s", PrepareFloat(node->Head.Pos.fX, 0), PrepareFloat(node->Head.Pos.fY, 1), PrepareFloat(node->Head.Pos.fZ, 2));
            sReturn << string_format("\r\n  orientation %s %s %s %s", PrepareFloat(node->Head.Orient.Get(QU_X), 0), PrepareFloat(node->Head.Orient.Get(QU_Y), 1), PrepareFloat(node->Head.Orient.Get(QU_Z), 2), PrepareFloat(node->Head.Orient.Get(QU_W), 3));
        }*/
    }
    else if(nDataType == CONVERT_ENDNODE){
        sReturn << "\r\nendnode";
    }
    else if(nDataType == CONVERT_LIGHT){
        Node * node = (Node*) Data;
        sReturn << string_format("\r\n  lightpriority %i", node->Light.nLightPriority);
        sReturn << string_format("\r\n  ndynamictype %i", node->Light.nDynamicType);
        sReturn << string_format("\r\n  ambientonly %i", node->Light.nAmbientOnly);
        sReturn << string_format("\r\n  affectdynamic %i", node->Light.nAffectDynamic);
        sReturn << string_format("\r\n  shadow %i", node->Light.nShadow);
        sReturn << string_format("\r\n  lensflares %i", node->Light.nFlare);
        sReturn << string_format("\r\n  fadinglight %i", node->Light.nFadingLight);
        sReturn << string_format("\r\n  flareradius %s", PrepareFloat(node->Light.fFlareRadius, 0)); //NWmax reads this as an int
        sReturn << string_format("\r\n  texturenames %i", node->Light.FlareTextureNames.size());
        for(int n = 0; n < node->Light.FlareTextureNames.size(); n++){
            sReturn<<"\r\n    "<<node->Light.FlareTextureNames[n].sName;
        }
        sReturn << string_format("\r\n  flaresizes %i", node->Light.FlareSizes.size());
        for(int n = 0; n < node->Light.FlareSizes.size(); n++){
            sReturn<<"\r\n    "<<node->Light.FlareSizes[n];
        }
        sReturn << string_format("\r\n  flarepositions %i", node->Light.FlarePositions.size());
        for(int n = 0; n < node->Light.FlarePositions.size(); n++){
            sReturn<<"\r\n    "<<node->Light.FlarePositions[n];
        }
        sReturn << string_format("\r\n  flarecolorshifts %i", node->Light.FlareColorShifts.size());
        for(int n = 0; n < node->Light.FlareColorShifts.size(); n++){
            sReturn<<"\r\n    "<<node->Light.FlareColorShifts[n].fR<<" "<<node->Light.FlareColorShifts[n].fG<<" "<<node->Light.FlareColorShifts[n].fB;
        }
    }
    else if(nDataType == CONVERT_EMITTER){
        Node * node = (Node*) Data;
        sReturn << string_format("\r\n  deadspace %s", PrepareFloat(node->Emitter.fDeadSpace, 0));
        sReturn << string_format("\r\n  blastradius %s", PrepareFloat(node->Emitter.fBlastRadius, 0));
        sReturn << string_format("\r\n  blastlength %s", PrepareFloat(node->Emitter.fBlastLength, 0));
        sReturn << string_format("\r\n  xgrid %i", node->Emitter.nxGrid);
        sReturn << string_format("\r\n  ygrid %i", node->Emitter.nyGrid);
        sReturn << string_format("\r\n  spawntype %i", node->Emitter.nSpawnType);
        sReturn << string_format("\r\n  update %s", node->Emitter.cUpdate.c_str());
        sReturn << string_format("\r\n  render %s", node->Emitter.cRender.c_str());
        sReturn << string_format("\r\n  blend %s", node->Emitter.cBlend.c_str());
        sReturn << string_format("\r\n  texture %s", node->Emitter.cTexture.c_str());
        sReturn << string_format("\r\n  chunkname %s", node->Emitter.cChunkName.c_str());
        sReturn << string_format("\r\n  twosidedtex %i", node->Emitter.nTwosidedTex);
        sReturn << string_format("\r\n  loop %i", node->Emitter.nLoop);
        //sReturn << string_format("\r\n  renderorder %i", node->Emitter.nRenderOrder);
        sReturn << string_format("\r\n  p2p %i", node->Emitter.nFlags & EMITTER_FLAG_P2P ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  p2p_sel %i", node->Emitter.nFlags & EMITTER_FLAG_P2P_SEL ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  affectedByWind %i", node->Emitter.nFlags & EMITTER_FLAG_AFFECTED_WIND ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  m_isTinted %i", node->Emitter.nFlags & EMITTER_FLAG_TINTED ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  bounce %i", node->Emitter.nFlags & EMITTER_FLAG_BOUNCE ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  random %i", node->Emitter.nFlags & EMITTER_FLAG_RANDOM ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  inherit %i", node->Emitter.nFlags & EMITTER_FLAG_INHERIT ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  inheritvel %i", node->Emitter.nFlags & EMITTER_FLAG_INHERIT_VEL ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  inherit_local %i", node->Emitter.nFlags & EMITTER_FLAG_INHERIT_LOCAL ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  splat %i", node->Emitter.nFlags & EMITTER_FLAG_SPLAT ? 1 : 0 ? 1 : 0);
        sReturn << string_format("\r\n  inherit_part %i", node->Emitter.nFlags & EMITTER_FLAG_INHERIT_PART ? 1 : 0 ? 1 : 0);
    }
    else if(nDataType == CONVERT_MESH){
        Node * node = (Node*) Data;
        sReturn << string_format("\r\n  diffuse %s %s %s", PrepareFloat(node->Mesh.fDiffuse.fR, 0), PrepareFloat(node->Mesh.fDiffuse.fG, 1), PrepareFloat(node->Mesh.fDiffuse.fB, 2));
        sReturn << string_format("\r\n  ambient %s %s %s", PrepareFloat(node->Mesh.fAmbient.fR, 0), PrepareFloat(node->Mesh.fAmbient.fG, 1), PrepareFloat(node->Mesh.fAmbient.fB, 2));
        sReturn << string_format("\r\n  rotatetexture %i", node->Mesh.nRotateTexture);
        sReturn << string_format("\r\n  shadow %i", node->Mesh.nShadow);
        sReturn << string_format("\r\n  render %i", node->Mesh.nRender);
        //sReturn << string_format("\r\n  specular 0.0 0.0 0.0", node->Mesh.nRender);
        sReturn << string_format("\r\n  shininess %i", node->Mesh.nShininess);
        sReturn << string_format("\r\n  animateuv %i", node->Mesh.nAnimateUV);
        if(node->Mesh.nAnimateUV){
            sReturn << string_format("\r\n  uvdirectionx %s", PrepareFloat(node->Mesh.fUVDirectionX, 0));
            sReturn << string_format("\r\n  uvdirectiony %s", PrepareFloat(node->Mesh.fUVDirectionY, 0));
            sReturn << string_format("\r\n  uvjitter %s", PrepareFloat(node->Mesh.fUVJitter, 0));
            sReturn << string_format("\r\n  uvjitterspeed %s", PrepareFloat(node->Mesh.fUVJitterSpeed, 0));
        }
        else{
            sReturn << "\r\n  uvdirectionx 0.0";
            sReturn << "\r\n  uvdirectiony 0.0";
            sReturn << "\r\n  uvjitter 0.0";
            sReturn << "\r\n  uvjitterspeed 0.0";
        }
        if(node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1) sReturn << "\r\n  tangentspace 1";
        else sReturn << "\r\n  tangentspace 0";
        //sReturn << string_format("\r\n  wirecolor 1 1 1");
        sReturn << string_format("\r\n  bitmap %s", node->Mesh.GetTexture(1));
        if(node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2) sReturn << string_format("\r\n  lightmap %s", node->Mesh.GetTexture(2));
        sReturn << string_format("\r\n  verts %i", node->Mesh.nNumberOfVerts);
        for(int n = 0; n < node->Mesh.nNumberOfVerts; n++){
            //Two possibilities - I put MDX if MDX is present, otherwise MDL
            if(Mdx.empty()) sReturn << string_format("\r\n    %s %s %s", PrepareFloat(node->Mesh.Vertices[n].fX, 0), PrepareFloat(node->Mesh.Vertices[n].fY, 1), PrepareFloat(node->Mesh.Vertices[n].fZ, 2));
            else sReturn << string_format("\r\n    %s %s %s", PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fX, 0), PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fY, 1), PrepareFloat(node->Mesh.Vertices[n].MDXData.vVertex.fZ, 2));
        }
        sReturn << string_format("\r\n  faces %i", node->Mesh.Faces.size());
        for(int n = 0; n < node->Mesh.Faces.size(); n++){
            //Two possibilities - I put MDX if MDX is present, otherwise MDL
            if(Mdx.empty())
                sReturn << string_format("\r\n    %i %i %i %i %i %i %i %i",
                    node->Mesh.Faces[n].nIndexVertex[0], node->Mesh.Faces[n].nIndexVertex[1], node->Mesh.Faces[n].nIndexVertex[2],
                    pown(2, node->Mesh.Faces[n].nSmoothingGroup - 1),
                    node->Mesh.Faces[n].nIndexVertex[0], node->Mesh.Faces[n].nIndexVertex[1], node->Mesh.Faces[n].nIndexVertex[2],
                    node->Mesh.Faces[n].nMaterialID);
            else
                sReturn << string_format("\r\n    %i %i %i %i %i %i %i %i",
                    node->Mesh.VertIndices[n].nValues[0], node->Mesh.VertIndices[n].nValues[1], node->Mesh.VertIndices[n].nValues[2],
                    pown(2, node->Mesh.Faces[n].nSmoothingGroup - 1),
                    node->Mesh.VertIndices[n].nValues[0], node->Mesh.VertIndices[n].nValues[1], node->Mesh.VertIndices[n].nValues[2],
                    node->Mesh.Faces[n].nMaterialID);
        }
        if(!Mdx.sBuffer.empty()){
            sReturn << string_format("\r\n  tverts %i", node->Mesh.nNumberOfVerts);
            for(int n = 0; n < node->Mesh.nNumberOfVerts; n++){
                sReturn << string_format("\r\n    %s %s 0.0", PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV1.fX, 0), PrepareFloat(node->Mesh.Vertices[n].MDXData.vUV1.fY, 1));
            }
        }
    }
    else if(nDataType == CONVERT_SKIN){
        Node * node = (Node*) Data;
        if(!Mdx.sBuffer.empty()){
            sReturn << "\r\n  weights "<<node->Mesh.nNumberOfVerts;
            for(int n = 0; n < node->Mesh.Vertices.size(); n++){
                sReturn << "\r\n   ";
                int i = 0;
                int nBoneNumber = (int) round(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightIndex[i]);
                //std::cout<<"Bone name index array size: "<<node->Skin.BoneNameIndexes.size()<<"\n";
                while(nBoneNumber != -1 && i < 4){
                    //std::cout<<"Reading bone number "<<nBoneNumber;
                    //std::cout<<", representing bone "<<FH[0].MH.Names.at(node->Skin.BoneNameIndexes.at(nBoneNumber)).sName.c_str()<<".\n";
                    int nNameIndex = node->Skin.BoneNameIndexes.at(nBoneNumber);
                    sReturn << " "<<FH[0].MH.Names.at(nNameIndex).sName.c_str()<<" "<<PrepareFloat(node->Mesh.Vertices.at(n).MDXData.Weights.fWeightValue[i], 0);
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
        sReturn << string_format("\r\n  displacement %s", PrepareFloat(node->Dangly.fDisplacement, 0));
        sReturn << string_format("\r\n  tightness %s", PrepareFloat(node->Dangly.fTightness, 0));
        sReturn << string_format("\r\n  period %s", PrepareFloat(node->Dangly.fPeriod, 0));
        sReturn << string_format("\r\n  constraints %i", node->Dangly.Constraints.size());
        for(int n = 0; n < node->Dangly.Constraints.size(); n++){
            sReturn << "\r\n    "<<PrepareFloat(node->Dangly.Constraints[n], 0);
        }
    }
    else if(nDataType == CONVERT_AABB){
        Node * node = (Node*) Data;
        sReturn << string_format("\r\n  aabb");
        RecursiveAabb(&node->Walkmesh.RootAabb, sReturn);
    }
    else if(nDataType == CONVERT_SABER){
        Node * node = (Node*) Data;
        sReturn << string_format("\r\n  diffuse %s %s %s", PrepareFloat(node->Mesh.fDiffuse.fR, 0), PrepareFloat(node->Mesh.fDiffuse.fG, 1), PrepareFloat(node->Mesh.fDiffuse.fB, 2));
        sReturn << string_format("\r\n  ambient %s %s %s", PrepareFloat(node->Mesh.fAmbient.fR, 0), PrepareFloat(node->Mesh.fAmbient.fG, 1), PrepareFloat(node->Mesh.fAmbient.fB, 2));
        sReturn << string_format("\r\n  rotatetexture %i", node->Mesh.nRotateTexture);
        sReturn << string_format("\r\n  shadow %i", node->Mesh.nShadow);
        sReturn << string_format("\r\n  render %i", node->Mesh.nRender);
        //sReturn << string_format("\r\n  specular 0.0 0.0 0.0", node->Mesh.nRender);
        //sReturn << cCat;
        sReturn << string_format("\r\n  shininess %i", node->Mesh.nShininess);
        sReturn << string_format("\r\n  animateuv %i", node->Mesh.nAnimateUV);
        if(node->Mesh.nAnimateUV){
            sReturn << string_format("\r\n  uvdirectionx %s", PrepareFloat(node->Mesh.fUVDirectionX, 0));
            sReturn << string_format("\r\n  uvdirectiony %s", PrepareFloat(node->Mesh.fUVDirectionY, 0));
            sReturn << string_format("\r\n  uvjitter %s", PrepareFloat(node->Mesh.fUVJitter, 0));
            sReturn << string_format("\r\n  uvjitterspeed %s", PrepareFloat(node->Mesh.fUVJitterSpeed, 0));
        }
        else{
            sReturn << "\r\n  uvdirectionx 0.0";
            sReturn << "\r\n  uvdirectiony 0.0";
            sReturn << "\r\n  uvjitter 0.0";
            sReturn << "\r\n  uvjitterspeed 0.0";
        }
        //sReturn << string_format("\r\n  wirecolor 1 1 1");
        //sReturn << cCat;
        sReturn << string_format("\r\n  bitmap %s", node->Mesh.GetTexture(1));
        if(node->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2) sReturn << string_format("\r\n  lightmap %s", node->Mesh.GetTexture(2));
        sReturn << string_format("\r\n  verts %i", node->Mesh.nNumberOfVerts);
        for(int n = 0; n < node->Mesh.nNumberOfVerts; n++){
            //Two possibilities
            //sReturn << string_format("\r\n    %s %s %s", node->Mesh.Vertices[n].fX, node->Mesh.Vertices[n].fY, node->Mesh.Vertices[n].fZ);
            sReturn << string_format("\r\n    %s %s %s", PrepareFloat(node->Saber.SaberData[n].vVertex.fX, 0), PrepareFloat(node->Saber.SaberData[n].vVertex.fY, 1), PrepareFloat(node->Saber.SaberData[n].vVertex.fZ, 2));
        }
        sReturn << string_format("\r\n  faces %i", node->Mesh.Faces.size());
        for(int n = 0; n < node->Mesh.Faces.size(); n++){
            sReturn << string_format("\r\n    %i %i %i %i %i %i %i 1",
                    node->Mesh.Faces[n].nIndexVertex[0], node->Mesh.Faces[n].nIndexVertex[1], node->Mesh.Faces[n].nIndexVertex[2],
                    pown(2, node->Mesh.Faces[n].nMaterialID - 1),
                    node->Mesh.Faces[n].nIndexVertex[0], node->Mesh.Faces[n].nIndexVertex[1], node->Mesh.Faces[n].nIndexVertex[2]);
                    //node->Mesh.VertIndices[n].nValues[0], node->Mesh.VertIndices[n].nValues[1], node->Mesh.VertIndices[n].nValues[2]);
        }
        sReturn << string_format("\r\n  tverts %i", node->Mesh.nNumberOfVerts);
        for(int n = 0; n < node->Mesh.nNumberOfVerts; n++){
            sReturn << string_format("\r\n    %s %s 0.0", PrepareFloat(node->Saber.SaberData[n].vUV.fX, 0), PrepareFloat(node->Saber.SaberData[n].vUV.fY, 1));
        }
    }
    /// TODO: cases where num(controllers) == 0 but num(controller data) > 0
    else if(nDataType == CONVERT_CONTROLLER_KEYED){
        Controller * ctrl = (Controller*) Data;
        Node & geonode = GetNodeByNameIndex(ctrl->nNameIndex);
        Location loc = geonode.GetLocation();
        Node & node = GetNodeByNameIndex(ctrl->nNameIndex, ctrl->nAnimation);
        sReturn<<"\r\n"<<ReturnControllerName(ctrl->nControllerType, geonode.Head.nType)<<"key";
        if(ctrl->nColumnCount == 2 && ctrl->nControllerType == CONTROLLER_HEADER_ORIENTATION){
            //Compressed orientation
            ///TO-DO: add the geometry value
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\r\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
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
                sReturn<<"\r\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                Orientation CtrlOrient;
                double fQX = node.Head.ControllerData[ctrl->nDataStart + n*4 + 0];
                double fQY = node.Head.ControllerData[ctrl->nDataStart + n*4 + 1];
                double fQZ = node.Head.ControllerData[ctrl->nDataStart + n*4 + 2];
                double fQW = node.Head.ControllerData[ctrl->nDataStart + n*4 + 3];
                CtrlOrient.Quaternion(fQX, fQY, fQZ, fQW);
                CtrlOrient.ConvertToAA();
                sReturn << PrepareFloat(CtrlOrient.Get(AA_X), 0) << " " << PrepareFloat(CtrlOrient.Get(AA_Y), 1) << " " << PrepareFloat(CtrlOrient.Get(AA_Z), 2) << " " << PrepareFloat(CtrlOrient.Get(AA_A), 3);
            }
        }
        else if(ctrl->nColumnCount == 19 && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //positionbezierkey
            std::cout<<"Warning! Converting positionbezierkey to ascii positionkey. The bezier data is lost.\n";
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\r\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData[ctrl->nDataStart + n*9 + (0 + 2)], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData[ctrl->nDataStart + n*9 + (3 + 2)], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData[ctrl->nDataStart + n*9 + (6 + 2)], 0);
            }
        }
        else if(ctrl->nColumnCount == 3 && ctrl->nControllerType == CONTROLLER_HEADER_POSITION){
            //normal position
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\r\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                sReturn << PrepareFloat(loc.vPosition.fX + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 0], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fY + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 1], 0);
                sReturn << " ";
                sReturn << PrepareFloat(loc.vPosition.fZ + node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + 2], 0);
            }

        }
        else if(ctrl->nColumnCount == 1 || ctrl->nColumnCount == 3){
            //default parser
            for(int n = 0; n < ctrl->nValueCount; n++){
                sReturn<<"\r\n  "<<PrepareFloat(node.Head.ControllerData[ctrl->nTimekeyStart + n], 0)<<" ";
                for(int i = 0; i < ctrl->nColumnCount; i++){
                    sReturn << PrepareFloat(node.Head.ControllerData[ctrl->nDataStart + n*ctrl->nColumnCount + i], 0);
                    if(i < ctrl->nColumnCount - 1) sReturn << " ";
                }
            }

        }
        else{
            std::string sLocation;
            if(ctrl->nAnimation == -1) sLocation = "geometry";
            else sLocation = FH[0].MH.Animations[ctrl->nAnimation].sName;
            std::cout<<"Controller data error for "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<" in "<<FH[0].MH.Names[ctrl->nNameIndex].sName<<" ("<<sLocation.c_str()<<")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
        sReturn << "\r\nendlist";
    }
    else if(nDataType == CONVERT_CONTROLLER_SINGLE){
        Controller * ctrl = (Controller*) Data;
        if(ctrl->nValueCount > 1){
            Error("Error! Single controller has more than one value. Skipping.");
            return;
        }
        Node & node = GetNodeByNameIndex(ctrl->nNameIndex);
        sReturn<<"\r\n  "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<"  ";
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
            else sLocation = FH[0].MH.Animations[ctrl->nAnimation].sName;
            std::cout<<"Controller data error for "<<ReturnControllerName(ctrl->nControllerType, node.Head.nType)<<" in "<<FH[0].MH.Names[ctrl->nNameIndex].sName<<" ("<<sLocation.c_str()<<")!\n";
            Error("A controller type is not being handled! Check the console and add the necessary code!");
        }
    }
}
