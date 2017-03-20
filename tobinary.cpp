#include "MDL.h"

void BinaryFile::WriteIntToPH(int nInt, int nPH, unsigned int & nContainer){
    ByteBlock4.i = nInt;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer[nPH+n] = (ByteBlock4.bytes[n]);
    }
    nContainer = nInt;
}

void BinaryFile::WriteInt(int nInt, int nKnown, int nBytes){
    if(nBytes == 1){
        sBuffer.push_back((char) nInt);
        bKnown.push_back(nKnown);
        nPosition++;
    }
    else if(nBytes == 2){
        ByteBlock2.i = nInt;
        int n = 0;
        for(n = 0; n < 2; n++){
            sBuffer.push_back(ByteBlock2.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else if(nBytes == 4){
        ByteBlock4.i = nInt;
        int n = 0;
        for(n = 0; n < 4; n++){
            sBuffer.push_back(ByteBlock4.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else if(nBytes == 8){
        ByteBlock8.i = nInt;
        int n = 0;
        for(n = 0; n < 8; n++){
            sBuffer.push_back(ByteBlock8.bytes[n]);
            bKnown.push_back(nKnown);
        }
        nPosition+=n;
    }
    else Error("Cannot convert an integer to anything but 1, 2, 4 and 8 byte representations!");
}

void BinaryFile::WriteFloat(float fFloat, int nKnown, int nBytes){
    ByteBlock4.f = fFloat;
    int n = 0;
    for(n = 0; n < 4; n++){
        sBuffer.push_back(ByteBlock4.bytes[n]);
        bKnown.push_back(nKnown);
    }
    nPosition+=n;
}

void BinaryFile::WriteString(std::string sString, int nKnown){
    int n = 0;
    for(n = 0; n < sString.length(); n++){
        sBuffer.push_back(sString.at(n));
        bKnown.push_back(nKnown);
    }
    nPosition+=n;
}

void BinaryFile::WriteByte(char cByte, int nKnown){
    sBuffer.push_back(cByte);
    bKnown.push_back(nKnown);
    nPosition++;
}

void MDL::GatherChildren(Node & NODE, std::vector<Node> & ArrayOfNodes){
    NODE.Head.Children.resize(0); //Reset child array
    NODE.Head.Children.reserve(ArrayOfNodes.size());
    //std::cout<<"Gathering children...\n";
    for(int n = 0; n < ArrayOfNodes.size(); n++){
        //std::cout<<"Running loop "<<n<<" of "<<ArrayOfNodes.size()<<", comparing current index "<<NODE.Head.nNameIndex<<" to child's parent index "<<ArrayOfNodes[n].Head.nParentIndex<<".\n";
        if(ArrayOfNodes[n].Head.nParentIndex == NODE.Head.nNameIndex){
            //std::cout<<"Found a child!\n";

            //The nodes with this index is a child, adopt it

            NODE.Head.Children.push_back(ArrayOfNodes[n]);
            GatherChildren(NODE.Head.Children.back(), ArrayOfNodes);
        }
    }
    NODE.Head.Children.shrink_to_fit();
    NODE.Head.ChildrenArray.ResetToSize(NODE.Head.Children.size());
    GetNodeByNameIndex(NODE.Head.nNameIndex, NODE.nAnimation).Head.ChildrenArray.ResetToSize(NODE.Head.Children.size());
    //std::cout<<"I am adopted node "<<NODE.Head.nNameIndex<<" and my parent is "<<NODE.Head.nParentIndex<<".\n";
    //std::cout<<"I am adopted node "<<NODE.Head.nNameIndex<<" and my animation is "<<NODE.nAnimation<<".\n";
    //std::cout<<"Final number of children: "<<NODE.Head.Children.size()<<"\n";
}

void MDL::FillBinaryFields(Node & NODE, int & nMeshCounter){
    NODE.Head.ControllerArray.ResetToSize(NODE.Head.Controllers.size());
    NODE.Head.ControllerDataArray.ResetToSize(NODE.Head.ControllerData.size());
    if(NODE.Head.nType & NODE_HAS_LIGHT){
        NODE.Light.FlareSizeArray.ResetToSize(NODE.Light.FlareSizes.size());
        NODE.Light.FlarePositionArray.ResetToSize(NODE.Light.FlarePositions.size());
        NODE.Light.FlareTextureNameArray.ResetToSize(NODE.Light.FlareTextureNames.size());
        NODE.Light.FlareColorShiftArray.ResetToSize(NODE.Light.FlareColorShifts.size());
        NODE.Light.UnknownArray.ResetToSize(0);
    }
    if(NODE.Head.nType & NODE_HAS_MESH){

        //inverted counter
        nMeshCounter++;
        int Quo = nMeshCounter/100;
        int Mod = nMeshCounter%100;
        NODE.Mesh.nMeshInvertedCounter = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
        //std::cout<<"Inverted counter: "<<NODE.Mesh.nMeshInvertedCounter<<".\n";


        if(!(NODE.Head.nType & NODE_HAS_SABER)) NODE.Mesh.IndexCounterArray.ResetToSize(1);
        if(!(NODE.Head.nType & NODE_HAS_SABER)) NODE.Mesh.IndexLocationArray.ResetToSize(1);
        if(!(NODE.Head.nType & NODE_HAS_SABER)) NODE.Mesh.MeshInvertedCounterArray.ResetToSize(1);
        NODE.Mesh.nNumberOfVerts = NODE.Mesh.Vertices.size();
        NODE.Mesh.FaceArray.ResetToSize(NODE.Mesh.Faces.size());
        NODE.Mesh.nVertIndicesCount = NODE.Mesh.FaceArray.GetCount()*3;

        //Take care of the mdx pointer stuff. The only thing we need set is the bitmap, which should be done on import.
        int nMDXsize = 0;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            NODE.Mesh.nOffsetToVerticesInMDX = nMDXsize;
            nMDXsize += 12;
        }
        else NODE.Mesh.nOffsetToVerticesInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
            NODE.Mesh.nOffsetToNormalsInMDX = nMDXsize;
            nMDXsize += 12;
        }
        else NODE.Mesh.nOffsetToNormalsInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            NODE.Mesh.nOffsetToUVsInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else NODE.Mesh.nOffsetToUVsInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            NODE.Mesh.nOffsetToUV2sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else NODE.Mesh.nOffsetToUV2sInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            NODE.Mesh.nOffsetToUV3sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else NODE.Mesh.nOffsetToUV3sInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            NODE.Mesh.nOffsetToUV4sInMDX = nMDXsize;
            nMDXsize += 8;
        }
        else NODE.Mesh.nOffsetToUV4sInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
            NODE.Mesh.nOffsetToUnknownStructInMDX = nMDXsize;
            nMDXsize += 36;
        }
        else NODE.Mesh.nOffsetToUnknownStructInMDX = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
            NODE.Mesh.nOffsetToUnusedMDXStructure1 = nMDXsize;
            nMDXsize += 36;
        }
        else NODE.Mesh.nOffsetToUnusedMDXStructure1 = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
            NODE.Mesh.nOffsetToUnusedMDXStructure2 = nMDXsize;
            nMDXsize += 36;
        }
        else NODE.Mesh.nOffsetToUnusedMDXStructure2 = -1;
        if(NODE.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
            NODE.Mesh.nOffsetToUnusedMDXStructure3 = nMDXsize;
            nMDXsize += 36;
        }
        else NODE.Mesh.nOffsetToUnusedMDXStructure3 = -1;
        if(NODE.Head.nType & NODE_HAS_SKIN){
            NODE.Skin.nPointerToStruct1InMDX = nMDXsize;
            nMDXsize += 16;
            NODE.Skin.nPointerToStruct2InMDX = nMDXsize;
            nMDXsize += 16;
        }
        NODE.Mesh.nMdxDataSize = nMDXsize;
        NODE.Mesh.nOffsetToUnknownInMDX = -1;

        //Determine adjacent faces
        for(int f = 0; f < NODE.Mesh.Faces.size(); f++){
            Face & face = NODE.Mesh.Faces.at(f);
            Vertex & v1 = NODE.Mesh.Vertices.at(face.nIndexVertex[0]);
            Vertex & v2 = NODE.Mesh.Vertices.at(face.nIndexVertex[1]);
            Vertex & v3 = NODE.Mesh.Vertices.at(face.nIndexVertex[2]);
            Vector Edge1 = v2 - v1;
            Vector Edge2 = v3 - v1;
            Vector Edge3 = v3 - v2;

            //This is for face normal
            face.vNormal = Edge1 / Edge2; //Cross product, unnormalized
            face.vNormal.Normalize();

            //This is for the distance.
            face.fDistance = - (face.vNormal.fX * v1.fX +
                                face.vNormal.fY * v1.fY +
                                face.vNormal.fZ * v1.fZ);

            //Get the area of the face and update total area
            double fA = Edge1.GetLength();
            double fB = Edge2.GetLength();
            double fC = Edge3.GetLength();
            double fS = (fA + fB + fC) / 2.0;
            face.fArea = sqrtf(fS * (fS - fA) * (fS - fB) * (fS - fC));
            NODE.Mesh.fTotalArea += face.fArea;


            for(int c = f+1; c < NODE.Mesh.Faces.size(); c++){
                Face & compareface = NODE.Mesh.Faces.at(c);
                std::vector<bool> VertMatches(3, false);
                std::vector<bool> VertMatchesCompare(3, false);
                for(int a = 0; a < 3; a++){
                    int nVertIndex = face.nIndexVertex[a];
                    for(int b = 0; b < 3; b++){
                        if(NODE.Mesh.Vertices.at(nVertIndex).fX == NODE.Mesh.Vertices.at(compareface.nIndexVertex[b]).fX &&
                            NODE.Mesh.Vertices.at(nVertIndex).fY == NODE.Mesh.Vertices.at(compareface.nIndexVertex[b]).fY &&
                            NODE.Mesh.Vertices.at(nVertIndex).fZ == NODE.Mesh.Vertices.at(compareface.nIndexVertex[b]).fZ ){
                            VertMatches.at(a) = true;
                            VertMatchesCompare.at(b) = true;
                            b = 3; // we can only have one matching vert in a face per vert. Once we find a match, we're done.
                        }
                    }
                }
                if(VertMatches.at(0) && VertMatches.at(1)){
                    if(face.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 0...\n";
                    else face.nAdjacentFaces[0] = c;
                }
                else if(VertMatches.at(1) && VertMatches.at(2)){
                    if(face.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 1...\n";
                    else face.nAdjacentFaces[1] = c;
                }
                else if(VertMatches.at(2) && VertMatches.at(0)){
                    if(face.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<f<<") for edge 2...\n";
                    else face.nAdjacentFaces[2] = c;
                }
                if(VertMatchesCompare.at(0) && VertMatchesCompare.at(1)){
                    if(compareface.nAdjacentFaces[0] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<c<<") for edge 0...\n";
                    else compareface.nAdjacentFaces[0] = f;
                }
                else if(VertMatchesCompare.at(1) && VertMatchesCompare.at(2)){
                    if(compareface.nAdjacentFaces[1] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<c<<") for edge 1...\n";
                    else compareface.nAdjacentFaces[1] = f;
                }
                else if(VertMatchesCompare.at(2) && VertMatchesCompare.at(0)){
                    if(compareface.nAdjacentFaces[2] != -1) std::cout<<"Well, we found too many adjacent faces (to "<<c<<") for edge 2...\n";
                    else compareface.nAdjacentFaces[2] = f;
                }
                if(face.nAdjacentFaces[0]!=-1 &&
                   face.nAdjacentFaces[1]!=-1 &&
                   face.nAdjacentFaces[2]!=-1 ){
                    c = NODE.Mesh.Faces.size(); //Found them all, maybe I finish early?
                }
            }
        }

        double fLargestRadius = 0.0;
        double fSumX = 0.0;
        double fSumY = 0.0;
        double fSumZ = 0.0;
        for(int v = 0; v < NODE.Mesh.Vertices.size(); v++){
            Vertex & vert = NODE.Mesh.Vertices.at(v);

            //Vertex normals
            vert.MDXData.vNormal = Vector(0.0, 0.0, 0.0); //Start with null vector
            unsigned int nSmoothing = 0;

            //First, get the welded patch smoothing group
            int nPatchVectorMax = FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).size();
            for(int p = 0; p < nPatchVectorMax; p++){
                if(FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).at(p).nNameIndex == NODE.Head.nNameIndex &&
                   FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).at(p).nVertex == v){
                    //We have found the welded patch. Save its smoothing groups (we will add the normals later on)
                    nSmoothing = FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).at(p).nSmoothingGroups;
                    p = nPatchVectorMax;
                }
            }
            for(int p = 0; p < FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).size(); p++){
                Patch & patch = FH[0].MH.PatchArrayPointers.at(vert.nLinkedFacesIndex).at(p);

                //check the smoothing groups. If we are compatible, then we may take this patch of faces into account
                if( patch.nSmoothingGroups & nSmoothing ){
                    //If the smoothing group of the unwelded face patch matches any smoothing group of the welded faces,
                    //then add this patch to the vertex normal calculation
                    for(int f = 0; f < patch.FaceIndices.size(); f++){
                        Face & face = GetNodeByNameIndex(patch.nNameIndex).Mesh.Faces.at(patch.FaceIndices.at(f));

                        //Add face normal to vertex normal calculation
                        vert.MDXData.vNormal += face.vNormal * face.fArea;
                    }
                }
                //Otherwise skip
            }
            vert.MDXData.vNormal.Normalize();
            //done with vertex normal

            //Bounding Box, Radius and Average calculations
            NODE.Mesh.vBBmin.fX = std::min(NODE.Mesh.vBBmin.fX, vert.fX);
            NODE.Mesh.vBBmin.fY = std::min(NODE.Mesh.vBBmin.fY, vert.fY);
            NODE.Mesh.vBBmin.fZ = std::min(NODE.Mesh.vBBmin.fZ, vert.fZ);
            NODE.Mesh.vBBmax.fX = std::max(NODE.Mesh.vBBmax.fX, vert.fX);
            NODE.Mesh.vBBmax.fY = std::max(NODE.Mesh.vBBmax.fY, vert.fY);
            NODE.Mesh.vBBmax.fZ = std::max(NODE.Mesh.vBBmax.fZ, vert.fZ);
            fSumX += vert.fX;
            fSumY += vert.fY;
            fSumZ += vert.fZ;
            fLargestRadius = std::max(fLargestRadius, sqrt(pow(vert.fX, 2.0) + pow(vert.fY, 2.0) + pow(vert.fZ, 2.0)));
        }
        NODE.Mesh.vAverage.fX = fSumX / NODE.Mesh.Vertices.size();
        NODE.Mesh.vAverage.fY = fSumY / NODE.Mesh.Vertices.size();
        NODE.Mesh.vAverage.fZ = fSumZ / NODE.Mesh.Vertices.size();
        NODE.Mesh.fRadius = fLargestRadius;
        NODE.Mesh.FaceArray.ResetToSize(NODE.Mesh.Faces.size());
    }
    if(NODE.Head.nType & NODE_HAS_SKIN){

        NODE.Skin.nNumberOfBonemap = FH[0].MH.ArrayOfNodes.size();
        NODE.Skin.TBoneArray.ResetToSize(FH[0].MH.ArrayOfNodes.size());
        NODE.Skin.QBoneArray.ResetToSize(FH[0].MH.ArrayOfNodes.size());
        NODE.Skin.Array8Array.ResetToSize(FH[0].MH.ArrayOfNodes.size());

        //First, declare our empty location as a starting point
        Location loc;

        //Now we need to construct a path by adding all the locations from this node through all its parents to the root
        int nIndex = NODE.Head.nNameIndex;
        while(nIndex != -1){
            Node & curnode = GetNodeByNameIndex(nIndex);

            //Construct base location
            Location locNode = curnode.GetLocation();
            locNode.vPosition.Reverse();
            locNode.oOrientation.ReverseW();
            locNode.vPosition.Rotate(locNode.oOrientation);

            //Add parent location to main location
            loc.vPosition += locNode.vPosition;
            loc.oOrientation *= locNode.oOrientation;
            //On the first round, because loc is initialized with the identity orientation, locParent orientation is simply copied

            nIndex = curnode.Head.nParentIndex;
        }
        //We now have a location loc, going from our current node to the root.
        //Now we need to add to that a similar kind of path of every node in the model

        //Loop through all the nodes, and do a similar operation to get the path for every node, then adding it to loc
        for(int n = 0; n < FH[0].MH.ArrayOfNodes.size(); n++){
            Location lRecord = loc; //Make a copy of loc
            Node & curnode = FH[0].MH.ArrayOfNodes.at(n);

            nIndex = curnode.Head.nNameIndex;
            std::vector<int> Indexes;
            //The price we have to pay for not going recursive
            while(nIndex != -1){
                Indexes.push_back(nIndex);
                nIndex = GetNodeByNameIndex(nIndex).Head.nParentIndex;
            }
            //std::cout<<"Our Indexes size is: "<<Indexes.size()<<".\n";
            for(int a = Indexes.size() - 1; a >= 0; a--){
                //std::cout<<"Our current a is: "<<a<<".\n";
                //std::cout<<"Our current index is: "<<Indexes.at(a)<<".\n";
                Node & curnode2 = GetNodeByNameIndex(Indexes.at(a));
                Location locNode = curnode2.GetLocation();
                locNode.vPosition.Rotate(lRecord.oOrientation); //Note: rotating with the Record rotation!
                lRecord.vPosition += locNode.vPosition;
                lRecord.oOrientation *= locNode.oOrientation;
            }

            //Oops! Forgot the last part in MDLOps, need to rotate the vector again.
            lRecord.vPosition.Reverse();
            lRecord.oOrientation.ReverseW();
            lRecord.vPosition.Rotate(lRecord.oOrientation);

            //By now, lRecord hold the base loc + the path for this node. This should now be exactly what gets written in T and Q Bones!
            NODE.Skin.Bones.at(n).TBone = lRecord.vPosition;
            NODE.Skin.Bones.at(n).QBone = lRecord.oOrientation;
        }
    }
    if(NODE.Head.nType & NODE_HAS_DANGLY){
        NODE.Dangly.ConstraintArray.ResetToSize(NODE.Dangly.Constraints.size());
        //NODE.Dangly.Data2.resize(NODE.Dangly.ConstraintArray.GetCount()); //For now just resize it.
    }
    if(NODE.Head.nType & NODE_HAS_SABER){
        NODE.Mesh.IndexCounterArray.ResetToSize(0);
        NODE.Mesh.IndexLocationArray.ResetToSize(0);
        NODE.Mesh.MeshInvertedCounterArray.ResetToSize(0);

        //inverted counter
        NODE.Mesh.nMeshInvertedCounter = 0;
        int Quo = nMeshCounter/100;
        int Mod = nMeshCounter%100;
        NODE.Saber.nInvCount1 = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
        nMeshCounter++;
        Quo = nMeshCounter/100;
        Mod = nMeshCounter%100;
        NODE.Saber.nInvCount2 = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
    }
}

void MDL::PrepareForBinary(){
    FileHeader & Data = FH[0];
    if(bK2){
        Data.MH.GH.nFunctionPointer0 = K2_FUNCTION_POINTER_0;
        Data.MH.GH.nFunctionPointer1 = K2_FUNCTION_POINTER_1;
    }
    else{
        Data.MH.GH.nFunctionPointer0 = K1_FUNCTION_POINTER_0;
        Data.MH.GH.nFunctionPointer1 = K1_FUNCTION_POINTER_1;
    }
    Data.MH.GH.nModelType = 2;
    Data.MH.GH.nRefCount = 0;
    Data.MH.GH.nPadding[0] = 0;
    Data.MH.GH.nPadding[1] = 0;
    Data.MH.GH.nPadding[2] = 0;
    Data.MH.nUnknown1[0] = 0;
    Data.MH.nUnknown1[1] = 0;
    Data.MH.nUnknown1[2] = 1;
    Data.MH.nUnknown2 = 0;

    std::cout<<"Building LinkedFaces array...\n";
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        //Currently, this takes all meshes, including skins, danglymeshes, walkmeshes and sabers
        if(Data.MH.ArrayOfNodes.at(n).Head.nType & NODE_HAS_MESH){
            Node & node = Data.MH.ArrayOfNodes.at(n);
            for(int v = 0; v < node.Mesh.Vertices.size(); v++){
                //For every vertex of every mesh

                //Proceed only if this vertex group hasn't been processed yet
                if(node.Mesh.Vertices.at(v).nLinkedFacesIndex == -1){
                    Vertex & vert = node.Mesh.Vertices.at(v); // Make life easier
                    vert.nLinkedFacesIndex = Data.MH.LinkedFacesPointers.size(); //Update reference to new vector that is about to be created
                    Data.MH.LinkedFacesPointers.push_back(std::vector<LinkedFace>()); //Create new vector
                    std::vector<LinkedFace> & LinkedFaceArray = Data.MH.LinkedFacesPointers.back(); //Get reference to the new vector

                    //We've already gone through the nodes up to n and linked any vertices, so we can skip those
                    for(int n2 = n; n2 < Data.MH.ArrayOfNodes.size(); n2++){
                        Node & node2 = Data.MH.ArrayOfNodes.at(n2);

                        //Loop through all the faces in the mesh and look for matching vertices - theoretically, there is no way to optimize this part
                        for(int f = 0; f < node2.Mesh.Faces.size(); f++){
                            Face & face = node2.Mesh.Faces.at(f);

                            //We are now checking the three vertices
                            for(int i = 0; i < 3; i++){
                                //Check if vertices are equal (enough)
                                if(vert.Compare(node2.Mesh.Vertices.at(face.nIndexVertex[i]))){

                                    //If they are equal, regardless of weldedness, add the face to the linked faces array
                                    LinkedFaceArray.push_back(LinkedFace(n2, f, face.nIndexVertex[i]));

                                    //Also update the other vert's linked face array reference index, so we can skip processing it later.
                                    node2.Mesh.Vertices.at(face.nIndexVertex[i]).nLinkedFacesIndex = vert.nLinkedFacesIndex;

                                    //Only one vertex in a face can match our vertex, so exit this small loop
                                    i = 3;
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

        for(int lf = 0; lf < LinkedFaceVector.size(); lf++){
            if(!LinkedFaceVector.at(lf).bAssignedToPatch){
                Patch newpatch;
                LinkedFace & patchhead = LinkedFaceVector.at(lf);
                newpatch.nNameIndex = patchhead.nNameIndex;
                newpatch.nVertex = patchhead.nVertex;
                patchhead.bAssignedToPatch = true;
                newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(patchhead.nNameIndex).Mesh.Faces.at(patchhead.nFace).nSmoothingGroup;
                newpatch.FaceIndices.push_back(patchhead.nFace); //Assign first linked face index to the patch
                for(int plf = lf+1; plf < LinkedFaceVector.size(); plf++){
                    if(!LinkedFaceVector.at(plf).bAssignedToPatch){
                        //The following check is the whole point of this whole patch business
                        //The patch only contains those faces which are welded, ie.
                        //they have the same mesh index and vert index as the point we are constructing this patch for
                        if(LinkedFaceVector.at(plf).nNameIndex == newpatch.nNameIndex &&
                           LinkedFaceVector.at(plf).nVertex == newpatch.nVertex){
                            LinkedFace & linked = LinkedFaceVector.at(plf);
                            linked.bAssignedToPatch = true;
                            newpatch.nSmoothingGroups = newpatch.nSmoothingGroups | GetNodeByNameIndex(linked.nNameIndex).Mesh.Faces.at(linked.nFace).nSmoothingGroup;
                            newpatch.FaceIndices.push_back(linked.nFace); //Assign linked face index to the patch
                        }
                    }
                }
                PatchVector.push_back(std::move(newpatch));
            }
        }
    }
    std::cout<<"Done creating patches!\n";

    int nMeshCounter = 0;
    //Take care of animations and their nodes
    Data.MH.NameArray.ResetToSize(Data.MH.ArrayOfNodes.size());
    Data.MH.AnimationArray.ResetToSize(Data.MH.Animations.size());
    for(int i = 0; i < Data.MH.Animations.size(); i++){
        Animation & anim = Data.MH.Animations[i];
        anim.nFunctionPointer0 = 4284816;
        anim.nFunctionPointer1 = 4522928;
        anim.nModelType = 5;
        anim.nNumberOfObjects = Data.MH.ArrayOfNodes.size();
        anim.SoundArray.ResetToSize(anim.Sounds.size());
        Node * root = nullptr;
        nMeshCounter = 0;
        for(int n = 0; n < anim.ArrayOfNodes.size(); n++){
            //Fill binary fields of the node used by the interface
            FillBinaryFields(anim.ArrayOfNodes.at(n), nMeshCounter);
            if(anim.ArrayOfNodes[n].Head.nParentIndex == -1){
                //std::cout<<"Found an animation root node!\n";
                //Claims to be the root
                if(root != nullptr){
                    std::cout<<"Error! Trying to set the animation root when the root has already been set. It seems there are two roots.\n";
                }
                else{
                    root = &anim.ArrayOfNodes[n];
                }
            }
        }
        if(root != nullptr){
            //We have indeed found a root, now let's build the hierarchy
            anim.RootAnimationNode = *root;
            GatherChildren(anim.RootAnimationNode, anim.ArrayOfNodes);
        }
    }

    Data.MH.GH.nTotalNumberOfNodes = Data.MH.ArrayOfNodes.size(); //This number is sometimes greater, possibly including supermodel nodes?
    Node * root = nullptr;
    nMeshCounter = 0;
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        //Fill binary fields of the node used by the interface
        FillBinaryFields(Data.MH.ArrayOfNodes.at(n), nMeshCounter);
        if(Data.MH.ArrayOfNodes[n].Head.nParentIndex == -1){
            //std::cout<<"Found a root node!\n";
            //Claims to be the root
            if(root != nullptr){
                std::cout<<"Error! Trying to reset the geometry root! Index: "<<n<<"\n";
            }
            else{
                root = &Data.MH.ArrayOfNodes[n];
            }
        }
    }
    if(root != nullptr){
        //We have indeed found a root, now let's build the hierarchy
        Data.MH.RootNode = *root;
        GatherChildren(Data.MH.RootNode, Data.MH.ArrayOfNodes);
    }
}

bool MDL::Compile(){
    nPosition = 0;
    sBuffer.resize(0);
    Mdx.nPosition = 0;
    Mdx.sBuffer.resize(0);

    FileHeader * Data = &FH[0];

    //File header
    WriteInt(0, 8);
    int PHnMdlLength = nPosition;
    WriteInt(0xFFFFFFFF, 1);
    int PHnMdxLength = nPosition;
    WriteInt(0xFFFFFFFF, 1);

    //GeoHeader
    WriteInt(Data->MH.GH.nFunctionPointer0, 6);
    WriteInt(Data->MH.GH.nFunctionPointer1, 6);
    Data->MH.GH.cName.resize(32);
    WriteString(Data->MH.GH.cName, 3);

    int PHnOffsetToRootNode = nPosition;
    WriteInt(0xFFFFFFFF, 6);
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
    WriteByte(Data->MH.GH.nPadding[0], 10); //Unknown, padding?
    WriteByte(Data->MH.GH.nPadding[1], 10);
    WriteByte(Data->MH.GH.nPadding[2], 10);

    //Model header
    WriteByte(Data->MH.nClassification, 7); //Classification
    WriteByte(Data->MH.nUnknown1[0], 10); //Unknown, padding?
    WriteByte(Data->MH.nUnknown1[1], 10);
    WriteByte(Data->MH.nUnknown1[2], 10);

    WriteInt(0, 8); //Child model count

    //Animation Array
    int PHnOffsetToAnimationArray = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(Data->MH.AnimationArray.nCount, 1);
    WriteInt(Data->MH.AnimationArray.nCount2, 1);

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
    WriteInt(Data->MH.NameArray.nCount, 1);
    WriteInt(Data->MH.NameArray.nCount2, 1);

    //Create Name array
    WriteIntToPH(nPosition - 12, PHnOffsetToNameArray, Data->MH.NameArray.nOffset);

    std::vector<int> PHnOffsetToName;
    for(int c = 0; c < Data->MH.Names.size(); c++){
        PHnOffsetToName.push_back(nPosition);
        WriteInt(0xFFFFFFFF, 6);
    }
    for(int c = 0; c < Data->MH.Names.size(); c++){
        WriteIntToPH(nPosition - 12, PHnOffsetToName.at(c), Data->MH.Names.at(c).nOffset);
        WriteString(Data->MH.Names[c].cName.c_str(), 3);
        WriteByte(0, 3);
    }

    //Create Animation array
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
        WriteInt(anim.nFunctionPointer0, 6);
        WriteInt(anim.nFunctionPointer1, 6);
        anim.cName.resize(32);
        WriteString(anim.cName, 3);
        int PHnOffsetToFirstNode = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(anim.nNumberOfObjects, 1);

        WriteInt(0, 8); //Unknown constant zeroes
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);
        WriteInt(0, 8);

        WriteByte(anim.nModelType, 7);
        WriteByte(anim.nPadding[0], 10); //unknown, padding?
        WriteByte(anim.nPadding[1], 10);
        WriteByte(anim.nPadding[2], 10);

        WriteFloat(anim.fLength, 2);
        WriteFloat(anim.fTransition, 2);

        anim.cName2.resize(32);
        WriteString(anim.cName2, 3);

        int PHnOffsetToSoundArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(anim.SoundArray.nCount, 1);
        WriteInt(anim.SoundArray.nCount2, 1);
        WriteInt(0, 8); //Unknown constant
        if(anim.SoundArray.GetCount() > 0){
            WriteIntToPH(nPosition - 12, PHnOffsetToSoundArray, Data->MH.Animations[c].SoundArray.nOffset);
            for(int b = 0; b < anim.Sounds.size(); b++){
                WriteFloat(anim.Sounds[b].fTime, 2);
                anim.Sounds[b].cName.resize(32);
                WriteString(anim.Sounds[b].cName, 3);
            }
        }
        else{
            WriteIntToPH(0, PHnOffsetToSoundArray, Data->MH.Animations[c].SoundArray.nOffset);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToFirstNode, Data->MH.Animations[c].nOffsetToRootAnimationNode);
        WriteNodes(anim.RootAnimationNode);
    }
    WriteIntToPH(nPosition - 12, PHnOffsetToRootNode, Data->MH.GH.nOffsetToRootNode);
    WriteIntToPH(nPosition - 12, PHnOffsetToHeadRootNode, Data->MH.nOffsetToHeadRootNode); //For now we will make these two equal in all cases;
    WriteNodes(Data->MH.RootNode);

    WriteIntToPH(nPosition - 12, PHnMdlLength, Data->nMdlLength);
    WriteIntToPH(Mdx.nPosition, PHnMdxLength, Data->nMdxLength);
    WriteIntToPH(Mdx.nPosition, PHnMdxLength2, Data->MH.nMdxLength2);
    nBufferSize = sBuffer.size();
    Mdx.nBufferSize = Mdx.sBuffer.size();
}

void MDL::WriteAabb(Aabb & aabb){
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
    WriteInt(aabb.nChildFlag, 10);
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
    FileHeader & Data = FH[0];

    GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).nOffset = nPosition - 12;
    WriteInt(node.Head.nType, 5, 2);
    WriteInt(node.Head.nID1, 5, 2);
    WriteInt(node.Head.nNameIndex, 5, 2);
    WriteInt(0, 8, 2);

    //Next is offset to root. For geo this is 0, for animations this is offset to animation
    if(node.nAnimation == -1){
        WriteInt(0, 6);
        GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.nOffsetToRoot = 0;
    }
    else{
        WriteInt(Data.MH.Animations[node.nAnimation].nOffset, 6);
        GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.nOffsetToRoot = Data.MH.Animations[node.nAnimation].nOffset;
    }

    //Next is offset to parent.
    if(node.Head.nParentIndex == -1){
        WriteInt(0, 6);
        GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.nOffsetToParent = 0;
    }
    else{
        //std::cout<<"Found parent "<<parent.Head.nNameIndex<<". His offset is "<<parent.nOffset<<".\n";
        WriteInt(GetNodeByNameIndex(node.Head.nParentIndex, node.nAnimation).nOffset, 6);
        GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.nOffsetToParent = GetNodeByNameIndex(node.Head.nParentIndex, node.nAnimation).nOffset;
    }

    //Write position
    WriteFloat(node.Head.vPos.fX, 2);
    WriteFloat(node.Head.vPos.fY, 2);
    WriteFloat(node.Head.vPos.fZ, 2);

    //Write orientation - convert first
    node.Head.oOrient.ConvertToQuaternions();
    WriteFloat(node.Head.oOrient.qW, 2);
    WriteFloat(node.Head.oOrient.qX, 2);
    WriteFloat(node.Head.oOrient.qY, 2);
    WriteFloat(node.Head.oOrient.qZ, 2);

    //Children Array
    int PHnOffsetToChildren = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.ChildrenArray.nCount, 1);
    WriteInt(node.Head.ChildrenArray.nCount2, 1);

    //Controller Array
    int PHnOffsetToControllers = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.ControllerArray.nCount, 1);
    WriteInt(node.Head.ControllerArray.nCount2, 1);

    //Controller Data Array
    int PHnOffsetToControllerData = nPosition;
    WriteInt(0xFFFFFFFF, 6);
    WriteInt(node.Head.ControllerDataArray.nCount, 1);
    WriteInt(node.Head.ControllerDataArray.nCount2, 1);


    //Here comes all the subheader bullshit.
    /// LIGHT HEADER
    int PHnOffsetToUnknownLightArray, PHnOffsetToFlareSizes, PHnOffsetToFlarePositions, PHnOffsetToFlareTextureNames, PHnOffsetToFlareColorShifts;
    if(node.Head.nType & NODE_HAS_LIGHT){
        WriteFloat(node.Light.fFlareRadius, 2);
        PHnOffsetToUnknownLightArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.UnknownArray.nCount, 1);
        WriteInt(node.Light.UnknownArray.nCount2, 1);
        PHnOffsetToFlareSizes = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareSizeArray.nCount, 1);
        WriteInt(node.Light.FlareSizeArray.nCount2, 1);
        PHnOffsetToFlarePositions = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlarePositionArray.nCount, 1);
        WriteInt(node.Light.FlarePositionArray.nCount2, 1);
        PHnOffsetToFlareColorShifts = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareColorShiftArray.nCount, 1);
        WriteInt(node.Light.FlareColorShiftArray.nCount2, 1);
        PHnOffsetToFlareTextureNames = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Light.FlareTextureNameArray.nCount, 1);
        WriteInt(node.Light.FlareTextureNameArray.nCount2, 1);

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
        WriteInt(0, 8); //Zero 1
        WriteInt(0, 8); //Zero 2

        WriteFloat(node.Emitter.fDeadSpace, 2);
        WriteFloat(node.Emitter.fBlastRadius, 2);
        WriteFloat(node.Emitter.fBlastLength, 2);

        WriteInt(node.Emitter.nxGrid, 4);
        WriteInt(node.Emitter.nyGrid, 4);
        WriteInt(node.Emitter.nSpawnType, 4);

        node.Emitter.cUpdate.resize(32);
        WriteString(node.Emitter.cUpdate, 3);
        node.Emitter.cRender.resize(32);
        WriteString(node.Emitter.cRender, 3);
        node.Emitter.cBlend.resize(32);
        WriteString(node.Emitter.cBlend, 3);
        node.Emitter.cTexture.resize(64);
        WriteString(node.Emitter.cTexture, 3);
        node.Emitter.cChunkName.resize(16);
        WriteString(node.Emitter.cChunkName, 3);

        WriteInt(node.Emitter.nTwosidedTex, 4);
        WriteInt(node.Emitter.nLoop, 4);
        WriteInt(node.Emitter.nRenderOrder, 5, 2);
        WriteInt(node.Emitter.nUnknown6, 10, 2);
        WriteInt(node.Emitter.nFlags, 4);
    }

    /// MESH HEADER
    int PHnOffsetToFaces, PHnOffsetToIndexCount, PHnOffsetToIndexLocation, PHnOffsetToInvertedCounter, PHnOffsetIntoMdx, PHnOffsetToVertArray;
    if(node.Head.nType & NODE_HAS_MESH){
        //Write function pointers
        if(node.Head.nType & NODE_HAS_DANGLY){
            if(bK2) WriteInt(4216864, 6);
            else WriteInt(4216640, 6);
            if(bK2) WriteInt(4216848, 6);
            else WriteInt(4216624, 6);
        }
        else if(node.Head.nType & NODE_HAS_SKIN){
            if(bK2) WriteInt(4216816, 6);
            else WriteInt(4216592, 6);
            if(bK2) WriteInt(4216832, 6);
            else WriteInt(4216608, 6);
        }
        else{
            if(bK2) WriteInt(4216880, 6);
            else WriteInt(4216656, 6);
            if(bK2) WriteInt(4216896, 6);
            else WriteInt(4216672, 6);
        }

        PHnOffsetToFaces = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Mesh.FaceArray.nCount, 1);
        WriteInt(node.Mesh.FaceArray.nCount2, 1);

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

        WriteInt(node.Mesh.nShininess, 4);

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
        WriteInt(node.Mesh.IndexCounterArray.nCount, 1);
        WriteInt(node.Mesh.IndexCounterArray.nCount2, 1);
        PHnOffsetToIndexLocation = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Mesh.IndexLocationArray.nCount, 1);
        WriteInt(node.Mesh.IndexLocationArray.nCount2, 1);
        PHnOffsetToInvertedCounter = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Mesh.MeshInvertedCounterArray.nCount, 1);
        WriteInt(node.Mesh.MeshInvertedCounterArray.nCount2, 1);

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
        WriteInt(node.Mesh.nOffsetToUnknownStructInMDX, 6);
        WriteInt(node.Mesh.nOffsetToUnusedMDXStructure1, 6);
        WriteInt(node.Mesh.nOffsetToUnusedMDXStructure2, 6);
        WriteInt(node.Mesh.nOffsetToUnusedMDXStructure3, 6);

        WriteInt(node.Mesh.nNumberOfVerts, 1, 2);
        WriteInt(node.Mesh.nTextureNumber, 4, 2);

        WriteByte(node.Mesh.nHasLightmap, 7);
        WriteByte(node.Mesh.nRotateTexture, 7);
        WriteByte(node.Mesh.nBackgroundGeometry, 7);
        WriteByte(node.Mesh.nShadow, 7);
        WriteByte(node.Mesh.nBeaming, 7);
        WriteByte(node.Mesh.nRender, 7);
        WriteByte(node.Mesh.nUnknown30, 10);
        WriteByte(node.Mesh.nUnknown31, 10);

        WriteInt(node.Mesh.nUnknown32, 10, 2);
        WriteInt(node.Mesh.nUnknown33, 10, 2);

        WriteFloat(node.Mesh.fUnknown7, 10);
        WriteFloat(node.Mesh.fTotalArea, 2);
        WriteInt(node.Mesh.nK2Unknown2, 8);

        PHnOffsetIntoMdx = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        PHnOffsetToVertArray = nPosition;
        WriteInt(0xFFFFFFFF, 6);
    }

    /// SKIN HEADER
    int PHnOffsetToBonemap, PHnOffsetToQBones, PHnOffsetToTBones, PHnOffsetToArray8;
    if(node.Head.nType & NODE_HAS_SKIN){
        WriteInt(0, 10); //Unknown int32
        WriteInt(0, 10); //Unknown int32
        WriteInt(0, 10); //Unknown int32
        WriteInt(node.Skin.nPointerToStruct1InMDX, 6);
        WriteInt(node.Skin.nPointerToStruct2InMDX, 6);
        PHnOffsetToBonemap = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.nNumberOfBonemap, 1);
        PHnOffsetToQBones = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.QBoneArray.nCount, 1);
        WriteInt(node.Skin.QBoneArray.nCount2, 1);
        PHnOffsetToTBones = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.TBoneArray.nCount, 1);
        WriteInt(node.Skin.TBoneArray.nCount2, 1);
        PHnOffsetToArray8 = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Skin.Array8Array.nCount, 1);
        WriteInt(node.Skin.Array8Array.nCount2, 1);
        for(int a = 0; a < 18; a++){
            WriteInt(node.Skin.nBoneIndexes[a], 5, 2);
        }
    }

    /// DANGLY HEADER
    int PHnOffsetToConstraints, PHnOffsetToData2;
    if(node.Head.nType & NODE_HAS_DANGLY){
        PHnOffsetToConstraints = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteInt(node.Dangly.ConstraintArray.nCount, 1);
        WriteInt(node.Dangly.ConstraintArray.nCount2, 1);
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
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData1, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Saber.nOffsetToSaberData1);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].vVertex.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vVertex.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData3, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Saber.nOffsetToSaberData3);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].vNormal.fX, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fY, 2);
            WriteFloat(node.Saber.SaberData[d].vNormal.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToSaberData2, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Saber.nOffsetToSaberData2);
        for(int d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(node.Saber.SaberData[d].fUV[0], 2);
            WriteFloat(node.Saber.SaberData[d].fUV[1], 2);
        }

    }

    /// AABB DATA
    if(node.Head.nType & NODE_HAS_AABB){
        WriteIntToPH(nPosition - 12, PHnOffsetToAabb, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Walkmesh.nOffsetToAabb);
        WriteAabb(node.Walkmesh.RootAabb);
    }

    /// DANGLY DATA
    if(node.Head.nType & NODE_HAS_DANGLY){
        WriteIntToPH(nPosition - 12, PHnOffsetToConstraints, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Dangly.ConstraintArray.nOffset);
        for(int d = 0; d < node.Dangly.Constraints.size(); d++){
            WriteFloat(node.Dangly.Constraints.at(d), 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToConstraints, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Dangly.ConstraintArray.nOffset);
        for(int d = 0; d < node.Dangly.Data2.size(); d++){
            WriteFloat(node.Dangly.Data2.at(d).fValues[0], 2);
            WriteFloat(node.Dangly.Data2.at(d).fValues[1], 2);
            WriteFloat(node.Dangly.Data2.at(d).fValues[2], 2);
        }
    }

    /// SKIN DATA
    if(node.Head.nType & NODE_HAS_SKIN){
        WriteIntToPH(nPosition - 12, PHnOffsetToBonemap, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Skin.nOffsetToBonemap);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).fBonemap, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToQBones, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Skin.QBoneArray.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).QBone.qW, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.qX, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.qY, 2);
            WriteFloat(node.Skin.Bones.at(d).QBone.qZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToTBones, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Skin.TBoneArray.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(node.Skin.Bones.at(d).TBone.fX, 2);
            WriteFloat(node.Skin.Bones.at(d).TBone.fY, 2);
            WriteFloat(node.Skin.Bones.at(d).TBone.fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToArray8, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Skin.Array8Array.nOffset);
        for(int d = 0; d < node.Skin.Bones.size(); d++){
            WriteInt(0, 4); //This array is not in use, might as well fill it with zeros
        }
    }

    /// MDX DATA

    if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_SABER)){
        WriteIntToPH(Mdx.nPosition, PHnOffsetIntoMdx, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.nOffsetIntoMdx);
        for(int d = 0; d < node.Mesh.nNumberOfVerts; d++){
            Vertex & vert = node.Mesh.Vertices.at(d);
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                Mdx.WriteFloat(vert.MDXData.vVertex.fX, 2);
                Mdx.WriteFloat(vert.MDXData.vVertex.fY, 2);
                Mdx.WriteFloat(vert.MDXData.vVertex.fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
                Mdx.WriteFloat(vert.MDXData.vNormal.fX, 2);
                Mdx.WriteFloat(vert.MDXData.vNormal.fY, 2);
                Mdx.WriteFloat(vert.MDXData.vNormal.fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
                Mdx.WriteFloat(vert.MDXData.fUV1[0], 2);
                Mdx.WriteFloat(vert.MDXData.fUV1[1], 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
                Mdx.WriteFloat(vert.MDXData.fUV2[0], 2);
                Mdx.WriteFloat(vert.MDXData.fUV2[1], 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
                Mdx.WriteFloat(vert.MDXData.fUV3[0], 2);
                Mdx.WriteFloat(vert.MDXData.fUV3[1], 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
                Mdx.WriteFloat(vert.MDXData.fUV4[0], 2);
                Mdx.WriteFloat(vert.MDXData.fUV4[1], 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
                Mdx.WriteFloat(vert.MDXData.vTangent1[0].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[0].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[0].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[1].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[1].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[1].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[2].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[2].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent1[2].fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
                Mdx.WriteFloat(vert.MDXData.vTangent2[0].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[0].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[0].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[1].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[1].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[1].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[2].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[2].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent2[2].fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
                Mdx.WriteFloat(vert.MDXData.vTangent3[0].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[0].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[0].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[1].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[1].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[1].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[2].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[2].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent3[2].fZ, 2);
            }
            if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
                Mdx.WriteFloat(vert.MDXData.vTangent4[0].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[0].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[0].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[1].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[1].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[1].fZ, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[2].fX, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[2].fY, 2);
                Mdx.WriteFloat(vert.MDXData.vTangent4[2].fZ, 2);
            }
            if(node.Head.nType & NODE_HAS_SKIN){
                Mdx.WriteFloat(vert.MDXData.fSkin1[0], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin1[1], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin1[2], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin1[3], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin2[0], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin2[1], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin2[2], 2);
                Mdx.WriteFloat(vert.MDXData.fSkin2[3], 2);
            }
        }

        /// Also write the extra empty vert data
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
            if(node.Head.nType & NODE_HAS_SKIN) Mdx.WriteFloat(1000000.0, 8);
            else Mdx.WriteFloat(10000000.0, 8);
            if(node.Head.nType & NODE_HAS_SKIN) Mdx.WriteFloat(1000000.0, 8);
            else Mdx.WriteFloat(10000000.0, 8);
            if(node.Head.nType & NODE_HAS_SKIN) Mdx.WriteFloat(1000000.0, 8);
            else Mdx.WriteFloat(10000000.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_NORMAL){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV1){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV2){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV3){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_UV4){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT1){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT2){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT3){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Mesh.nMdxDataBitmap & MDX_FLAG_HAS_TANGENT4){
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
        if(node.Head.nType & NODE_HAS_SKIN){
            Mdx.WriteFloat(1.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
            Mdx.WriteFloat(0.0, 8);
        }
    }

    /// MESH DATA
    if(node.Head.nType & NODE_HAS_MESH){
        WriteIntToPH(nPosition - 12, PHnOffsetToFaces, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.FaceArray.nOffset);
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
        WriteIntToPH(nPosition - 12, PHnOffsetToIndexCount, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.IndexCounterArray.nOffset);
        WriteInt(node.Mesh.nVertIndicesCount, 1);
        WriteIntToPH(nPosition - 12, PHnOffsetToVertArray, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.nOffsetToVertArray);
        for(int d = 0; d < node.Mesh.Vertices.size(); d++){
            WriteFloat(node.Mesh.Vertices.at(d).fX, 2);
            WriteFloat(node.Mesh.Vertices.at(d).fY, 2);
            WriteFloat(node.Mesh.Vertices.at(d).fZ, 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToIndexLocation, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.IndexLocationArray.nOffset);
        int PHnPointerToIndexLocation = nPosition;
        WriteInt(0xFFFFFFFF, 6);
        WriteIntToPH(nPosition - 12, PHnOffsetToInvertedCounter, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.MeshInvertedCounterArray.nOffset);
        WriteInt(node.Mesh.nMeshInvertedCounter, 4);
        WriteIntToPH(nPosition - 12, PHnPointerToIndexLocation, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Mesh.nVertIndicesLocation);
        for(int d = 0; d < node.Mesh.VertIndices.size(); d++){
            WriteInt(node.Mesh.VertIndices.at(d).nValues[0], 5, 2);
            WriteInt(node.Mesh.VertIndices.at(d).nValues[1], 5, 2);
            WriteInt(node.Mesh.VertIndices.at(d).nValues[2], 5, 2);
        }



    }

    /// LIGHT DATA
    if(node.Head.nType & NODE_HAS_LIGHT){
        WriteIntToPH(nPosition - 12, PHnOffsetToUnknownLightArray, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.UnknownArray.nOffset);
        /// TODO: Write down the array data. Since I have never seen this data, I have no idea how I would write it down if it were present.
        /// So, this will be determined sometime in the future, or maybe never if the array is completely unused.
        WriteIntToPH(nPosition - 12, PHnOffsetToFlareTextureNames, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.FlareTextureNameArray.nOffset);
        std::vector<int> PHnTextureNameOffset;
        for(int d = 0; d < node.Light.FlareTextureNames.size(); d++){
            PHnTextureNameOffset.push_back(nPosition);
            WriteInt(0xFFFFFFFF, 6);
        }
        for(int d = 0; d < node.Light.FlareTextureNames.size(); d++){
            WriteIntToPH(nPosition - 12, PHnTextureNameOffset.at(d), GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.FlareTextureNames.at(d).nOffset);
            WriteString(node.Light.FlareTextureNames.at(d).cName.c_str(), 3);
            WriteByte(0, 3);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToFlareSizes, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.FlareSizeArray.nOffset);
        for(int d = 0; d < node.Light.FlareSizes.size(); d++){
            WriteFloat(node.Light.FlareSizes.at(d), 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToFlarePositions, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.FlarePositionArray.nOffset);
        for(int d = 0; d < node.Light.FlarePositions.size(); d++){
            WriteFloat(node.Light.FlarePositions.at(d), 2);
        }
        WriteIntToPH(nPosition - 12, PHnOffsetToFlareColorShifts, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Light.FlareColorShiftArray.nOffset);
        for(int d = 0; d < node.Light.FlareColorShifts.size(); d++){
            WriteFloat(node.Light.FlareColorShifts.at(d).fR, 2);
            WriteFloat(node.Light.FlareColorShifts.at(d).fG, 2);
            WriteFloat(node.Light.FlareColorShifts.at(d).fB, 2);
        }
    }
    /// DONE WITH SUBHEADERS

    //We get to the children array
    WriteIntToPH(nPosition - 12, PHnOffsetToChildren, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.ChildrenArray.nOffset);
    std::vector<int> PHnOffsetToChild;
    for(int a = 0; a < node.Head.Children.size(); a++){
        PHnOffsetToChild.push_back(nPosition);
        WriteInt(0xFFFFFFFF, 6);
    }
    for(int a = 0; a < node.Head.Children.size(); a++){
        WriteIntToPH(nPosition - 12, PHnOffsetToChild.at(a), GetNodeByNameIndex(node.Head.Children.at(a).Head.nNameIndex, node.nAnimation).nOffset);
        WriteNodes(node.Head.Children.at(a));
    }

    //We get to the controller array
    if(node.Head.ControllerArray.GetCount() > 0) WriteIntToPH(nPosition-12, PHnOffsetToControllers, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.ControllerArray.nOffset);
    else WriteIntToPH(0, PHnOffsetToControllers, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.ControllerArray.nOffset);
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
    if(node.Head.ControllerDataArray.GetCount() > 0) WriteIntToPH(nPosition-12, PHnOffsetToControllerData, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.ControllerDataArray.nOffset);
    else WriteIntToPH(0, PHnOffsetToControllerData, GetNodeByNameIndex(node.Head.nNameIndex, node.nAnimation).Head.ControllerDataArray.nOffset);
    for(int a = 0; a < node.Head.ControllerData.size(); a++){
        WriteFloat(node.Head.ControllerData.at(a), 2);
    }

    //Done you are, padawan
}
