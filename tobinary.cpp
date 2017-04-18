#include "MDL.h"
//#include <fstream>

/**
    Functions:
    MDL::DoCalculations()
    MDL::CalculateMeshData()
    MDL::Compile()
    MDL::WriteAabb()
    MDL::WriteNodes()
/**/

void MDL::DoCalculations(Node & node, int & nMeshCounter){
    if(node.Head.nType & NODE_HAS_SABER){
        //inverted counter
        nMeshCounter++;
        int Quo = nMeshCounter/100;
        int Mod = nMeshCounter%100;
        node.Saber.nInvCount1 = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
        nMeshCounter++;
        Quo = nMeshCounter/100;
        Mod = nMeshCounter%100;
        node.Saber.nInvCount2 = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
    }
    else{
        //inverted counter
        nMeshCounter++;
        int Quo = nMeshCounter/100;
        int Mod = nMeshCounter%100;
        node.Mesh.nMeshInvertedCounter = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);

        if(node.Head.nType & NODE_HAS_SKIN){
            //First, declare our empty location as a starting point
            Vector vPos;
            Quaternion qOrient;

            //Now we need to construct a path by adding all the locations from this node through all its parents to the root
            int nIndex = node.Head.nNameIndex;
            Vector vCurPosition;
            Quaternion qCurOrientation;
            while(nIndex != -1){
                Node & curnode = GetNodeByNameIndex(nIndex);

                //Construct base location
                Location locNode = curnode.GetLocation();
                vCurPosition = locNode.vPosition * -1.0;
                qCurOrientation = locNode.oOrientation.GetQuaternion();
                qCurOrientation = qCurOrientation.reverse();
                vCurPosition.Rotate(qCurOrientation);

                //Add parent location to main location
                vPos += vCurPosition;
                qOrient *= qCurOrientation;
                //On the first round, because loc is initialized with the identity orientation, locNode orientation is simply copied

                nIndex = curnode.Head.nParentIndex;
            }
            //We now have a location loc, going from our current node to the root.
            //Now we need to add to that a similar kind of path of every node in the model

            //Loop through all the nodes, and do a similar operation to get the path for every node, then adding it to loc
            for(int n = 0; n < FH->MH.ArrayOfNodes.size(); n++){
                Vector vRecord = vPos; //Make copy
                Quaternion qRecord = qOrient; //Make copy
                Node & curnode = FH->MH.ArrayOfNodes.at(n);

                nIndex = curnode.Head.nNameIndex;
                std::vector<int> Indexes;
                //The price we have to pay for not going recursive
                while(nIndex != -1){
                    Indexes.push_back(nIndex);
                    nIndex = GetNodeByNameIndex(nIndex).Head.nParentIndex;
                }
                //std::cout<<"Our Indexes size is: "<<Indexes.size()<<".\n";
                for(int a = Indexes.size() - 1; a >= 0; a--){
                    Node & curnode2 = GetNodeByNameIndex(Indexes.at(a));
                    Location locNode = curnode2.GetLocation();
                    vCurPosition = locNode.vPosition;
                    qCurOrientation = locNode.oOrientation.GetQuaternion();
                    vCurPosition.Rotate(qRecord); //Note: rotating with the Record rotation!
                    vRecord += vCurPosition;
                    qRecord *= qCurOrientation;
                }

                //Oops! Forgot the last part in MDLOps, need to rotate the vector again.
                vRecord *= -1.0;
                qRecord = qRecord.reverse();
                vRecord.Rotate(qRecord);

                //By now, lRecord holds the base loc + the path for this node. This should now be exactly what gets written in T and Q Bones!
                node.Skin.Bones.at(n).TBone = vRecord;
                node.Skin.Bones.at(n).QBone.SetQuaternion(qRecord);
            }

        }

        //Take care of the mdx pointer stuff. The only thing we need set is the bitmap, which should be done on import.
        int nMDXsize = 0;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            node.Mesh.nOffsetToVerticesInMDX = nMDXsize;
            nMDXsize += 12;
        }
        else node.Mesh.nOffsetToVerticesInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
            node.Mesh.nOffsetToNormalsInMDX = nMDXsize;
            nMDXsize += 12;
        }
        else node.Mesh.nOffsetToNormalsInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            node.Mesh.nOffsetToUVsInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToUVsInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            node.Mesh.nOffsetToUV2sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToUV2sInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            node.Mesh.nOffsetToUV3sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToUV3sInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            node.Mesh.nOffsetToUV4sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToUV4sInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
            node.Mesh.nOffsetToTangentSpaceInMDX = nMDXsize;
            nMDXsize += 36;
        }
        else node.Mesh.nOffsetToTangentSpaceInMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
            node.Mesh.nOffsetToTangentSpace2InMDX = nMDXsize;
            nMDXsize += 36;
        }
        else node.Mesh.nOffsetToTangentSpace2InMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
            node.Mesh.nOffsetToTangentSpace3InMDX = nMDXsize;
            nMDXsize += 36;
        }
        else node.Mesh.nOffsetToTangentSpace3InMDX = -1;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
            node.Mesh.nOffsetToTangentSpace4InMDX = nMDXsize;
            nMDXsize += 36;
        }
        else node.Mesh.nOffsetToTangentSpace4InMDX = -1;
        if(node.Head.nType & NODE_HAS_SKIN){
            node.Skin.nOffsetToWeightValuesInMDX = nMDXsize;
            nMDXsize += 16;
            node.Skin.nOffsetToBoneIndexInMDX = nMDXsize;
            nMDXsize += 16;
        }
        node.Mesh.nMdxDataSize = nMDXsize;
        node.Mesh.nOffsetToUnknownInMDX = -1;
    }
}

void MDL::CalculateMeshData(){
    std::cout<<"Calculating mesh data...\n";
    FileHeader & Data = *FH;

    /// PART 1 ///
    /// Create patches through linked faces
    //This will take a while and needs to be optimized for speed, anything that can be taken out of it, should be
    CreatePatches();

    /// PART 2 ///
    /// Do the necessary mesh calculations
    /// Mesh: inverted counter, MDX data offsets
    /// Skin: MDX data offsets, T-bones, Q-bones
    /// Saber: inverted counter
    Report("Calculating mesh data...");
    int nMeshCounter = 0;
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);
        if(node.Head.nType & NODE_HAS_MESH) DoCalculations(node, nMeshCounter);
    }

    /// PART 3 ///
    /// Calculate vertex normals and vertex tangent space vectors
    Report("Calculating vertex normals and vertex tangent space vectors...");
    for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
        int nPatchCount = Data.MH.PatchArrayPointers.at(pg).size();
        for(int p = 0; p < nPatchCount; p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            Vertex & vert = GetNodeByNameIndex(patch.nNameIndex).Mesh.Vertices.at(patch.nVertex);

            for(int p2 = 0; p2 < nPatchCount; p2++){
                Patch & patch2 = Data.MH.PatchArrayPointers.at(pg).at(p2);
                if(patch2.nSmoothingGroups & patch.nSmoothingGroups){
                    for(int f = 0; f < patch2.FaceIndices.size(); f++){
                        Face & face = GetNodeByNameIndex(patch2.nNameIndex).Mesh.Faces.at(patch2.FaceIndices.at(f));
                        Vertex & v1 = GetNodeByNameIndex(patch2.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[0]);
                        Vertex & v2 = GetNodeByNameIndex(patch2.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[1]);
                        Vertex & v3 = GetNodeByNameIndex(patch2.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[2]);
                        Vector Edge1 = v2 - v1;
                        Vector Edge2 = v3 - v1;
                        Vector Edge3 = v3 - v2;

                        Vector vAdd = face.vNormal;
                        if(bSmoothAreaWeighting) vAdd *= face.fArea;
                        if(bSmoothAngleWeighting){
                            if(patch.nVertex == face.nIndexVertex[0]){
                                vAdd *= Angle(Edge1, Edge2);
                            }
                            else if(patch.nVertex == face.nIndexVertex[1]){
                                vAdd *= Angle(Edge1, Edge3);
                            }
                            else if(patch.nVertex == face.nIndexVertex[2]){
                                vAdd *= Angle(Edge2, Edge3);
                            }
                        }

                        vert.MDXData.vNormal += vAdd;
                        vert.MDXData.vTangent1[0] += face.vBitangent;
                        vert.MDXData.vTangent1[1] += face.vTangent;
                        vert.MDXData.vTangent1[2] += (face.vBitangent / face.vTangent);
                    }
                }
            }
            vert.MDXData.vNormal.Normalize();
            vert.MDXData.vTangent1[0].Normalize();
            vert.MDXData.vTangent1[1].Normalize();
            vert.MDXData.vTangent1[2].Normalize();
        }
    }

    /// No need for the patches anymore, get rid of them
    Data.MH.PatchArrayPointers.clear();
    Data.MH.PatchArrayPointers.shrink_to_fit();

    /// DONE ///
    std::cout<<"Done calculating mesh data...\n";
}

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
    if(bK2){
        Data->MH.GH.nFunctionPointer0 = K2_FUNCTION_POINTER_0;
        Data->MH.GH.nFunctionPointer1 = K2_FUNCTION_POINTER_1;
    }
    else{
        Data->MH.GH.nFunctionPointer0 = K1_FUNCTION_POINTER_0;
        Data->MH.GH.nFunctionPointer1 = K1_FUNCTION_POINTER_1;
    }
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
    WriteInt(Data->MH.nUnknown2, 10);

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
        if(bK2){
            anim.nFunctionPointer0 = 4284816;
            anim.nFunctionPointer1 = 4522928;
        }
        else{
            anim.nFunctionPointer0 = 4273392;
            anim.nFunctionPointer1 = 4451552;
        }
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

        int PHnOffsetToSoundArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(anim.Sounds.size(), 1);
        WriteInt(anim.Sounds.size(), 1);
        WriteInt(0, 8); //Unknown constant
        if(anim.Sounds.size() > 0){
            anim.SoundArray.ResetToSize(anim.Sounds.size());
            WriteIntToPH(nPosition - 12, PHnOffsetToSoundArray, anim.SoundArray.nOffset);
            for(int b = 0; b < anim.Sounds.size(); b++){
                WriteFloat(anim.Sounds[b].fTime, 2);
                anim.Sounds[b].sName.resize(32);
                WriteString(anim.Sounds[b].sName, 3);
            }
        }
        else WriteIntToPH(0, PHnOffsetToSoundArray, Data->MH.Animations[c].SoundArray.nOffset);

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
    WriteInt(node.Head.nID1, 5, 2);
    WriteInt(node.Head.nNameIndex, 5, 2);
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
        //std::cout<<"Found parent "<<parent.Head.nNameIndex<<". His offset is "<<parent.nOffset<<".\n";
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
    if(node.Head.nType & NODE_HAS_LIGHT){
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
    if(node.Head.nType & NODE_HAS_EMITTER){
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
        WriteInt(node.Emitter.nUnknown1, 10, 2);
        WriteInt(node.Emitter.nFrameBlending, 7, 1);

        node.Emitter.cDepthTextureName.resize(32);
        WriteString(node.Emitter.cDepthTextureName, 3);

        WriteInt(node.Emitter.nUnknown2, 10, 1);
        WriteInt(node.Emitter.nFlags, 10);
    }

    /// MESH HEADER
    int PHnOffsetToFaces, PHnOffsetToIndexCount, PHnOffsetToIndexLocation, PHnOffsetToInvertedCounter, PHnOffsetIntoMdx, PHnOffsetToVertArray;
    if(node.Head.nType & NODE_HAS_MESH){
        //Write function pointers
        if(node.Head.nType & NODE_HAS_DANGLY){
            if(bK2) node.Mesh.nFunctionPointer0 = 4216864;
            else node.Mesh.nFunctionPointer0 = 4216640;
            if(bK2) node.Mesh.nFunctionPointer1 = 4216848;
            else node.Mesh.nFunctionPointer1 = 4216624;
        }
        else if(node.Head.nType & NODE_HAS_SKIN){
            if(bK2) node.Mesh.nFunctionPointer0 = 4216816;
            else node.Mesh.nFunctionPointer0 = 4216592;
            if(bK2) node.Mesh.nFunctionPointer1 = 4216832;
            else node.Mesh.nFunctionPointer1 = 4216608;
        }
        else{
            if(bK2) node.Mesh.nFunctionPointer0 = 4216880;
            else node.Mesh.nFunctionPointer0 = 4216656;
            if(bK2) node.Mesh.nFunctionPointer1 = 4216896;
            else node.Mesh.nFunctionPointer1 = 4216672;
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
        WriteInt(node.Mesh.nMdxDataSize, 1);
        WriteInt(node.Mesh.nMdxDataBitmap, 4);
        WriteInt(node.Mesh.nOffsetToVerticesInMDX, 6);
        WriteInt(node.Mesh.nOffsetToNormalsInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUnknownInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUVsInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUV2sInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUV3sInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUV4sInMDX, 6);
        WriteInt(node.Mesh.nOffsetToTangentSpaceInMDX, 6);
        WriteInt(node.Mesh.nOffsetToTangentSpace2InMDX, 6);
        WriteInt(node.Mesh.nOffsetToTangentSpace3InMDX, 6);
        WriteInt(node.Mesh.nOffsetToTangentSpace4InMDX, 6);

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
        if(bK2) WriteByte(node.Mesh.nUnknown1, 10);
        if(bK2) WriteInt(node.Mesh.nDirtTexture, 5, 2);
        if(bK2) WriteInt(node.Mesh.nDirtCoordSpace, 5, 2);

        if(bK2) WriteByte(node.Mesh.nHideInHolograms, 7);
        if(bK2) WriteByte(node.Mesh.nUnknown2, 10);

        WriteInt(node.Mesh.nUnknown4, 10, 2);

        WriteFloat(node.Mesh.fTotalArea, 2);
        WriteInt(0, 8);

        PHnOffsetIntoMdx = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToVertArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
    }

    /// SKIN HEADER
    int PHnOffsetToBonemap, PHnOffsetToQBones, PHnOffsetToTBones, PHnOffsetToArray8;
    if(node.Head.nType & NODE_HAS_SKIN){
        WriteInt(0, 8); //Unknown int32
        WriteInt(0, 8); //Unknown int32
        WriteInt(0, 8); //Unknown int32
        WriteInt(node.Skin.nOffsetToWeightValuesInMDX, 6);
        WriteInt(node.Skin.nOffsetToBoneIndexInMDX, 6);
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
            WriteInt(node.Skin.nBoneIndexes[a], 5, 2);
        }
    }

    /// DANGLY HEADER
    int PHnOffsetToConstraints, PHnOffsetToData2;
    if(node.Head.nType & NODE_HAS_DANGLY){
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
    if(node.Head.nType & NODE_HAS_AABB){
        PHnOffsetToAabb = nPosition;
        WriteInt(0xFFFFFFFF, 6);
    }

    /// SABER HEADER
    int PHnOffsetToSaberData1, PHnOffsetToSaberData2, PHnOffsetToSaberData3;
    if(node.Head.nType & NODE_HAS_SABER){
        PHnOffsetToSaberData1 = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToSaberData3 = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToSaberData2 = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Saber.nInvCount1, 4);
        WriteInt(node.Saber.nInvCount2, 4);
    }

    //now comes all the data in reversed order
    /// SABER DATA
    if(node.Head.nType & NODE_HAS_SABER){
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData1, node.Saber.nOffsetToSaberData1);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            node.Saber.SaberData[d].nOffsetVertex = nPosition;
            WriteFloat(node.Saber.SaberData[d].vVertex.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData3, node.Saber.nOffsetToSaberData3);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            node.Saber.SaberData[d].nOffsetNormal = nPosition;
            WriteFloat(node.Saber.SaberData[d].vNormal.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData2, node.Saber.nOffsetToSaberData2);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            node.Saber.SaberData[d].nOffsetUV = nPosition;
            WriteFloat(node.Saber.SaberData[d].vUV.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vUV.fY, 2);
        }

    }

    /// AABB DATA
    if(node.Head.nType & NODE_HAS_AABB){
        WriteIntToPH(nPosition - 12, PHnOffsetToAabb, node.Walkmesh.nOffsetToAabb);
        WriteAabb(node.Walkmesh.RootAabb);
    }

    /// DANGLY DATA
    if(node.Head.nType & NODE_HAS_DANGLY){
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
    if(node.Head.nType & NODE_HAS_SKIN){
        node.Skin.nNumberOfBonemap = node.Skin.Bones.size();
        if(node.Skin.Bones.size() > 0) WriteIntToPH(nPosition - 12, PHnOffsetToBonemap, node.Skin.nOffsetToBonemap);
        else WriteIntToPH(0, PHnOffsetToBonemap, node.Skin.nOffsetToBonemap);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).fBonemap, 2);
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
            WriteInt(0, 9); //This array is not in use, might as well fill it with zeros
        }
    }

    /// MDX DATA

    if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_SABER)){
        WriteIntToPH(Mdx->nPosition, PHnOffsetIntoMdx, node.Mesh.nOffsetIntoMdx);
        for(int d = 0; d < node.Mesh.nNumberOfVerts; d++){
            Vertex & vert = node.Mesh.Vertices.at(d);
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                Mdx->WriteFloat(vert.MDXData.vVertex.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vVertex.fY, 2);
                Mdx->WriteFloat(vert.MDXData.vVertex.fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
                Mdx->WriteFloat(vert.MDXData.vNormal.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vNormal.fY, 2);
                Mdx->WriteFloat(vert.MDXData.vNormal.fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
                Mdx->WriteFloat(vert.MDXData.vUV1.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV1.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
                Mdx->WriteFloat(vert.MDXData.vUV2.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV2.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
                Mdx->WriteFloat(vert.MDXData.vUV3.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV3.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
                Mdx->WriteFloat(vert.MDXData.vUV4.fX, 2);
                Mdx->WriteFloat(vert.MDXData.vUV4.fY, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
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
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
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
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
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
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
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
            if(node.Head.nType & NODE_HAS_SKIN){
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[0], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[1], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[2], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightValue[3], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightIndex[0], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightIndex[1], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightIndex[2], 2);
                Mdx->WriteFloat(vert.MDXData.Weights.fWeightIndex[3], 2);
            }
        }

        /// Also write the extra empty vert data
        node.Mesh.MDXData.nNameIndex = node.Head.nNameIndex;
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            if(node.Head.nType & NODE_HAS_SKIN) node.Mesh.MDXData.vVertex.Set(1000000.0, 1000000.0, 1000000.0);
            else node.Mesh.MDXData.vVertex.Set(10000000.0, 10000000.0, 10000000.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vVertex.fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
            node.Mesh.MDXData.vNormal.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fY, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vNormal.fZ, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            node.Mesh.MDXData.vUV1.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV1.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV1.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            node.Mesh.MDXData.vUV2.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV2.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV2.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            node.Mesh.MDXData.vUV3.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV3.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV3.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            node.Mesh.MDXData.vUV4.Set(0.0, 0.0, 0.0);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV4.fX, 8);
            Mdx->WriteFloat(node.Mesh.MDXData.vUV4.fY, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
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
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
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
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
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
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
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
        if(node.Head.nType & NODE_HAS_SKIN){
            node.Mesh.MDXData.Weights.fWeightValue[0] = 1.0;
            node.Mesh.MDXData.Weights.fWeightValue[1] = 0.0;
            node.Mesh.MDXData.Weights.fWeightValue[2] = 0.0;
            node.Mesh.MDXData.Weights.fWeightValue[3] = 0.0;
            node.Mesh.MDXData.Weights.fWeightIndex[0] = 0.0;
            node.Mesh.MDXData.Weights.fWeightIndex[1] = 0.0;
            node.Mesh.MDXData.Weights.fWeightIndex[2] = 0.0;
            node.Mesh.MDXData.Weights.fWeightIndex[3] = 0.0;
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[0], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[1], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[2], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightValue[3], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightIndex[0], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightIndex[1], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightIndex[2], 8);
            Mdx->WriteFloat(node.Mesh.MDXData.Weights.fWeightIndex[3], 8);
        }
    }

    /// MESH DATA
    if(node.Head.nType & NODE_HAS_MESH){
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
        if(node.Head.nType & NODE_HAS_SABER){
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
            WriteIntToPH(nPosition - 12, PHnOffsetToVertArray, node.Mesh.nOffsetToVertArray);
            for(int d = 0; d < node.Mesh.Vertices.size(); d++){
                WriteFloat(node.Mesh.Vertices.at(d).fX, 2);
                WriteFloat(node.Mesh.Vertices.at(d).fY, 2);
                WriteFloat(node.Mesh.Vertices.at(d).fZ, 2);
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
                WriteInt(node.Mesh.VertIndices.at(d).nValues[0], 5, 2);
                WriteInt(node.Mesh.VertIndices.at(d).nValues[1], 5, 2);
                WriteInt(node.Mesh.VertIndices.at(d).nValues[2], 5, 2);
            }
        }
    }

    /// LIGHT DATA
    if(node.Head.nType & NODE_HAS_LIGHT){
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
        WriteInt(ctrl.nPadding[0], 10, 1);
        WriteInt(ctrl.nPadding[1], 10, 1);
        WriteInt(ctrl.nPadding[2], 10, 1);
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
