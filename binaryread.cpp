#include "MDL.h"
#include <algorithm>

/**
    Functions:
    MDL::LinearizeGeometry()
    MDL::LinearizeAnimation()
    MDL::DecompileModel()
    MDL::ParseAabb()
    MDL::ParseNode()
/**/

bool bReadSmoothing = false;

/// Linearize the nodes from being contained inside one another to the ArrayOfNodes in Name Index order.
void MDL::LinearizeGeometry(Node & node, std::vector<Node> & ArrayOfNodes){
    for(Node & child : node.Head.Children){
        if(child.nOffset != 0) LinearizeGeometry(child, ArrayOfNodes);
    }
    ArrayOfNodes.at(node.Head.nNameIndex) = std::move(node);
}

/// Linearize the nodes from being contained inside one another to the ArrayOfNodes in Node Index order (root first, then first child, its first child, etc.).
void MDL::LinearizeAnimation(Node & node, std::vector<Node> & ArrayOfNodes){
    ModelHeader & Data = FH->MH;

    ArrayOfNodes.push_back(std::move(node));
    Node & node1 = ArrayOfNodes.back();
    for(unsigned int n = 0; n < node1.Head.Children.size(); n++){
        Node & child = node1.Head.Children.at(n);
        if(child.nOffset != 0) LinearizeAnimation(child, ArrayOfNodes);
    }
}

/// This function will read the binary model.
/// When bMinimal is true, the algorithm will not read data such as the animations and will not output most of the debugging messages.
void MDL::DecompileModel(bool bMinimal){
    if(sBuffer.empty()) return;

    src = BinarySource;
    ReportObject ReportMdl (*this);

    /// Start timer
    Timer tDecompile;

    int nNodeCounter;
    nPosition = 0; /// Set reading position to beginning of file.

    FH.reset(new FileHeader());
    if(!bMinimal) ReportMdl << "Begin decompiling ";
    Report("Decompiling...");

    FileHeader & Data = *FH;
    //std::cout << "Data ready.\n";
    std::string sFileHeader = "File Header > ";

    //First read the file header, geometry header and model header
    ReadNumber(&Data.nZero, 8, sFileHeader + "Padding");
    //std::cout << "Read first value, pos " << nPosition << ".\n";
    ReadNumber(&Data.nMdlLength, 1, sFileHeader + "MDL Length");
    ReadNumber(&Data.nMdxLength, 1, sFileHeader + "MDX Length");
    MarkDataBorder(nPosition - 1);
    //if(!bMinimal) ReportMdl << "File header read, pos " << nPosition << ".\n";

    std::string sGeometryHeader = "Geometry Header > ";

    ReadNumber(&Data.MH.GH.nFunctionPointer0, 9, sGeometryHeader + "Function Pointer 1");
    ReadNumber(&Data.MH.GH.nFunctionPointer1, 9, sGeometryHeader + "Function Pointer 2");
    //std::cout << "Read function pointers, now pos is: " << nPosition << ".\n";

    //Get game and platform
    //std::cout << "Function pointer 0: "<< Data.MH.GH.nFunctionPointer0 <<".\n";
    if(Data.MH.GH.nFunctionPointer0 == FN_PTR_PC_K1_MODEL_1) bK2 = false, bXbox = false;
    else if(Data.MH.GH.nFunctionPointer0 == FN_PTR_PC_K2_MODEL_1) bK2 = true, bXbox = false;
    else if(Data.MH.GH.nFunctionPointer0 == FN_PTR_XBOX_K2_MODEL_1) bK2 = true, bXbox = true;
    else if(Data.MH.GH.nFunctionPointer0 == FN_PTR_XBOX_K1_MODEL_1)  bK2 = false, bXbox = true;
    else throw mdlexception("Cannot interpret model function pointer (" + std::to_string(Data.MH.GH.nFunctionPointer0) + "), so cannot determine game and platform.");
    //std::cout << "Determined game.\n";

    ReadString(&Data.MH.GH.sName, 32, 3, sGeometryHeader + "Model Name");
    if(!bMinimal) ReportMdl << Data.MH.GH.sName.c_str() << ".\n";
    ReadNumber(&Data.MH.GH.nOffsetToRootNode, 6, sGeometryHeader + "Offset To Root Node");
    ReadNumber(&Data.MH.GH.nTotalNumberOfNodes, 1, sGeometryHeader + "Total Number Of Nodes");

    ReadNumber(&Data.MH.GH.RuntimeArray1.nOffset, 8, sGeometryHeader + "Runtime Array 1 > Offset");
    ReadNumber(&Data.MH.GH.RuntimeArray1.nCount, 8, sGeometryHeader + "Runtime Array 1 > Count 1");
    ReadNumber(&Data.MH.GH.RuntimeArray1.nCount2, 8, sGeometryHeader + "Runtime Array 1 > Count 2");
    ReadNumber(&Data.MH.GH.RuntimeArray2.nOffset, 8, sGeometryHeader + "Runtime Array 2 > Offset");
    ReadNumber(&Data.MH.GH.RuntimeArray2.nCount, 8, sGeometryHeader + "Runtime Array 2 > Count 1");
    ReadNumber(&Data.MH.GH.RuntimeArray2.nCount2, 8, sGeometryHeader + "Runtime Array 2 > Count 2");
    ReadNumber(&Data.MH.GH.nRefCount, 8, sGeometryHeader + "Reference Count");

    ReadNumber(&Data.MH.GH.nModelType, 7, sGeometryHeader + "Model Type");
    ReadNumber(&Data.MH.GH.nPadding[0], 11, sGeometryHeader + "Padding");
    ReadNumber(&Data.MH.GH.nPadding[1], 11, sGeometryHeader + "Padding");
    ReadNumber(&Data.MH.GH.nPadding[2], 11, sGeometryHeader + "Padding");
    MarkDataBorder(nPosition - 1);
    //if(!bMinimal) ReportMdl << "Geometry header read.\n";

    std::string sModelHeader = "Model Header > ";

    ReadNumber(&Data.MH.nClassification, 7, sModelHeader + "Classification");
    ReadNumber(&Data.MH.nSubclassification, 10, sModelHeader + "\"Subclassification\"");
    ReadNumber(&Data.MH.nUnknown, 8, sModelHeader + "Unknown");
    ReadNumber(&Data.MH.nAffectedByFog, 7, sModelHeader + "Affected By Fog");

    /// If the proper flags are on, enable SG reading
    bReadSmoothing = (bWriteSmoothing && Data.MH.nUnknown == 1) ? true : false;

    ReadNumber(&Data.MH.nChildModelCount, 8, sModelHeader + "Child Model Count");

    ReadNumber(&Data.MH.AnimationArray.nOffset, 6, sModelHeader + "Animation Array > Offset");
    ReadNumber(&Data.MH.AnimationArray.nCount, 1, sModelHeader + "Animation Array > Count 1");
    ReadNumber(&Data.MH.AnimationArray.nCount2, 1, sModelHeader + "Animation Array > Count 2");
    ReadNumber(&Data.MH.nSupermodelReference, 11, sModelHeader + "Supermodel Reference");

    Data.MH.vBBmin.fX = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Min > X");
    Data.MH.vBBmin.fY = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Min > Y");
    Data.MH.vBBmin.fZ = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Min > Z");
    Data.MH.vBBmax.fX = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Max > X");
    Data.MH.vBBmax.fY = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Max > Y");
    Data.MH.vBBmax.fZ = ReadNumber<float>(nullptr, 2, sModelHeader + "Bounding Box Max > Z");
    Data.MH.fRadius = ReadNumber<float>(nullptr, 2, sModelHeader + "Radius");
    Data.MH.fScale = ReadNumber<float>(nullptr, 2, sModelHeader + "Scale");

    ReadString(&Data.MH.cSupermodelName, 32, 3, sModelHeader + "Supermodel Name");

    ReadNumber(&Data.MH.nOffsetToHeadRootNode, 6, sModelHeader + "Offset To Head Root Node");
    ReadNumber(&Data.MH.nPadding, 8, sModelHeader + "Padding");
    ReadNumber(&Data.MH.nMdxLength2, 1, sModelHeader + "MDX Length");
    ReadNumber(&Data.MH.nOffsetIntoMdx, 8, sModelHeader + "MDX Offset");

    ReadNumber(&Data.MH.NameArray.nOffset, 6, sModelHeader + "Name Array > Offset");
    ReadNumber(&Data.MH.NameArray.nCount, 1, sModelHeader + "Name Array > Count 1");
    ReadNumber(&Data.MH.NameArray.nCount2, 1, sModelHeader + "Name Array > Count 2");
    MarkDataBorder(nPosition - 1);
    //if(!bMinimal) ReportMdl << "Model header read.\n";

    /// The header is fully done!
    /// Now we're equipped to disassemble the rest
    /// First index names array
    //if(!bMinimal) ReportMdl << "Reading names.\n";
    if(Data.MH.NameArray.nCount > 0){
        std::string sNames = "Names > ";
        Data.MH.Names.resize(Data.MH.NameArray.nCount);
        nPosition = MDL_OFFSET + Data.MH.NameArray.nOffset;
        for(int n = 0; n < Data.MH.NameArray.nCount; n++){
            ReadNumber(&Data.MH.Names[n].nOffset, 6, sNames + "Offset " + std::to_string(n));
            MarkDataBorder(nPosition - 1);
            unsigned nPosData = MDL_OFFSET + Data.MH.Names[n].nOffset;
            ReadString(&Data.MH.Names[n].sName, 0, 3, sNames + "Name " + std::to_string(n) + " (" + std::string(reinterpret_cast<const char*>(&GetBuffer().at(nPosData))) + ")", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    //if(!bMinimal) ReportMdl << "Name array read.\n";

    Report("Decompiling animations...");
    //if(!bMinimal) ReportMdl << "Reading animations.\n";
    /// Next, animations. Skip them if we're reading minimally
    if(Data.MH.AnimationArray.nCount > 0 && !bMinimal){
        Data.MH.Animations.resize(Data.MH.AnimationArray.nCount);
        nPosition = MDL_OFFSET + Data.MH.AnimationArray.nOffset;
        SavePosition(0);
        for(int n = 0; n < Data.MH.AnimationArray.nCount; n++){
            RestorePosition(0);
            std::string sAnimation = "Anim " + std::to_string(n) + " > ";

            Animation & anim = Data.MH.Animations.at(n);
            ReadNumber(&anim.nOffset, 6, sAnimation + "Offset");
            MarkDataBorder(nPosition - 1);

            SavePosition(0);
            nPosition = MDL_OFFSET + anim.nOffset;
            sAnimation = "Anim "  + std::string(reinterpret_cast<const char*>(&GetBuffer().at(nPosition + 8))) + " (" + std::to_string(n) + ") > ";
            ReadNumber(&anim.nFunctionPointer0, 9, sAnimation + "Function Pointer 1");
            ReadNumber(&anim.nFunctionPointer1, 9, sAnimation + "Function Pointer 2");

            ReadString(&anim.sName, 32, 3, sAnimation + "Name");

            ReadNumber(&anim.nOffsetToRootAnimationNode, 6, sAnimation + "Offset To Root Node");
            ReadNumber(&anim.nNumberOfNames, 1, sAnimation + "Total Number Of Nodes");

            ReadNumber(&anim.RuntimeArray1.nOffset, 8, sAnimation + "Runtime Array 1 > Offset");
            ReadNumber(&anim.RuntimeArray1.nCount, 8, sAnimation + "Runtime Array 1 > Count 1");
            ReadNumber(&anim.RuntimeArray1.nCount2, 8, sAnimation + "Runtime Array 1 > Count 2");
            ReadNumber(&anim.RuntimeArray2.nOffset, 8, sAnimation + "Runtime Array 2 > Offset");
            ReadNumber(&anim.RuntimeArray2.nCount, 8, sAnimation + "Runtime Array 2 > Count 1");
            ReadNumber(&anim.RuntimeArray2.nCount2, 8, sAnimation + "Runtime Array 2 > Count 2");
            ReadNumber(&anim.nRefCount, 8, sAnimation + "Reference Count");

            ReadNumber(&anim.nModelType, 7, sAnimation + "Model Type");
            ReadNumber(&anim.nPadding[0], 11, sAnimation + "Padding");
            ReadNumber(&anim.nPadding[1], 11, sAnimation + "Padding");
            ReadNumber(&anim.nPadding[2], 11, sAnimation + "Padding");
            MarkDataBorder(nPosition - 1);

            anim.fLength = ReadNumber<float>(nullptr, 2, sAnimation + "Length");
            anim.fTransition = ReadNumber<float>(nullptr, 2, sAnimation + "Transition");

            ReadString(&anim.sAnimRoot, 32, 3, sAnimation + "AnimRoot");

            ReadNumber(&anim.EventArray.nOffset, 6, sAnimation + "Event Array > Offset");
            ReadNumber(&anim.EventArray.nCount, 1, sAnimation + "Event Array > Count 1");
            ReadNumber(&anim.EventArray.nCount2, 1, sAnimation + "Event Array > Count 2");
            ReadNumber(&anim.nPadding2, 8, sAnimation + "Padding");
            MarkDataBorder(nPosition - 1);

            if(anim.EventArray.nCount > 0){
                anim.Events.resize(anim.EventArray.nCount);
                nPosition = MDL_OFFSET + anim.EventArray.nOffset; /// No need to save the previous position because we've finished the header
                //std::cout << string_format("Offset to Animation Events is %i\n", anim.EventArray.nOffset);
                for(int e = 0; e < anim.Events.size(); e++){
                    Event & event = anim.Events.at(e);
                    event.fTime = ReadNumber<float>(nullptr, 2, sAnimation + "Event " + std::to_string(e) + " > Time");
                    ReadString(&event.sName, 32, 3, sAnimation + "Event " + std::to_string(e) + " > Name (" + std::string(reinterpret_cast<const char*>(&GetBuffer().at(nPosition))) + ")");
                    MarkDataBorder(nPosition - 1);
                }
            }

            /// We're done with the header, now we delve into animation nodes. It's a bit scary :(

            /// Prepare root node
            anim.RootAnimationNode.nOffset = anim.nOffsetToRootAnimationNode;
            anim.RootAnimationNode.nAnimation = n;

            /// Prepare for parsing and parse
            std::vector<unsigned int> offsets;
            offsets.reserve(Data.MH.nNodeCount);
            Vector vFromRoot;
            Quaternion qFromRoot;
            ParseNode(anim.RootAnimationNode, offsets, vFromRoot, qFromRoot, sAnimation);

            /// Prepare for linearization and linearize
            anim.ArrayOfNodes.clear();
            anim.ArrayOfNodes.reserve(Data.MH.Names.size());
            LinearizeAnimation(anim.RootAnimationNode, anim.ArrayOfNodes);
        }
    }
    //if(!bMinimal) ReportMdl << "Animation array read.\n";
    Report("Decompiling geometry...");
    //if(!bMinimal) ReportMdl << "Reading geometry.\n";
    if(Data.MH.Names.size() > 0){
        /// Set offset and animation
        Data.MH.RootNode.nOffset = Data.MH.GH.nOffsetToRootNode;
        Data.MH.RootNode.nAnimation = -1;

        std::string sGeometry = "Geometry > ";

        /// Prepare variables for parsing nodes and then parse the nodes.
        std::vector<unsigned int> offsets;
        offsets.reserve(Data.MH.nNodeCount);
        Vector vFromRoot;
        Quaternion qFromRoot;
        ParseNode(Data.MH.RootNode, offsets, vFromRoot, qFromRoot, sGeometry, bMinimal);

        /// Record total real node count
        Data.MH.nNodeCount = offsets.size();
        //std::cout << string_format("Node count for the Geometry: %i, compared to the number in the header, %i.\n", nNodeCounter, Data.MH.GH.nNumberOfNodes);
        //Data.MH.ArrayOfNodes.clear();

        /// Linearize read geometry into an Array Of Nodes
        Data.MH.ArrayOfNodes.resize(Data.MH.nNodeCount); /// Only create as many nodes as there actually are, ignore the extra names
        LinearizeGeometry(Data.MH.RootNode, Data.MH.ArrayOfNodes);

        /// Build Array of Indices By Tree Order
        Data.MH.NameIndicesInTreeOrder.reserve(Data.MH.ArrayOfNodes.size());
        Data.MH.BuildTreeOrderArray(Data.MH.ArrayOfNodes.front());

        /// Immediately fix all the skin bone->name maps
        for(Node & node : Data.MH.ArrayOfNodes){
            if(node.Head.nType & NODE_SKIN){
                int nBoneCount = 0;
                for(Bone & bone : node.Skin.Bones){
                    if(bone.nBonemap.Valid()) nBoneCount++;
                }
                node.Skin.BoneNameIndices.resize(nBoneCount);
                for(int n = 0; n < node.Skin.Bones.size(); n++){
                    Bone & bone = node.Skin.Bones.at(n);
                    if(bone.nBonemap.Valid()){
                        node.Skin.BoneNameIndices.at(bone.nBonemap) = Data.MH.NameIndicesInTreeOrder.at(n);
                    }
                    bone.nNameIndex = Data.MH.NameIndicesInTreeOrder.at(n);
                }
            }
        }

    }
    //if(!bMinimal) ReportMdl << "Geometry read.\n";

    /// Here we'll go around and fix all the animation node numbers to match the geometry nodes.
    for(Animation & anim : Data.MH.Animations){
        /// Fix the name indices in node, name indices in controllers and child indices.
        for(Node & anim_node : anim.ArrayOfNodes){
            for(Node & geom_node : Data.MH.ArrayOfNodes){
                /// When you find the corresponding geom node, copy the name index
                if(anim_node.Head.nSupernodeNumber == geom_node.Head.nSupernodeNumber){
                    if(anim_node.Head.nNameIndex != geom_node.Head.nNameIndex){
                        /// We also need to correct it in all the controllers
                        for(Controller & ctrl : anim_node.Head.Controllers){
                            ctrl.nNameIndex = geom_node.Head.nNameIndex;
                        }
                        /// We need to find the parent node to change the child index
                        for(Node & anim_node2 : anim.ArrayOfNodes) if(anim_node2.Head.nSupernodeNumber == anim_node.Head.nParentIndex){
                            for(auto & child_ind : anim_node2.Head.ChildIndices) if(child_ind == anim_node.Head.nNameIndex)
                            {
                                child_ind = geom_node.Head.nNameIndex;
                                break;
                            }
                            break;
                        }
                        /// Finally, change the name index on the node itself
                        anim_node.Head.nNameIndex = geom_node.Head.nNameIndex;
                    }
                    break;
                }
            }
        }
        /// Fix parent indices.
        for(Node & anim_node : anim.ArrayOfNodes){
            for(Node & geom_node : Data.MH.ArrayOfNodes){
                /// When you find the corresponding parent geom node, copy the name index
                /// Q: why is the parent index set to the supernode number at this point?
                /// A: because otherwise how the heck are we gonna know that its the right one
                if(anim_node.Head.nParentIndex == geom_node.Head.nSupernodeNumber){
                    if(anim_node.Head.nParentIndex != geom_node.Head.nNameIndex){
                        anim_node.Head.nParentIndex = geom_node.Head.nNameIndex;
                    }
                    break;
                }
            }
        }
        /// Only now can we record the child indices // This is now done above already
        /*
        for(Node & anim_node : anim.ArrayOfNodes){
            anim_node.Head.ChildIndices.clear();
            for(Node & child : anim_node.Head.Children){
                anim_node.Head.ChildIndices.push_back(child.Head.nNameIndex);
            }
        }
        */
    }

    if(!bMinimal) ReportMdl << "Decompiled model in " << tDecompile.GetTime() << ".\n";
    if(!bReadSmoothing && bBinaryPostProcess && Mdx && !bMinimal) BinaryPostProcess();
}

static unsigned nAabbCount = 0;
void MDL::ParseAabb(Aabb & aabb, unsigned int nHighestOffset, const std::string & sPrefix){
    std::string sAabb = sPrefix + "AABB " + std::to_string(nAabbCount) + " > ";
    nAabbCount++;
    if(aabb.nOffset > nHighestOffset) nHighestOffset = aabb.nOffset;
    else{
        MessageBox(NULL, "The aabb (walkmesh) tree seems to be looping, the .mdl might be broken. =(", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    if(aabb.nOffset == -1) throw mdlexception("An aabb node has offset -1.");

    unsigned int nPosData = aabb.nOffset + MDL_OFFSET;
    aabb.vBBmin.fX = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Min > X", &nPosData);
    aabb.vBBmin.fY = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Min > Y", &nPosData);
    aabb.vBBmin.fZ = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Min > Z", &nPosData);
    aabb.vBBmax.fX = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Max > X", &nPosData);
    aabb.vBBmax.fY = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Max > Y", &nPosData);
    aabb.vBBmax.fZ = ReadNumber<float>(nullptr, 2, sAabb + "Bounding Box Max > Z", &nPosData);
    ReadNumber(&aabb.nChild1, 6, sAabb + "Child 1 Offset", &nPosData);
    ReadNumber(&aabb.nChild2, 6, sAabb + "Child 2 Offset", &nPosData);
    ReadNumber(aabb.nID.GetPtr(), 4, sAabb + "Face Index", &nPosData);
    ReadNumber(&aabb.nProperty, 4, sAabb + "Second Child Property", &nPosData);
    MarkDataBorder(nPosData - 1);

    if(aabb.nChild1 > 0){
        aabb.Child1.resize(1);
        aabb.Child1[0].nOffset = aabb.nChild1;
        ParseAabb(aabb.Child1[0], nHighestOffset, sPrefix);
    }
    if(aabb.nChild2 > 0){
        aabb.Child2.resize(1);
        aabb.Child2[0].nOffset = aabb.nChild2;
        ParseAabb(aabb.Child2[0], nHighestOffset, sPrefix);
    }
}

void MDL::ParseNode(Node & node, std::vector<unsigned int> & offsets, Vector vFromRoot, Quaternion qFromRoot, const std::string & sPrefix, bool bMinimal){
    SavePosition(1);
    nPosition = MDL_OFFSET + node.nOffset;
    unsigned int nPosData = 0;

    offsets.push_back(node.nOffset);

    std::string sNode = sPrefix + std::string(GetFileData()->MH.Names.at(*(unsigned short*) &sBuffer.at(nPosition + 4)).sName.c_str()) + " (" + std::to_string(offsets.size() - 1) + ") > ";

    ReadNumber(node.Head.nType.GetPtr(), 5, sNode + "Type");
    ReadNumber(node.Head.nSupernodeNumber.GetPtr(), 5, sNode + "Supernode Index");
    ReadNumber(node.Head.nNameIndex.GetPtr(), 5, sNode + "Name Index");
    ReadNumber(&node.Head.nPadding1, 8, sNode + "Padding");

    ReportObject ReportMdl (*this);
    //ReportMdl << "Reading " << (node.nAnimation.Valid() ? "animation" : "geometry") << " node " << GetNodeName(node) << " at offset " << nPosition - 8 << ".\n";

    if(!(node.Head.nType & NODE_HEADER)) throw mdlexception(std::string("The ") + (node.nAnimation.Valid() ? "animation" : "geometry") + " node " + GetNodeName(node) + " does not have a NODE_HEADER.");

    ReadNumber(node.Head.nOffsetToRoot.GetPtr(), 6, sNode + "Offset To Root");
    ReadNumber(node.Head.nOffsetToParent.GetPtr(), 6, sNode + "Offset To Parent");
    node.Head.vPos.fX = ReadNumber<float>(nullptr, 2, sNode + "Position > X");
    node.Head.vPos.fY = ReadNumber<float>(nullptr, 2, sNode + "Position > Y");
    node.Head.vPos.fZ = ReadNumber<float>(nullptr, 2, sNode + "Position > Z");

    double fQW = ReadNumber<float>(nullptr, 2, sNode + "Orientation > W");
    double fQX = ReadNumber<float>(nullptr, 2, sNode + "Orientation > X");
    double fQY = ReadNumber<float>(nullptr, 2, sNode + "Orientation > Y");
    double fQZ = ReadNumber<float>(nullptr, 2, sNode + "Orientation > Z");
    node.Head.oOrient.SetQuaternion(fQX, fQY, fQZ, fQW);

    /// Let's do the transformations/translations here. First orientation, then translation.
    Vector vAdd = node.Head.vPos;
    vAdd.Rotate(qFromRoot);
    vFromRoot += vAdd;
    qFromRoot *= node.Head.oOrient.GetQuaternion();

    node.Head.vFromRoot = vFromRoot;
    node.Head.qFromRoot = qFromRoot;

    ReadNumber(&node.Head.ChildrenArray.nOffset, 6, sNode + "Child Array > Offset");
    ReadNumber(&node.Head.ChildrenArray.nCount, 1, sNode + "Child Array > Count 1");
    ReadNumber(&node.Head.ChildrenArray.nCount2, 1, sNode + "Child Array > Count 2");

    ReadNumber(&node.Head.ControllerArray.nOffset, 6, sNode + "Controller Array > Offset");
    ReadNumber(&node.Head.ControllerArray.nCount, 1, sNode + "Controller Array > Count 1");
    ReadNumber(&node.Head.ControllerArray.nCount2, 1, sNode + "Controller Array > Count 2");

    ReadNumber(&node.Head.ControllerDataArray.nOffset, 6, sNode + "Controller Data Array > Offset");
    ReadNumber(&node.Head.ControllerDataArray.nCount, 1, sNode + "Controller Data Array > Count 1");
    ReadNumber(&node.Head.ControllerDataArray.nCount2, 1, sNode + "Controller Data Array > Count 2");
    MarkDataBorder(nPosition - 1);

    if(node.Head.ControllerDataArray.nCount > 0 && !bMinimal){
        /// We gots controll data!
        node.Head.ControllerData.resize(node.Head.ControllerDataArray.nCount);
        nPosData = MDL_OFFSET + node.Head.ControllerDataArray.nOffset;
        for(int n = 0; n < node.Head.ControllerDataArray.nCount; n++){
            node.Head.ControllerData[n] = ReadNumber<float>(nullptr, 2, sNode + "Controller Data " + std::to_string(n), &nPosData);
            MarkDataBorder(nPosData - 1);
            //if(n == node.Head.ControllerDataArray.nCount) std::cout << string_format("Just filled %i floats of Controller Data\n", n);
        }
    }

    if(node.Head.ControllerArray.nCount > 0 && !bMinimal){
        node.Head.Controllers.resize(node.Head.ControllerArray.nCount);
        nPosData = MDL_OFFSET + node.Head.ControllerArray.nOffset;
        for(int n = 0; n < node.Head.ControllerArray.nCount; n++){
            Controller & ctrl = node.Head.Controllers.at(n);
            std::string sController = sNode + "Controller " + std::to_string(n) + " > ";
            ReadNumber(&ctrl.nControllerType, 4, sController + "Type", &nPosData);
            ReadNumber(&ctrl.nUnknown2, 10, sController + "Unknown", &nPosData);
            ReadNumber(&ctrl.nValueCount, 5, sController + "Value Count", &nPosData);
            ReadNumber(&ctrl.nTimekeyStart, 5, sController + "Timekey Start", &nPosData);
            ReadNumber(&ctrl.nDataStart, 5, sController + "Data Start", &nPosData);

            ReadNumber(&ctrl.nColumnCount, 5, sController + "Column Count", &nPosData);
            ReadNumber(&ctrl.nPadding[0], 11, sController + "Padding", &nPosData);
            ReadNumber(&ctrl.nPadding[1], 11, sController + "Padding", &nPosData);
            ReadNumber(&ctrl.nPadding[2], 11, sController + "Padding", &nPosData);
            MarkDataBorder(nPosData - 1);

            ctrl.nNameIndex = node.Head.nNameIndex;
            ctrl.nAnimation = node.nAnimation;

            if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION && node.nAnimation.Valid()){
                if(ctrl.nColumnCount == 2 && !FH->MH.bCompressQuaternions) FH->MH.bCompressQuaternions = true;
                else if(ctrl.nColumnCount != 2 && FH->MH.bCompressQuaternions) FH->MH.bCompressQuaternions = false;
            }

            int nCount = ctrl.nColumnCount & 0x0F;
            if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION && nCount == 2) nCount = 1;
            if(ctrl.nColumnCount & 0x10) nCount *= 3;
            if(nCount * ctrl.nValueCount + ctrl.nDataStart > node.Head.ControllerData.size())
                Warning("Missing controller data on an animation controller on node '" + GetNodeName(node) + "' in animation '" + FH->MH.Animations.at(node.nAnimation).sName.c_str() + "'.");
        }
    }

    unsigned nSavePosition = nPosition;

    if(node.Head.ChildrenArray.nCount > 0){
        /// We gots children!
        node.Head.Children.resize(node.Head.ChildrenArray.nCount);
        node.Head.ChildIndices.clear();
        nPosData = MDL_OFFSET + node.Head.ChildrenArray.nOffset;
        for(int n = 0; n < node.Head.Children.size(); n++){
            Node & child = node.Head.Children.at(n);
            ReadNumber(&child.nOffset, 6, sNode + "Child Offset" + std::to_string(n), &nPosData);
            MarkDataBorder(nPosData - 1);
            child.nAnimation = node.nAnimation;

            /// If this is a geometry node, then the name index is reliable and we may use it for the parent index.
            /// If however this is an animation node, the name index is not reliable, so we will put in the supernode number instead
            /// and then use it later to find the correct name index.
            child.Head.nParentIndex = !node.nAnimation.Valid() ? node.Head.nNameIndex : node.Head.nSupernodeNumber;

            if(std::find(offsets.begin(), offsets.end(), child.nOffset) == offsets.end()){
                ParseNode(child, offsets, vFromRoot, qFromRoot, sPrefix, bMinimal);
            }
            else{
                /**
                    Okay, so it seems that the offset of our child has already been parsed. This means that there is a loop in the model.
                    I will solve this loop now by not parsing this child. So this will break the loop, but it will leave this unread child behind.
                    It is now the job of the linearization functions to ignore this child. I will mark this child by setting its offset to 0.
                    So, when the offset of a node is 0, linearization of that node should not take place.
                /**/
                child.nOffset = 0;
            }

            node.Head.ChildIndices.push_back(child.Head.nNameIndex); /// Sorting based on child indices? Does that even work for animations?
        }
    }

    /// If we only want to read minimally, we're done at this point
    if(bMinimal) return;

    /// Return the position to where we were before doing all the children in between
    nPosition = nSavePosition;

    if(node.Head.nType & NODE_LIGHT){
        std::string sLight = sNode + "Light > ";
        node.Light.fFlareRadius = ReadNumber<float>(nullptr, 2, sLight + "Flare Radius");
        ReadNumber(&node.Light.UnknownArray.nOffset, 8, sLight + "Unknown Array > Offset");
        ReadNumber(&node.Light.UnknownArray.nCount, 8, sLight + "Unknown Array > Count 1");
        ReadNumber(&node.Light.UnknownArray.nCount2, 8, sLight + "Unknown Array > Count 2");
        ReadNumber(&node.Light.FlareSizeArray.nOffset, 6, sLight + "Flare Sizes > Offset");
        ReadNumber(&node.Light.FlareSizeArray.nCount, 1, sLight + "Flare Sizes > Count 1");
        ReadNumber(&node.Light.FlareSizeArray.nCount2, 1, sLight + "Flare Sizes > Count 2");
        ReadNumber(&node.Light.FlarePositionArray.nOffset, 6, sLight + "Flare Positions > Offset");
        ReadNumber(&node.Light.FlarePositionArray.nCount, 1, sLight + "Flare Positions > Count 1");
        ReadNumber(&node.Light.FlarePositionArray.nCount2, 1, sLight + "Flare Positions > Count 2");
        ReadNumber(&node.Light.FlareColorShiftArray.nOffset, 6, sLight + "Flare Color Shifts > Offset");
        ReadNumber(&node.Light.FlareColorShiftArray.nCount, 1, sLight + "Flare Color Shifts > Count 1");
        ReadNumber(&node.Light.FlareColorShiftArray.nCount2, 1, sLight + "Flare Color Shifts > Count 2");
        ReadNumber(&node.Light.FlareTextureNameArray.nOffset, 6, sLight + "Flare Texture Names > Offset");
        ReadNumber(&node.Light.FlareTextureNameArray.nCount, 1, sLight + "Flare Texture Names > Count 1");
        ReadNumber(&node.Light.FlareTextureNameArray.nCount2, 1, sLight + "Flare Texture Names > Count 2");

        ReadNumber(node.Light.nLightPriority.GetPtr(), 4, sLight + "Light Priority");
        ReadNumber(node.Light.nAmbientOnly.GetPtr(), 4, sLight + "Ambient Only");
        ReadNumber(node.Light.nDynamicType.GetPtr(), 4, sLight + "Dynamic Type");
        ReadNumber(node.Light.nAffectDynamic.GetPtr(), 4, sLight + "Affect Dynamic");
        ReadNumber(node.Light.nShadow.GetPtr(), 4, sLight + "Shadow");
        ReadNumber(node.Light.nFlare.GetPtr(), 4, sLight + "Flare");
        ReadNumber(node.Light.nFadingLight.GetPtr(), 4, sLight + "Fading Light");
        MarkDataBorder(nPosition - 1);

        if(node.Light.FlareTextureNameArray.nCount > 0){
            node.Light.FlareTextureNames.resize(node.Light.FlareTextureNameArray.nCount);
            nPosData = MDL_OFFSET + node.Light.FlareTextureNameArray.nOffset;
            for(int n = 0; n < node.Light.FlareTextureNameArray.nCount; n++){
                ReadNumber(&node.Light.FlareTextureNames[n].nOffset, 6, sLight + "Flare Texture Name Offset " + std::to_string(n), &nPosData);
                MarkDataBorder(nPosData - 1);
                unsigned nPosData2 = MDL_OFFSET + node.Light.FlareTextureNames[n].nOffset;
                ReadString(&node.Light.FlareTextureNames[n].sName, 0, 3, sLight + "Flare Texture Name String " + std::to_string(n), &nPosData2);
            }
        }
        if(node.Light.FlareSizeArray.nCount > 0){
            node.Light.FlareSizes.resize(node.Light.FlareSizeArray.nCount);
            nPosData = MDL_OFFSET + node.Light.FlareSizeArray.nOffset;
            for(int n = 0; n < node.Light.FlareSizeArray.nCount; n++){
                node.Light.FlareSizes[n] = ReadNumber<float>(nullptr, 2, sLight + "Flare Size " + std::to_string(n), &nPosData);
                MarkDataBorder(nPosData - 1);
            }
        }
        if(node.Light.FlarePositionArray.nCount > 0){
            node.Light.FlarePositions.resize(node.Light.FlarePositionArray.nCount);
            nPosData = MDL_OFFSET + node.Light.FlarePositionArray.nOffset;
            for(int n = 0; n < node.Light.FlarePositionArray.nCount; n++){
                node.Light.FlarePositions[n] = ReadNumber<float>(nullptr, 2, sLight + "Flare Position " + std::to_string(n), &nPosData);
                MarkDataBorder(nPosData - 1);
            }
        }
        if(node.Light.FlareColorShiftArray.nCount > 0){
            node.Light.FlareColorShifts.resize(node.Light.FlareColorShiftArray.nCount);
            nPosData = MDL_OFFSET + node.Light.FlareColorShiftArray.nOffset;
            for(int n = 0; n < node.Light.FlareColorShiftArray.nCount; n++){
                node.Light.FlareColorShifts[n].fR = ReadNumber<float>(nullptr, 2, sLight + "Flare Color Shift " + std::to_string(n) + " > R", &nPosData);
                node.Light.FlareColorShifts[n].fG = ReadNumber<float>(nullptr, 2, sLight + "Flare Color Shift " + std::to_string(n) + " > G", &nPosData);
                node.Light.FlareColorShifts[n].fB = ReadNumber<float>(nullptr, 2, sLight + "Flare Color Shift " + std::to_string(n) + " > B", &nPosData);
                MarkDataBorder(nPosData - 1);
            }
        }
    }

    if(node.Head.nType & NODE_EMITTER){
        std::string sEmitter = sNode + "Emitter > ";
        node.Emitter.fDeadSpace = ReadNumber<float>(nullptr, 2, sEmitter + "Dead Space");
        node.Emitter.fBlastRadius = ReadNumber<float>(nullptr, 2, sEmitter + "Blast Radius");
        node.Emitter.fBlastLength = ReadNumber<float>(nullptr, 2, sEmitter + "Blast Length");

        ReadNumber(&node.Emitter.nBranchCount, 1, sEmitter + "Branch Count");
        node.Emitter.fControlPointSmoothing = ReadNumber<float>(nullptr, 2, sEmitter + "Control Point Smoothing");

        ReadNumber(&node.Emitter.nxGrid, 4, sEmitter + "xGrid");
        ReadNumber(&node.Emitter.nyGrid, 4, sEmitter + "yGrid");

        ReadNumber(&node.Emitter.nSpawnType, 4, sEmitter + "SpawnType");

        ReadString(&node.Emitter.cUpdate, 32, 3, sEmitter + "Update");
        ReadString(&node.Emitter.cRender, 32, 3, sEmitter + "Render");
        ReadString(&node.Emitter.cBlend, 32, 3, sEmitter + "Blend");
        ReadString(&node.Emitter.cTexture, 32, 3, sEmitter + "Texture");
        ReadString(&node.Emitter.cChunkName, 16, 3, sEmitter + "Chunk Name");

        ReadNumber(&node.Emitter.nTwosidedTex, 4, sEmitter + "Twosided Tex");
        ReadNumber(&node.Emitter.nLoop, 4, sEmitter + "Loop");
        ReadNumber(&node.Emitter.nRenderOrder, 5, sEmitter + "Render Order");
        ReadNumber(&node.Emitter.nFrameBlending, 7, sEmitter + "Frame Blending");
        ReadString(&node.Emitter.cDepthTextureName, 32, 3, sEmitter + "Depth Texture Name");
        ReadNumber(&node.Emitter.nPadding1, 11, sEmitter + "Padding");
        ReadNumber(&node.Emitter.nFlags, 4, sEmitter + "Flags");
        MarkDataBorder(nPosition - 1);
    }

    if(node.Head.nType & NODE_REFERENCE){
        std::string sReference = sNode + "Reference > ";
        ReadString(&node.Reference.sRefModel, 32, 3, sReference + "Reference Model");
        ReadNumber(&node.Reference.nReattachable, 4, sReference + "Reattachable");
        MarkDataBorder(nPosition - 1);
    }

    if(node.Head.nType & NODE_MESH){
        std::string sMesh = sNode + "Mesh > ";
      try{
        ReadNumber(&node.Mesh.nFunctionPointer0, 9, sMesh + "Function Pointer 1");
        ReadNumber(&node.Mesh.nFunctionPointer1, 9, sMesh + "Function Pointer 2");

        ReadNumber(&node.Mesh.FaceArray.nOffset, 6, sMesh + "Face Array > Offset");
        ReadNumber(&node.Mesh.FaceArray.nCount, 1, sMesh + "Face Array > Count 1");
        ReadNumber(&node.Mesh.FaceArray.nCount2, 1, sMesh + "Face Array > Count 2");

        node.Mesh.vBBmin.fX = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Min > X");
        node.Mesh.vBBmin.fY = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Min > Y");
        node.Mesh.vBBmin.fZ = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Min > Z");
        node.Mesh.vBBmax.fX = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Max > X");
        node.Mesh.vBBmax.fY = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Max > Y");
        node.Mesh.vBBmax.fZ = ReadNumber<float>(nullptr, 2, sMesh + "Bounding Box Max > Z");
        node.Mesh.fRadius = ReadNumber<float>(nullptr, 2, sMesh + "Radius");
        node.Mesh.vAverage.fX = ReadNumber<float>(nullptr, 2, sMesh + "Average > X");
        node.Mesh.vAverage.fY = ReadNumber<float>(nullptr, 2, sMesh + "Average > Y");
        node.Mesh.vAverage.fZ = ReadNumber<float>(nullptr, 2, sMesh + "Average > Z");
        node.Mesh.fDiffuse.fR = ReadNumber<float>(nullptr, 2, sMesh + "Diffuse > R");
        node.Mesh.fDiffuse.fG = ReadNumber<float>(nullptr, 2, sMesh + "Diffuse > G");
        node.Mesh.fDiffuse.fB = ReadNumber<float>(nullptr, 2, sMesh + "Diffuse > B");
        node.Mesh.fAmbient.fR = ReadNumber<float>(nullptr, 2, sMesh + "Ambient > R");
        node.Mesh.fAmbient.fG = ReadNumber<float>(nullptr, 2, sMesh + "Ambient > G");
        node.Mesh.fAmbient.fB = ReadNumber<float>(nullptr, 2, sMesh + "Ambient > B");

        ReadNumber(&node.Mesh.nTransparencyHint, 4, sMesh + "Transparency Hint");

        ReadString(&node.Mesh.cTexture1, 32, 3, sMesh + "Bitmap 1");
        ReadString(&node.Mesh.cTexture2, 32, 3, sMesh + "Bitmap 2");
        ReadString(&node.Mesh.cTexture3, 12, 3, sMesh + "Texture 0");
        ReadString(&node.Mesh.cTexture4, 12, 3, sMesh + "Texture 1");

        ReadNumber(&node.Mesh.IndexCounterArray.nOffset, 6, sMesh + "Index Count Array > Offset");
        ReadNumber(&node.Mesh.IndexCounterArray.nCount, 1, sMesh + "Index Count Array > Count 1");
        ReadNumber(&node.Mesh.IndexCounterArray.nCount2, 1, sMesh + "Index Count Array > Count 2");

        ReadNumber(&node.Mesh.IndexLocationArray.nOffset, 6, sMesh + "Index Loc Array > Offset");
        ReadNumber(&node.Mesh.IndexLocationArray.nCount, 1, sMesh + "Index Loc Array > Count 1");
        ReadNumber(&node.Mesh.IndexLocationArray.nCount2, 1, sMesh + "Index Loc Array > Count 2");

        ReadNumber(&node.Mesh.MeshInvertedCounterArray.nOffset, 6, sMesh + "Inv Counter Array > Offset");
        ReadNumber(&node.Mesh.MeshInvertedCounterArray.nCount, 1, sMesh + "Inv Counter Array > Count 1");
        ReadNumber(&node.Mesh.MeshInvertedCounterArray.nCount2, 1, sMesh + "Inv Counter Array > Count 2");

        ReadNumber(&node.Mesh.nUnknown3[0], 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nUnknown3[1], 8, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nUnknown3[2], 8, sMesh + "Unknown");

        ReadNumber(&node.Mesh.nSaberUnknown1, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown2, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown3, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown4, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown5, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown6, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown7, 11, sMesh + "Unknown");
        ReadNumber(&node.Mesh.nSaberUnknown8, 11, sMesh + "Unknown");

        ReadNumber(&node.Mesh.nAnimateUV, 4, sMesh + "Animate UV");
        node.Mesh.fUVDirectionX = ReadNumber<float>(nullptr, 2, sMesh + "UV Direction X");
        node.Mesh.fUVDirectionY = ReadNumber<float>(nullptr, 2, sMesh + "UV Direction Y");
        node.Mesh.fUVJitter = ReadNumber<float>(nullptr, 2, sMesh + "UV Jitter");
        node.Mesh.fUVJitterSpeed = ReadNumber<float>(nullptr, 2, sMesh + "UV Jitter Speed");

        ReadNumber(&node.Mesh.nMdxDataSize, 1, sMesh + "MDX Data Size");
        ReadNumber(&node.Mesh.nMdxDataBitmap, 4, sMesh + "MDX Data Bitmap");

        ReadNumber(node.Mesh.nOffsetToMdxVertex.GetPtr(), 6, sMesh + "Offset To MDX Verts");
        ReadNumber(node.Mesh.nOffsetToMdxNormal.GetPtr(), 6, sMesh + "Offset To MDX Normals");
        ReadNumber(node.Mesh.nOffsetToMdxColor.GetPtr(), 6, sMesh + "Offset To MDX Colors");
        ReadNumber(node.Mesh.nOffsetToMdxUV1.GetPtr(), 6, sMesh + "Offset To MDX UVs 1");
        ReadNumber(node.Mesh.nOffsetToMdxUV2.GetPtr(), 6, sMesh + "Offset To MDX UVs 2");
        ReadNumber(node.Mesh.nOffsetToMdxUV3.GetPtr(), 6, sMesh + "Offset To MDX UVs 3");
        ReadNumber(node.Mesh.nOffsetToMdxUV4.GetPtr(), 6, sMesh + "Offset To MDX UVs 4");
        ReadNumber(node.Mesh.nOffsetToMdxTangent1.GetPtr(), 6, sMesh + "Offset To MDX Tangent Space 1");
        ReadNumber(node.Mesh.nOffsetToMdxTangent2.GetPtr(), 6, sMesh + "Offset To MDX Tangent Space 2");
        ReadNumber(node.Mesh.nOffsetToMdxTangent3.GetPtr(), 6, sMesh + "Offset To MDX Tangent Space 3");
        ReadNumber(node.Mesh.nOffsetToMdxTangent4.GetPtr(), 6, sMesh + "Offset To MDX Tangent Space 4");

        ReadNumber(&node.Mesh.nNumberOfVerts, 1, sMesh + "Vert Number");
        ReadNumber(&node.Mesh.nTextureNumber, 1, sMesh + "Texure Number");

        ReadNumber(&node.Mesh.nHasLightmap, 7, sMesh + "Has Lightmap");
        ReadNumber(&node.Mesh.nRotateTexture, 7, sMesh + "Rotate Texture");
        ReadNumber(&node.Mesh.nBackgroundGeometry, 7, sMesh + "Background Geometry");
        ReadNumber(&node.Mesh.nShadow, 7, sMesh + "Shadow");
        ReadNumber(&node.Mesh.nBeaming, 7, sMesh + "Beaming");
        ReadNumber(&node.Mesh.nRender, 7, sMesh + "Render");

        if(bK2) ReadNumber(&node.Mesh.nDirtEnabled, 7, sMesh + "Dirt Enabled");
        if(bK2) ReadNumber(&node.Mesh.nPadding1, 11, sMesh + "Padding");
        if(bK2) ReadNumber(&node.Mesh.nDirtTexture, 5, sMesh + "Dirt Texture");
        if(bK2) ReadNumber(&node.Mesh.nDirtCoordSpace, 5, sMesh + "Dirt Coord Space");
        if(bK2) ReadNumber(&node.Mesh.nHideInHolograms, 7, sMesh + "Hide In Holograms");
        if(bK2) ReadNumber(&node.Mesh.nPadding2, 11, sMesh + "Padding");

        ReadNumber(&node.Mesh.nPadding3, 11, sMesh + "Padding");

        node.Mesh.fTotalArea = ReadNumber<float>(nullptr, 2, sMesh + "Total Area");
        ReadNumber(&node.Mesh.nPadding, 8, sMesh + "Padding");
        ReadNumber(&node.Mesh.nOffsetIntoMdx, 6, sMesh + "Offset Into MDX");
        if(!bXbox) ReadNumber(&node.Mesh.nOffsetToVertArray, 6, sMesh + "Offset To Vert Array");
        MarkDataBorder(nPosition - 1);
      }
      catch(const std::exception & e){
        throw mdlexception("In " + GetNodeName(node) + ", reading trimesh header: " + e.what());
      }

        if(node.Mesh.IndexCounterArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + node.Mesh.IndexCounterArray.nOffset;
            ReadNumber(&node.Mesh.nVertIndicesCount, 1, sMesh + "Vert Indices Count", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
        else node.Mesh.nVertIndicesCount = 0;

        if(node.Mesh.IndexLocationArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + node.Mesh.IndexLocationArray.nOffset;
            ReadNumber(&node.Mesh.nVertIndicesLocation, 6, sMesh + "Vert Indices Loc", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
        else node.Mesh.nVertIndicesLocation = 0;

        if(node.Mesh.MeshInvertedCounterArray.nCount > 0){
            //I am assuming here that the pointer can only ever be a single one
            nPosData = MDL_OFFSET + node.Mesh.MeshInvertedCounterArray.nOffset;
            ReadNumber(node.Mesh.nMeshInvertedCounter.GetPtr(), 4, sMesh + "Inverted Counter", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
        else node.Mesh.nMeshInvertedCounter = 0;

        if(node.Mesh.FaceArray.nCount > 0){
            node.Mesh.Faces.resize(node.Mesh.FaceArray.nCount);
            if(node.Mesh.IndexLocationArray.nCount > 0) node.Mesh.VertIndices.resize(node.Mesh.FaceArray.nCount);
            nPosData = MDL_OFFSET + node.Mesh.FaceArray.nOffset;
            unsigned int nPosData2;
            if(node.Mesh.IndexLocationArray.nCount > 0) nPosData2 = MDL_OFFSET + node.Mesh.nVertIndicesLocation;
            for(int n = 0; n < node.Mesh.FaceArray.nCount; n++){
                node.Mesh.Faces.at(n).nNameIndex = node.Head.nNameIndex;

                node.Mesh.Faces[n].vNormal.fX = ReadNumber<float>(nullptr, 2, sMesh + "Face " + std::to_string(n) + " > Normal > X", &nPosData);
                node.Mesh.Faces[n].vNormal.fY = ReadNumber<float>(nullptr, 2, sMesh + "Face " + std::to_string(n) + " > Normal > Y", &nPosData);
                node.Mesh.Faces[n].vNormal.fZ = ReadNumber<float>(nullptr, 2, sMesh + "Face " + std::to_string(n) + " > Normal > Z", &nPosData);
                node.Mesh.Faces[n].fDistance = ReadNumber<float>(nullptr, 2, sMesh + "Face " + std::to_string(n) + " > Plane Distance", &nPosData);

                ReadNumber(&node.Mesh.Faces[n].nMaterialID, 4, sMesh + "Face " + std::to_string(n) + " > Material ID", &nPosData);
                //if(!bBinaryPostProcess) node.Mesh.Faces.at(n).nSmoothingGroup = node.Mesh.Faces[n].nMaterialID;

                ReadNumber(node.Mesh.Faces[n].nAdjacentFaces[0].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Adjacent Face 0", &nPosData);
                ReadNumber(node.Mesh.Faces[n].nAdjacentFaces[1].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Adjacent Face 1", &nPosData);
                ReadNumber(node.Mesh.Faces[n].nAdjacentFaces[2].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Adjacent Face 2", &nPosData);
                ReadNumber(node.Mesh.Faces[n].nIndexVertex[0].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 0", &nPosData);
                ReadNumber(node.Mesh.Faces[n].nIndexVertex[1].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 1", &nPosData);
                ReadNumber(node.Mesh.Faces[n].nIndexVertex[2].GetPtr(), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 2", &nPosData);
                MarkDataBorder(nPosData - 1);

                if(node.Mesh.IndexLocationArray.nCount > 0){
                    //std::cout << string_format("Reading VertIndices for face %i of %i, at position %i.\n", n, node.Mesh.FaceArray.nCount, nPosData2);
                    ReadNumber(&node.Mesh.VertIndices[n].at(0), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 0", &nPosData2);
                    ReadNumber(&node.Mesh.VertIndices[n].at(1), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 1", &nPosData2);
                    ReadNumber(&node.Mesh.VertIndices[n].at(2), 5, sMesh + "Face " + std::to_string(n) + " > Vert Index 2", &nPosData2);
                    MarkDataBorder(nPosData2 - 1);
                }
            }
            if(bReadSmoothing){
                for(int n = 0; n < node.Mesh.FaceArray.nCount; n++){
                    ReadNumber(&node.Mesh.Faces.at(n).nSmoothingGroup, 4, sMesh + "Face " + std::to_string(n) + "Smoothing Groups", &nPosData);
                    MarkDataBorder(nPosData - 1);
                }
            }
        }
    }

    if(node.Head.nType & NODE_DANGLY){
        std::string sDangly = sNode + "Dangly > ";
        ReadNumber(&node.Dangly.ConstraintArray.nOffset, 6, sDangly + "Constraint Array > Offset");
        ReadNumber(&node.Dangly.ConstraintArray.nCount, 1, sDangly + "Constraint Array > Count 1");
        ReadNumber(&node.Dangly.ConstraintArray.nCount2, 1, sDangly + "Constraint Array > Count 2");

        node.Dangly.fDisplacement = ReadNumber<float>(nullptr, 2, sDangly + "Displacement");
        node.Dangly.fTightness = ReadNumber<float>(nullptr, 2, sDangly + "Tightness");
        node.Dangly.fPeriod = ReadNumber<float>(nullptr, 2, sDangly + "Period");

        ReadNumber(&node.Dangly.nOffsetToData2, 6, sDangly + "Offset To Vert Data");
        MarkDataBorder(nPosition - 1);

        if(node.Dangly.ConstraintArray.nCount > 0){
            node.Dangly.Constraints.resize(node.Dangly.ConstraintArray.nCount);
            node.Dangly.Data2.resize(node.Dangly.ConstraintArray.nCount);
            nPosData = MDL_OFFSET + node.Dangly.ConstraintArray.nOffset;
            unsigned int nPosData2 = MDL_OFFSET + node.Dangly.nOffsetToData2;
            for(int n = 0; n < node.Dangly.ConstraintArray.nCount; n++){
                node.Dangly.Constraints.at(n) = ReadNumber<float>(nullptr, 2, sDangly + "Constraint " + std::to_string(n), &nPosData);
                MarkDataBorder(nPosData - 1);

                node.Dangly.Data2.at(n).fX = ReadNumber<float>(nullptr, 2, sDangly + "Vertex " + std::to_string(n) + " > X", &nPosData2);
                node.Dangly.Data2.at(n).fY = ReadNumber<float>(nullptr, 2, sDangly + "Vertex " + std::to_string(n) + " > Y", &nPosData2);
                node.Dangly.Data2.at(n).fZ = ReadNumber<float>(nullptr, 2, sDangly + "Vertex " + std::to_string(n) + " > Z", &nPosData2);
                MarkDataBorder(nPosData2 - 1);
            }
        }
    }

    if(node.Head.nType & NODE_SKIN){
        std::string sSkin = sNode + "Skin > ";
        ReadNumber(&node.Skin.UnknownArray.nOffset, 8, sSkin + "Unknown Array > Offset");
        ReadNumber(&node.Skin.UnknownArray.nCount, 8, sSkin + "Unknown Array > Count 1");
        ReadNumber(&node.Skin.UnknownArray.nCount2, 8, sSkin + "Unknown Array > Count 2");
        ReadNumber(&node.Skin.nOffsetToMdxWeightValues, 6, sSkin + "Offset To MDX Weights");
        ReadNumber(&node.Skin.nOffsetToMdxBoneIndices, 6, sSkin + "Offset To MDX Indices");

        ReadNumber(&node.Skin.nOffsetToBonemap, 6, sSkin + "Bonemap Offset");
        ReadNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "Bonemap Count");

        ReadNumber(&node.Skin.QBoneArray.nOffset, 6, sSkin + "QBone Array > Offset");
        ReadNumber(&node.Skin.QBoneArray.nCount, 1, sSkin + "QBone Array > Count 1");
        ReadNumber(&node.Skin.QBoneArray.nCount2, 1, sSkin + "QBone Array > Count 2");

        ReadNumber(&node.Skin.TBoneArray.nOffset, 6, sSkin + "TBone Array > Offset");
        ReadNumber(&node.Skin.TBoneArray.nCount, 1, sSkin + "TBone Array > Count 1");
        ReadNumber(&node.Skin.TBoneArray.nCount2, 1, sSkin + "TBone Array > Count 2");

        ReadNumber(&node.Skin.Array8Array.nOffset, 6, sSkin + "Garbage Array > Offset");
        ReadNumber(&node.Skin.Array8Array.nCount, 1, sSkin + "Garbage Array > Count 1");
        ReadNumber(&node.Skin.Array8Array.nCount2, 1, sSkin + "Garbage Array > Count 2");

        for(int n = 0; n < 16; n++){
            ReadNumber(&node.Skin.nBoneIndices[n], 5, sSkin + "Bone Index " + std::to_string(n));
        }
        ReadNumber(&node.Skin.nPadding1, 11, sSkin + "Padding");
        ReadNumber(&node.Skin.nPadding2, 11, sSkin + "Padding");
        MarkDataBorder(nPosition - 1);

        if(node.Skin.nNumberOfBonemap != node.Skin.QBoneArray.nCount ||
           node.Skin.nNumberOfBonemap != node.Skin.TBoneArray.nCount ||
           node.Skin.nNumberOfBonemap != node.Skin.Array8Array.nCount){
            Error("Unexpected Error! The bone numbers do not match up for " + GetNodeName(node) + "! Will try to load the data anyway. ");
        }
        if(node.Skin.nNumberOfBonemap > 0){
            node.Skin.Bones.resize(node.Skin.nNumberOfBonemap);
            node.Skin.BoneNameIndices.resize(node.Mesh.nNumberOfVerts);
            nPosData = MDL_OFFSET + node.Skin.nOffsetToBonemap;
            unsigned int nPosData2 = MDL_OFFSET + node.Skin.QBoneArray.nOffset;
            unsigned int nPosData3 = MDL_OFFSET + node.Skin.TBoneArray.nOffset;
            unsigned int nPosData4 = MDL_OFFSET + node.Skin.Array8Array.nOffset;
            for(int n = 0; n < node.Skin.nNumberOfBonemap; n++){
                if(bXbox) ReadNumber(&node.Skin.Bones[n].nBonemap, 5, sSkin + "Bonemap " + std::to_string(n), &nPosData);
                else node.Skin.Bones[n].nBonemap = (unsigned short) ReadNumber<float>(nullptr, 2, sSkin + "Bonemap " + std::to_string(n), &nPosData);
                MarkDataBorder(nPosData - 1);

                double fQW = ReadNumber<float>(nullptr, 2, sSkin + "QBone " + std::to_string(n) + " > W", &nPosData2);
                double fQX = ReadNumber<float>(nullptr, 2, sSkin + "QBone " + std::to_string(n) + " > X", &nPosData2);
                double fQY = ReadNumber<float>(nullptr, 2, sSkin + "QBone " + std::to_string(n) + " > Y", &nPosData2);
                double fQZ = ReadNumber<float>(nullptr, 2, sSkin + "QBone " + std::to_string(n) + " > Z", &nPosData2);
                MarkDataBorder(nPosData2 - 1);
                node.Skin.Bones[n].QBone.SetQuaternion(fQX, fQY, fQZ, fQW);

                node.Skin.Bones[n].TBone.fX = ReadNumber<float>(nullptr, 2, sSkin + "TBone " + std::to_string(n) + " > X", &nPosData3);
                node.Skin.Bones[n].TBone.fY = ReadNumber<float>(nullptr, 2, sSkin + "TBone " + std::to_string(n) + " > Y", &nPosData3);
                node.Skin.Bones[n].TBone.fZ = ReadNumber<float>(nullptr, 2, sSkin + "TBone " + std::to_string(n) + " > Z", &nPosData3);
                MarkDataBorder(nPosData3 - 1);

                ReadNumber(&node.Skin.Bones[n].nPadding, 11, sSkin + "Garbage " + std::to_string(n), &nPosData4);
                if(n + 1 >= node.Skin.nNumberOfBonemap) MarkDataBorder(nPosData4 - 1);
            }
        }
    }

    ///Need to do this later so that the Skin data is already read
    if(node.Head.nType & NODE_MESH){
        std::string sMdx = GetNodeName(node) + " > ";
        std::string sMesh = sNode + "Mesh > ";
        if(node.Mesh.nNumberOfVerts > 0){
            //ReportMdl << "Node " << node.Head.nNameIndex << ": \n";
            node.Mesh.Vertices.resize(node.Mesh.nNumberOfVerts);
            nPosData = MDL_OFFSET + node.Mesh.nOffsetToVertArray;
            unsigned int nPosData2 = node.Mesh.nOffsetIntoMdx;
            if(nPosData2 > 0 && Mdx) Mdx->MarkDataBorder(nPosData2 - 1);
            unsigned nMaxDataStructs = 0;
            for(int n = 0; n < node.Mesh.nNumberOfVerts; n++){
                nMaxDataStructs++;
                if(!bXbox){
                    node.Mesh.Vertices[n].nOffset = nPosData;
                    node.Mesh.Vertices[n].fX = ReadNumber<float>(nullptr, 2, sMesh + "Vertex " + std::to_string(n) + " > X", &nPosData);
                    node.Mesh.Vertices[n].fY = ReadNumber<float>(nullptr, 2, sMesh + "Vertex " + std::to_string(n) + " > Y", &nPosData);
                    node.Mesh.Vertices[n].fZ = ReadNumber<float>(nullptr, 2, sMesh + "Vertex " + std::to_string(n) + " > Z", &nPosData);
                    MarkDataBorder(nPosData - 1);

                    node.Mesh.Vertices[n].vFromRoot = node.Mesh.Vertices[n];
                    node.Mesh.Vertices[n].vFromRoot.Rotate(node.Head.qFromRoot);
                    node.Mesh.Vertices[n].vFromRoot += node.Head.vFromRoot;
                }

                node.Mesh.Vertices[n].MDXData.nNameIndex = node.Head.nNameIndex;
                if(node.Mesh.nMdxDataSize > 0 && Mdx){
                try{
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxVertex;
                        node.Mesh.Vertices[n].MDXData.vVertex.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Vertex " + std::to_string(n) + " > X", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vVertex.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Vertex " + std::to_string(n) + " > Y", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vVertex.fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Vertex " + std::to_string(n) + " > Z", &nPosData2);

                        if(bXbox){
                            node.Mesh.Vertices[n].fX = node.Mesh.Vertices[n].MDXData.vVertex.fX;
                            node.Mesh.Vertices[n].fY = node.Mesh.Vertices[n].MDXData.vVertex.fY;
                            node.Mesh.Vertices[n].fZ = node.Mesh.Vertices[n].MDXData.vVertex.fZ;

                            node.Mesh.Vertices[n].vFromRoot = node.Mesh.Vertices[n];
                            node.Mesh.Vertices[n].vFromRoot.Rotate(node.Head.qFromRoot);
                            node.Mesh.Vertices[n].vFromRoot += node.Head.vFromRoot;
                        }
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxNormal;
                        if(!bXbox){
                            node.Mesh.Vertices[n].MDXData.vNormal.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Normal " + std::to_string(n) + " > X", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.vNormal.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Normal " + std::to_string(n) + " > Y", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.vNormal.fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Normal " + std::to_string(n) + " > Z", &nPosData2);
                        }
                        else{
                            node.Mesh.Vertices[n].MDXData.vNormal = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 2, sMdx + "Normal " + std::to_string(n), &nPosData2));
                        }
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_COLOR){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxColor;
                        node.Mesh.Vertices[n].MDXData.cColor.fR = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Color " + std::to_string(n) + " > R", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.cColor.fG = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Color " + std::to_string(n) + " > G", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.cColor.fB = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Color " + std::to_string(n) + " > B", &nPosData2);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV1;
                        node.Mesh.Vertices[n].MDXData.vUV1.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 1 " + std::to_string(n) + " > U", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vUV1.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 1 " + std::to_string(n) + " > V", &nPosData2);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV2;
                        node.Mesh.Vertices[n].MDXData.vUV2.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 2 " + std::to_string(n) + " > U", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vUV2.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 2 " + std::to_string(n) + " > V", &nPosData2);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV3;
                        node.Mesh.Vertices[n].MDXData.vUV3.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 3 " + std::to_string(n) + " > U", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vUV3.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 3 " + std::to_string(n) + " > V", &nPosData2);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV4;
                        node.Mesh.Vertices[n].MDXData.vUV4.fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 4 " + std::to_string(n) + " > U", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.vUV4.fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "UVs 4 " + std::to_string(n) + " > V", &nPosData2);
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent1;
                        for(int b = 0; b < 3; b++){
                            if(bXbox){
                                node.Mesh.Vertices[n].MDXData.vTangent1[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 2, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n), &nPosData2));
                            }
                            else{
                                node.Mesh.Vertices[n].MDXData.vTangent1[b].fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > X", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent1[b].fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Y", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent1[b].fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Z", &nPosData2);
                            }
                        }
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent2;
                        for(int b = 0; b < 3; b++){
                            if(bXbox){
                                node.Mesh.Vertices[n].MDXData.vTangent2[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 2, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n), &nPosData2));
                            }
                            else{
                                node.Mesh.Vertices[n].MDXData.vTangent2[b].fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > X", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent2[b].fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Y", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent2[b].fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Z", &nPosData2);
                            }
                        }
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent3;
                        for(int b = 0; b < 3; b++){
                            if(bXbox){
                                node.Mesh.Vertices[n].MDXData.vTangent3[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 2, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n), &nPosData2));
                            }
                            else{
                                node.Mesh.Vertices[n].MDXData.vTangent3[b].fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > X", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent3[b].fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Y", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent3[b].fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Z", &nPosData2);
                            }
                        }
                    }
                    if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent4;
                        for(int b = 0; b < 3; b++){
                            if(bXbox){
                                node.Mesh.Vertices[n].MDXData.vTangent4[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 2, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n), &nPosData2));
                            }
                            else{
                                node.Mesh.Vertices[n].MDXData.vTangent4[b].fX = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > X", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent4[b].fY = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Y", &nPosData2);
                                node.Mesh.Vertices[n].MDXData.vTangent4[b].fZ = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " " + std::to_string(n) + " > Z", &nPosData2);
                            }
                        }
                    }
                    if(node.Head.nType & NODE_SKIN){
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Skin.nOffsetToMdxWeightValues;
                        node.Mesh.Vertices[n].MDXData.Weights.fWeightValue[0] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Value 0", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.Weights.fWeightValue[1] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Value 1", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.Weights.fWeightValue[2] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Value 2", &nPosData2);
                        node.Mesh.Vertices[n].MDXData.Weights.fWeightValue[3] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Value 3", &nPosData2);
                        nPosData2 = node.Mesh.nOffsetIntoMdx + n * node.Mesh.nMdxDataSize + node.Skin.nOffsetToMdxBoneIndices;
                        if(bXbox){
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[0] = Mdx->ReadNumber((unsigned short*) nullptr, 5, sMdx + "Weight " + std::to_string(n) + " > Index 0", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[1] = Mdx->ReadNumber((unsigned short*) nullptr, 5, sMdx + "Weight " + std::to_string(n) + " > Index 1", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[2] = Mdx->ReadNumber((unsigned short*) nullptr, 5, sMdx + "Weight " + std::to_string(n) + " > Index 2", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[3] = Mdx->ReadNumber((unsigned short*) nullptr, 5, sMdx + "Weight " + std::to_string(n) + " > Index 3", &nPosData2);
                        }
                        else{
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[0] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Index 0", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[1] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Index 1", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[2] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Index 2", &nPosData2);
                            node.Mesh.Vertices[n].MDXData.Weights.nWeightIndex[3] = Mdx->ReadNumber<float>(nullptr, 2, sMdx + "Weight " + std::to_string(n) + " > Index 3", &nPosData2);
                        }
                    }
                }
                catch(const std::exception & e){
                    std::cout << "Offset into MDX for " << GetNodeName(node) << " is " << node.Mesh.nOffsetIntoMdx << "\n";
                    throw mdlexception("Error reading MDX Data for '" + GetNodeName(node)  + "': " + e.what());
                }

                Mdx->MarkDataBorder(node.Mesh.nOffsetIntoMdx + (n + 1) * node.Mesh.nMdxDataSize - 1);

                }
            }

            /**
                Read the floats after each data block
                This is just an empty Mdx struct with the length of what came before it.
                But there is some logic to it. The first three numbers are floats, with values depending on node type:
                    // 1,000,000 for skins
                    //10,000,000 for meshes, danglymeshes
            **/
            //std::cout << "Read data, now let's read the dummy data" << std::endl;

            sMdx = GetNodeName(node) + " > Extra Data > ";
            if(node.Mesh.nMdxDataSize > 0 && Mdx){
                node.Mesh.MDXData.nNameIndex = node.Head.nNameIndex;
              try{
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_VERTEX){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxVertex;
                    node.Mesh.MDXData.vVertex.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Vertex > X", &nPosData2);
                    node.Mesh.MDXData.vVertex.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Vertex > Y", &nPosData2);
                    node.Mesh.MDXData.vVertex.fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Vertex > Z", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_NORMAL){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxNormal;
                    if(!bXbox){
                        node.Mesh.MDXData.vNormal.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Normal > X", &nPosData2);
                        node.Mesh.MDXData.vNormal.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Normal > Y", &nPosData2);
                        node.Mesh.MDXData.vNormal.fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Normal > Z", &nPosData2);
                    }
                    else{
                        node.Mesh.MDXData.vNormal = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 8, sMdx + "Normal", &nPosData2));
                    }
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_COLOR){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxColor;
                    node.Mesh.MDXData.cColor.fR = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Color > R", &nPosData2);
                    node.Mesh.MDXData.cColor.fG = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Color > G", &nPosData2);
                    node.Mesh.MDXData.cColor.fB = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Color > B", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV1;
                    node.Mesh.MDXData.vUV1.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 1 > U", &nPosData2);
                    node.Mesh.MDXData.vUV1.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 1 > V", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV2;
                    node.Mesh.MDXData.vUV2.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 2 > U", &nPosData2);
                    node.Mesh.MDXData.vUV2.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 2 > V", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV3;
                    node.Mesh.MDXData.vUV3.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 3 > U", &nPosData2);
                    node.Mesh.MDXData.vUV3.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 3 > V", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxUV4;
                    node.Mesh.MDXData.vUV4.fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 4 > U", &nPosData2);
                    node.Mesh.MDXData.vUV4.fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "UVs 4 > V", &nPosData2);
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent1;
                    for(int b = 0; b < 3; b++){
                        if(bXbox){
                            node.Mesh.MDXData.vTangent1[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 8, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")), &nPosData2));
                        }
                        else{
                            node.Mesh.MDXData.vTangent1[b].fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > X", &nPosData2);
                            node.Mesh.MDXData.vTangent1[b].fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Y", &nPosData2);
                            node.Mesh.MDXData.vTangent1[b].fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 1 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Z", &nPosData2);
                        }
                    }
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT2){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent2;
                    for(int b = 0; b < 3; b++){
                        if(bXbox){
                            node.Mesh.MDXData.vTangent2[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 8, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")), &nPosData2));
                        }
                        else{
                            node.Mesh.MDXData.vTangent2[b].fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > X", &nPosData2);
                            node.Mesh.MDXData.vTangent2[b].fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Y", &nPosData2);
                            node.Mesh.MDXData.vTangent2[b].fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 2 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Z", &nPosData2);
                        }
                    }
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT3){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent3;
                    for(int b = 0; b < 3; b++){
                        if(bXbox){
                            node.Mesh.MDXData.vTangent3[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 8, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")), &nPosData2));
                        }
                        else{
                            node.Mesh.MDXData.vTangent3[b].fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > X", &nPosData2);
                            node.Mesh.MDXData.vTangent3[b].fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Y", &nPosData2);
                            node.Mesh.MDXData.vTangent3[b].fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 3 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Z", &nPosData2);
                        }
                    }
                }
                if(node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT4){
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Mesh.nOffsetToMdxTangent4;
                    for(int b = 0; b < 3; b++){
                        if(bXbox){
                            node.Mesh.MDXData.vTangent4[b] = DecompressVector(Mdx->ReadNumber<unsigned>(nullptr, 8, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")), &nPosData2));
                        }
                        else{
                            node.Mesh.MDXData.vTangent4[b].fX = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > X", &nPosData2);
                            node.Mesh.MDXData.vTangent4[b].fY = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Y", &nPosData2);
                            node.Mesh.MDXData.vTangent4[b].fZ = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Tangent Space 4 > " + (b == 0 ? "Bitangent" : (b == 1 ? "Tangent" : "Normal")) + " > Z", &nPosData2);
                        }
                    }
                }
                if(node.Head.nType & NODE_SKIN){
                    //if(node.Skin.nOffsetToMdxWeightValues != 32) std::cout << string_format("Warning! MDX Skin Data Pointer 1 in %s is not 32! I might be reading wrong!\n", FH->MH.Names[node.Head.nNameIndex].sName.c_str());
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Skin.nOffsetToMdxWeightValues;
                    node.Mesh.MDXData.Weights.fWeightValue[0] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Value 0", &nPosData2);
                    node.Mesh.MDXData.Weights.fWeightValue[1] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Value 1", &nPosData2);
                    node.Mesh.MDXData.Weights.fWeightValue[2] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Value 2", &nPosData2);
                    node.Mesh.MDXData.Weights.fWeightValue[3] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Value 3", &nPosData2);
                    //if(node.Skin.nOffsetToMdxBoneIndices != 48) std::cout << string_format("Warning! MDX Skin Data Pointer 2 in %s is not 48! I might be reading wrong!\n", FH->MH.Names[node.Head.nNameIndex].sName.c_str());
                    nPosData2 = node.Mesh.nOffsetIntoMdx + nMaxDataStructs * node.Mesh.nMdxDataSize + node.Skin.nOffsetToMdxBoneIndices;
                    if(bXbox){
                        node.Mesh.MDXData.Weights.nWeightIndex[0] = Mdx->ReadNumber((unsigned short*) nullptr, 8, sMdx + "Weight > Index 0", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[1] = Mdx->ReadNumber((unsigned short*) nullptr, 8, sMdx + "Weight > Index 1", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[2] = Mdx->ReadNumber((unsigned short*) nullptr, 8, sMdx + "Weight > Index 2", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[3] = Mdx->ReadNumber((unsigned short*) nullptr, 8, sMdx + "Weight > Index 3", &nPosData2);
                    }
                    else{
                        node.Mesh.MDXData.Weights.nWeightIndex[0] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Index 0", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[1] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Index 1", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[2] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Index 2", &nPosData2);
                        node.Mesh.MDXData.Weights.nWeightIndex[3] = Mdx->ReadNumber<float>(nullptr, 8, sMdx + "Weight > Index 3", &nPosData2);
                    }
                }
                Mdx->MarkDataBorder(node.Mesh.nOffsetIntoMdx + (nMaxDataStructs + 1) * node.Mesh.nMdxDataSize - 1);
              }
              catch(mdlexception & e){
                /// If an exception is thrown, it is likely because we hit the end of the file.
                /// In such a case we may just continue
              }
            }
            //std::cout << "Read dummy data" << std::endl;
        }
    }

    if(node.Head.nType & NODE_AABB){
        try{
            std::string sAabb = sNode + "AABB > ";
            ReadNumber(&node.Walkmesh.nOffsetToAabb, 6, sAabb + "Offset To Root AABB");
            MarkDataBorder(nPosition - 1);

            nAabbCount = 0;
            if(node.Walkmesh.nOffsetToAabb > 0){
                /// Set the offset on the root aabb, then start the recursive aabb reading.
                node.Walkmesh.RootAabb.nOffset = node.Walkmesh.nOffsetToAabb;
                ParseAabb(node.Walkmesh.RootAabb, 10, sNode);
            }
        }
        catch(const std::exception & e){
            throw mdlexception("In " + GetNodeName(node) + ", reading aabb: " + e.what());
        }
    }

    if(node.Head.nType & NODE_SABER){
        std::string sSaber = "Lightsaber > ";
        ReadNumber(&node.Saber.nOffsetToSaberVerts, 6, sSaber + "Offset To Verts");
        ReadNumber(&node.Saber.nOffsetToSaberUVs, 6, sSaber + "Offset To UVs");
        ReadNumber(&node.Saber.nOffsetToSaberNormals, 6, sSaber + "Offset To Normals");
        ReadNumber(&node.Saber.nInvCount1, 4, sSaber + "Inverted Counter 1");
        ReadNumber(&node.Saber.nInvCount2, 4, sSaber + "Inverted Counter 2");
        MarkDataBorder(nPosition - 1);

        node.Saber.SaberData.resize(176);
        nPosData = MDL_OFFSET + node.Saber.nOffsetToSaberVerts;
        unsigned int nPosData2 = MDL_OFFSET + node.Saber.nOffsetToSaberUVs;
        unsigned int nPosData3 = MDL_OFFSET + node.Saber.nOffsetToSaberNormals;
        for(int n = 0; n < 176; n++){
            node.Saber.SaberData[n].vVertex.fX = ReadNumber<float>(nullptr, 2, sSaber + "Vertex " + std::to_string(n) + " > X", &nPosData);
            node.Saber.SaberData[n].vVertex.fY = ReadNumber<float>(nullptr, 2, sSaber + "Vertex " + std::to_string(n) + " > Y", &nPosData);
            node.Saber.SaberData[n].vVertex.fZ = ReadNumber<float>(nullptr, 2, sSaber + "Vertex " + std::to_string(n) + " > Z", &nPosData);
            MarkDataBorder(nPosData - 1);

            node.Saber.SaberData[n].vNormal.fX = ReadNumber<float>(nullptr, 2, sSaber + "Normal " + std::to_string(n) + " > X", &nPosData3);
            node.Saber.SaberData[n].vNormal.fY = ReadNumber<float>(nullptr, 2, sSaber + "Normal " + std::to_string(n) + " > Y", &nPosData3);
            node.Saber.SaberData[n].vNormal.fZ = ReadNumber<float>(nullptr, 2, sSaber + "Normal " + std::to_string(n) + " > Z", &nPosData3);
            MarkDataBorder(nPosData3 - 1);

            node.Saber.SaberData[n].vUV1.fX = ReadNumber<float>(nullptr, 2, sSaber + "UV " + std::to_string(n) + " > U", &nPosData2);
            node.Saber.SaberData[n].vUV1.fY = ReadNumber<float>(nullptr, 2, sSaber + "UV " + std::to_string(n) + " > V", &nPosData2);
            MarkDataBorder(nPosData2 - 1);
        }
    }
}

//This is the BWM counterpart to MDL's DecompileModel().
void BWM::DecompileBWM(ReportObject & ReportMdl){
    if(sBuffer.empty()) return;

    /// Start timer
    Timer tDecompile;

    Bwm.reset(new BWMHeader);

    BWMHeader & Data = *Bwm;

    nPosition = 0;

    try{
        std::string sHeader ("Header > ");

        std::string sFileType, sVersion;

        /// Mark version info
        ReadString(&sFileType, 4, 3, sHeader + "File Type");
        ReadString(&sVersion, 4, 3, sHeader + "Version");

        ReadNumber(&Data.nType, 4, sHeader + "Type");
        Data.vUse1.fX = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 1 > X");
        Data.vUse1.fY = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 1 > Y");
        Data.vUse1.fZ = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 1 > Z");
        Data.vUse2.fX = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 2 > X");
        Data.vUse2.fY = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 2 > Y");
        Data.vUse2.fZ = ReadNumber<float>(nullptr, 2, sHeader + "Relative Use Vector 2 > Z");
        Data.vDwk1.fX = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 1 > X");
        Data.vDwk1.fY = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 1 > Y");
        Data.vDwk1.fZ = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 1 > Z");
        Data.vDwk2.fX = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 2 > X");
        Data.vDwk2.fY = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 2 > Y");
        Data.vDwk2.fZ = ReadNumber<float>(nullptr, 2, sHeader + "Absolute Use Vector 2 > Z");
        Data.vPosition.fX = ReadNumber<float>(nullptr, 2, sHeader + "Position > X");
        Data.vPosition.fY = ReadNumber<float>(nullptr, 2, sHeader + "Position > Y");
        Data.vPosition.fZ = ReadNumber<float>(nullptr, 2, sHeader + "Position > Z");

        ReadNumber(&Data.nNumberOfVerts, 1, sHeader + "Vert Count"); /// This is not equal in the wok and mdl
        ReadNumber(&Data.nOffsetToVerts, 6, sHeader + "Vert Offset");
        ReadNumber(&Data.nNumberOfFaces, 1, sHeader + "Face Count"); /// In my test model this equals the number in the mdl
        ReadNumber(&Data.nOffsetToIndices, 6, sHeader + "Vert Indices Offset");
        ReadNumber(&Data.nOffsetToMaterials, 6, sHeader + "Material IDs Offset");
        ReadNumber(&Data.nOffsetToNormals, 6, sHeader + "Normals Offset");
        ReadNumber(&Data.nOffsetToDistances, 6, sHeader + "Distances Offset");
        ReadNumber(&Data.nNumberOfAabb, 1, sHeader + "AABB Count"); /// In my test model this equals the number of aabb in the mdl
        ReadNumber(&Data.nOffsetToAabb, 6, sHeader + "AABB Offset");
        ReadNumber(&Data.nPadding, 11, sHeader + "Unknown");
        ReadNumber(&Data.nNumberOfAdjacentFaces, 1, sHeader + "Adjacent Edges Count");
        ReadNumber(&Data.nOffsetToAdjacentFaces, 6, sHeader + "Adjacent Edges Offset");
        ReadNumber(&Data.nNumberOfEdges, 1, sHeader + "Outer Edges Count");
        ReadNumber(&Data.nOffsetToEdges, 6, sHeader + "Outer Edges Offset");
        ReadNumber(&Data.nNumberOfPerimeters, 1, sHeader + "Perimeters Count");
        ReadNumber(&Data.nOffsetToPerimeters, 6, sHeader + "Perimeters Offset");
        MarkDataBorder(nPosition - 1);
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM header: ") + e.what());
    }

    unsigned nPosData = 0;

    try{
        Data.verts.resize(Data.nNumberOfVerts);
        nPosData = Data.nOffsetToVerts;
        for(int n = 0; n < Data.nNumberOfVerts; n++){
            //Collect verts
            Data.verts.at(n).fX = ReadNumber<float>(nullptr, 2, "Vertex " + std::to_string(n) + " > X", &nPosData);
            Data.verts.at(n).fY = ReadNumber<float>(nullptr, 2, "Vertex " + std::to_string(n) + " > Y", &nPosData);
            Data.verts.at(n).fZ = ReadNumber<float>(nullptr, 2, "Vertex " + std::to_string(n) + " > Z", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM vertex data: ") + e.what());
    }
    try{
        Data.faces.resize(Data.nNumberOfFaces);
        nPosData = Data.nOffsetToIndices;
        for(int n = 0; n < Data.nNumberOfFaces; n++){
            //Collect indices
            Data.faces.at(n).nIndexVertex[0] = ReadNumber<signed int>(nullptr, 4, "Face " + std::to_string(n) + " > Vert Index 0", &nPosData);
            Data.faces.at(n).nIndexVertex[1] = ReadNumber<signed int>(nullptr, 4, "Face " + std::to_string(n) + " > Vert Index 1", &nPosData);
            Data.faces.at(n).nIndexVertex[2] = ReadNumber<signed int>(nullptr, 4, "Face " + std::to_string(n) + " > Vert Index 2", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM face data: ") + e.what());
    }
    try{
        nPosData = Data.nOffsetToMaterials;
        for(int n = 0; n < Data.nNumberOfFaces; n++){
            //Collect materials
            ReadNumber(&Data.faces.at(n).nMaterialID, 4, "Face " + std::to_string(n) + " > Material ID", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM surface ID data: ") + e.what());
    }
    try{
        nPosData = Data.nOffsetToNormals;
        for(int n = 0; n < Data.nNumberOfFaces; n++){
            //Collect normals
            Data.faces.at(n).vNormal.fX = ReadNumber<float>(nullptr, 2, "Face " + std::to_string(n) + " > Normal > X", &nPosData);
            Data.faces.at(n).vNormal.fY = ReadNumber<float>(nullptr, 2, "Face " + std::to_string(n) + " > Normal > Y", &nPosData);
            Data.faces.at(n).vNormal.fZ = ReadNumber<float>(nullptr, 2, "Face " + std::to_string(n) + " > Normal > Z", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM face normal data: ") + e.what());
    }
    try{
        nPosData = Data.nOffsetToDistances;
        for(int n = 0; n < Data.nNumberOfFaces; n++){
            //Collect distances
            Data.faces.at(n).fDistance = ReadNumber<float>(nullptr, 2, "Face " + std::to_string(n) + " > Plane Distance", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM plane distance data: ") + e.what());
    }
    try{
        Data.aabb.resize(Data.nNumberOfAabb);
        nPosData = Data.nOffsetToAabb;
        for(int n = 0; n < Data.nNumberOfAabb; n++){
            //Collect aabb
            Data.aabb.at(n).vBBmin.fX = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Min > X", &nPosData);
            Data.aabb.at(n).vBBmin.fY = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Min > Y", &nPosData);
            Data.aabb.at(n).vBBmin.fZ = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Min > Z", &nPosData);
            Data.aabb.at(n).vBBmax.fX = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Max > X", &nPosData);
            Data.aabb.at(n).vBBmax.fY = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Max > Y", &nPosData);
            Data.aabb.at(n).vBBmax.fZ = ReadNumber<float>(nullptr, 2, "AABB " + std::to_string(n) + " > Bounding Box Max > Z", &nPosData);
            Data.aabb.at(n).nID = ReadNumber<signed int>(nullptr, 4, "AABB " + std::to_string(n) + " > Face Index", &nPosData);
            ReadNumber(&Data.aabb.at(n).nExtra, 4, "AABB " + std::to_string(n) + " > Unknown", &nPosData);
            ReadNumber(&Data.aabb.at(n).nProperty, 4, "AABB " + std::to_string(n) + " > Second Child Property", &nPosData);
            ReadNumber(&Data.aabb.at(n).nChild1, 4, "AABB " + std::to_string(n) + " > Child 1 Index", &nPosData);
            ReadNumber(&Data.aabb.at(n).nChild2, 4, "AABB " + std::to_string(n) + " > Child 2 Index", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM aabb data: ") + e.what());
    }
    try{
        nPosData = Data.nOffsetToAdjacentFaces;
        for(int n = 0; n < Data.nNumberOfAdjacentFaces; n++){
            if(n < Data.faces.size()){
                Data.faces.at(n).nAdjacentFaces[0] = ReadNumber<signed int>(nullptr, 4, "Walkable Face " + std::to_string(n) + " > Adjacent Edge 0", &nPosData);
                Data.faces.at(n).nAdjacentFaces[1] = ReadNumber<signed int>(nullptr, 4, "Walkable Face " + std::to_string(n) + " > Adjacent Edge 1", &nPosData);
                Data.faces.at(n).nAdjacentFaces[2] = ReadNumber<signed int>(nullptr, 4, "Walkable Face " + std::to_string(n) + " > Adjacent Edge 2", &nPosData);
                MarkDataBorder(nPosData - 1);
            }
            else Error("More adjacent faces than faces!");
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM adjacent edge data: ") + e.what());
    }
    try{
        Data.edges.resize(Data.nNumberOfEdges);
        nPosData = Data.nOffsetToEdges;
        for(int n = 0; n < Data.nNumberOfEdges; n++){
            ReadNumber(&Data.edges.at(n).nIndex, 4, "Outer Edge " + std::to_string(n) + " > Index", &nPosData);
            ReadNumber(&Data.edges.at(n).nTransition, 4, "Outer Edge " + std::to_string(n) + " > Transition", &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM edge data: ") + e.what());
    }
    try{
        Data.perimeters.resize(Data.nNumberOfPerimeters);
        nPosData = Data.nOffsetToPerimeters;
        for(int n = 0; n < Data.nNumberOfPerimeters; n++){
            ReadNumber(&Data.perimeters.at(n), 4, "Perimeter " + std::to_string(n), &nPosData);
            MarkDataBorder(nPosData - 1);
        }
    }
    catch(const std::exception & e){
        throw mdlexception(std::string("Reading BWM perimeter data: ") + e.what());
    }

    ReportMdl << "Decompiled BWM in " << tDecompile.GetTime() << ".\n";
}
