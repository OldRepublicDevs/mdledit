#include "MDL.h"
#include <fstream>
#include <iomanip>
#include <Shlwapi.h>

int BinaryFile::ReadInt(unsigned int * nCurPos, int nMarking, int nBytes){
    //std::cout<<string_format("ReadInt() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > nBufferSize){
        std::cout<<string_format("ReadInt(): Reading past buffer size in %s, aborting and returning -1.\n", GetName());
        return -1;
    }
    if(nBytes == 4){
        int n = 0;
        while(n < 4){
            ByteBlock4.bytes[n] = sBuffer[*nCurPos + n];
            n++;
        }
        MarkBytes(*nCurPos, 4, nMarking);
        *nCurPos += 4;
        //std::cout<<"ReadInt() return: "<<ByteBlock4.i<<"\n";
        return ByteBlock4.i;
    }
    else if(nBytes ==2){
        int n = 0;
        while(n < 2){
            ByteBlock2.bytes[n] = sBuffer[*nCurPos + n];
            n++;
        }
        MarkBytes(*nCurPos, 2, nMarking);
        *nCurPos += 2;
        return ByteBlock2.i;
    }
    else if(nBytes == 1){
        int nReturn = (int) sBuffer[*nCurPos];
        MarkBytes(*nCurPos, 1, nMarking);
        *nCurPos += 1;
        return nReturn;
    }
    else return -1;
}

float BinaryFile::ReadFloat(unsigned int * nCurPos, int nMarking, int nBytes){
    //std::cout<<string_format("ReadFloat() at position %i.\n", *nCurPos);
    if(*nCurPos+nBytes > nBufferSize){
        std::cout<<string_format("ReadFloat(): Reading past buffer size in %s, aborting and returning -1.0.\n", GetName());
        return -1.0;
    }
    int n = 0;
    while(n < 4){
        ByteBlock4.bytes[n] = sBuffer[*nCurPos + n];
        n++;
    }
    MarkBytes(*nCurPos, 4, nMarking);
    *nCurPos += 4;
    return ByteBlock4.f;
}

void BinaryFile::ReadString(std::string & sArray1, unsigned int * nCurPos, int nMarking, int nNumber){
    if(*nCurPos+nNumber > nBufferSize){
        std::cout<<string_format("ReadInt(): Reading past buffer size in %s, aborting.\n", GetName());
        return;
    }
    sArray1.assign(&sBuffer[*nCurPos], nNumber);
    //std::cout<<"ReadString(): "<<sArray1<<"\n";
    MarkBytes(*nCurPos, nNumber, nMarking);
    *nCurPos += nNumber;
}

void BinaryFile::MarkBytes(unsigned int nOffset, int nLength, int nClass){
    int n = 0;
    //std::cout<<"Setting known: offset "<<nOffset<<" length "<<nLength<<" class "<<nClass<<"\n.";
    while(n < nLength && n < nBufferSize){
        if(bKnown[nOffset + n] != 0) std::cout<<string_format("MarkBytes(): Warning! Data already interpreted as %i at offset %i in %s!\n", bKnown[nOffset + n], nOffset + n, GetName());
        bKnown[nOffset + n] = nClass;
        n++;
    }
}

void MDL::DecompileModel(){
    if(sBuffer.empty() || nBufferSize == 0) return;

    int nNodeCounter;
    unsigned int nPos = 0;

    FH.resize(1);
    std::cout<<"Begin decompiling.\n";

    FileHeader & Data = FH[0];

    //First read the file header, geometry header and model header
    Data.nZero = ReadInt(&nPos, 8);
    Data.nMdlLength = ReadInt(&nPos, 1);
    Data.nMdxLength = ReadInt(&nPos, 1);
    std::cout<<"File header read.\n";

    Data.MH.GH.nFunctionPointer0 = ReadInt(&nPos, 9);
    Data.MH.GH.nFunctionPointer1 = ReadInt(&nPos, 9);

    //Get game
    if(Data.MH.GH.nFunctionPointer0 == K1_FUNCTION_POINTER_0) bK2 = false;
    else bK2 = true;

    ReadString(Data.MH.GH.sName, &nPos, 3, 32);
    Data.MH.GH.nOffsetToRootNode = ReadInt(&nPos, 6);
    Data.MH.GH.nTotalNumberOfNodes = ReadInt(&nPos, 1);

    Data.MH.GH.RuntimeArray1.nOffset = ReadInt(&nPos, 8);
    Data.MH.GH.RuntimeArray1.nCount = ReadInt(&nPos, 8);
    Data.MH.GH.RuntimeArray1.nCount2 = ReadInt(&nPos, 8);
    Data.MH.GH.RuntimeArray2.nOffset = ReadInt(&nPos, 8);
    Data.MH.GH.RuntimeArray2.nCount = ReadInt(&nPos, 8);
    Data.MH.GH.RuntimeArray2.nCount2 = ReadInt(&nPos, 8);
    Data.MH.GH.nRefCount = ReadInt(&nPos, 8);

    MarkBytes(nPos, 1, 7);
    Data.MH.GH.nModelType = sBuffer[nPos];
    nPos++;
    MarkBytes(nPos, 3, 10);
    Data.MH.GH.nPadding[0] = sBuffer[nPos];
    nPos++;
    Data.MH.GH.nPadding[1] = sBuffer[nPos];
    nPos++;
    Data.MH.GH.nPadding[2] = sBuffer[nPos];
    nPos++;
    std::cout<<"Geometry header read.\n";

    MarkBytes(nPos, 1, 7);
    Data.MH.nClassification = sBuffer[nPos];
    nPos++;
    MarkBytes(nPos, 3, 10);
    Data.MH.nUnknown1[0] = sBuffer[nPos];
    nPos++;
    Data.MH.nUnknown1[1] = sBuffer[nPos];
    nPos++;
    Data.MH.nUnknown1[2] = sBuffer[nPos];
    nPos++;

    Data.MH.nChildModelCount = ReadInt(&nPos, 8);

    Data.MH.AnimationArray.nOffset = ReadInt(&nPos, 6);
    Data.MH.AnimationArray.nCount = ReadInt(&nPos, 1);
    Data.MH.AnimationArray.nCount2 = ReadInt(&nPos, 1);
    Data.MH.nSupermodelReference = ReadInt(&nPos, 10);

    Data.MH.vBBmin.fX = ReadFloat(&nPos, 2);
    Data.MH.vBBmin.fY = ReadFloat(&nPos, 2);
    Data.MH.vBBmin.fZ = ReadFloat(&nPos, 2);
    Data.MH.vBBmax.fX = ReadFloat(&nPos, 2);
    Data.MH.vBBmax.fY = ReadFloat(&nPos, 2);
    Data.MH.vBBmax.fZ = ReadFloat(&nPos, 2);
    Data.MH.fRadius = ReadFloat(&nPos, 2);
    Data.MH.fScale = ReadFloat(&nPos, 2);

    ReadString(Data.MH.cSupermodelName, &nPos, 3, 32);

    Data.MH.nOffsetToHeadRootNode = ReadInt(&nPos, 6);
    Data.MH.nUnknown2 = ReadInt(&nPos, 8);
    Data.MH.nMdxLength2 = ReadInt(&nPos, 1);
    Data.MH.nOffsetIntoMdx = ReadInt(&nPos, 8);

    Data.MH.NameArray.nOffset = ReadInt(&nPos, 6);
    Data.MH.NameArray.nCount = ReadInt(&nPos, 1);
    Data.MH.NameArray.nCount2 = ReadInt(&nPos, 1);
    std::cout<<"Model header read.\n";

    //The header is fully done!
    //Now we're equipped to disassemble the rest
    //First index names array
    if(Data.MH.NameArray.nCount > 0){
        Data.MH.Names.resize(Data.MH.NameArray.nCount);
        int n = 0;
        while(n < Data.MH.NameArray.nCount){
            nPos = MDL_OFFSET + Data.MH.NameArray.nOffset + n*4;
            Data.MH.Names[n].nOffset = ReadInt(&nPos, 6);
            Data.MH.Names[n].sName = (const char*) &sBuffer[MDL_OFFSET + Data.MH.Names[n].nOffset];
            MarkBytes(MDL_OFFSET + Data.MH.Names[n].nOffset, Data.MH.Names[n].sName.size()+1, 3);
            n++;
        }
    }
    std::cout<<"Name array read.\n";

    //Next, animations
    if(Data.MH.AnimationArray.nCount > 0){
        //Data.MH.Animations = new Animation [Data.MH.AnimationArray.nCount];
        Data.MH.Animations.resize(Data.MH.AnimationArray.nCount);
        int n = 0;
        while(n < Data.MH.AnimationArray.nCount){
            nPos = MDL_OFFSET + Data.MH.AnimationArray.nOffset + n*4;
            Data.MH.Animations[n].nOffset = ReadInt(&nPos, 6);

            nPos = MDL_OFFSET + Data.MH.Animations[n].nOffset;
            Data.MH.Animations[n].nFunctionPointer0 = ReadInt(&nPos, 9);
            Data.MH.Animations[n].nFunctionPointer1 = ReadInt(&nPos, 9);

            ReadString(Data.MH.Animations[n].sName, &nPos, 3, 32);

            Data.MH.Animations[n].nOffsetToRootAnimationNode = ReadInt(&nPos, 6);
            Data.MH.Animations[n].nNumberOfObjects = ReadInt(&nPos, 1);

            Data.MH.Animations[n].RuntimeArray1.nOffset = ReadInt(&nPos, 8);
            Data.MH.Animations[n].RuntimeArray1.nCount = ReadInt(&nPos, 8);
            Data.MH.Animations[n].RuntimeArray1.nCount2 = ReadInt(&nPos, 8);
            Data.MH.Animations[n].RuntimeArray2.nOffset = ReadInt(&nPos, 8);
            Data.MH.Animations[n].RuntimeArray2.nCount = ReadInt(&nPos, 8);
            Data.MH.Animations[n].RuntimeArray2.nCount2 = ReadInt(&nPos, 8);
            Data.MH.Animations[n].nRefCount = ReadInt(&nPos, 8);

            MarkBytes(nPos, 1, 7);
            Data.MH.Animations[n].nModelType = sBuffer[nPos];
            nPos++;
            MarkBytes(nPos, 3, 10);
            Data.MH.Animations[n].nPadding[0] = sBuffer[nPos];
            nPos++;
            Data.MH.Animations[n].nPadding[1] = sBuffer[nPos];
            nPos++;
            Data.MH.Animations[n].nPadding[2] = sBuffer[nPos];
            nPos++;

            Data.MH.Animations[n].fLength = ReadFloat(&nPos, 2);
            Data.MH.Animations[n].fTransition = ReadFloat(&nPos, 2); //Likely transition, but almost surely a float

            ReadString(Data.MH.Animations[n].sAnimRoot, &nPos, 3, 32);

            Data.MH.Animations[n].SoundArray.nOffset = ReadInt(&nPos, 6);
            Data.MH.Animations[n].SoundArray.nCount = ReadInt(&nPos, 1);
            Data.MH.Animations[n].SoundArray.nCount2 = ReadInt(&nPos, 1);
            Data.MH.Animations[n].nPadding2 = ReadInt(&nPos, 8);

            if(Data.MH.Animations[n].SoundArray.nCount > 0){
                Data.MH.Animations[n].Sounds.resize(Data.MH.Animations[n].SoundArray.nCount);
                int i = 0;
                nPos = MDL_OFFSET + Data.MH.Animations[n].SoundArray.nOffset;
                //std::cout<<string_format("Offset to Animation Sounds is %i\n", Data.MH.Animations[n].SoundArray.nOffset);
                while(i < Data.MH.Animations[n].SoundArray.nCount){
                    Data.MH.Animations[n].Sounds[i].fTime = ReadFloat(&nPos, 2);
                    ReadString(Data.MH.Animations[n].Sounds[i].sName, &nPos, 3, 32);
                    i++;
                }
            }

            //We're done with the header, now we delve into animation nodes. It's a bit scary :(
            Data.MH.Animations[n].RootAnimationNode.nOffset = Data.MH.Animations[n].nOffsetToRootAnimationNode;
            Data.MH.Animations[n].RootAnimationNode.nAnimation = n;
            nNodeCounter = 0;
            Vector vFromRoot;
            ParseNode(&(Data.MH.Animations[n].RootAnimationNode), &nNodeCounter, vFromRoot);
            //Data.MH.Animations[n].nNodeCount = nNodeCounter;
            //std::cout<<string_format("Node count for Animation %i: %i, compared to the number in the header, %i.\n", n, nNodeCounter, Data.MH.Animations[n].nNumberOfObjects);
            Data.MH.Animations[n].ArrayOfNodes.clear();
            Data.MH.Animations[n].ArrayOfNodes.reserve(Data.MH.Names.size());
            LinearizeAnimations(Data.MH.Animations[n].RootAnimationNode, Data.MH.Animations[n].ArrayOfNodes);

            n++;
        }
    }
    std::cout<<"Animation array read.\n";
    if(Data.MH.Names.size() > 0){
        Data.MH.RootNode.nOffset = Data.MH.GH.nOffsetToRootNode;
        Data.MH.RootNode.nAnimation = -1;
        //std::cout<<string_format("Offset to Root Node: %i\n", Data.MH.RootNode.nOffset);
        nNodeCounter = 0;
        Vector vFromRoot;
        ParseNode(&(Data.MH.RootNode), &nNodeCounter, vFromRoot);
        //std::cout<<string_format("Node count for the Geometry: %i, compared to the number in the header, %i.\n", nNodeCounter, Data.MH.GH.nNumberOfNodes);
        //Data.MH.ArrayOfNodes.clear();
        Data.MH.ArrayOfNodes.resize(Data.MH.Names.size());
        LinearizeGeometry(Data.MH.RootNode, Data.MH.ArrayOfNodes);
    }
    std::cout<<"Geometry read.\n";
    std::cout<<"Done decompiling!\n";
    if(bDetermineSmoothing && !Mdx.empty()) DetermineSmoothing();
}

void MDL::ParseAabb(Aabb * AABB, unsigned int nHighestOffset){
    if(AABB->nOffset > nHighestOffset) nHighestOffset = AABB->nOffset;
    else if(nHighestOffset == 0xffffffff) return;
    else{
        MessageBox(NULL, "The aabb (walkmesh) tree seems to be looping, the .mdl might be broken. =(", "Error", MB_OK | MB_ICONERROR);
        nHighestOffset = 0xffffffff;
        return;
    }
    unsigned int nPos = AABB->nOffset + MDL_OFFSET;
    AABB->vBBmin.fX = ReadFloat(&nPos, 2);
    AABB->vBBmin.fY = ReadFloat(&nPos, 2);
    AABB->vBBmin.fZ = ReadFloat(&nPos, 2);
    AABB->vBBmax.fX = ReadFloat(&nPos, 2);
    AABB->vBBmax.fY = ReadFloat(&nPos, 2);
    AABB->vBBmax.fZ = ReadFloat(&nPos, 2);
    AABB->nChild1 = ReadInt(&nPos, 6);
    AABB->nChild2 = ReadInt(&nPos, 6);
    AABB->nID = ReadInt(&nPos, 4);
    AABB->nProperty = ReadInt(&nPos, 4);

    if(AABB->nChild1 > 0){
        //AABB->Child1 = new Aabb;
        AABB->Child1.resize(1);
        AABB->Child1[0].nOffset = AABB->nChild1;
        ParseAabb(&AABB->Child1[0], nHighestOffset);
    }
    if(AABB->nChild2 > 0){
        AABB->Child2.resize(1);
        AABB->Child2[0].nOffset = AABB->nChild2;
        ParseAabb(&AABB->Child2[0], nHighestOffset);
    }
}

void MDL::ParseNode(Node * NODE, int * nNodeCounter, Vector vFromRoot){
    unsigned int nPos = MDL_OFFSET + NODE->nOffset;
    unsigned int nPosData;
    (*nNodeCounter)++;

    NODE->Head.nType = ReadInt(&nPos, 5, 2);
    NODE->Head.nID1 = ReadInt(&nPos, 5, 2);
    NODE->Head.nNameIndex = ReadInt(&nPos, 5, 2);
    NODE->Head.nPadding1 = ReadInt(&nPos, 8, 2);

    //By now, we can figure out how long the header is going to be and all that we're including in it.

    if(NODE->Head.nType & NODE_HAS_HEADER){
        NODE->Head.nOffsetToRoot = ReadInt(&nPos, 6);
        NODE->Head.nOffsetToParent = ReadInt(&nPos, 6);
        NODE->Head.vPos.fX = ReadFloat(&nPos, 2);
        NODE->Head.vPos.fY = ReadFloat(&nPos, 2);
        NODE->Head.vPos.fZ = ReadFloat(&nPos, 2);

        double fQW = ReadFloat(&nPos, 2);
        double fQX = ReadFloat(&nPos, 2);
        double fQY = ReadFloat(&nPos, 2);
        double fQZ = ReadFloat(&nPos, 2);
        NODE->Head.oOrient.Quaternion(fQX, fQY, fQZ, fQW);
        NODE->Head.oOrient.ConvertToAA(); // just convert from quaternions right away

        NODE->Head.ChildrenArray.nOffset = ReadInt(&nPos, 6);
        NODE->Head.ChildrenArray.nCount = ReadInt(&nPos, 1);
        NODE->Head.ChildrenArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Head.ControllerArray.nOffset = ReadInt(&nPos, 6);
        NODE->Head.ControllerArray.nCount = ReadInt(&nPos, 1);
        NODE->Head.ControllerArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Head.ControllerDataArray.nOffset = ReadInt(&nPos, 6);
        NODE->Head.ControllerDataArray.nCount = ReadInt(&nPos, 1);
        NODE->Head.ControllerDataArray.nCount2 = ReadInt(&nPos, 1);

        if(NODE->Head.ControllerDataArray.nCount > 0){
            //We gots controll data!
            NODE->Head.ControllerData.resize(NODE->Head.ControllerDataArray.nCount);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Head.ControllerDataArray.nOffset;
            while(n < NODE->Head.ControllerDataArray.nCount){
                NODE->Head.ControllerData[n] = ReadFloat(&nPosData, 2);
                n++;
                //if(n == NODE->Head.ControllerDataArray.nCount) std::cout<<string_format("Just filled %i floats of Controller Data\n", n);
            }
        }

        if(NODE->Head.ControllerArray.nCount > 0){
            NODE->Head.Controllers.resize(NODE->Head.ControllerArray.nCount);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Head.ControllerArray.nOffset;
            while(n < NODE->Head.ControllerArray.nCount){
                NODE->Head.Controllers[n].nControllerType = ReadInt(&nPosData, 4);
                NODE->Head.Controllers[n].nUnknown2 = ReadInt(&nPosData, 10, 2);
                NODE->Head.Controllers[n].nValueCount = ReadInt(&nPosData, 5, 2);
                NODE->Head.Controllers[n].nTimekeyStart = ReadInt(&nPosData, 5, 2);
                NODE->Head.Controllers[n].nDataStart = ReadInt(&nPosData, 5, 2);

                MarkBytes(nPosData, 1, 7);
                NODE->Head.Controllers[n].nColumnCount = sBuffer[nPosData];
                nPosData += 1;
                MarkBytes(nPosData, 3, 10);
                NODE->Head.Controllers[n].nPadding[0] = sBuffer[nPosData];
                nPosData += 1;
                NODE->Head.Controllers[n].nPadding[1] = sBuffer[nPosData];
                nPosData += 1;
                NODE->Head.Controllers[n].nPadding[2] = sBuffer[nPosData];
                nPosData += 1;

                NODE->Head.Controllers[n].nNameIndex = NODE->Head.nNameIndex;
                NODE->Head.Controllers[n].nAnimation = NODE->nAnimation;

                n++;
            }
        }

        /// Let's do the transformations/translations here. First orientation, then translation.
        Location loc = NODE->GetLocation();
        //vFromRoot.Rotate(NODE->Head.oOrient); //Why am I not rotating with loc's orientation??? Let's try it
        vFromRoot.Rotate(loc.oOrientation);
        vFromRoot+= loc.vPosition;

        NODE->Head.vFromRoot = vFromRoot;

        if(NODE->Head.ChildrenArray.nCount > 0){
            //We gots children!
            NODE->Head.Children.resize(NODE->Head.ChildrenArray.nCount);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Head.ChildrenArray.nOffset;
            while(n < NODE->Head.ChildrenArray.nCount){
                NODE->Head.Children[n].nOffset = ReadInt(&nPosData, 6);
                NODE->Head.Children[n].nAnimation = NODE->nAnimation;
                NODE->Head.Children[n].Head.nParentIndex = NODE->Head.nNameIndex;
                ParseNode(&NODE->Head.Children[n], nNodeCounter, vFromRoot);
                n++;
            }
        }
    }

    if(NODE->Head.nType & NODE_HAS_LIGHT){
        NODE->Light.fFlareRadius = ReadFloat(&nPos, 2);
        NODE->Light.UnknownArray.nOffset = ReadInt(&nPos, 6);
        NODE->Light.UnknownArray.nCount = ReadInt(&nPos, 1);
        NODE->Light.UnknownArray.nCount2 = ReadInt(&nPos, 1);
        NODE->Light.FlareSizeArray.nOffset = ReadInt(&nPos, 6);
        NODE->Light.FlareSizeArray.nCount = ReadInt(&nPos, 1);
        NODE->Light.FlareSizeArray.nCount2 = ReadInt(&nPos, 1);
        NODE->Light.FlarePositionArray.nOffset = ReadInt(&nPos, 6);
        NODE->Light.FlarePositionArray.nCount = ReadInt(&nPos, 1);
        NODE->Light.FlarePositionArray.nCount2 = ReadInt(&nPos, 1);
        NODE->Light.FlareColorShiftArray.nOffset = ReadInt(&nPos, 6);
        NODE->Light.FlareColorShiftArray.nCount = ReadInt(&nPos, 1);
        NODE->Light.FlareColorShiftArray.nCount2 = ReadInt(&nPos, 1);
        NODE->Light.FlareTextureNameArray.nOffset = ReadInt(&nPos, 6);
        NODE->Light.FlareTextureNameArray.nCount = ReadInt(&nPos, 1);
        NODE->Light.FlareTextureNameArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Light.nLightPriority = ReadInt(&nPos, 4);
        NODE->Light.nAmbientOnly = ReadInt(&nPos, 4);
        NODE->Light.nDynamicType = ReadInt(&nPos, 4);
        NODE->Light.nAffectDynamic = ReadInt(&nPos, 4);
        NODE->Light.nShadow = ReadInt(&nPos, 4);
        NODE->Light.nFlare = ReadInt(&nPos, 4);
        NODE->Light.nFadingLight = ReadInt(&nPos, 4);

        if(NODE->Light.FlareTextureNameArray.nCount > 0){
            NODE->Light.FlareTextureNames.resize(NODE->Light.FlareTextureNameArray.nCount);
            int n = 0;
            unsigned int nPos = MDL_OFFSET + NODE->Light.FlareTextureNameArray.nOffset;
            while(n < NODE->Light.FlareTextureNameArray.nCount){
                NODE->Light.FlareTextureNames[n].nOffset = ReadInt(&nPos, 6);
                NODE->Light.FlareTextureNames[n].sName = (const char*) &sBuffer[MDL_OFFSET + NODE->Light.FlareTextureNames[n].nOffset];
                MarkBytes(MDL_OFFSET + NODE->Light.FlareTextureNames[n].nOffset, NODE->Light.FlareTextureNames[n].sName.size() + 1, 3);
                n++;
            }
        }
        if(NODE->Light.FlareSizeArray.nCount > 0){
            NODE->Light.FlareSizes.resize(NODE->Light.FlareSizeArray.nCount);
            int n = 0;
            unsigned int nPos = MDL_OFFSET + NODE->Light.FlareSizeArray.nOffset;
            while(n < NODE->Light.FlareSizeArray.nCount){
                NODE->Light.FlareSizes[n] = ReadFloat(&nPos, 2);
                n++;
            }
        }
        if(NODE->Light.FlarePositionArray.nCount > 0){
            NODE->Light.FlarePositions.resize(NODE->Light.FlarePositionArray.nCount);
            int n = 0;
            unsigned int nPos = MDL_OFFSET + NODE->Light.FlarePositionArray.nOffset;
            while(n < NODE->Light.FlarePositionArray.nCount){
                NODE->Light.FlarePositions[n] = ReadFloat(&nPos, 2);
                n++;
            }
        }
        if(NODE->Light.FlareColorShiftArray.nCount > 0){
            NODE->Light.FlareColorShifts.resize(NODE->Light.FlareColorShiftArray.nCount);
            int n = 0;
            unsigned int nPos = MDL_OFFSET + NODE->Light.FlareColorShiftArray.nOffset;
            while(n < NODE->Light.FlareColorShiftArray.nCount){
                NODE->Light.FlareColorShifts[n].fR = ReadFloat(&nPos, 2);
                NODE->Light.FlareColorShifts[n].fG = ReadFloat(&nPos, 2);
                NODE->Light.FlareColorShifts[n].fB = ReadFloat(&nPos, 2);
                n++;
            }
        }
    }

    if(NODE->Head.nType & NODE_HAS_EMITTER){
        NODE->Emitter.fDeadSpace = ReadFloat(&nPos, 2);
        NODE->Emitter.fBlastRadius = ReadFloat(&nPos, 2);
        NODE->Emitter.fBlastLength = ReadFloat(&nPos, 2);

        NODE->Emitter.nBranchCount = ReadInt(&nPos, 1);
        NODE->Emitter.fControlPointSmoothing = ReadFloat(&nPos, 2);

        NODE->Emitter.nxGrid = ReadInt(&nPos, 4);
        NODE->Emitter.nyGrid = ReadInt(&nPos, 4);

        NODE->Emitter.nSpawnType = ReadInt(&nPos, 4);

        ReadString(NODE->Emitter.cUpdate, &nPos, 3, 32);
        ReadString(NODE->Emitter.cRender, &nPos, 3, 32);
        ReadString(NODE->Emitter.cBlend, &nPos, 3, 32);
        ReadString(NODE->Emitter.cTexture, &nPos, 3, 32);
        ReadString(NODE->Emitter.cChunkName, &nPos, 3, 16);

        NODE->Emitter.nTwosidedTex = ReadInt(&nPos, 4);
        NODE->Emitter.nLoop = ReadInt(&nPos, 4);
        NODE->Emitter.nUnknown1 = ReadInt(&nPos, 10, 2);
        NODE->Emitter.nFrameBlending = ReadInt(&nPos, 7, 1);
        ReadString(NODE->Emitter.cDepthTextureName, &nPos, 3, 32);
        NODE->Emitter.nUnknown2 = ReadInt(&nPos, 10, 1);
        NODE->Emitter.nFlags = ReadInt(&nPos, 10);
    }

    if(NODE->Head.nType & NODE_HAS_MESH){
        NODE->Mesh.nFunctionPointer0 = ReadInt(&nPos, 9);
        NODE->Mesh.nFunctionPointer1 = ReadInt(&nPos, 9);

        NODE->Mesh.FaceArray.nOffset = ReadInt(&nPos, 6);
        NODE->Mesh.FaceArray.nCount = ReadInt(&nPos, 1);
        NODE->Mesh.FaceArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Mesh.vBBmin.fX = ReadFloat(&nPos, 2);
        NODE->Mesh.vBBmin.fY = ReadFloat(&nPos, 2);
        NODE->Mesh.vBBmin.fZ = ReadFloat(&nPos, 2);
        NODE->Mesh.vBBmax.fX = ReadFloat(&nPos, 2);
        NODE->Mesh.vBBmax.fY = ReadFloat(&nPos, 2);
        NODE->Mesh.vBBmax.fZ = ReadFloat(&nPos, 2);
        NODE->Mesh.fRadius = ReadFloat(&nPos, 2);
        NODE->Mesh.vAverage.fX = ReadFloat(&nPos, 2);
        NODE->Mesh.vAverage.fY = ReadFloat(&nPos, 2);
        NODE->Mesh.vAverage.fZ = ReadFloat(&nPos, 2);
        NODE->Mesh.fDiffuse.fR = ReadFloat(&nPos, 2);
        NODE->Mesh.fDiffuse.fG = ReadFloat(&nPos, 2);
        NODE->Mesh.fDiffuse.fB = ReadFloat(&nPos, 2);
        NODE->Mesh.fAmbient.fR = ReadFloat(&nPos, 2);
        NODE->Mesh.fAmbient.fG = ReadFloat(&nPos, 2);
        NODE->Mesh.fAmbient.fB = ReadFloat(&nPos, 2);

        NODE->Mesh.nShininess = ReadInt(&nPos, 4);

        ReadString(NODE->Mesh.cTexture1, &nPos, 3, 32);
        ReadString(NODE->Mesh.cTexture2, &nPos, 3, 32);
        ReadString(NODE->Mesh.cTexture3, &nPos, 3, 12);
        ReadString(NODE->Mesh.cTexture4, &nPos, 3, 12);

        NODE->Mesh.IndexCounterArray.nOffset = ReadInt(&nPos, 6);
        NODE->Mesh.IndexCounterArray.nCount = ReadInt(&nPos, 1);
        NODE->Mesh.IndexCounterArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Mesh.IndexLocationArray.nOffset = ReadInt(&nPos, 6);
        NODE->Mesh.IndexLocationArray.nCount = ReadInt(&nPos, 1);
        NODE->Mesh.IndexLocationArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Mesh.MeshInvertedCounterArray.nOffset = ReadInt(&nPos, 6);
        NODE->Mesh.MeshInvertedCounterArray.nCount = ReadInt(&nPos, 1);
        NODE->Mesh.MeshInvertedCounterArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Mesh.nUnknown3[0] = ReadInt(&nPos, 8);
        NODE->Mesh.nUnknown3[1] = ReadInt(&nPos, 8);
        NODE->Mesh.nUnknown3[2] = ReadInt(&nPos, 8);

        MarkBytes(nPos, 8, 7);
        NODE->Mesh.nSaberUnknown1 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown2 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown3 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown4 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown5 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown6 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown7 = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nSaberUnknown8 = sBuffer[nPos];
        nPos++;

        NODE->Mesh.nAnimateUV = ReadInt(&nPos, 4);
        NODE->Mesh.fUVDirectionX = ReadFloat(&nPos, 2);
        NODE->Mesh.fUVDirectionY = ReadFloat(&nPos, 2);
        NODE->Mesh.fUVJitter = ReadFloat(&nPos, 2);
        NODE->Mesh.fUVJitterSpeed = ReadFloat(&nPos, 2);

        NODE->Mesh.nMdxDataSize = ReadInt(&nPos, 1);
        NODE->Mesh.nMdxDataBitmap = ReadInt(&nPos, 4);

        NODE->Mesh.nOffsetToVerticesInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToNormalsInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToUnknownInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToUVsInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToUV2sInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToUV3sInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToUV4sInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToTangentSpaceInMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToTangentSpace2InMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToTangentSpace3InMDX = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToTangentSpace4InMDX = ReadInt(&nPos, 6);

        NODE->Mesh.nNumberOfVerts = ReadInt(&nPos, 1, 2);
        NODE->Mesh.nTextureNumber = ReadInt(&nPos, 5, 2);

        MarkBytes(nPos, 6, 7);
        NODE->Mesh.nHasLightmap = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nRotateTexture = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nBackgroundGeometry = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nShadow = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nBeaming = sBuffer[nPos];
        nPos++;
        NODE->Mesh.nRender = sBuffer[nPos];
        nPos++;

        NODE->Mesh.nDirtEnabled = ReadInt(&nPos, 7, 1);
        NODE->Mesh.nUnknown1 = ReadInt(&nPos, 10, 1);
        NODE->Mesh.nDirtTexture = ReadInt(&nPos, 5, 2);
        NODE->Mesh.nDirtCoordSpace = ReadInt(&nPos, 5, 2);

        NODE->Mesh.nHideInHolograms = ReadInt(&nPos, 7, 1);
        NODE->Mesh.nUnknown2 = ReadInt(&nPos, 10, 1);

        NODE->Mesh.nUnknown4 = ReadInt(&nPos, 10, 2);

        NODE->Mesh.fTotalArea = ReadFloat(&nPos, 2);
        NODE->Mesh.nPadding = ReadInt(&nPos, 8);
        NODE->Mesh.nOffsetIntoMdx = ReadInt(&nPos, 6);
        NODE->Mesh.nOffsetToVertArray = ReadInt(&nPos, 6);

        if(NODE->Mesh.IndexCounterArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + NODE->Mesh.IndexCounterArray.nOffset;
            NODE->Mesh.nVertIndicesCount = ReadInt(&nPosData, 1); // In MDX?
        }
        else NODE->Mesh.nVertIndicesCount = 0;

        if(NODE->Mesh.IndexLocationArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + NODE->Mesh.IndexLocationArray.nOffset;
            NODE->Mesh.nVertIndicesLocation = ReadInt(&nPosData, 6);
        }
        else NODE->Mesh.nVertIndicesLocation = 0;

        if(NODE->Mesh.MeshInvertedCounterArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + NODE->Mesh.MeshInvertedCounterArray.nOffset;
            NODE->Mesh.nMeshInvertedCounter = ReadInt(&nPosData, 4);  // In MDX?
        }
        else NODE->Mesh.nMeshInvertedCounter = 0;

        if(NODE->Mesh.FaceArray.nCount > 0){
            NODE->Mesh.Faces.resize(NODE->Mesh.FaceArray.nCount);
            if(NODE->Mesh.IndexLocationArray.nCount > 0) NODE->Mesh.VertIndices.resize(NODE->Mesh.FaceArray.nCount);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Mesh.FaceArray.nOffset;
            unsigned int nPosData2;
            if(NODE->Mesh.IndexLocationArray.nCount > 0) nPosData2 = MDL_OFFSET + NODE->Mesh.nVertIndicesLocation;
            while(n < NODE->Mesh.FaceArray.nCount){
                NODE->Mesh.Faces[n].vNormal.fX = ReadFloat(&nPosData, 2);
                NODE->Mesh.Faces[n].vNormal.fY = ReadFloat(&nPosData, 2);
                NODE->Mesh.Faces[n].vNormal.fZ = ReadFloat(&nPosData, 2);
                NODE->Mesh.Faces[n].fDistance = ReadFloat(&nPosData, 2);

                NODE->Mesh.Faces[n].nMaterialID = ReadInt(&nPosData, 4);
                if(!bDetermineSmoothing) NODE->Mesh.Faces.at(n).nSmoothingGroup = NODE->Mesh.Faces[n].nMaterialID;

                NODE->Mesh.Faces[n].nAdjacentFaces[0] = ReadInt(&nPosData, 5, 2);
                NODE->Mesh.Faces[n].nAdjacentFaces[1] = ReadInt(&nPosData, 5, 2);
                NODE->Mesh.Faces[n].nAdjacentFaces[2] = ReadInt(&nPosData, 5, 2);
                NODE->Mesh.Faces[n].nIndexVertex[0] = ReadInt(&nPosData, 5, 2);
                NODE->Mesh.Faces[n].nIndexVertex[1] = ReadInt(&nPosData, 5, 2);
                NODE->Mesh.Faces[n].nIndexVertex[2] = ReadInt(&nPosData, 5, 2);

                if(NODE->Mesh.IndexLocationArray.nCount > 0){
                    //std::cout<<string_format("Reading VertIndices for face %i of %i, at position %i.\n", n, NODE->Mesh.FaceArray.nCount, nPosData2);
                    NODE->Mesh.VertIndices[n].nValues[0] = ReadInt(&nPosData2, 5, 2);
                    NODE->Mesh.VertIndices[n].nValues[1] = ReadInt(&nPosData2, 5, 2);
                    NODE->Mesh.VertIndices[n].nValues[2] = ReadInt(&nPosData2, 5, 2);
                }

                n++;
            }
        }
    }

    if(NODE->Head.nType & NODE_HAS_DANGLY){
        NODE->Dangly.ConstraintArray.nOffset = ReadInt(&nPos, 6);
        NODE->Dangly.ConstraintArray.nCount = ReadInt(&nPos, 1);
        NODE->Dangly.ConstraintArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Dangly.fDisplacement = ReadFloat(&nPos, 2);
        NODE->Dangly.fTightness = ReadFloat(&nPos, 2);
        NODE->Dangly.fPeriod = ReadFloat(&nPos, 2);

        NODE->Dangly.nOffsetToData2 = ReadInt(&nPos, 6);

        if(NODE->Dangly.ConstraintArray.nCount > 0){
            NODE->Dangly.Constraints.resize(NODE->Dangly.ConstraintArray.nCount);
            NODE->Dangly.Data2.resize(NODE->Dangly.ConstraintArray.nCount);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Dangly.ConstraintArray.nOffset;
            unsigned int nPosData2 = MDL_OFFSET + NODE->Dangly.nOffsetToData2;
            while(n < NODE->Dangly.ConstraintArray.nCount){
                NODE->Dangly.Constraints.at(n) = ReadFloat(&nPosData, 2);

                NODE->Dangly.Data2.at(n).fX = ReadFloat(&nPosData2, 2);
                NODE->Dangly.Data2.at(n).fY = ReadFloat(&nPosData2, 2);
                NODE->Dangly.Data2.at(n).fZ = ReadFloat(&nPosData2, 2);

                n++;
            }
        }
    }

    if(NODE->Head.nType & NODE_HAS_SKIN){
        NODE->Skin.UnknownArray.nOffset = ReadInt(&nPos, 8);
        NODE->Skin.UnknownArray.nCount = ReadInt(&nPos, 8);
        NODE->Skin.UnknownArray.nCount2 = ReadInt(&nPos, 8);
        NODE->Skin.nOffsetToWeightValuesInMDX = ReadInt(&nPos, 6);
        NODE->Skin.nOffsetToBoneIndexInMDX = ReadInt(&nPos, 6);

        NODE->Skin.nOffsetToBonemap = ReadInt(&nPos, 6);
        NODE->Skin.nNumberOfBonemap = ReadInt(&nPos, 1);

        NODE->Skin.QBoneArray.nOffset = ReadInt(&nPos, 6);
        NODE->Skin.QBoneArray.nCount = ReadInt(&nPos, 1);
        NODE->Skin.QBoneArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Skin.TBoneArray.nOffset = ReadInt(&nPos, 6);
        NODE->Skin.TBoneArray.nCount = ReadInt(&nPos, 1);
        NODE->Skin.TBoneArray.nCount2 = ReadInt(&nPos, 1);

        NODE->Skin.Array8Array.nOffset = ReadInt(&nPos, 6);
        NODE->Skin.Array8Array.nCount = ReadInt(&nPos, 1);
        NODE->Skin.Array8Array.nCount2 = ReadInt(&nPos, 1);

        for(int n = 0; n < 18; n++){
            NODE->Skin.nBoneIndexes[n] = ReadInt(&nPos, 5, 2);
        }

        if(NODE->Skin.nNumberOfBonemap != FH[0].MH.Names.size() || NODE->Skin.nNumberOfBonemap != NODE->Skin.QBoneArray.nCount || NODE->Skin.nNumberOfBonemap != NODE->Skin.TBoneArray.nCount || NODE->Skin.nNumberOfBonemap != NODE->Skin.Array8Array.nCount){
            Error("Unexpected Error! The bone numbers do not match up for " + FH[0].MH.Names.at(NODE->Head.nNameIndex).sName + "! I will try to load the data anyway. ");
        }
        if(NODE->Skin.nNumberOfBonemap > 0){
            NODE->Skin.Bones.resize(NODE->Skin.nNumberOfBonemap);
            NODE->Skin.BoneNameIndexes.resize(NODE->Mesh.nNumberOfVerts, -1);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Skin.nOffsetToBonemap;
            unsigned int nPosData2 = MDL_OFFSET + NODE->Skin.QBoneArray.nOffset;
            unsigned int nPosData3 = MDL_OFFSET + NODE->Skin.TBoneArray.nOffset;
            unsigned int nPosData4 = MDL_OFFSET + NODE->Skin.Array8Array.nOffset;
            while(n < NODE->Skin.nNumberOfBonemap){
                NODE->Skin.Bones[n].fBonemap = ReadFloat(&nPosData, 2);
                if(NODE->Skin.Bones[n].fBonemap != -1.0){
                    NODE->Skin.BoneNameIndexes[(int) NODE->Skin.Bones[n].fBonemap] = n;
                }

                double fQW = ReadFloat(&nPosData2, 2);
                double fQX = ReadFloat(&nPosData2, 2);
                double fQY = ReadFloat(&nPosData2, 2);
                double fQZ = ReadFloat(&nPosData2, 2);
                NODE->Skin.Bones[n].QBone.Quaternion(fQX, fQY, fQZ, fQW);

                NODE->Skin.Bones[n].TBone.fX = ReadFloat(&nPosData3, 2);
                NODE->Skin.Bones[n].TBone.fY = ReadFloat(&nPosData3, 2);
                NODE->Skin.Bones[n].TBone.fZ = ReadFloat(&nPosData3, 2);

                MarkBytes(nPosData4, 4, 8);
                nPosData4+=4;
                //NODE->Skin.Bones[n].fArray8 = ReadFloat(&nPosData4, 2);

                n++;
            }
        }
    }

    ///Need to do this later so that the Skin data is already read
    if(NODE->Head.nType & NODE_HAS_MESH){
        if(NODE->Mesh.nNumberOfVerts > 0){
            NODE->Mesh.Vertices.resize(NODE->Mesh.nNumberOfVerts);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Mesh.nOffsetToVertArray;
            unsigned int nPosData2 = NODE->Mesh.nOffsetIntoMdx;
            while(n < NODE->Mesh.nNumberOfVerts){
                NODE->Mesh.Vertices[n].fX = ReadFloat(&nPosData, 2);
                NODE->Mesh.Vertices[n].fY = ReadFloat(&nPosData, 2);
                NODE->Mesh.Vertices[n].fZ = ReadFloat(&nPosData, 2);
                /*
                Vector vTransform = NODE->Mesh.Vertices[n];
                Location loc = NODE->GetLocation();
                vTransform.Rotate(loc.oOrientation);
                vTransform += vFromRoot;

                NODE->Mesh.Vertices.at(n).fFromRootX = vTransform.fX;
                NODE->Mesh.Vertices.at(n).fFromRootY = vTransform.fY;
                NODE->Mesh.Vertices.at(n).fFromRootZ = vTransform.fZ;
                */
                //NODE->Mesh.Vertices.at(n).fFromRootX = vFromRoot.fX + NODE->Mesh.Vertices[n].fX;
                //NODE->Mesh.Vertices.at(n).fFromRootY = vFromRoot.fY + NODE->Mesh.Vertices[n].fY;
                //NODE->Mesh.Vertices.at(n).fFromRootZ = vFromRoot.fZ + NODE->Mesh.Vertices[n].fZ;

                if(NODE->Mesh.nMdxDataSize > 0 && !Mdx.empty()){
                    NODE->Mesh.Vertices[n].MDXData.nNameIndex = NODE->Head.nNameIndex;
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToVerticesInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vVertex.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vVertex.fY = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vVertex.fZ = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToNormalsInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vNormal.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vNormal.fY = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vNormal.fZ = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUVsInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vUV1.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vUV1.fY = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV2sInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vUV2.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vUV2.fY = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV3sInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vUV3.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vUV3.fY = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV4sInMDX;
                        NODE->Mesh.Vertices[n].MDXData.vUV4.fX = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.vUV4.fY = Mdx.ReadFloat(&nPosData2, 2);
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpaceInMDX;
                        for(int b = 0; b < 3; b++){
                            NODE->Mesh.Vertices[n].MDXData.vTangent1[b].fX = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent1[b].fY = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent1[b].fZ = Mdx.ReadFloat(&nPosData2, 2);
                        }
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace2InMDX;
                        for(int b = 0; b < 3; b++){
                            NODE->Mesh.Vertices[n].MDXData.vTangent2[b].fX = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent2[b].fY = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent2[b].fZ = Mdx.ReadFloat(&nPosData2, 2);
                        }
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace3InMDX;
                        for(int b = 0; b < 3; b++){
                            NODE->Mesh.Vertices[n].MDXData.vTangent3[b].fX = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent3[b].fY = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent3[b].fZ = Mdx.ReadFloat(&nPosData2, 2);
                        }
                    }
                    if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace4InMDX;
                        for(int b = 0; b < 3; b++){
                            NODE->Mesh.Vertices[n].MDXData.vTangent4[b].fX = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent4[b].fY = Mdx.ReadFloat(&nPosData2, 2);
                            NODE->Mesh.Vertices[n].MDXData.vTangent4[b].fZ = Mdx.ReadFloat(&nPosData2, 2);
                        }
                    }
                    if(NODE->Head.nType & NODE_HAS_SKIN){
                        //if(NODE->Skin.nOffsetToWeightValuesInMDX != 32) std::cout<<string_format("Warning! MDX Skin Data Pointer 1 in %s is not 32! I might be reading wrong!\n", FH[0].MH.Names[NODE->Head.nNameIndex].sName.c_str());
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Skin.nOffsetToWeightValuesInMDX;
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightValue[0] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightValue[1] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightValue[2] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightValue[3] = Mdx.ReadFloat(&nPosData2, 2);
                        //if(NODE->Skin.nOffsetToBoneIndexInMDX != 48) std::cout<<string_format("Warning! MDX Skin Data Pointer 2 in %s is not 48! I might be reading wrong!\n", FH[0].MH.Names[NODE->Head.nNameIndex].sName.c_str());
                        nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Skin.nOffsetToBoneIndexInMDX;
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightIndex[0] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightIndex[1] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightIndex[2] = Mdx.ReadFloat(&nPosData2, 2);
                        NODE->Mesh.Vertices[n].MDXData.Weights.fWeightIndex[3] = Mdx.ReadFloat(&nPosData2, 2);
                    }
                }
                n++;
            }
            //Read the floats after each data block
            /*
                This is just an empty Mdx struct with the length of what came before it.
                But there is some logic to it. The first three numbers are floats, with values depending on node type:
                    // 1,000,000 for skins
                    //10,000,000 for meshes, danglymeshes
            */
            if(NODE->Mesh.nMdxDataSize > 0 && !Mdx.sBuffer.empty()){
                NODE->Mesh.MDXData.nNameIndex = NODE->Head.nNameIndex;
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToVerticesInMDX;
                    NODE->Mesh.MDXData.vVertex.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vVertex.fY = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vVertex.fZ = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToNormalsInMDX;
                    NODE->Mesh.MDXData.vNormal.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vNormal.fY = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vNormal.fZ = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUVsInMDX;
                    NODE->Mesh.MDXData.vUV1.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vUV1.fY = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV2sInMDX;
                    NODE->Mesh.MDXData.vUV2.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vUV2.fY = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV3sInMDX;
                    NODE->Mesh.MDXData.vUV3.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vUV3.fY = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToUV4sInMDX;
                    NODE->Mesh.MDXData.vUV4.fX = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.vUV4.fY = Mdx.ReadFloat(&nPosData2, 8);
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpaceInMDX;
                    for(int b = 0; b < 3; b++){
                        NODE->Mesh.MDXData.vTangent1[b].fX = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent1[b].fY = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent1[b].fZ = Mdx.ReadFloat(&nPosData2, 8);
                    }
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace2InMDX;
                    for(int b = 0; b < 3; b++){
                        NODE->Mesh.MDXData.vTangent2[b].fX = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent2[b].fY = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent2[b].fZ = Mdx.ReadFloat(&nPosData2, 8);
                    }
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace3InMDX;
                    for(int b = 0; b < 3; b++){
                        NODE->Mesh.MDXData.vTangent3[b].fX = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent3[b].fY = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent3[b].fZ = Mdx.ReadFloat(&nPosData2, 8);
                    }
                }
                if(NODE->Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Mesh.nOffsetToTangentSpace4InMDX;
                    for(int b = 0; b < 3; b++){
                        NODE->Mesh.MDXData.vTangent4[b].fX = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent4[b].fY = Mdx.ReadFloat(&nPosData2, 8);
                        NODE->Mesh.MDXData.vTangent4[b].fZ = Mdx.ReadFloat(&nPosData2, 8);
                    }
                }
                if(NODE->Head.nType & NODE_HAS_SKIN){
                    //if(NODE->Skin.nOffsetToWeightValuesInMDX != 32) std::cout<<string_format("Warning! MDX Skin Data Pointer 1 in %s is not 32! I might be reading wrong!\n", FH[0].MH.Names[NODE->Head.nNameIndex].sName.c_str());
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Skin.nOffsetToWeightValuesInMDX;
                    NODE->Mesh.MDXData.Weights.fWeightValue[0] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightValue[1] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightValue[2] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightValue[3] = Mdx.ReadFloat(&nPosData2, 8);
                    //if(NODE->Skin.nOffsetToBoneIndexInMDX != 48) std::cout<<string_format("Warning! MDX Skin Data Pointer 2 in %s is not 48! I might be reading wrong!\n", FH[0].MH.Names[NODE->Head.nNameIndex].sName.c_str());
                    nPosData2 = NODE->Mesh.nOffsetIntoMdx + n * NODE->Mesh.nMdxDataSize + NODE->Skin.nOffsetToBoneIndexInMDX;
                    NODE->Mesh.MDXData.Weights.fWeightIndex[0] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightIndex[1] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightIndex[2] = Mdx.ReadFloat(&nPosData2, 8);
                    NODE->Mesh.MDXData.Weights.fWeightIndex[3] = Mdx.ReadFloat(&nPosData2, 8);
                }
            }
        }
    }

    if(NODE->Head.nType & NODE_HAS_AABB){
        //std::cout<<string_format("Look for AABB at %i\n", nPos);
        NODE->Walkmesh.nOffsetToAabb = ReadInt(&nPos, 6);
        //std::cout<<string_format("Offset to AABB is %i\n", NODE->Walkmesh.nOffsetToAabb);

        if(NODE->Walkmesh.nOffsetToAabb > 0){
            NODE->Walkmesh.RootAabb.nOffset = NODE->Walkmesh.nOffsetToAabb;
            ParseAabb(&NODE->Walkmesh.RootAabb, 10);
        }
    }

    if(NODE->Head.nType & NODE_HAS_SABER){
        NODE->Saber.nOffsetToSaberData1 = ReadInt(&nPos, 6);
        NODE->Saber.nOffsetToSaberData2 = ReadInt(&nPos, 6);
        NODE->Saber.nOffsetToSaberData3 = ReadInt(&nPos, 6);
        NODE->Saber.nInvCount1 = ReadInt(&nPos, 4);
        NODE->Saber.nInvCount2 = ReadInt(&nPos, 4);
        NODE->Saber.nNumberOfSaberData = NODE->Mesh.nNumberOfVerts;

        if(NODE->Mesh.nNumberOfVerts > 0){
            NODE->Saber.SaberData.resize(NODE->Mesh.nNumberOfVerts);
            int n = 0;
            nPosData = MDL_OFFSET + NODE->Saber.nOffsetToSaberData1;
            unsigned int nPosData2 = MDL_OFFSET + NODE->Saber.nOffsetToSaberData2;
            unsigned int nPosData3 = MDL_OFFSET + NODE->Saber.nOffsetToSaberData3;
            while(n < NODE->Mesh.nNumberOfVerts){
                NODE->Saber.SaberData[n].vVertex.fX = ReadFloat(&nPosData, 2);
                NODE->Saber.SaberData[n].vVertex.fY = ReadFloat(&nPosData, 2);
                NODE->Saber.SaberData[n].vVertex.fZ = ReadFloat(&nPosData, 2);

                NODE->Saber.SaberData[n].vUV.fX = ReadFloat(&nPosData2, 2);
                NODE->Saber.SaberData[n].vUV.fY = ReadFloat(&nPosData2, 2);

                NODE->Saber.SaberData[n].vNormal.fX = ReadFloat(&nPosData3, 4);
                NODE->Saber.SaberData[n].vNormal.fY = ReadFloat(&nPosData3, 4);
                NODE->Saber.SaberData[n].vNormal.fZ = ReadFloat(&nPosData3, 4);
                n++;
            }
        }
    }
}

/// This function will become the main binary decompilation post-processing function
void MDL::DetermineSmoothing(){
    //std::cout<<"Calculating smoothing groups...\n";
    FileHeader & Data = FH[0];

            //Create file
            std::string sDir = Model.sFullPath;
            sDir.reserve(MAX_PATH);
            PathRemoveFileSpec(&sDir[0]);
            sDir.resize(strlen(sDir.c_str()));
            sDir += "\\debug.txt";
            std::cout<<"Will write smoothing debug to: "<<sDir.c_str()<<"\n";
            std::ofstream file(sDir.c_str());

            if(!file.is_open()){
                std::cout<<"debug.txt does not exist. No debug will be written.\n";
            }
    int nNumOfVerts = 0;
    int nNumOfFoundNormals = 0;
    int nNumOfTS = 0;
    int nNumOfFoundTS = 0;
    int nNumOfFoundTSB = 0;
    int nNumOfFoundTST = 0;
    int nNumOfFoundTSN = 0;

    ///I will first do it the inefficient way, but a clear way, so I can merge stuff later if need be.
    // The point is twofold - calculate area and calculate tangent space vectors
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_MESH){
            for(int f = 0; f < Data.MH.ArrayOfNodes.at(n).Mesh.Faces.size(); f++){
                Face & face = Data.MH.ArrayOfNodes.at(n).Mesh.Faces.at(f);
                Vertex & v1 = Data.MH.ArrayOfNodes.at(n).Mesh.Vertices.at(face.nIndexVertex[0]);
                Vertex & v2 = Data.MH.ArrayOfNodes.at(n).Mesh.Vertices.at(face.nIndexVertex[1]);
                Vertex & v3 = Data.MH.ArrayOfNodes.at(n).Mesh.Vertices.at(face.nIndexVertex[2]);
                Vector & v1UV = v1.MDXData.vUV1;
                Vector & v2UV = v2.MDXData.vUV1;
                Vector & v3UV = v3.MDXData.vUV1;
                Vector Edge1 = v2 - v1;
                Vector Edge2 = v3 - v1;
                Vector Edge3 = v3 - v2;
                Vector EUV1 = v2UV - v1UV;
                Vector EUV2 = v3UV - v1UV;
                Vector EUV3 = v3UV - v2UV;

                //Tangent and Bitangent calculation
                //Now comes the calculation. Will be using edges 1 and 2
                double r = (EUV1.fX * EUV2.fY - EUV1.fY * EUV2.fX);
                //This is division, need to check for 0
                if(r != 0){
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
                    //???
                    //ndix UR's magic factor
                    r = 2406.6388;
                }
                face.vTangent = r * (Edge1 * EUV2.fY - Edge2 * EUV1.fY);
                face.vBitangent = r * (Edge2 * EUV1.fX - Edge1 * EUV2.fX);
                face.vTangent.Normalize();
                face.vBitangent.Normalize();
                if(face.vTangent.Null()){
                    face.vTangent = Vector(1.0, 0.0, 0.0);
                }
                if(face.vBitangent.Null()){
                    face.vBitangent = Vector(1.0, 0.0, 0.0);
                }
                //Handedness
                Vector vCross = (face.vNormal / face.vTangent);
                double fDot = vCross * face.vBitangent;
                file<<Data.MH.ArrayOfNodes.at(n).GetName()<<" > face "<<f<<" > vNormal×vTangent="<<vCross.Print()<<", (vNormal×vTangent)*vBitangent="<<fDot<<", Bitangent "<<face.vBitangent.Print()<<", Tangent "<<face.vTangent.Print()<<"\n";
                if(fDot > 0.0000000001){
                    face.vTangent *= -1.0;
                }
                //Now check if we need to invert  T and B. But first we need a UV normal
                Vector vNormalUV = EUV1 / EUV2; //cross product
                if(vNormalUV.fZ < 0.0){
                    face.vTangent *= -1.0;
                    face.vBitangent *= -1.0;
                }
                //Vector vCross = face.vBitangent / face.vTangent;
                //file<<Data.MH.Names.at(Data.MH.ArrayOfNodes.at(n).Head.nNameIndex).cName<<" > face "<<f<<" > Bitangent "<<face.vBitangent.Print()<<", Tangent "<<face.vTangent.Print()<<", Normal "<<face.vNormal.Print()<<", Bitangent × Tangent "<<vCross.Print()<<"\n";

                //Area calculation
                face.fArea = HeronFormula(Edge1, Edge2, Edge3);
                face.fAreaUV = HeronFormula(EUV1, EUV2, EUV3);
            }
        }
    }

    CreatePatches(true, file);
    /*
    std::cout<<"Building LinkedFaces array... (this may take a while)\n";
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        //Currently, this takes all meshes, including skins, danglymeshes, walkmeshes and sabers
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_MESH){
            Node & node = Data.MH.ArrayOfNodes.at(n);
            nNumOfVerts += node.Mesh.Vertices.size();
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1) nNumOfTS += node.Mesh.Vertices.size();
            for(int v = 0; v < node.Mesh.Vertices.size(); v++){
                //For every vertex of every mesh

                //Proceed only if this vertex group hasn't been processed yet
                if(node.Mesh.Vertices.at(v).nLinkedFacesIndex == -1){
                    Vertex & vert = node.Mesh.Vertices.at(v); // Make life easier
                    vert.nLinkedFacesIndex = Data.MH.LinkedFacesPointers.size(); //Update reference to new vector that is about to be created
                    Data.MH.LinkedFacesPointers.push_back(std::vector<LinkedFace>()); //Create new vector
                    std::vector<LinkedFace> & LinkedFaceArray = Data.MH.LinkedFacesPointers.back(); //Get reference to the new vector
                    Vector vCoords = Vector(vert.fFromRootX, vert.fFromRootY, vert.fFromRootZ);
                    //file<<"Created LinkedFace group "<<vert.nLinkedFacesIndex<<" with vert "<<v<<" ("<<vert.fX<<", "<<vert.fY<<", "<<vert.fZ<<") in "<<Data.MH.Names.at(n).cName<<".\n";

                    //We've already gone through the nodes up to n and linked any vertices, so we can skip those
                    for(int n2 = n; n2 < Data.MH.ArrayOfNodes.size(); n2++){

                        if(Data.MH.ArrayOfNodes.at(n2).Head.nType & NODE_HAS_MESH){
                            Node & node2 = Data.MH.ArrayOfNodes.at(n2);

                            //Loop through all the faces in the mesh and look for matching vertices - theoretically, there is no way to optimize this part
                            for(int f = 0; f < node2.Mesh.Faces.size(); f++){
                                Face & face = node2.Mesh.Faces.at(f);

                                //We are now checking the three vertices
                                for(int i = 0; i < 3; i++){
                                    //Check if vertices are equal (enough)
                                    Vector vCoords2 = Vector(node2.Mesh.Vertices.at(face.nIndexVertex[i]).fFromRootX,
                                                             node2.Mesh.Vertices.at(face.nIndexVertex[i]).fFromRootY,
                                                             node2.Mesh.Vertices.at(face.nIndexVertex[i]).fFromRootZ);
                                    if(vCoords.Compare(vCoords2)){

                                        //If they are equal, regardless of weldedness, add the face to the linked faces array
                                        LinkedFaceArray.push_back(LinkedFace(n2, f, face.nIndexVertex[i]));
                                        //if(nVertIndex == -1) nVertIndex = i;

                                        //Also update the other vert's linked face array reference index, so we can skip processing it later.
                                        node2.Mesh.Vertices.at(face.nIndexVertex[i]).nLinkedFacesIndex = vert.nLinkedFacesIndex;

                                        //Only one vertex in a face can match our vertex, so exit this small loop
                                        ///Well, it turns out that's not true <_< *swears*
                                        //i = 3;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout<<"Done building LinkedFaces array!\n";
    std::cout<<"Creating patches... \n";
    for(int v = 0; v < Data.MH.LinkedFacesPointers.size(); v++){
        //For every vector of linked faces, create a vector of patches
        std::vector<LinkedFace> & LinkedFaceVector = Data.MH.LinkedFacesPointers.at(v);
        Data.MH.PatchArrayPointers.push_back(std::vector<Patch>());
        std::vector<Patch> & PatchVector = Data.MH.PatchArrayPointers.back();
        int ngroup = Data.MH.PatchArrayPointers.size() - 1;
        Vertex & vert = GetNodeByNameIndex(LinkedFaceVector.at(0).nNameIndex).Mesh.Vertices.at(LinkedFaceVector.at(0).nVertex);
        file<<"Group "<<ngroup<<" ("<<vert.fFromRootX<<", "<<vert.fFromRootY<<", "<<vert.fFromRootZ<<")\n";

        for(int lf = 0; lf < LinkedFaceVector.size(); lf++){
            if(!LinkedFaceVector.at(lf).bAssignedToPatch){
                Patch newpatch;
                LinkedFace & patchhead = LinkedFaceVector.at(lf);
                newpatch.nNameIndex = patchhead.nNameIndex;
                newpatch.nVertex = patchhead.nVertex;
                patchhead.bAssignedToPatch = true;
                std::stringstream ssReport;
                ///The smoothing groups are not known
                //newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(patchhead.nNameIndex).Mesh.Faces.at(patchhead.nFace).nSmoothingGroup;
                newpatch.FaceIndices.push_back(patchhead.nFace); //Assign first linked face index to the patch
                ssReport << patchhead.nFace << " ";
                for(int plf = lf+1; plf < LinkedFaceVector.size(); plf++){
                    if(!LinkedFaceVector.at(plf).bAssignedToPatch){
                        //The following check is the whole point of this whole patch business
                        //The patch only contains those faces which are welded, ie.
                        //they have the same mesh index and vert index as the point we are constructing this patch for
                        if(LinkedFaceVector.at(plf).nNameIndex == newpatch.nNameIndex &&
                           LinkedFaceVector.at(plf).nVertex == newpatch.nVertex){
                            LinkedFace & linked = LinkedFaceVector.at(plf);
                            linked.bAssignedToPatch = true;
                            ///The smoothing groups are not known
                            //newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(linked.nNameIndex).Mesh.Faces.at(linked.nFace).nSmoothingGroup;
                            newpatch.FaceIndices.push_back(linked.nFace); //Assign linked face index to the patch
                            ssReport << linked.nFace << " ";
                        }
                    }
                }
                file<<"   patch "<<PatchVector.size()<<" ("<<Data.MH.Names.at(newpatch.nNameIndex).sName.c_str()<<", vert "<<newpatch.nVertex<<") contains faces: "<<ssReport.str()<<"\n";
                PatchVector.push_back(std::move(newpatch));
            }
        }
    }
    std::cout<<"Done creating patches!\n";
    */

    std::vector<std::vector<unsigned long int>> nSmoothingGroupNumbers;
    nSmoothingGroupNumbers.resize(Data.MH.PatchArrayPointers.size());
    for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
        int nPatchCount = Data.MH.PatchArrayPointers.at(pg).size();
        //std::cout<<"Checking "<<nPatchCount<<" patches...\n";
        bool bSingleNormal = true;
        if(nPatchCount<2) bSingleNormal = false;
        Vector FirstNormal = GetNodeByNameIndex(Data.MH.PatchArrayPointers.at(pg).at(0).nNameIndex).Mesh.Vertices.at(Data.MH.PatchArrayPointers.at(pg).at(0).nVertex).MDXData.vNormal;
        for(int p = 0; p < nPatchCount && bSingleNormal; p++){
            if(!FirstNormal.Compare(GetNodeByNameIndex(Data.MH.PatchArrayPointers.at(pg).at(p).nNameIndex).Mesh.Vertices.at(Data.MH.PatchArrayPointers.at(pg).at(p).nVertex).MDXData.vNormal)){
                bSingleNormal = false;
            }
        }
        if(bSingleNormal){
            //std::cout<<"All normals equal for patch group "<<pg<<". Expecting errors for the next "<<nPatchCount<<" patches.\n";
            file<<"All normals equal for patch group "<<pg<<". Expecting errors for the next "<<nPatchCount<<" patches.\n";
        }
        int nFailedNormals = 0;
        std::stringstream ssVN;
        for(int p = 0; p < nPatchCount; p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            Vertex & vert = GetNodeByNameIndex(patch.nNameIndex).Mesh.Vertices.at(patch.nVertex);
            Vector vNormal = vert.MDXData.vNormal;
            Vector vBitangent = vert.MDXData.vTangent1[0];
            Vector vTangent = vert.MDXData.vTangent1[1];
            Vector vNormalTS = vert.MDXData.vTangent1[2];
            Vector vNormalBase;
            Vector vTangentBase;
            Vector vBitangentBase;
            Vector vTangentNormalBase;
            patch.SmoothedPatches.reserve(nPatchCount);
            patch.SmoothedPatches.clear();
            file<<"Calculating for group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<")";
            if(GetNodeByNameIndex(patch.nNameIndex).Head.nType & NODE_HAS_DANGLY && GetNodeByNameIndex(patch.nNameIndex).Dangly.Constraints.at(patch.nVertex) == 0.0){
                file<<"\r\nThis is in a danglymesh and the constraint is 0, expecting errors.";
            }
            //First add this patch's face normals
            double fTotalArea = 0.0;
            bool bBadUV = false;
            for(int f = 0; f < patch.FaceIndices.size(); f++){
                Face & face = GetNodeByNameIndex(patch.nNameIndex).Mesh.Faces.at(patch.FaceIndices.at(f));
                Vertex & v1 = GetNodeByNameIndex(patch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[0]);
                Vertex & v2 = GetNodeByNameIndex(patch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[1]);
                Vertex & v3 = GetNodeByNameIndex(patch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[2]);
                Vector Edge1 = v2 - v1;
                Vector Edge2 = v3 - v1;
                Vector Edge3 = v3 - v2;

                if(face.fAreaUV == 0.0) bBadUV = true;

                fTotalArea +=  face.fArea;
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
                vNormalBase += vAdd;
                vTangentBase += face.vTangent;
                vBitangentBase += face.vBitangent;

                vTangentNormalBase += (face.vBitangent / face.vTangent);
            }
            //Now we've got a base, now we've got to construct a matching vertex normal
            //Create a boolean to track this
            bool bFound = false;
            bool bFoundTS = false;

            //First check if this patch's normal is enough
            Vector vCheck = vNormalBase;
            vCheck.Normalize();
            file<<"\n   Comparing normal ("<<vNormal.fX<<", "<<vNormal.fY<<", "<<vNormal.fZ<<") to base ("<<vCheck.fX<<", "<<vCheck.fY<<", "<<vCheck.fZ<<").";
            if(vCheck.Compare(vNormal)){
                bFound = true;
            }

            //If not, sigh and go into the expensive loops
            if(!bFound){
                bFound = FindNormal(0, nPatchCount, p, pg, vNormalBase, vNormal, patch.SmoothedPatches, file);
            }

            if(bFound){
                //yay
                //But there's nothing to do right now, we need to wait until all the patches in the group are checked
                nNumOfFoundNormals++;
                //file<<"Found a match for patch "<<p<<"'s vertex normal! :) \n";
                //file<<">Found a match for vertex normal, patch "<<p<<" in group "<<pg<<", vertex "<<patch.nVertex<<" in "<<Data.MH.Names.at(patch.nNameIndex).sName<<" :) \n";
                file<<"\n:)    MATCH FOUND!\n";
            }
            else if(GetNodeByNameIndex(patch.nNameIndex).Head.nType & NODE_HAS_DANGLY && GetNodeByNameIndex(patch.nNameIndex).Dangly.Constraints.at(patch.nVertex) == 0.0){
                nFailedNormals++;
                ssVN<<"No match for vertex normal in group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<", constr. 0)\n";
                file<<"\n:(    NO MATCH FOUND - CONSTRAINT 0!\n";
            }
            else if(fTotalArea == 0.0){
                //file<<"Patch area's 0 for vertex normal, patch "<<p<<" in group "<<pg<<", vertex "<<patch.nVertex<<" in "<<Data.MH.Names.at(patch.nNameIndex).sName<<" :( \n";
                file<<"\n:/    BAD GEOMETRY - NO MATCH FOUND!\n";
                nNumOfVerts--;
                ssVN<<"Bad geometry for vertex normal in group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<")\n";
            }
            else{
                //file<<"Found no match for vertex normal, patch "<<p<<" in group "<<pg<<", vertex "<<patch.nVertex<<" in "<<Data.MH.Names.at(patch.nNameIndex).sName<<" :( \n";
                file<<"\n:(    NO MATCH FOUND!\n";
                nFailedNormals++;
                ssVN<<"No match for vertex normal in group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<")\n";
            }
            if(nFailedNormals == nPatchCount) std::cout<<"No matching vertex normals in patch group "<<pg<<".\n";
            else if(p+1 == nPatchCount) std::cout<<ssVN.str();

            //Also check if this patch is enough for tangent space
            if(GetNodeByNameIndex(patch.nNameIndex).Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
                Vector vCheckT = vTangentBase;
                Vector vCheckB = vBitangentBase;
                Vector vCheckN = vTangentNormalBase; //Might be the other way around, not sure
                vCheckT.Normalize();
                vCheckB.Normalize();
                vCheckN.Normalize();
                file<<"   Comparing TS bitangent ("<<vBitangent.fX<<", "<<vBitangent.fY<<", "<<vBitangent.fZ<<") to base ("<<vCheckB.fX<<", "<<vCheckB.fY<<", "<<vCheckB.fZ<<").";
                file<<"\n   Comparing TS tangent ("<<vTangent.fX<<", "<<vTangent.fY<<", "<<vTangent.fZ<<") to base ("<<vCheckT.fX<<", "<<vCheckT.fY<<", "<<vCheckT.fZ<<").";
                file<<"\n   Comparing TS normal ("<<vNormalTS.fX<<", "<<vNormalTS.fY<<", "<<vNormalTS.fZ<<") to base ("<<vCheckN.fX<<", "<<vCheckN.fY<<", "<<vCheckN.fZ<<").";
                std::vector<int> nDummyPatchVector;
                if(bFound){
                    if(patch.SmoothedPatches.size() == 0){
                        file<<"\n   Correct:";
                        bool b1 = false;
                        if(vCheckB.Compare(vBitangent)){
                            file<<" bitangent";
                            if(!bBadUV) nNumOfFoundTSB++;
                            b1 = true;
                        }
                        if(vCheckT.Compare(vTangent)){
                            file<<" tangent";
                            if(!bBadUV) nNumOfFoundTST++;
                            b1 = true;
                        }
                        if(vCheckN.Compare(vNormalTS)){
                            file<<" normal";
                            if(!bBadUV) nNumOfFoundTSN++;
                            b1 = true;
                        }
                        if(!b1) file<<" none";
                        if(vCheckB.Compare(vBitangent) && vCheckT.Compare(vTangent) && vCheckN.Compare(vNormalTS) && bBadUV) bFoundTS = true;
                        else file << " (incomplete)";
                    }
                    else{
                        //std::vector<int> nDummyPatchVector;
                        int nFound = 0; //FindTangentSpace(0, nPatchCount, p, pg, vTangentBase, vBitangentBase, vTangentNormalBase, vTangent, vBitangent, vNormalTS, nDummyPatchVector, file);

                        /**/
                        for(int t = 0; t < patch.SmoothedPatches.size(); t++){
                            Patch & curpatch = Data.MH.PatchArrayPointers.at(pg).at(patch.SmoothedPatches.at(t));
                            for(int f = 0; f < curpatch.FaceIndices.size(); f++){
                                Face & face = GetNodeByNameIndex(curpatch.nNameIndex).Mesh.Faces.at(curpatch.FaceIndices.at(f));
                                vTangentBase += face.vTangent;
                                vBitangentBase += face.vBitangent;
                                vTangentNormalBase += (face.vBitangent / face.vTangent);
                            }
                        }//Check if this is it
                        vCheckT = vTangentBase;
                        vCheckB = vBitangentBase;
                        vCheckN = vTangentNormalBase;
                        vCheckT.Normalize();
                        vCheckB.Normalize();
                        vCheckN.Normalize();
                        file<<"\n   Comparing TS bitangent ("<<vBitangent.fX<<", "<<vBitangent.fY<<", "<<vBitangent.fZ<<") to proposed ("<<vCheckB.fX<<", "<<vCheckB.fY<<", "<<vCheckB.fZ<<").";
                        file<<"\n   Comparing TS tangent ("<<vTangent.fX<<", "<<vTangent.fY<<", "<<vTangent.fZ<<") to proposed ("<<vCheckT.fX<<", "<<vCheckT.fY<<", "<<vCheckT.fZ<<").";
                        file<<"\n   Comparing TS normal ("<<vNormalTS.fX<<", "<<vNormalTS.fY<<", "<<vNormalTS.fZ<<") to proposed ("<<vCheckN.fX<<", "<<vCheckN.fY<<", "<<vCheckN.fZ<<").";
                        if(vCheckB.Compare(vBitangent) || vCheckT.Compare(vTangent) || vCheckN.Compare(vNormalTS)){
                            file<<"\n   Correct:";
                            if(vCheckB.Compare(vBitangent)){
                                file<<" bitangent";
                                nFound = nFound | 1;
                            }
                            if(vCheckT.Compare(vTangent)){
                                file<<" tangent";
                                nFound = nFound | 2;
                            }
                            if(vCheckN.Compare(vNormalTS)){
                                file<<" normal";
                                nFound = nFound | 4;
                            }
                            if(nFound != 7) file << " (incomplete)";
                        }
                        /**/

                        if(nFound == 7 && !bBadUV) bFoundTS = true;
                        if(nFound & 1 && !bBadUV) nNumOfFoundTSB++;
                        if(nFound & 2 && !bBadUV) nNumOfFoundTST++;
                        if(nFound & 4 && !bBadUV) nNumOfFoundTSN++;
                    }
                }
                else if(vCheckB.Compare(vBitangent) || vCheckT.Compare(vTangent) || vCheckN.Compare(vNormalTS)){
                    file<<"\n   Correct:";
                    bool b1 = false;
                    if(vCheckB.Compare(vBitangent)){
                        file<<" bitangent";
                        if(!bBadUV) nNumOfFoundTSB++;
                        b1 = true;
                    }
                    if(vCheckT.Compare(vTangent)){
                        file<<" tangent";
                        if(!bBadUV) nNumOfFoundTST++;
                        b1 = true;
                    }
                    if(vCheckN.Compare(vNormalTS)){
                        file<<" normal";
                        if(!bBadUV) nNumOfFoundTSN++;
                        b1 = true;
                    }
                    if(!b1) file<<" none";
                    if(vCheckB.Compare(vBitangent) && vCheckT.Compare(vTangent) && vCheckN.Compare(vNormalTS) && !bBadUV) bFoundTS = true;
                    else file << " (incomplete)";
                }
                else{
                    std::vector<int> nDummyPatchVector;
                    int nFound = FindTangentSpace(0, nPatchCount, p, pg, vTangentBase, vBitangentBase, vTangentNormalBase, vTangent, vBitangent, vNormalTS, nDummyPatchVector, file);

                    if(nFound == 7 && !bBadUV) bFoundTS = true;
                    if(nFound & 1 && !bBadUV) nNumOfFoundTSB++;
                    if(nFound & 2 && !bBadUV) nNumOfFoundTST++;
                    if(nFound & 4 && !bBadUV) nNumOfFoundTSN++;
                }
                if(bFoundTS){
                    file<<"\n:)    MATCH FOUND!\n";
                    nNumOfFoundTS++;
                }
                else if(bBadUV){
                    std::cout<<"Bad UV in group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<")\n";
                    file<<"\n:/    BAD UV!\n";
                    nNumOfTS--;
                }
                else{
                    std::cout<<"No match for tangent space in group "<<pg<<", patch "<<p<<"/"<<nPatchCount - 1<<" ("<<Data.MH.Names.at(patch.nNameIndex).sName<<", vert "<<patch.nVertex<<")\n";
                    file<<"\n:(    NO MATCH FOUND!\n";
                }
            }
        }
        //std::cout<<"Done comparing normals for patch group "<<pg<<"!\n";
        //When we get here all the data in the patch group has been worked over.
        //Our patches should now contain the info about which patches they smooth to.
        //Now we need to generate smoothing group numbers
        //std::cout<<"Max number of smoothing groups is: "<<nPatchCount*(nPatchCount - 1)/2 + 1<<".\n";
        for(int n = 0; n < nPatchCount*(nPatchCount - 1)/2 + 1; n++){
            nSmoothingGroupNumbers.at(pg).push_back(0);
        }

        int nSmoothingGroupCounter = 0;
        for(int p = 0; p < nPatchCount; p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            //std::cout<<"Patch "<<p<<" smooths to "<< patch.SmoothedPatches.size()<<" patches...\n";
            for(int i = 0; i < patch.SmoothedPatches.size(); i++){
                if(patch.SmoothedPatches.at(i) > p){
                    //So now we have patch p smoothing over to the patch in the array.at(i)
                    //update them to include a common smoothing number, then elevate the counter
                    std::vector<int> SmoothingGroup;
                    SmoothingGroup.push_back(p);
                    SmoothingGroup.push_back(patch.SmoothedPatches.at(i));
                    patch.SmoothingGroupNumbers.push_back(&(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));
                    Data.MH.PatchArrayPointers.at(pg).at(patch.SmoothedPatches.at(i)).SmoothingGroupNumbers.push_back(&(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));

                    GenerateSmoothingNumber(SmoothingGroup, nSmoothingGroupNumbers.at(pg), nSmoothingGroupCounter, pg);

                    nSmoothingGroupCounter++;
                    //std::cout<<"Added smoothing groups for at least patch "<<patch.SmoothedPatches.at(i)<<" and patch "<<p<<"...\n";
                }
            }
            //Once we get here, we have checked (for patch p) all the patches we smooth over to and added their indexes
            //In case the patch doesn't smooth to any other patch, store an additional identity smoothing group for it
            if(patch.SmoothingGroupNumbers.size() == 0){
                //std::cout<<"Patch "<<p<<" in patch group "<<pg<<" has no smoothing group, create one at "<<nSmoothingGroupCounter<<".\n";
                patch.SmoothingGroupNumbers.push_back((unsigned long int*) &(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));
                nSmoothingGroupCounter++;
            }
        }
        //std::cout<<"Done generating smoothing groups!\n";
        //Once we get back here, our prototype smoothing groups are all set up for the current patch group
        //At this point, I could turn all the remaining smoothing group numbers to 0,
        //because they are unused and could get in the way latter... but I will do that if the issue actually comes up

        //Now we need to simplify the smoothing group system from just the binary connections
        //SimplifySmoothingNumber(nSmoothingGroupCounter, nSmoothingGroupNumbers, pg);

        //Now comes the part when we check our faces for existing numbers,
        //updating our numbers, collapsing the numbers into a single int and
        //finally updating the faces with the SMOOTHING GROUPS!
        /*
        for(int p = 0; p < Data.MH.PatchArrayPointers.at(pg).size(); p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            unsigned long int nExistingSG = 0;
            for(int f = 0; f < patch.FaceIndices.size(); f++){
                nExistingSG = nExistingSG | GetNodeByNameIndex(patch.nNameIndex).Mesh.Faces.at(patch.FaceIndices.at(f)).nSmoothingGroup;
            }
            if(nExistingSG == 0){
                for(int nSG = 0; nSG < patch.SmoothingGroupNumbers.size(); nSG++){

                }
            }
        }*/
        /**/
        //std::cout<<"Last loop!\n";
        for(int p = 0; p < Data.MH.PatchArrayPointers.at(pg).size(); p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            unsigned long int nExistingSG = 0;
            for(int i = 0; i < patch.SmoothingGroupNumbers.size(); i++){
                nExistingSG = nExistingSG | *patch.SmoothingGroupNumbers.at(i);
            }
            patch.nSmoothingGroups = (unsigned int) nExistingSG;
            for(int f = 0; f < patch.FaceIndices.size(); f++){
                GetNodeByNameIndex(patch.nNameIndex).Mesh.Faces.at(patch.FaceIndices.at(f)).nSmoothingGroup = nExistingSG;
            }
        }
        /**/
        //std::cout<<"Done updating face smoothing groups!\n";
    }
    /*
    /// Here we finally apply the smoothing groups to the faces in a principled way.
    for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
        for(int p = 0; p < Data.MH.PatchArrayPointers.at(pg).size(); p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            if(pg == 0 && p == 0){
                /// We have to start somewhere, so this is our starting point, we just fill it with values one to sgn
                for(int sgn = 0; sgn < patch.SmoothingGroupNumbers; sgn++){
                    *patch.SmoothingGroupNumbers.at(sgn) = pown(2, sgn);
                }
            }
            /// Now go through all the faces in the patch
            for(int f = 0; f < patch.FaceIndices.size(); f++){
                Node & node = GetNodeByNameIndex(patch.nNameIndex);
                Face & face = node.Mesh.Faces.at(patch.FaceIndices.at(f));
                /// Now we need to make sure that all three patches (one for every vert) have exactly one slot
                /// If they do, then we first copy  the number from our vert (this vert just by convention) to
                /// all the patches where the number is 0, under the condition that our current number is unused in the patch.
                /// Otherwise, start from 1 going up and find an unused one in the patch, and apply that one instead.
                /// Then we bitOR all three numbers together into the smoothing number.

                /** Alternative algorithm
                The first thing we do is fill out all our 0 numbers. I will be traveling between patch groups that have been filled before
                so this should happen rarely, and it shouldn't endanger our already written data. Now we proceed from patch group to patch group,
                go through the patches and through every face, if the face does not have a double number in any corner, then we copy our number to those
                slots, provided that that number is not used already in the patch group. If it is, then we probably need to generate a new number to replace it.
                The way I see it now (which is not very clearly), it should work out more or less fine if we only process ever face once.
                /**


                if(face.nSmoothingGroup == 0){
                    for(int v = 0; v < 3; v++){
                        Vertex & vert = node.Mesh.Vertices.at(face.nIndexVertex[v]);
                        std::vector<Patch> & PatchVector = Data.MH.PatchArrayPointers.at(vert.nLinkedFacesIndex);
                        bool bFound = false;
                        for(int p2 = 0; !bFound; ){
                            if(PatchVector.at(p2).nVertex == face.nIndexVertex[v]){
                                bFound = true;
                            }
                            else p2++;
                        }
                        Patch & vertpatch = PatchVector.at(p2);
                    }
                }
                else{

                }
            }
        }
    }
    */


    double fPercentage = ((double)nNumOfFoundNormals / (double)nNumOfVerts) * 100.0;
    std::cout<<"Done calculating smoothing groups! Found normals: "<<nNumOfFoundNormals<<"/"<<nNumOfVerts<<" ("<<std::setprecision(4)<<fPercentage<<"%)\n";
    fPercentage = ((double)nNumOfFoundTS / (double)nNumOfTS) * 100.0;
    std::cout<<"Found tangent spaces: "<<nNumOfFoundTS<<"/"<<nNumOfTS<<" ("<<std::setprecision(4)<<fPercentage<<"%)\n";
    fPercentage = ((double)nNumOfFoundTSB / (double)nNumOfTS) * 100.0;
    std::cout<<"  Found bitangents: "<<nNumOfFoundTSB<<"/"<<nNumOfTS<<" ("<<std::setprecision(4)<<fPercentage<<"%)\n";
    fPercentage = ((double)nNumOfFoundTST / (double)nNumOfTS) * 100.0;
    std::cout<<"  Found tangents: "<<nNumOfFoundTST<<"/"<<nNumOfTS<<" ("<<std::setprecision(4)<<fPercentage<<"%)\n";
    fPercentage = ((double)nNumOfFoundTSN / (double)nNumOfTS) * 100.0;
    std::cout<<"  Found normals: "<<nNumOfFoundTSN<<"/"<<nNumOfTS<<" ("<<std::setprecision(4)<<fPercentage<<"%)\n";

            //Close file
            file.close();
}

void MDL::GenerateSmoothingNumber(std::vector<int> & SmoothingGroup, const std::vector<unsigned long int> & nSmoothingGroupNumbers, const int & nSmoothingGroupCounter, const int & pg){
    FileHeader & Data = FH[0];
    bool bExists = false;

    for(int p = 0; p < Data.MH.PatchArrayPointers.at(pg).size(); p++){
        for(int sp = 0; sp < SmoothingGroup.size() && !bExists; sp++){
            if(SmoothingGroup.at(sp) == p) bExists = true;
        }
        if(!bExists){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            bool bFound = true;
            for(int sp = 0; sp < SmoothingGroup.size() && bFound; sp++){
                bFound = false;
                for(int i = 0; i < patch.SmoothedPatches.size() && !bFound; i++){
                    if(patch.SmoothedPatches.at(i) == SmoothingGroup.at(sp)){
                        bFound = true;
                    }
                }
            }
            if(bFound){
                SmoothingGroup.push_back(p);
                Data.MH.PatchArrayPointers.at(pg).at(p).SmoothingGroupNumbers.push_back((long unsigned int*)&(nSmoothingGroupNumbers.at(nSmoothingGroupCounter)));
            }
        }
    }
}

bool MDL::FindNormal(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup, const Vector & vNormalBase, const Vector & vNormal, std::vector<int> & CurrentlySmoothedPatches, std::ofstream & file){
    FileHeader & Data = FH[0];
    for(int nCount = nPatchCount - 1; nCount >= nCheckFrom; nCount--){
        if(nCount != nCurrentPatch){
            Patch & ourpatch = Data.MH.PatchArrayPointers.at(nCurrentPatchGroup).at(nCount);

            //First reset our normal
            Vector vWorking = vNormalBase;
            std::vector<int> OurSmoothedPatches = CurrentlySmoothedPatches;
            //std::cout<<"Working ("<<vWorking.fX<<", "<<vWorking.fY<<", "<<vWorking.fZ<<").\n";

            //Add the current patch
            for(int f = 0; f < ourpatch.FaceIndices.size(); f++){
                Face & face = GetNodeByNameIndex(ourpatch.nNameIndex).Mesh.Faces.at(ourpatch.FaceIndices.at(f));
                Vertex & v1 = GetNodeByNameIndex(ourpatch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[0]);
                Vertex & v2 = GetNodeByNameIndex(ourpatch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[1]);
                Vertex & v3 = GetNodeByNameIndex(ourpatch.nNameIndex).Mesh.Vertices.at(face.nIndexVertex[2]);
                Vector Edge1 = v2 - v1;
                Vector Edge2 = v3 - v1;
                Vector Edge3 = v3 - v2;

                Vector vAdd = face.vNormal;
                if(bSmoothAreaWeighting) vAdd *= face.fArea;
                if(bSmoothAngleWeighting){
                    if(ourpatch.nVertex == face.nIndexVertex[0]){
                        vAdd *= Angle(Edge1, Edge2);
                    }
                    else if(ourpatch.nVertex == face.nIndexVertex[1]){
                        vAdd *= Angle(Edge1, Edge3);
                    }
                    else if(ourpatch.nVertex == face.nIndexVertex[2]){
                        vAdd *= Angle(Edge2, Edge3);
                    }
                }
                vWorking += vAdd;
            }
            //We have just smoothed for patch nCount, add it to the list
            OurSmoothedPatches.push_back(nCount);
            //std::cout<<"Working ("<<vWorking.fX<<", "<<vWorking.fY<<", "<<vWorking.fZ<<").\n";

            //Check if this is it
            Vector vCheck = vWorking;
            vCheck.Normalize();
            file<<"\n   Comparing normal ("<<vNormal.fX<<", "<<vNormal.fY<<", "<<vNormal.fZ<<") to proposed ("<<vCheck.fX<<", "<<vCheck.fY<<", "<<vCheck.fZ<<"). Included patches:";
            for(int g = 0; g < OurSmoothedPatches.size(); g++){
                file<<" "<<OurSmoothedPatches.at(g);
            }
            //file<<"\n";
            if(vCheck.Compare(vNormal)){
                Data.MH.PatchArrayPointers.at(nCurrentPatchGroup).at(nCurrentPatch).SmoothedPatches = OurSmoothedPatches;
                return true;
            }

            if(FindNormal(nCount+1, nPatchCount, nCurrentPatch, nCurrentPatchGroup, vWorking, vNormal, OurSmoothedPatches, file)){
                return true;
            }
        }
    }
    return false;
}

int MDL::FindTangentSpace(int nCheckFrom, const int & nPatchCount, const int & nCurrentPatch, const int & nCurrentPatchGroup,
                           const Vector & vTangentBase, const Vector & vBitangentBase, const Vector & vNormalBase,
                           const Vector & vTangent, const Vector & vBitangent, const Vector & vNormal,
                           std::vector<int> & CurrentlySmoothedPatches, std::ofstream & file){
    FileHeader & Data = FH[0];
    for(int nCount = nPatchCount - 1; nCount >= nCheckFrom; nCount--){
        if(nCount != nCurrentPatch){
            Patch & ourpatch = Data.MH.PatchArrayPointers.at(nCurrentPatchGroup).at(nCount);

            //First reset our normal
            Vector vWorkingT = vTangentBase;
            Vector vWorkingB = vBitangentBase;
            Vector vWorkingN = vNormalBase;
            std::vector<int> OurSmoothedPatches = CurrentlySmoothedPatches;
            //std::cout<<"Working ("<<vWorking.fX<<", "<<vWorking.fY<<", "<<vWorking.fZ<<").\n";

            //Add the current patch
            for(int f = 0; f < ourpatch.FaceIndices.size(); f++){
                Face & face = GetNodeByNameIndex(ourpatch.nNameIndex).Mesh.Faces.at(ourpatch.FaceIndices.at(f));

                vWorkingT += face.vTangent;
                vWorkingB += face.vBitangent;

                vWorkingN += (face.vBitangent / face.vTangent);
            }
            //We have just smoothed for patch nCount, add it to the list
            OurSmoothedPatches.push_back(nCount);
            //std::cout<<"Working ("<<vWorking.fX<<", "<<vWorking.fY<<", "<<vWorking.fZ<<").\n";

            //Check if this is it
            Vector vCheckT = vWorkingT;
            Vector vCheckB = vWorkingB;
            Vector vCheckN = vWorkingN;
            vCheckT.Normalize();
            vCheckB.Normalize();
            vCheckN.Normalize();
            file<<"\n   Comparing TS bitangent ("<<vBitangent.fX<<", "<<vBitangent.fY<<", "<<vBitangent.fZ<<") to proposed ("<<vCheckB.fX<<", "<<vCheckB.fY<<", "<<vCheckB.fZ<<").";
            file<<"\n   Comparing TS tangent ("<<vTangent.fX<<", "<<vTangent.fY<<", "<<vTangent.fZ<<") to proposed ("<<vCheckT.fX<<", "<<vCheckT.fY<<", "<<vCheckT.fZ<<").";
            file<<"\n   Comparing TS normal ("<<vNormal.fX<<", "<<vNormal.fY<<", "<<vNormal.fZ<<") to proposed ("<<vCheckN.fX<<", "<<vCheckN.fY<<", "<<vCheckN.fZ<<").";
            file<<"\n   Included patches:";
            for(int g = 0; g < OurSmoothedPatches.size(); g++){
                file<<" "<<OurSmoothedPatches.at(g);
            }
            //file<<"\n";
            if(vCheckB.Compare(vBitangent) || vCheckT.Compare(vTangent) || vCheckN.Compare(vNormal)){
                file<<"\n   Correct:";
                int nReturn = 0;
                if(vCheckB.Compare(vBitangent)){
                    file<<" bitangent";
                    nReturn = nReturn | 1;
                }
                if(vCheckT.Compare(vTangent)){
                    file<<" tangent";
                    nReturn = nReturn | 2;
                }
                if(vCheckN.Compare(vNormal)){
                    file<<" normal";
                    nReturn = nReturn | 4;
                }
                if(nReturn != 7) file << " (incomplete)";
                CurrentlySmoothedPatches = OurSmoothedPatches;
                return nReturn;
            }

            int nReturn = FindTangentSpace(nCount+1, nPatchCount, nCurrentPatch, nCurrentPatchGroup, vWorkingT, vWorkingB, vWorkingN, vTangent, vBitangent, vNormal, OurSmoothedPatches, file);
            if(nReturn > 0){
                CurrentlySmoothedPatches = OurSmoothedPatches;
                return nReturn;
            }
        }
    }
    return 0;
}
