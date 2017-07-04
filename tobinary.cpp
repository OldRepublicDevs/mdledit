#include "MDL.h"
//#include <fstream>

/**
    Functions:
    MDL::Compile()
    MDL::WriteAabb()
    MDL::WriteNodes()
/**/

bool MDL::Compile(){
    nPosition = 0;
    sBuffer.resize(0);
    bKnown.resize(0);
    Mdx.reset(new MDX());

    std::unique_ptr<FileHeader> & Data = FH;
    Report("Compiling model...");

    //File header
    WriteInt(0, 8);
    int PHnMdlLength = nPosition;
    WriteInt(0xFFFFFFFF, 1);
    int PHnMdxLength = nPosition;
    WriteInt(0xFFFFFFFF, 1);

    //GeoHeader
    Data->MH.GH.nFunctionPointer0 = FunctionPointer1(FN_PTR_MODEL);
    Data->MH.GH.nFunctionPointer1 = FunctionPointer2(FN_PTR_MODEL);
    WriteInt(Data->MH.GH.nFunctionPointer0, 9);
    WriteInt(Data->MH.GH.nFunctionPointer1, 9);
    Data->MH.GH.sName.resize(32);
    WriteString(Data->MH.GH.sName, 3);

    int PHnOffsetToRootNode = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    //Don't overwrite, we've already got this value.
    WriteInt(Data->MH.GH.nTotalNumberOfNodes, 1);

    //Unknown empty bytes
    WriteInt(0, 8);
    WriteInt(0, 8);
    WriteInt(0, 8);
    WriteInt(0, 8);
    WriteInt(0, 8);
    WriteInt(0, 8);

    WriteInt(0, 8); //Ref. count

    WriteByte(Data->MH.GH.nModelType, 7); //Model type
    WriteByte(Data->MH.GH.nPadding[0], 11); //padding
    WriteByte(Data->MH.GH.nPadding[1], 11);
    WriteByte(Data->MH.GH.nPadding[2], 11);

    //Model header
    WriteByte(Data->MH.nClassification, 7); //Classification
    WriteByte(Data->MH.nUnknown1[0], 10); //Unknown
    WriteByte(Data->MH.nUnknown1[1], 7);
    WriteByte(Data->MH.nUnknown1[2], 7);

    WriteInt(0, 8); //Child model count

    //Animation Array
    int PHnOffsetToAnimationArray = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(Data->MH.Animations.size(), 1);
    WriteInt(Data->MH.Animations.size(), 1);

    //Unknown supermodel int?
    WriteInt(Data->MH.nPadding, 10);

    //Floats
    WriteFloat(Data->MH.vBBmin.fX, 2);
    WriteFloat(Data->MH.vBBmin.fY, 2);
    WriteFloat(Data->MH.vBBmin.fZ, 2);
    WriteFloat(Data->MH.vBBmax.fX, 2);
    WriteFloat(Data->MH.vBBmax.fY, 2);
    WriteFloat(Data->MH.vBBmax.fZ, 2);
    WriteFloat(Data->MH.fRadius, 2);
    WriteFloat(Data->MH.fScale, 2);

    Data->MH.cSupermodelName.resize(32);
    WriteString(Data->MH.cSupermodelName, 3);

    int PHnOffsetToHeadRootNode = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(0, 8);
    int PHnMdxLength2 = nPosition;
    WriteInt(0xFFFFFFFF, 1);
    WriteInt(0, 8); //Mdx offset

    int PHnOffsetToNameArray = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(Data->MH.Names.size(), 1);
    WriteInt(Data->MH.Names.size(), 1);

    //Create Name array
    Data->MH.NameArray.ResetToSize(Data->MH.Names.size());
    WriteIntToPH(nPosition - 12, PHnOffsetToNameArray, Data->MH.NameArray.nOffset);

    std::vector<int> PHnOffsetToName;
    for(int c = 0; c < Data->MH.Names.size(); c++){
        PHnOffsetToName.push_back(nPosition);
        WriteInt(0xFFFFFFFF, 6);
    }
    for(int c = 0; c < Data->MH.Names.size(); c++){
        WriteIntToPH(nPosition - 12, PHnOffsetToName.at(c), Data->MH.Names.at(c).nOffset);
        WriteString(Data->MH.Names[c].sName.c_str(), 3);
        WriteByte(0, 3);
    }

    //Create Animation array
    Data->MH.AnimationArray.ResetToSize(Data->MH.Animations.size());
    WriteIntToPH(nPosition - 12, PHnOffsetToAnimationArray, Data->MH.AnimationArray.nOffset);
    std::vector<int> pnOffsetsToAnimation;
    for(int c = 0; c < Data->MH.Animations.size(); c++){
        pnOffsetsToAnimation.push_back(nPosition);
        WriteInt(0xFFFFFFFF, 6);
    }
    for(int c = 0; c < Data->MH.Animations.size(); c++){
        //This is where we fill EVERYTHING about the animation
        Animation & anim = Data->MH.Animations[c];
        WriteIntToPH(nPosition - 12, pnOffsetsToAnimation[c], Data->MH.Animations[c].nOffset);
        anim.nFunctionPointer0 = FunctionPointer1(FN_PTR_ANIM);
        anim.nFunctionPointer1 = FunctionPointer2(FN_PTR_ANIM);
        WriteInt(anim.nFunctionPointer0, 9);
        WriteInt(anim.nFunctionPointer1, 9);
        anim.sName.resize(32);
        WriteString(anim.sName, 3);
        int PHnOffsetToFirstNode = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        anim.nNumberOfNames = Data->MH.Names.size();
        WriteInt(anim.nNumberOfNames, 1);

        WriteInt(0, 8); //Unknown constant zeroes
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);

        WriteByte(anim.nModelType, 7);
        WriteByte(anim.nPadding[0], 11);
        WriteByte(anim.nPadding[1], 11);
        WriteByte(anim.nPadding[2], 11);

        WriteFloat(anim.fLength, 2);
        WriteFloat(anim.fTransition, 2);

        anim.sAnimRoot.resize(32);
        WriteString(anim.sAnimRoot, 3);

        int PHnOffsetToEventArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(anim.Events.size(), 1);
        WriteInt(anim.Events.size(), 1);
        WriteInt(0, 8); //Unknown constant
        if(anim.Events.size() > 0){
            anim.EventArray.ResetToSize(anim.Events.size());
            WriteIntToPH(nPosition - 12, PHnOffsetToEventArray, anim.EventArray.nOffset);
            for(int b = 0; b < anim.Events.size(); b++){
                WriteFloat(anim.Events[b].fTime, 2);
                anim.Events[b].sName.resize(32);
                WriteString(anim.Events[b].sName, 3);
            }
        }
        else WriteIntToPH(0, PHnOffsetToEventArray, Data->MH.Animations[c].EventArray.nOffset);

        WriteIntToPH(nPosition - 12, PHnOffsetToFirstNode, Data->MH.Animations[c].nOffsetToRootAnimationNode);
        WriteNodes(anim.ArrayOfNodes.front());
    }

    WriteIntToPH(nPosition - 12, PHnOffsetToRootNode, Data->MH.GH.nOffsetToRootNode);
    WriteIntToPH(nPosition - 12, PHnOffsetToHeadRootNode, Data->MH.nOffsetToHeadRootNode); //For now we will make these two equal in all cases
    WriteNodes(Data->MH.ArrayOfNodes.front());

    WriteIntToPH(nPosition - 12, PHnMdlLength, Data->nMdlLength);
    WriteIntToPH(Mdx->nPosition, PHnMdxLength, Data->nMdxLength);
    WriteIntToPH(Mdx->nPosition, PHnMdxLength2, Data->MH.nMdxLength2);

    bLoaded = true;
    Mdx->bLoaded = true;
    if(Wok) Wok->Compile();
    if(Pwk) Pwk->Compile();
    if(Dwk0) Dwk0->Compile();
    if(Dwk1) Dwk1->Compile();
    if(Dwk2) Dwk2->Compile();
}

void MDL::WriteAabb(Aabb & aabb){
    aabb.nOffset = nPosition - 12;
    WriteFloat(aabb.vBBmin.fX, 2);
    WriteFloat(aabb.vBBmin.fY, 2);
    WriteFloat(aabb.vBBmin.fZ, 2);
    WriteFloat(aabb.vBBmax.fX, 2);
    WriteFloat(aabb.vBBmax.fY, 2);
    WriteFloat(aabb.vBBmax.fZ, 2);
    int PHnChild1 = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    int PHnChild2 = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(aabb.nID, 4);
    WriteInt(aabb.nProperty, 4);
    if(aabb.Child1.size() > 0){
        WriteIntToPH(nPosition - 12, PHnChild1, aabb.nChild1);
        WriteAabb(aabb.Child1.front());
    }
    if(aabb.Child2.size() > 0){
        WriteIntToPH(nPosition - 12, PHnChild2, aabb.nChild2);
        WriteAabb(aabb.Child2.front());
    }
}

void MDL::WriteNodes(Node & node){
    FileHeader & Data = *FH;

    node.nOffset = nPosition - 12;
    WriteInt(node.Head.nType, 5, 2);
    WriteInt(node.Head.nSupernodeNumber, 5, 2);
    WriteInt(node.Head.nNodeNumber, 5, 2);
    WriteInt(0, 8, 2);

    //Next is offset to root. For geo this is 0, for animations this is offset to animation
    if(node.nAnimation == -1){
        WriteInt(0, 6);
        node.Head.nOffsetToRoot = 0;
    }
    else{
        WriteInt(Data.MH.Animations[node.nAnimation].nOffset, 6);
        node.Head.nOffsetToRoot = Data.MH.Animations[node.nAnimation].nOffset;
    }

    //Next is offset to parent.
    if(node.Head.nParentIndex == -1){
        WriteInt(0, 6);
        node.Head.nOffsetToParent = 0;
    }
    else{
        //std::cout<<"Found parent "<<parent.Head.nNodeNumber<<". His offset is "<<parent.nOffset<<".\n";
        WriteInt(GetNodeByNameIndex(node.Head.nParentIndex, node.nAnimation).nOffset, 6);
        node.Head.nOffsetToParent = GetNodeByNameIndex(node.Head.nParentIndex, node.nAnimation).nOffset;
    }

    //Write position
    WriteFloat(node.Head.vPos.fX, 2);
    WriteFloat(node.Head.vPos.fY, 2);
    WriteFloat(node.Head.vPos.fZ, 2);

    //Write orientation - convert first
    WriteFloat(node.Head.oOrient.GetQuaternion().fW, 2);
    WriteFloat(node.Head.oOrient.GetQuaternion().vAxis.fX, 2);
    WriteFloat(node.Head.oOrient.GetQuaternion().vAxis.fY, 2);
    WriteFloat(node.Head.oOrient.GetQuaternion().vAxis.fZ, 2);

    //Children Array
    int PHnOffsetToChildren = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.ChildIndices.size(), 1);
    WriteInt(node.Head.ChildIndices.size(), 1);

    //Controller Array
    int PHnOffsetToControllers = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.Controllers.size(), 1);
    WriteInt(node.Head.Controllers.size(), 1);

    //Controller Data Array
    int PHnOffsetToControllerData = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.ControllerData.size(), 1);
    WriteInt(node.Head.ControllerData.size(), 1);


    //Here comes all the subheader bullshit.
    /// LIGHT HEADER
    int PHnOffsetToUnknownLightArray, PHnOffsetToFlareSizes, PHnOffsetToFlarePositions, PHnOffsetToFlareTextureNames, PHnOffsetToFlareColorShifts;
    if(node.Head.nType & NODE_LIGHT){
        WriteFloat(node.Light.fFlareRadius, 2);
        //PHnOffsetToUnknownLightArray = nPosition;
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        PHnOffsetToFlareSizes = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareSizes.size(), 1);
        WriteInt(node.Light.FlareSizes.size(), 1);
        PHnOffsetToFlarePositions = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlarePositions.size(), 1);
        WriteInt(node.Light.FlarePositions.size(), 1);
        PHnOffsetToFlareColorShifts = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareColorShifts.size(), 1);
        WriteInt(node.Light.FlareColorShifts.size(), 1);
        PHnOffsetToFlareTextureNames = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareTextureNames.size(), 1);
        WriteInt(node.Light.FlareTextureNames.size(), 1);

        WriteInt(node.Light.nLightPriority, 4);
        WriteInt(node.Light.nAmbientOnly, 4);
        WriteInt(node.Light.nDynamicType, 4);
        WriteInt(node.Light.nAffectDynamic, 4);
        WriteInt(node.Light.nShadow, 4);
        WriteInt(node.Light.nFlare, 4);
        WriteInt(node.Light.nFadingLight, 4);
    }

    /// EMITTER HEADER
    if(node.Head.nType & NODE_EMITTER){
        WriteFloat(node.Emitter.fDeadSpace, 2);
        WriteFloat(node.Emitter.fBlastRadius, 2);
        WriteFloat(node.Emitter.fBlastLength, 2);

        WriteInt(node.Emitter.nBranchCount, 1);
        WriteFloat(node.Emitter.fControlPointSmoothing, 2);

        WriteInt(node.Emitter.nxGrid, 4);
        WriteInt(node.Emitter.nyGrid, 4);
        WriteInt(node.Emitter.nSpawnType, 4);

        node.Emitter.cUpdate.resize(32);
        WriteString(node.Emitter.cUpdate, 3);
        node.Emitter.cRender.resize(32);
        WriteString(node.Emitter.cRender, 3);
        node.Emitter.cBlend.resize(32);
        WriteString(node.Emitter.cBlend, 3);
        node.Emitter.cTexture.resize(32);
        WriteString(node.Emitter.cTexture, 3);
        node.Emitter.cChunkName.resize(16);
        WriteString(node.Emitter.cChunkName, 3);

        WriteInt(node.Emitter.nTwosidedTex, 4);
        WriteInt(node.Emitter.nLoop, 4);
        WriteInt(node.Emitter.nRenderOrder, 5, 2);
        WriteInt(node.Emitter.nFrameBlending, 7, 1);

        node.Emitter.cDepthTextureName.resize(32);
        WriteString(node.Emitter.cDepthTextureName, 3);

        WriteInt(node.Emitter.nPadding1, 11, 1);
        WriteInt(node.Emitter.nFlags, 10);
    }

    /// REFERENCE HEADER
    if(node.Head.nType & NODE_REFERENCE){
        node.Reference.sRefModel.resize(32);
        WriteString(node.Reference.sRefModel, 3);
        WriteInt(node.Reference.nReattachable, 4);
    }

    /// MESH HEADER
    int PHnOffsetToFaces, PHnOffsetToIndexCount, PHnOffsetToIndexLocation, PHnOffsetToInvertedCounter, PHnOffsetIntoMdx, PHnOffsetToVertArray;
    if(node.Head.nType & NODE_MESH){
        //Write function pointers
        if(node.Head.nType & NODE_DANGLY){
            node.Mesh.nFunctionPointer0 = FunctionPointer1(FN_PTR_DANGLY);
            node.Mesh.nFunctionPointer1 = FunctionPointer2(FN_PTR_DANGLY);
        }
        else if(node.Head.nType & NODE_SKIN){
            node.Mesh.nFunctionPointer0 = FunctionPointer1(FN_PTR_SKIN);
            node.Mesh.nFunctionPointer1 = FunctionPointer2(FN_PTR_SKIN);
        }
        else{
            node.Mesh.nFunctionPointer0 = FunctionPointer1(FN_PTR_MESH);
            node.Mesh.nFunctionPointer1 = FunctionPointer2(FN_PTR_MESH);
        }
        WriteInt(node.Mesh.nFunctionPointer0, 9);
        WriteInt(node.Mesh.nFunctionPointer1, 9);
        PHnOffsetToFaces = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Mesh.Faces.size(), 1);
        WriteInt(node.Mesh.Faces.size(), 1);

        //Tons of floats
        WriteFloat(node.Mesh.vBBmin.fX, 2);
        WriteFloat(node.Mesh.vBBmin.fY, 2);
        WriteFloat(node.Mesh.vBBmin.fZ, 2);
        WriteFloat(node.Mesh.vBBmax.fX, 2);
        WriteFloat(node.Mesh.vBBmax.fY, 2);
        WriteFloat(node.Mesh.vBBmax.fZ, 2);
        WriteFloat(node.Mesh.fRadius, 2);
        WriteFloat(node.Mesh.vAverage.fX, 2);
        WriteFloat(node.Mesh.vAverage.fY, 2);
        WriteFloat(node.Mesh.vAverage.fZ, 2);
        WriteFloat(node.Mesh.fDiffuse.fR, 2);
        WriteFloat(node.Mesh.fDiffuse.fG, 2);
        WriteFloat(node.Mesh.fDiffuse.fB, 2);
        WriteFloat(node.Mesh.fAmbient.fR, 2);
        WriteFloat(node.Mesh.fAmbient.fG, 2);
        WriteFloat(node.Mesh.fAmbient.fB, 2);

        WriteInt(node.Mesh.nTransparencyHint, 4);

        node.Mesh.cTexture1.resize(32);
        WriteString(node.Mesh.cTexture1, 3);
        node.Mesh.cTexture2.resize(32);
        WriteString(node.Mesh.cTexture2, 3);
        node.Mesh.cTexture3.resize(12);
        WriteString(node.Mesh.cTexture3, 3);
        node.Mesh.cTexture4.resize(12);
        WriteString(node.Mesh.cTexture4, 3);

        PHnOffsetToIndexCount = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(1, 1);
        WriteInt(1, 1);
        PHnOffsetToIndexLocation = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(1, 1);
        WriteInt(1, 1);
        PHnOffsetToInvertedCounter = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(1, 1);
        WriteInt(1, 1);

        //Write unknown -1 -1 0
        WriteInt(-1, 8);
        WriteInt(-1, 8);
        WriteInt(0, 8);

        WriteByte(node.Mesh.nSaberUnknown1, 10);
        WriteByte(node.Mesh.nSaberUnknown2, 10);
        WriteByte(node.Mesh.nSaberUnknown3, 10);
        WriteByte(node.Mesh.nSaberUnknown4, 10);
        WriteByte(node.Mesh.nSaberUnknown5, 10);
        WriteByte(node.Mesh.nSaberUnknown6, 10);
        WriteByte(node.Mesh.nSaberUnknown7, 10);
        WriteByte(node.Mesh.nSaberUnknown8, 10);

        //animated UV stuff
        WriteInt(node.Mesh.nAnimateUV, 4);
        WriteFloat(node.Mesh.fUVDirectionX, 2);
        WriteFloat(node.Mesh.fUVDirectionY, 2);
        WriteFloat(node.Mesh.fUVJitter, 2);
        WriteFloat(node.Mesh.fUVJitterSpeed, 2);

        //MDX stuff
        //Take care of the mdx pointer stuff. The only thing we need set is the bitmap, which should be done on import.
        int nMDXsize = 0;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            node.Mesh.nOffsetToMdxVertex = nMDXsize;
            nMDXsize += 12;
        }
        else node.Mesh.nOffsetToMdxVertex = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
            node.Mesh.nOffsetToMdxNormal = nMDXsize;
            if(bXbox) nMDXsize += 4;
            else nMDXsize += 12;
        }
        else node.Mesh.nOffsetToMdxNormal = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
            node.Mesh.nOffsetToMdxUV1 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV1 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
            node.Mesh.nOffsetToMdxUV2 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV2 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
            node.Mesh.nOffsetToMdxUV3 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV3 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
            node.Mesh.nOffsetToMdxUV4 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV4 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
            node.Mesh.nOffsetToMdxTangent1 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent1 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
            node.Mesh.nOffsetToMdxTangent2 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent2 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
            node.Mesh.nOffsetToMdxTangent3 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent3 = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
            node.Mesh.nOffsetToMdxTangent4 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent4 = -1;
        if(node.Head.nType & NODE_SKIN){
            node.Skin.nOffsetToMdxWeightValues = nMDXsize;
            nMDXsize += 16;
            node.Skin.nOffsetToMdxBoneIndices = nMDXsize;
            nMDXsize += 16;
        }
        node.Mesh.nMdxDataSize = nMDXsize;
        node.Mesh.nOffsetToMdxColor = -1;

        WriteInt(node.Mesh.nMdxDataSize, 1);
        WriteInt(node.Mesh.nMdxDataBitmap, 4);
        WriteInt(node.Mesh.nOffsetToMdxVertex, 6);
        WriteInt(node.Mesh.nOffsetToMdxNormal, 6);
        WriteInt(node.Mesh.nOffsetToMdxColor, 6);
        WriteInt(node.Mesh.nOffsetToMdxUV1, 6);
        WriteInt(node.Mesh.nOffsetToMdxUV2, 6);
        WriteInt(node.Mesh.nOffsetToMdxUV3, 6);
        WriteInt(node.Mesh.nOffsetToMdxUV4, 6);
        WriteInt(node.Mesh.nOffsetToMdxTangent1, 6);
        WriteInt(node.Mesh.nOffsetToMdxTangent2, 6);
        WriteInt(node.Mesh.nOffsetToMdxTangent3, 6);
        WriteInt(node.Mesh.nOffsetToMdxTangent4, 6);

        node.Mesh.nNumberOfVerts = node.Mesh.Vertices.size();
        WriteInt(node.Mesh.nNumberOfVerts, 1, 2);
        WriteInt(node.Mesh.nTextureNumber, 5, 2);

        WriteByte(node.Mesh.nHasLightmap, 7);
        WriteByte(node.Mesh.nRotateTexture, 7);
        WriteByte(node.Mesh.nBackgroundGeometry, 7);
        WriteByte(node.Mesh.nShadow, 7);
        WriteByte(node.Mesh.nBeaming, 7);
        WriteByte(node.Mesh.nRender, 7);

        if(bK2) WriteByte(node.Mesh.nDirtEnabled, 7);
        if(bK2) WriteByte(node.Mesh.nPadding1, 11);
        if(bK2) WriteInt(node.Mesh.nDirtTexture, 5, 2);
        if(bK2) WriteInt(node.Mesh.nDirtCoordSpace, 5, 2);
        if(bK2) WriteByte(node.Mesh.nHideInHolograms, 7);
        if(bK2) WriteByte(node.Mesh.nPadding2, 11);

        WriteInt(node.Mesh.nPadding3, 11, 2);

        WriteFloat(node.Mesh.fTotalArea, 2);
        WriteInt(0, 8);

        PHnOffsetIntoMdx = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        if(!bXbox){
            PHnOffsetToVertArray = nPosition;
            WriteInt(0xFFFFFFFF, 6);
        }
    }

    /// SKIN HEADER
    int PHnOffsetToBonemap, PHnOffsetToQBones, PHnOffsetToTBones, PHnOffsetToArray8;
    if(node.Head.nType & NODE_SKIN){
        WriteInt(0, 8); //Unknown int32
        WriteInt(0, 8); //Unknown int32
        WriteInt(0, 8); //Unknown int32
        WriteInt(node.Skin.nOffsetToMdxWeightValues, 6);
        WriteInt(node.Skin.nOffsetToMdxBoneIndices, 6);
        PHnOffsetToBonemap = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.Bones.size(), 1);
        PHnOffsetToQBones = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.Bones.size(), 1);
        WriteInt(node.Skin.Bones.size(), 1);
        PHnOffsetToTBones = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.Bones.size(), 1);
        WriteInt(node.Skin.Bones.size(), 1);
        PHnOffsetToArray8 = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.Bones.size(), 1);
        WriteInt(node.Skin.Bones.size(), 1);
        for(int a = 0; a < 18; a++){
            WriteInt(node.Skin.nBoneIndices[a], 5, 2);
        }
    }

    /// DANGLY HEADER
    int PHnOffsetToConstraints, PHnOffsetToData2;
    if(node.Head.nType & NODE_DANGLY){
        PHnOffsetToConstraints = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Dangly.Constraints.size(), 1);
        WriteInt(node.Dangly.Constraints.size(), 1);
        WriteFloat(node.Dangly.fDisplacement, 2);
        WriteFloat(node.Dangly.fTightness, 2);
        WriteFloat(node.Dangly.fPeriod, 2);
        PHnOffsetToData2 = nPosition;
        WriteInt(0xFFFFFFFF, 6);

    }

    /// AABB HEADER
    int PHnOffsetToAabb;
    if(node.Head.nType & NODE_AABB){
        PHnOffsetToAabb = nPosition;
        WriteInt(0xFFFFFFFF, 6);
    }

    /// SABER HEADER
    int PHnOffsetToSaberVerts, PHnOffsetToSaberUVs, PHnOffsetToSaberNormals;
    if(node.Head.nType & NODE_SABER){
        PHnOffsetToSaberVerts = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToSaberNormals = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToSaberUVs = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Saber.nInvCount1, 4);
        WriteInt(node.Saber.nInvCount2, 4);
    }

    //now comes all the data in reversed order
    /// SABER DATA
    if(node.Head.nType & NODE_SABER){
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberVerts, node.Saber.nOffsetToSaberVerts);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].vVertex.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberNormals, node.Saber.nOffsetToSaberNormals);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].vNormal.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberUVs, node.Saber.nOffsetToSaberUVs);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].vUV1.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vUV1.fY, 2);
        }

    }

    /// AABB DATA
    if(node.Head.nType & NODE_AABB){
        WriteIntToPH(nPosition - 12, PHnOffsetToAabb, node.Walkmesh.nOffsetToAabb);
        WriteAabb(node.Walkmesh.RootAabb);
    }

    /// DANGLY DATA
    if(node.Head.nType & NODE_DANGLY){
        node.Dangly.ConstraintArray.ResetToSize(node.Dangly.Constraints.size());
        if(node.Dangly.Constraints.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToConstraints, node.Dangly.ConstraintArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToConstraints, node.Dangly.ConstraintArray.nOffset);
        for(int d = 0; d < node.Dangly.Constraints.size(); d++){
            WriteFloat(node.Dangly.Constraints.at(d), 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToData2, node.Dangly.nOffsetToData2);
        for(int d = 0; d < node.Dangly.Data2.size(); d++){
            WriteFloat(node.Dangly.Data2.at(d).fX, 2);
            WriteFloat(node.Dangly.Data2.at(d).fY, 2);
            WriteFloat(node.Dangly.Data2.at(d).fZ, 2);
        }
    }

    /// SKIN DATA
    if(node.Head.nType & NODE_SKIN){
        node.Skin.nNumberOfBonemap = node.Skin.Bones.size();
        if(node.Skin.Bones.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToBonemap, node.Skin.nOffsetToBonemap);
        else WriteIntToPH(0, PHnOffsetToBonemap, node.Skin.nOffsetToBonemap);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            if(bXbox) WriteInt(node.Skin.Bones.at(d).nBonemap, 5, 2);
            else WriteFloat( (double) node.Skin.Bones.at(d).nBonemap, 2);
        }
        node.Skin.QBoneArray.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToQBones, node.Skin.QBoneArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToQBones, node.Skin.QBoneArray.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).QBone.GetQuaternion().fW, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fX, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fY, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fZ, 2);
        }
        node.Skin.TBoneArray.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToTBones, node.Skin.TBoneArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToTBones, node.Skin.TBoneArray.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).TBone.fX, 2);
            WriteFloat(node.Skin.Bones.at(d).TBone.fY, 2);
            WriteFloat(node.Skin.Bones.at(d).TBone.fZ, 2);
        }
        node.Skin.Array8Array.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToArray8, node.Skin.Array8Array.nOffset);
        else WriteIntToPH(0, PHnOffsetToArray8, node.Skin.Array8Array.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteInt(0, 11); //This array is not in use, might as well fill it with zeros
        }
    }

    /// MDX DATA

    if(node.Head.nType & NODE_MESH && !(node.Head.nType & NODE_SABER)){
        WriteIntToPH(Mdx->nPosition, PHnOffsetIntoMdx, node.Mesh.nOffsetIntoMdx);
        for(int d = 0; d < node.Mesh.nNumberOfVerts; d++){
            Vertex & vert = node.Mesh.Vertices.at(d);
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                Mdx->WriteFloat(vert.MDXData.vVertex.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vVertex.fY, 2);
                Mdx->WriteFloat(vert.MDXData.vVertex.fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
                if(!bXbox){
                    Mdx->WriteFloat(vert.MDXData.vNormal.fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vNormal.fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vNormal.fZ, 2);
                }
                else Mdx->WriteInt(CompressVector(vert.MDXData.vNormal), 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                Mdx->WriteFloat(vert.MDXData.vUV1.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV1.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
                Mdx->WriteFloat(vert.MDXData.vUV2.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV2.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
                Mdx->WriteFloat(vert.MDXData.vUV3.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV3.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
                Mdx->WriteFloat(vert.MDXData.vUV4.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV4.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                if(!bXbox){
                    Mdx->WriteFloat(vert.MDXData.vTangent1[0].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[0].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[0].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[1].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[1].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[1].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[2].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[2].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent1[2].fZ, 2);
                }
                else{
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent1[0]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent1[1]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent1[2]), 2);
                }
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
                if(!bXbox){
                    Mdx->WriteFloat(vert.MDXData.vTangent2[0].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[0].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[0].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[1].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[1].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[1].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[2].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[2].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent2[2].fZ, 2);
                }
                else{
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent2[0]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent2[1]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent2[2]), 2);
                }
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
                if(!bXbox){
                    Mdx->WriteFloat(vert.MDXData.vTangent3[0].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[0].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[0].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[1].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[1].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[1].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[2].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[2].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent3[2].fZ, 2);
                }
                else{
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent3[0]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent3[1]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent3[2]), 2);
                }
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
                if(!bXbox){
                    Mdx->WriteFloat(vert.MDXData.vTangent4[0].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[0].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[0].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[1].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[1].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[1].fZ, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[2].fX, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[2].fY, 2);
                    Mdx->WriteFloat(vert.MDXData.vTangent4[2].fZ, 2);
                }
                else{
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent4[0]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent4[1]), 2);
                    Mdx->WriteInt(CompressVector(vert.MDXData.vTangent4[2]), 2);
                }
            }
            if(node.Head.nType & NODE_SKIN){
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[0], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[1], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[2], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[3], 2);
                if(bXbox){
                    Mdx->WriteInt(vert.MDXData.Weights.nWeightIndex[0], 5, 2);
                    Mdx->WriteInt(vert.MDXData.Weights.nWeightIndex[1], 5, 2);
                    Mdx->WriteInt(vert.MDXData.Weights.nWeightIndex[2], 5, 2);
                    Mdx->WriteInt(vert.MDXData.Weights.nWeightIndex[3], 5, 2);
                }
                else{
                    Mdx->WriteFloat(vert.MDXData.Weights.nWeightIndex[0], 2);
                    Mdx->WriteFloat(vert.MDXData.Weights.nWeightIndex[1], 2);
                    Mdx->WriteFloat(vert.MDXData.Weights.nWeightIndex[2], 2);
                    Mdx->WriteFloat(vert.MDXData.Weights.nWeightIndex[3], 2);
                }
            }
        }

        /// Also write the extra empty vert data
        node.Mesh.MDXData.nNodeNumber = node.Head.nNodeNumber;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            if(node.Head.nType & NODE_SKIN) node.Mesh.MDXData.vVertex.Set(1000000.0, 1000000.0, 1000000.0);
            else node.Mesh.MDXData.vVertex.Set(10000000.0, 10000000.0, 10000000.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
            node.Mesh.MDXData.vNormal.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
            node.Mesh.MDXData.vUV1.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV1.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV1.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
            node.Mesh.MDXData.vUV2.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV2.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV2.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
            node.Mesh.MDXData.vUV3.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV3.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV3.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
            node.Mesh.MDXData.vUV4.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV4.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV4.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
            node.Mesh.MDXData.vTangent1[0].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent1[1].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent1[2].Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[0].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[0].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[0].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[1].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[1].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[1].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[2].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[2].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent1[2].fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
            node.Mesh.MDXData.vTangent2[0].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent2[1].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent2[2].Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[0].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[0].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[0].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[1].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[1].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[1].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[2].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[2].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent2[2].fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
            node.Mesh.MDXData.vTangent3[0].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent3[1].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent3[2].Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[0].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[0].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[0].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[1].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[1].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[1].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[2].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[2].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent3[2].fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
            node.Mesh.MDXData.vTangent4[0].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent4[1].Set(0.0, 0.0, 0.0);
            node.Mesh.MDXData.vTangent4[2].Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[0].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[0].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[0].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[1].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[1].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[1].fZ, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[2].fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[2].fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vTangent4[2].fZ, 8);
        }
        if(node.Head.nType & NODE_SKIN){
            node.Mesh.MDXData.Weights.fWeightValue[0] = 1.0;
            node.Mesh.MDXData.Weights.fWeightValue[1] = 0.0;
            node.Mesh.MDXData.Weights.fWeightValue[2] = 0.0;
            node.Mesh.MDXData.Weights.fWeightValue[3] = 0.0;
            node.Mesh.MDXData.Weights.nWeightIndex[0] = 0;
            node.Mesh.MDXData.Weights.nWeightIndex[1] = 0;
            node.Mesh.MDXData.Weights.nWeightIndex[2] = 0;
            node.Mesh.MDXData.Weights.nWeightIndex[3] = 0;
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[0], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[1], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[2], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[3], 8);
            if(bXbox){
                Mdx->WriteInt(node.Mesh.MDXData.Weights.nWeightIndex[0], 8, 2);
                Mdx->WriteInt(node.Mesh.MDXData.Weights.nWeightIndex[1], 8, 2);
                Mdx->WriteInt(node.Mesh.MDXData.Weights.nWeightIndex[2], 8, 2);
                Mdx->WriteInt(node.Mesh.MDXData.Weights.nWeightIndex[3], 8, 2);
            }
            else{
                Mdx->WriteFloat(node.Mesh.MDXData.Weights.nWeightIndex[0], 8);
                Mdx->WriteFloat(node.Mesh.MDXData.Weights.nWeightIndex[1], 8);
                Mdx->WriteFloat(node.Mesh.MDXData.Weights.nWeightIndex[2], 8);
                Mdx->WriteFloat(node.Mesh.MDXData.Weights.nWeightIndex[3], 8);
            }
        }
    }

    /// MESH DATA
    if(node.Head.nType & NODE_MESH){
        node.Mesh.FaceArray.ResetToSize(node.Mesh.Faces.size());
        if(node.Mesh.Faces.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToFaces, node.Mesh.FaceArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToFaces, node.Mesh.FaceArray.nOffset);
        for(int d = 0; d < node.Mesh.Faces.size(); d++){
            Face & face = node.Mesh.Faces.at(d);
            WriteFloat(face.vNormal.fX, 2);
            WriteFloat(face.vNormal.fY, 2);
            WriteFloat(face.vNormal.fZ, 2);
            WriteFloat(face.fDistance, 2);
            WriteInt(face.nMaterialID, 4);
            WriteInt(face.nAdjacentFaces[0], 5, 2);
            WriteInt(face.nAdjacentFaces[1], 5, 2);
            WriteInt(face.nAdjacentFaces[2], 5, 2);
            WriteInt(face.nIndexVertex[0], 5, 2);
            WriteInt(face.nIndexVertex[1], 5, 2);
            WriteInt(face.nIndexVertex[2], 5, 2);
        }
        if(node.Head.nType & NODE_SABER){
            node.Mesh.IndexCounterArray.ResetToSize(0);
            WriteIntToPH(0, PHnOffsetToIndexCount, node.Mesh.IndexCounterArray.nOffset);
            WriteIntToPH(nPosition - 12, PHnOffsetToVertArray, node.Mesh.nOffsetToVertArray);
            for(int d = 0; d < node.Mesh.Vertices.size(); d++){
                WriteFloat(node.Mesh.Vertices.at(d).fX, 2);
                WriteFloat(node.Mesh.Vertices.at(d).fY, 2);
                WriteFloat(node.Mesh.Vertices.at(d).fZ, 2);
            }
            node.Mesh.IndexLocationArray.ResetToSize(0);
            WriteIntToPH(0, PHnOffsetToIndexLocation, node.Mesh.IndexLocationArray.nOffset);
            node.Mesh.MeshInvertedCounterArray.ResetToSize(0);
            WriteIntToPH(0, PHnOffsetToInvertedCounter, node.Mesh.MeshInvertedCounterArray.nOffset);
            //WriteIntToPH(0, PHnPointerToIndexLocation, node.Mesh.nVertIndicesLocation);
        }
        else{
            node.Mesh.IndexCounterArray.ResetToSize(1);
            WriteIntToPH(nPosition - 12, PHnOffsetToIndexCount, node.Mesh.IndexCounterArray.nOffset);
            node.Mesh.nVertIndicesCount = node.Mesh.Faces.size() * 3;
            WriteInt(node.Mesh.nVertIndicesCount, 1);
            if(!bXbox){
                WriteIntToPH(nPosition - 12, PHnOffsetToVertArray, node.Mesh.nOffsetToVertArray);
                for(int d = 0; d < node.Mesh.Vertices.size(); d++){
                    WriteFloat(node.Mesh.Vertices.at(d).fX, 2);
                    WriteFloat(node.Mesh.Vertices.at(d).fY, 2);
                    WriteFloat(node.Mesh.Vertices.at(d).fZ, 2);
                }
            }
            node.Mesh.IndexLocationArray.ResetToSize(1);
            WriteIntToPH(nPosition - 12, PHnOffsetToIndexLocation, node.Mesh.IndexLocationArray.nOffset);
            int PHnPointerToIndexLocation = nPosition;
            WriteInt(0xFFFFFFFF, 6);
            node.Mesh.MeshInvertedCounterArray.ResetToSize(1);
            WriteIntToPH(nPosition - 12, PHnOffsetToInvertedCounter, node.Mesh.MeshInvertedCounterArray.nOffset);
            WriteInt(node.Mesh.nMeshInvertedCounter, 4);
            WriteIntToPH(nPosition - 12, PHnPointerToIndexLocation, node.Mesh.nVertIndicesLocation);
            for(int d = 0; d < node.Mesh.VertIndices.size(); d++){
                WriteInt(node.Mesh.VertIndices.at(d).at(0), 5, 2);
                WriteInt(node.Mesh.VertIndices.at(d).at(1), 5, 2);
                WriteInt(node.Mesh.VertIndices.at(d).at(2), 5, 2);
            }
        }
    }

    /// LIGHT DATA
    if(node.Head.nType & NODE_LIGHT){
        //WriteIntToPH(nPosition - 12, PHnOffsetToUnknownLightArray, node.Light.UnknownArray.nOffset);
        node.Light.FlareTextureNameArray.ResetToSize(node.Light.FlareTextureNames.size());
        if(node.Light.FlareTextureNames.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToFlareTextureNames, node.Light.FlareTextureNameArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToFlareTextureNames, node.Light.FlareTextureNameArray.nOffset);
        std::vector<int> PHnTextureNameOffset;
        for(int d = 0; d < node.Light.FlareTextureNames.size(); d++){
            PHnTextureNameOffset.push_back(nPosition);
            WriteInt(0xFFFFFFFF, 6);
        }
        for(int d = 0; d < node.Light.FlareTextureNames.size(); d++){
            WriteIntToPH(nPosition - 12, PHnTextureNameOffset.at(d), node.Light.FlareTextureNames.at(d).nOffset);
            WriteString(node.Light.FlareTextureNames.at(d).sName.c_str(), 3);
            WriteByte(0, 3);
        }
        node.Light.FlareSizeArray.ResetToSize(node.Light.FlareSizes.size());
        if(node.Light.FlareSizes.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToFlareSizes, node.Light.FlareSizeArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToFlareSizes, node.Light.FlareSizeArray.nOffset);
        for(int d = 0; d < node.Light.FlareSizes.size(); d++){
            WriteFloat(node.Light.FlareSizes.at(d), 2);
        }
        node.Light.FlarePositionArray.ResetToSize(node.Light.FlarePositions.size());
        if(node.Light.FlarePositions.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToFlarePositions, node.Light.FlarePositionArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToFlarePositions, node.Light.FlarePositionArray.nOffset);
        for(int d = 0; d < node.Light.FlarePositions.size(); d++){
            WriteFloat(node.Light.FlarePositions.at(d), 2);
        }
        node.Light.FlareColorShiftArray.ResetToSize(node.Light.FlareColorShifts.size());
        if(node.Light.FlareColorShifts.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToFlareColorShifts, node.Light.FlareColorShiftArray.nOffset);
        else WriteIntToPH(0, PHnOffsetToFlareColorShifts, node.Light.FlareColorShiftArray.nOffset);
        for(int d = 0; d < node.Light.FlareColorShifts.size(); d++){
            WriteFloat(node.Light.FlareColorShifts.at(d).fR, 2);
            WriteFloat(node.Light.FlareColorShifts.at(d).fG, 2);
            WriteFloat(node.Light.FlareColorShifts.at(d).fB, 2);
        }
    }
    /// DONE WITH SUBHEADERS

    //We get to the children array
    node.Head.ChildrenArray.ResetToSize(node.Head.Children.size());
    WriteIntToPH(nPosition - 12, PHnOffsetToChildren, node.Head.ChildrenArray.nOffset);
    std::vector<int> PHnOffsetToChild;
    for(int a = 0; a < node.Head.ChildIndices.size(); a++){
        PHnOffsetToChild.push_back(nPosition);
        WriteInt(0xFFFFFFFF, 6);
    }

    for(int a = 0; a < node.Head.ChildIndices.size(); a++){
        WriteIntToPH(nPosition - 12, PHnOffsetToChild.at(a), GetNodeByNameIndex(node.Head.ChildIndices.at(a), node.nAnimation).nOffset);
        WriteNodes(GetNodeByNameIndex(node.Head.ChildIndices.at(a), node.nAnimation));
        //WriteNodes(node.Head.Children.at(a));
    }

    //We get to the controller array
    node.Head.ControllerArray.ResetToSize(node.Head.Controllers.size());
    if(node.Head.Controllers.size() > 0) WriteIntToPH(nPosition-12, PHnOffsetToControllers, node.Head.ControllerArray.nOffset);
    else WriteIntToPH(0, PHnOffsetToControllers, node.Head.ControllerArray.nOffset);
    for(int a = 0; a < node.Head.Controllers.size(); a++){
        Controller & ctrl = node.Head.Controllers.at(a);
        WriteInt(ctrl.nControllerType, 4);
        WriteInt(ctrl.nUnknown2, 10, 2);
        WriteInt(ctrl.nValueCount, 5, 2);
        WriteInt(ctrl.nTimekeyStart, 5, 2);
        WriteInt(ctrl.nDataStart, 5, 2);
        WriteInt(ctrl.nColumnCount, 7, 1);
        WriteInt(ctrl.nPadding[0], 11, 1);
        WriteInt(ctrl.nPadding[1], 11, 1);
        WriteInt(ctrl.nPadding[2], 11, 1);
    }

    //We get to the controller data array
    node.Head.ControllerDataArray.ResetToSize(node.Head.ControllerData.size());
    if(node.Head.ControllerData.size() > 0) WriteIntToPH(nPosition-12, PHnOffsetToControllerData, node.Head.ControllerDataArray.nOffset);
    else WriteIntToPH(0, PHnOffsetToControllerData, node.Head.ControllerDataArray.nOffset);
    for(int a = 0; a < node.Head.ControllerData.size(); a++){
        WriteFloat(node.Head.ControllerData.at(a), 2);
    }

    //Done you are, padawan
}

void BWM::Compile(){
    if(!Bwm) return;
    BWMHeader & data = *Bwm;
    nPosition = 0;
    sBuffer.resize(0);
    bKnown.resize(0);

    WriteString("BWM V1.0", 3);
    WriteInt(data.nType, 4);

    WriteFloat(data.vUse1.fX, 2);
    WriteFloat(data.vUse1.fY, 2);
    WriteFloat(data.vUse1.fZ, 2);
    WriteFloat(data.vUse2.fX, 2);
    WriteFloat(data.vUse2.fY, 2);
    WriteFloat(data.vUse2.fZ, 2);
    WriteFloat(data.vDwk1.fX, 2);
    WriteFloat(data.vDwk1.fY, 2);
    WriteFloat(data.vDwk1.fZ, 2);
    WriteFloat(data.vDwk2.fX, 2);
    WriteFloat(data.vDwk2.fY, 2);
    WriteFloat(data.vDwk2.fZ, 2);

    WriteFloat(data.vPosition.fX, 2);
    WriteFloat(data.vPosition.fY, 2);
    WriteFloat(data.vPosition.fZ, 2);

    data.nNumberOfVerts = data.verts.size();
    WriteInt(data.nNumberOfVerts, 1);
    int nOffsetVerts = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    data.nNumberOfFaces = data.faces.size();
    WriteInt(data.nNumberOfFaces, 1);
    int nOffsetVertIndices = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    int nOffsetMaterials = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    int nOffsetNormals = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    int nOffsetDistances = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    data.nNumberOfAabb = data.aabb.size();
    WriteInt(data.nNumberOfAabb, 1);
    int nOffsetAabb = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    WriteInt(data.nPadding, 10);

    int nCount = 0;
    for(int f = 0; f < data.faces.size(); f++){
        if(data.faces.at(f).nMaterialID != 7) nCount++;
    }
    data.nNumberOfAdjacentFaces = nCount;
    WriteInt(data.nNumberOfAdjacentFaces, 1);
    int nOffsetAdjacent = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    data.nNumberOfEdges = data.edges.size();
    WriteInt(data.nNumberOfEdges, 1);
    int nOffsetEdges = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    data.nNumberOfPerimeters = data.perimeters.size();
    WriteInt(data.nNumberOfPerimeters, 1);
    int nOffsetPerimeters = nPosition;
    WriteInt(0xFFFFFFFF, 6);

    if(data.verts.size() == 0) WriteIntToPH(0, nOffsetVerts, data.nOffsetToVerts);
    else WriteIntToPH(nPosition, nOffsetVerts, data.nOffsetToVerts);
    for(int v = 0; v < data.verts.size(); v++){
        Vector & vert = data.verts.at(v);
        WriteFloat(vert.fX, 2);
        WriteFloat(vert.fY, 2);
        WriteFloat(vert.fZ, 2);
    }

    if(data.faces.size() == 0) WriteIntToPH(0, nOffsetVertIndices, data.nOffsetToIndices);
    else WriteIntToPH(nPosition, nOffsetVertIndices, data.nOffsetToIndices);
    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        WriteInt(face.nIndexVertex.at(0), 4);
        WriteInt(face.nIndexVertex.at(1), 4);
        WriteInt(face.nIndexVertex.at(2), 4);
    }
    if(data.faces.size() == 0) WriteIntToPH(0, nOffsetMaterials, data.nOffsetToMaterials);
    else WriteIntToPH(nPosition, nOffsetMaterials, data.nOffsetToMaterials);
    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        WriteInt(face.nMaterialID, 4);
    }
    if(data.faces.size() == 0) WriteIntToPH(0, nOffsetNormals, data.nOffsetToNormals);
    else WriteIntToPH(nPosition, nOffsetNormals, data.nOffsetToNormals);
    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        WriteFloat(face.vNormal.fX, 2);
        WriteFloat(face.vNormal.fY, 2);
        WriteFloat(face.vNormal.fZ, 2);
    }
    if(data.faces.size() == 0) WriteIntToPH(0, nOffsetDistances, data.nOffsetToDistances);
    else WriteIntToPH(nPosition, nOffsetDistances, data.nOffsetToDistances);
    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        WriteFloat(face.fDistance, 2);
    }

    if(data.aabb.size() == 0) WriteIntToPH(0, nOffsetAabb, data.nOffsetToAabb);
    else WriteIntToPH(nPosition, nOffsetAabb, data.nOffsetToAabb);
    for(int v = 0; v < data.aabb.size(); v++){
        Aabb & aabb = data.aabb.at(v);
        WriteFloat(aabb.vBBmin.fX, 2);
        WriteFloat(aabb.vBBmin.fY, 2);
        WriteFloat(aabb.vBBmin.fZ, 2);
        WriteFloat(aabb.vBBmax.fX, 2);
        WriteFloat(aabb.vBBmax.fY, 2);
        WriteFloat(aabb.vBBmax.fZ, 2);
        WriteInt(aabb.nID, 4);
        WriteInt(aabb.nExtra, 4);
        WriteInt(aabb.nProperty, 4);
        WriteInt(aabb.nChild1, 4);
        WriteInt(aabb.nChild2, 4);
    }

    if(data.faces.size() == 0 || data.nType == 0) WriteIntToPH(0, nOffsetAdjacent, data.nOffsetToAdjacentFaces);
    else WriteIntToPH(nPosition, nOffsetAdjacent, data.nOffsetToAdjacentFaces);
    for(int f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);
        if(face.nMaterialID != 7){
            WriteInt(face.nAdjacentFaces.at(0), 4);
            WriteInt(face.nAdjacentFaces.at(1), 4);
            WriteInt(face.nAdjacentFaces.at(2), 4);
        }
    }
    if(data.edges.size() == 0) WriteIntToPH(0, nOffsetEdges, data.nOffsetToEdges);
    else WriteIntToPH(nPosition, nOffsetEdges, data.nOffsetToEdges);
    for(int f = 0; f < data.edges.size(); f++){
        Edge & edge = data.edges.at(f);
        WriteInt(edge.nIndex, 4);
        WriteInt(edge.nTransition, 4);
    }
    if(data.perimeters.size() == 0) WriteIntToPH(0, nOffsetPerimeters, data.nOffsetToPerimeters);
    else WriteIntToPH(nPosition, nOffsetPerimeters, data.nOffsetToPerimeters);
    for(int f = 0; f < data.perimeters.size(); f++){
        int & perimeter = data.perimeters.at(f);
        WriteInt(perimeter, 4);
    }
    bLoaded = true;
}
