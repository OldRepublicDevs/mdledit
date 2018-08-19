#include "MDL.h"
#include <algorithm>

/**
    Functions:
    MDL::Compile()
    MDL::WriteAabb()
    MDL::WriteNodes()
/**/

unsigned nMdxPrevPadding = 0;

unsigned long placeholder_data = 0xFFFFFFFFFFFFFFFF;
unsigned char * placeholder = reinterpret_cast<unsigned char*>(&placeholder_data);

bool MDL::Compile(){
    ReportObject ReportMdl(*this);
    Timer tCompile;
    nPosition = 0;
    sBuffer.resize(0);
    bKnown.resize(0);
    positions.clear();
    if(!Mdx) Mdx.reset(new MDX());
    else{
        Mdx->GetBuffer().clear();
        Mdx->GetKnownData().clear();
        Mdx->nPosition = 0;
        Mdx->positions.clear();
    }
    Report("Compiling model...");

    FileHeader &Data = *FH;

    std::string sFileHeader = "File Header > ";

    /// Setting up the pointer to the model header data
    Data.MH.dataRegions.emplace_back("MDL", 0, 208);

    /// File header
    WriteNumber(&Data.nZero, 8, sFileHeader + "Padding");
    unsigned PHnMdlLength = WriteBytes(placeholder, 4, 1, sFileHeader + "MDL Length"); // to be filled later
    unsigned PHnMdxLength = WriteBytes(placeholder, 4, 1, sFileHeader + "MDX Length"); // to be filled later
    MarkDataBorder(nPosition - 1);

    /// Geo Header
    std::string sGeometryHeader = "Geometry Header > ";

    // Function pointers
    Data.MH.GH.nFunctionPointer0 = FunctionPointer1(FN_PTR_MODEL);
    Data.MH.GH.nFunctionPointer1 = FunctionPointer2(FN_PTR_MODEL);
    WriteNumber(&Data.MH.GH.nFunctionPointer0, 9, sGeometryHeader + "Function Pointer 1");
    WriteNumber(&Data.MH.GH.nFunctionPointer1, 9, sGeometryHeader + "Function Pointer 2");

    // Model name
    Data.MH.GH.sName.resize(32);
    WriteString(&Data.MH.GH.sName, 32, 3, sGeometryHeader + "Model Name");

    // Write placeholder for root node offset
    unsigned PHnOffsetToRootNode = WriteBytes(placeholder, 4, 6, sGeometryHeader + "Offset To Root Node");

    // Total number of nodes
    WriteNumber(&Data.MH.GH.nTotalNumberOfNodes, 1, sGeometryHeader + "Total Number Of Nodes");

    // Empty runtime arrays
    WriteNumber(&Data.MH.GH.RuntimeArray1.nOffset, 8, sGeometryHeader + "Runtime Array 1 > Offset");
    WriteNumber(&Data.MH.GH.RuntimeArray1.nCount, 8, sGeometryHeader + "Runtime Array 1 > Count 1");
    WriteNumber(&Data.MH.GH.RuntimeArray1.nCount2, 8, sGeometryHeader + "Runtime Array 1 > Count 2");
    WriteNumber(&Data.MH.GH.RuntimeArray2.nOffset, 8, sGeometryHeader + "Runtime Array 2 > Offset");
    WriteNumber(&Data.MH.GH.RuntimeArray2.nCount, 8, sGeometryHeader + "Runtime Array 2 > Count 1");
    WriteNumber(&Data.MH.GH.RuntimeArray2.nCount2, 8, sGeometryHeader + "Runtime Array 2 > Count 2");

    // Reference count
    WriteNumber(&Data.MH.GH.nRefCount, 8, sGeometryHeader + "Reference Count");

    // Model type
    WriteNumber(&Data.MH.GH.nModelType, 7, sGeometryHeader + "Model Type");

    // Padding (3 bytes)
    WriteNumber(&Data.MH.GH.nPadding[0], 11, sGeometryHeader + "Padding");
    WriteNumber(&Data.MH.GH.nPadding[1], 11, sGeometryHeader + "Padding");
    WriteNumber(&Data.MH.GH.nPadding[2], 11, sGeometryHeader + "Padding");

    // Mark the end of geo header
    MarkDataBorder(nPosition - 1);

    /// Model header
    std::string sModelHeader = "Model Header > ";

    // Classification
    WriteNumber(&Data.MH.nClassification, 7, sModelHeader + "Classification");

    // "Sub-classification" (not understood)
    WriteNumber(&Data.MH.nSubclassification, 10, sModelHeader + "\"Subclassification\"");

    // Empty byte (can be used for SG presence marking)
    if(bWriteSmoothing) Data.MH.nUnknown = 1;
    WriteNumber(&Data.MH.nUnknown, 8, sModelHeader + "Unknown");

    // Affected By Fog
    WriteNumber(&Data.MH.nAffectedByFog, 7, sModelHeader + "Affected By Fog");

    // Child model count
    WriteNumber(&Data.MH.nChildModelCount, 8, sModelHeader + "Child Model Count");

    // Animation ArrayHead
    Data.MH.AnimationArray.ResetToSize(Data.MH.Animations.size());          // First, reset to size
    unsigned PHnOffsetToAnimationArray = WriteBytes(placeholder, 4, 6, sModelHeader + "Animation Array > Offset");     // Write placeholder offset
    WriteNumber(&Data.MH.AnimationArray.nCount, 1, sModelHeader + "Animation Array > Count 1");                         // Write count1
    WriteNumber(&Data.MH.AnimationArray.nCount2, 1, sModelHeader + "Animation Array > Count 2");                        // Write count2

    // Supermodel Reference
    WriteNumber(&Data.MH.nSupermodelReference, 11, sModelHeader + "Supermodel Reference");

    // BB min and max, radius, scale
    WriteFloat(&Data.MH.vBBmin.fX, 2, sModelHeader + "Bounding Box Min > X");
    WriteFloat(&Data.MH.vBBmin.fY, 2, sModelHeader + "Bounding Box Min > Y");
    WriteFloat(&Data.MH.vBBmin.fZ, 2, sModelHeader + "Bounding Box Min > Z");
    WriteFloat(&Data.MH.vBBmax.fX, 2, sModelHeader + "Bounding Box Max > X");
    WriteFloat(&Data.MH.vBBmax.fY, 2, sModelHeader + "Bounding Box Max > Y");
    WriteFloat(&Data.MH.vBBmax.fZ, 2, sModelHeader + "Bounding Box Max > Z");
    WriteFloat(&Data.MH.fRadius, 2, sModelHeader + "Radius");
    WriteFloat(&Data.MH.fScale, 2, sModelHeader + "Scale");

    // Supermodel name
    Data.MH.cSupermodelName.resize(32);
    WriteString(&Data.MH.cSupermodelName, 32, 3, sModelHeader + "Supermodel Name");

    // Offset to head root node
    unsigned PHnOffsetToHeadRootNode = WriteBytes(placeholder, 4, 6, sModelHeader + "Offset To Head Root Node");

    // Padding (4 bytes)
    WriteNumber(&Data.MH.nPadding, 8, sModelHeader + "Padding");

    // Mdx size
    unsigned PHnMdxLength2 = WriteBytes(placeholder, 4, 1, sModelHeader + "MDX Length");

    // Mdx offset
    WriteNumber(&Data.MH.nOffsetIntoMdx, 8, sModelHeader + "MDX Offset");

    // Name ArrayHead
    Data.MH.NameArray.ResetToSize(Data.MH.Names.size());
    unsigned PHnOffsetToNameArray = WriteBytes(placeholder, 4, 6, sModelHeader + "Name Array > Offset");
    WriteNumber(&Data.MH.NameArray.nCount, 1, sModelHeader + "Name Array > Count 1");
    WriteNumber(&Data.MH.NameArray.nCount2, 1, sModelHeader + "Name Array > Count 2");

    // Mark the end of model header
    MarkDataBorder(nPosition - 1);

    /// Create Name array
    std::string sNames = "Names > ";

    // Record the offset of the name array
    Data.MH.NameArray.nOffset = nPosition - 12;
    WriteNumber(&Data.MH.NameArray.nOffset, 0, "", &PHnOffsetToNameArray);

    std::vector<unsigned> PHnOffsetToName;
    for(unsigned c = 0; c < Data.MH.Names.size(); c++){
        // Write placeholder
        PHnOffsetToName.push_back(WriteBytes(placeholder, 4, 6, sNames + "Offset " + std::to_string(c)));

        MarkDataBorder(nPosition - 1);
    }
    for(unsigned c = 0; c < Data.MH.Names.size(); c++){
        // Write offset to placeholder
        Data.MH.Names.at(c).nOffset = nPosition - 12;
        WriteNumber(&Data.MH.Names.at(c).nOffset, 0, "", &PHnOffsetToName.at(c));

        // Write name
        WriteString(&Data.MH.Names[c].sName, 0, 3, sNames + "Name " + std::to_string(c) + " (" + std::string(Data.MH.Names[c].sName.c_str()) + ")");

        MarkDataBorder(nPosition - 1);
    }

    /// Create Animation array

    // Write offset to placeholder
    Data.MH.AnimationArray.nOffset = nPosition - 12;
    WriteNumber(&Data.MH.AnimationArray.nOffset, 0, "", &PHnOffsetToAnimationArray);

    std::vector<unsigned> pnOffsetsToAnimation;
    for(unsigned c = 0; c < Data.MH.Animations.size(); c++){
        std::string sAnimation = "Anim " + std::to_string(c) + " > ";
        // Write placeholder
        pnOffsetsToAnimation.push_back(WriteBytes(placeholder, 4, 6, sAnimation + "Offset"));

        MarkDataBorder(nPosition - 1);
    }
    for(unsigned c = 0; c < Data.MH.Animations.size(); c++){
        /// This is where we fill EVERYTHING about the animation
        Animation & anim = Data.MH.Animations[c];
        std::string sAnimation = "Anim "  + std::string(anim.sName.c_str()) + " (" + std::to_string(c) + ") > ";

        // Write offset to placeholder
        Data.MH.Animations[c].nOffset = nPosition - 12;
        WriteNumber(&Data.MH.Animations[c].nOffset, 0, "", &pnOffsetsToAnimation[c]);

        /// Setting up the pointer to the data
        anim.dataRegions.emplace_back("MDL", anim.nOffset, 136);

        // Write function pointers
        anim.nFunctionPointer0 = FunctionPointer1(FN_PTR_ANIM);
        anim.nFunctionPointer1 = FunctionPointer2(FN_PTR_ANIM);
        WriteNumber(&anim.nFunctionPointer0, 9, sAnimation + "Function Pointer 1");
        WriteNumber(&anim.nFunctionPointer1, 9, sAnimation + "Function Pointer 2");

        // Animation name
        anim.sName.resize(32);
        WriteString(&anim.sName, 32, 3, sAnimation + "Name");

        // Offset to root node
        unsigned PHnOffsetToFirstNode = WriteBytes(placeholder, 4, 6, sAnimation + "Offset To Root Node");

        // Number of nodes (total possible = number of names)
        anim.nNumberOfNames = Data.MH.Names.size();
        WriteNumber(&anim.nNumberOfNames, 1, sAnimation + "Total Number Of Nodes");

        // Empty runtime arrays
        WriteNumber(&anim.RuntimeArray1.nOffset, 8, sAnimation + "Runtime Array 1 > Offset");
        WriteNumber(&anim.RuntimeArray1.nCount, 8, sAnimation + "Runtime Array 1 > Count 1");
        WriteNumber(&anim.RuntimeArray1.nCount2, 8, sAnimation + "Runtime Array 1 > Count 2");
        WriteNumber(&anim.RuntimeArray2.nOffset, 8, sAnimation + "Runtime Array 2 > Offset");
        WriteNumber(&anim.RuntimeArray2.nCount, 8, sAnimation + "Runtime Array 2 > Count 1");
        WriteNumber(&anim.RuntimeArray2.nCount2, 8, sAnimation + "Runtime Array 2 > Count 2");

        // Reference count
        WriteNumber(&anim.nRefCount, 8, sAnimation + "Reference Count");

        // Model Type
        WriteNumber(&anim.nModelType, 7, sAnimation + "Model Type");

        // Padding (3 bytes)
        WriteNumber(&anim.nPadding[0], 11, sAnimation + "Padding");
        WriteNumber(&anim.nPadding[1], 11, sAnimation + "Padding");
        WriteNumber(&anim.nPadding[2], 11, sAnimation + "Padding");

        MarkDataBorder(nPosition - 1);

        // Animation length
        WriteFloat(&anim.fLength, 2, sAnimation + "Length");

        // Animation transition
        WriteFloat(&anim.fTransition, 2, sAnimation + "Transition");

        // AnimRoot
        anim.sAnimRoot.resize(32);
        WriteString(&anim.sAnimRoot, 32, 3, sAnimation + "AnimRoot");

        // Event ArrayHead
        anim.EventArray.ResetToSize(anim.Events.size());
        unsigned PHnOffsetToEventArray = WriteBytes(placeholder, 4, 6, sAnimation + "Event Array > Offset");
        WriteNumber(&anim.EventArray.nCount, 1, sAnimation + "Event Array > Count 1");
        WriteNumber(&anim.EventArray.nCount2, 1, sAnimation + "Event Array > Count 2");

        // Padding (4 bytes)
        WriteNumber(&anim.nPadding2, 8, sAnimation + "Padding");

        MarkDataBorder(nPosition - 1);

        if(anim.Events.size() > 0){
            // Enter the offset to placeholder
            anim.EventArray.nOffset = nPosition - 12;
            WriteNumber(&anim.EventArray.nOffset, 0, "", &PHnOffsetToEventArray);
            for(unsigned b = 0; b < anim.Events.size(); b++){
                WriteFloat(&anim.Events[b].fTime, 2, sAnimation + "Event " + std::to_string(b) + " > Time");
                anim.Events[b].sName.resize(32);
                WriteString(&anim.Events[b].sName, 32, 3, sAnimation + "Event " + std::to_string(b) + " > Name (" + std::string(anim.Events[b].sName.c_str()) + ")");
                MarkDataBorder(nPosition - 1);
            }
        }
        else{
            Data.MH.Animations[c].EventArray.nOffset = 0;
            WriteNumber(&Data.MH.Animations[c].EventArray.nOffset, 0, "", &PHnOffsetToEventArray);
        }

        Data.MH.Animations[c].nOffsetToRootAnimationNode = nPosition - 12;
        WriteNumber(&Data.MH.Animations[c].nOffsetToRootAnimationNode, 0, "", &PHnOffsetToFirstNode);
        std::vector<unsigned> indices;
        indices.reserve(anim.ArrayOfNodes.size());
        WriteNodes(anim.ArrayOfNodes.front(), indices, sAnimation);
    }

    Data.MH.GH.nOffsetToRootNode = nPosition - 12;
    WriteNumber(&Data.MH.GH.nOffsetToRootNode, 0, "", &PHnOffsetToRootNode);
    if(!Data.MH.bHeadLink || !NodeExists("neck_g")){
        Data.MH.nOffsetToHeadRootNode = nPosition - 12;
        WriteNumber(&Data.MH.nOffsetToHeadRootNode, 0, "", &PHnOffsetToHeadRootNode); //For now we will make these two equal in all cases
    }
    else Data.MH.nOffsetToHeadRootNode = PHnOffsetToHeadRootNode;
    nMdxPrevPadding = 0;
    std::string sGeometry = "Geometry > ";
    std::vector<unsigned> indices;
    indices.reserve(Data.MH.ArrayOfNodes.size());
    WriteNodes(Data.MH.ArrayOfNodes.front(), indices, sGeometry);
    for(Node &node : Data.MH.ArrayOfNodes) if((node.Head.nType & NODE_MESH) && !(node.Head.nType & NODE_SABER) && !(node.Head.nType & NODE_SKIN)) WriteMDX(node, GetNodeName(node) + " > ");
    for(Node &node : Data.MH.ArrayOfNodes) if(node.Head.nType & (NODE_SKIN)) WriteMDX(node, GetNodeName(node) + " > ");

    Data.nMdlLength = nPosition - 12;
    WriteNumber(&Data.nMdlLength, 0, "", &PHnMdlLength);
    Data.nMdxLength = Mdx->nPosition;
    WriteNumber(&Data.nMdxLength, 0, "", &PHnMdxLength);
    Data.MH.nMdxLength2 = Mdx->nPosition;
    WriteNumber(&Data.MH.nMdxLength2, 0, "", &PHnMdxLength2);

    positions.shrink_to_fit();

    if(Wok) Wok->Compile();
    if(Pwk) Pwk->Compile();
    if(Dwk0) Dwk0->Compile();
    if(Dwk1) Dwk1->Compile();
    if(Dwk2) Dwk2->Compile();
    ReportMdl << "Model compiled in " << tCompile.GetTime() << ".\n";
    return true;
}

static unsigned nAabbCount = 0;
void MDL::WriteAabb(Aabb & aabb, const std::string & sPrefix){
    aabb.nOffset = nPosition - 12;
    std::string sAabb = sPrefix + "AABB " + std::to_string(nAabbCount) + " > ";
    nAabbCount++;

    /// Setting up the pointer to the data
    aabb.dataRegions.emplace_back("MDL", nPosition, 40);

    // BB min and max
    WriteFloat(&aabb.vBBmin.fX, 2, sAabb + "Bounding Box Min > X");
    WriteFloat(&aabb.vBBmin.fY, 2, sAabb + "Bounding Box Min > Y");
    WriteFloat(&aabb.vBBmin.fZ, 2, sAabb + "Bounding Box Min > Z");
    WriteFloat(&aabb.vBBmax.fX, 2, sAabb + "Bounding Box Max > X");
    WriteFloat(&aabb.vBBmax.fY, 2, sAabb + "Bounding Box Max > Y");
    WriteFloat(&aabb.vBBmax.fZ, 2, sAabb + "Bounding Box Max > Z");

    // Child offsets
    unsigned PHnChild1 = WriteNumber(aabb.nChild1.GetPtr(), 6, sAabb + "Child 1 Offset");
    unsigned PHnChild2 = WriteNumber(aabb.nChild2.GetPtr(), 6, sAabb + "Child 2 Offset");

    // Face ID - this is stored as a short in mdledit because face IDs only fit in a short in other places. Here it has to be converted to int
    signed int nConvertedID = aabb.nID.Valid() ? static_cast<signed int>(aabb.nID) : -1;
    WriteNumber(&nConvertedID, 4, sAabb + "Face Index");

    // Write significant plane
    WriteNumber(&aabb.nProperty, 4, sAabb + "Second Child Property");

    MarkDataBorder(nPosition - 1);

    // Go trough children
    if(aabb.Child1.size() > 0){
        aabb.nChild1 = nPosition - 12;
        WriteNumber(&aabb.nChild1, 0, "", &PHnChild1);
        WriteAabb(aabb.Child1.front(), sPrefix);
    }
    if(aabb.Child2.size() > 0){
        aabb.nChild2 = nPosition - 12;
        WriteNumber(&aabb.nChild2, 0, "", &PHnChild2);
        WriteAabb(aabb.Child2.front(), sPrefix);
    }
}

void MDL::WriteMDX(Node & node, const std::string & sPrefix){
    if(!(node.Head.nType & NODE_MESH)) return;
    unsigned PHnOffsetIntoMdx = node.Mesh.nOffsetIntoMdx;
    std::string sMdx = sPrefix;

    if((node.Head.nType & NODE_SABER) || node.Mesh.Vertices.size() == 0){
        node.Mesh.nOffsetIntoMdx = 0;
        WriteNumber(&node.Mesh.nOffsetIntoMdx, 0, "", &PHnOffsetIntoMdx);
        return;
    }

    /// First, write the padding that we collected from the previous MDX item
    std::string sPadding;
    sPadding.resize(nMdxPrevPadding, '\0');
    if(sPadding.size()) Mdx->WriteString(&sPadding, sPadding.size(), 0, "Padding");
    if(Mdx->nPosition > 0) Mdx->MarkDataBorder(Mdx->nPosition - 1);

    node.Mesh.nOffsetIntoMdx = Mdx->nPosition;
    WriteNumber(&node.Mesh.nOffsetIntoMdx, 0, "", &PHnOffsetIntoMdx);

    for(unsigned d = 0; d < node.Mesh.nNumberOfVerts; d++){
        Vertex &vert = node.Mesh.Vertices.at(d);
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_VERTEX){

            Mdx->WriteFloat(&vert.MDXData.vVertex.fX, 2, sMdx + "Vertex " + std::to_string(d) + " > X");
            Mdx->WriteFloat(&vert.MDXData.vVertex.fY, 2, sMdx + "Vertex " + std::to_string(d) + " > Y");
            Mdx->WriteFloat(&vert.MDXData.vVertex.fZ, 2, sMdx + "Vertex " + std::to_string(d) + " > Z");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_NORMAL){
            if(!bXbox){
                Mdx->WriteFloat(&vert.MDXData.vNormal.fX, 2, sMdx + "Normal " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vNormal.fY, 2, sMdx + "Normal " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vNormal.fZ, 2, sMdx + "Normal " + std::to_string(d) + " > Z");
            }
            else{
                unsigned nCompressed = CompressVector(vert.MDXData.vNormal);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Normal " + std::to_string(d));
            }
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_COLOR){
            Mdx->WriteFloat(&vert.MDXData.cColor.fR, 2, sMdx + "Color " + std::to_string(d) + " > R");
            Mdx->WriteFloat(&vert.MDXData.cColor.fG, 2, sMdx + "Color " + std::to_string(d) + " > G");
            Mdx->WriteFloat(&vert.MDXData.cColor.fB, 2, sMdx + "Color " + std::to_string(d) + " > B");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV1){
            Mdx->WriteFloat(&vert.MDXData.vUV1.fX, 2, sMdx + "UVs 1 " + std::to_string(d) + " > U");
            Mdx->WriteFloat(&vert.MDXData.vUV1.fY, 2, sMdx + "UVs 1 " + std::to_string(d) + " > V");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV2){
            Mdx->WriteFloat(&vert.MDXData.vUV2.fX, 2, sMdx + "UVs 2 " + std::to_string(d) + " > U");
            Mdx->WriteFloat(&vert.MDXData.vUV2.fY, 2, sMdx + "UVs 2 " + std::to_string(d) + " > V");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV3){
            Mdx->WriteFloat(&vert.MDXData.vUV3.fX, 2, sMdx + "UVs 3 " + std::to_string(d) + " > U");
            Mdx->WriteFloat(&vert.MDXData.vUV3.fY, 2, sMdx + "UVs 3 " + std::to_string(d) + " > V");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV4){
            Mdx->WriteFloat(&vert.MDXData.vUV4.fX, 2, sMdx + "UVs 4 " + std::to_string(d) + " > U");
            Mdx->WriteFloat(&vert.MDXData.vUV4.fY, 2, sMdx + "UVs 4 " + std::to_string(d) + " > V");
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT1){
            if(!bXbox){
                Mdx->WriteFloat(&vert.MDXData.vTangent1[0].fX, 2, sMdx + "Tangent Space 1 > Bitangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[0].fY, 2, sMdx + "Tangent Space 1 > Bitangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[0].fZ, 2, sMdx + "Tangent Space 1 > Bitangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[1].fX, 2, sMdx + "Tangent Space 1 > Tangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[1].fY, 2, sMdx + "Tangent Space 1 > Tangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[1].fZ, 2, sMdx + "Tangent Space 1 > Tangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[2].fX, 2, sMdx + "Tangent Space 1 > Normal " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[2].fY, 2, sMdx + "Tangent Space 1 > Normal " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent1[2].fZ, 2, sMdx + "Tangent Space 1 > Normal " + std::to_string(d) + " > Z");
            }
            else{
                unsigned nCompressed = CompressVector(vert.MDXData.vTangent1[0]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 1 > Bitangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent1[1]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 1 > Tangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent1[2]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 1 > Normal " + std::to_string(d));
            }
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT2){
            if(!bXbox){
                Mdx->WriteFloat(&vert.MDXData.vTangent2[0].fX, 2, sMdx + "Tangent Space 2 > Bitangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[0].fY, 2, sMdx + "Tangent Space 2 > Bitangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[0].fZ, 2, sMdx + "Tangent Space 2 > Bitangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[1].fX, 2, sMdx + "Tangent Space 2 > Tangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[1].fY, 2, sMdx + "Tangent Space 2 > Tangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[1].fZ, 2, sMdx + "Tangent Space 2 > Tangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[2].fX, 2, sMdx + "Tangent Space 2 > Normal " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[2].fY, 2, sMdx + "Tangent Space 2 > Normal " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent2[2].fZ, 2, sMdx + "Tangent Space 2 > Normal " + std::to_string(d) + " > Z");
            }
            else{
                unsigned nCompressed = CompressVector(vert.MDXData.vTangent2[0]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 2 > Bitangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent2[1]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 2 > Tangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent2[2]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 2 > Normal " + std::to_string(d));
            }
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT3){
            if(!bXbox){
                Mdx->WriteFloat(&vert.MDXData.vTangent3[0].fX, 2, sMdx + "Tangent Space 3 > Bitangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[0].fY, 2, sMdx + "Tangent Space 3 > Bitangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[0].fZ, 2, sMdx + "Tangent Space 3 > Bitangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[1].fX, 2, sMdx + "Tangent Space 3 > Tangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[1].fY, 2, sMdx + "Tangent Space 3 > Tangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[1].fZ, 2, sMdx + "Tangent Space 3 > Tangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[2].fX, 2, sMdx + "Tangent Space 3 > Normal " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[2].fY, 2, sMdx + "Tangent Space 3 > Normal " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent3[2].fZ, 2, sMdx + "Tangent Space 3 > Normal " + std::to_string(d) + " > Z");
            }
            else{
                unsigned nCompressed = CompressVector(vert.MDXData.vTangent3[0]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 3 > Bitangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent3[1]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 3 > Tangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent3[2]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 3 > Normal " + std::to_string(d));
            }
        }
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT4){
            if(!bXbox){
                Mdx->WriteFloat(&vert.MDXData.vTangent4[0].fX, 2, sMdx + "Tangent Space 4 > Bitangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[0].fY, 2, sMdx + "Tangent Space 4 > Bitangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[0].fZ, 2, sMdx + "Tangent Space 4 > Bitangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[1].fX, 2, sMdx + "Tangent Space 4 > Tangent " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[1].fY, 2, sMdx + "Tangent Space 4 > Tangent " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[1].fZ, 2, sMdx + "Tangent Space 4 > Tangent " + std::to_string(d) + " > Z");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[2].fX, 2, sMdx + "Tangent Space 4 > Normal " + std::to_string(d) + " > X");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[2].fY, 2, sMdx + "Tangent Space 4 > Normal " + std::to_string(d) + " > Y");
                Mdx->WriteFloat(&vert.MDXData.vTangent4[2].fZ, 2, sMdx + "Tangent Space 4 > Normal " + std::to_string(d) + " > Z");
            }
            else{
                unsigned nCompressed = CompressVector(vert.MDXData.vTangent4[0]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 4 > Bitangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent4[1]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 4 > Tangent " + std::to_string(d));
                nCompressed = CompressVector(vert.MDXData.vTangent4[2]);
                Mdx->WriteNumber(&nCompressed, 2, sMdx + "Tangent Space 4 > Normal " + std::to_string(d));
            }
        }
        if(node.Head.nType &NODE_SKIN){
            Mdx->WriteFloat(&vert.MDXData.Weights.fWeightValue[0], 2, sMdx + "Weight " + std::to_string(d) + " > Value 0");
            Mdx->WriteFloat(&vert.MDXData.Weights.fWeightValue[1], 2, sMdx + "Weight " + std::to_string(d) + " > Value 1");
            Mdx->WriteFloat(&vert.MDXData.Weights.fWeightValue[2], 2, sMdx + "Weight " + std::to_string(d) + " > Value 2");
            Mdx->WriteFloat(&vert.MDXData.Weights.fWeightValue[3], 2, sMdx + "Weight " + std::to_string(d) + " > Value 3");
            if(bXbox){
                Mdx->WriteNumber(vert.MDXData.Weights.nWeightIndex[0].GetPtr(), 5, sMdx + "Weight " + std::to_string(d) + " > Index 0");
                Mdx->WriteNumber(vert.MDXData.Weights.nWeightIndex[1].GetPtr(), 5, sMdx + "Weight " + std::to_string(d) + " > Index 1");
                Mdx->WriteNumber(vert.MDXData.Weights.nWeightIndex[2].GetPtr(), 5, sMdx + "Weight " + std::to_string(d) + " > Index 2");
                Mdx->WriteNumber(vert.MDXData.Weights.nWeightIndex[3].GetPtr(), 5, sMdx + "Weight " + std::to_string(d) + " > Index 3");
            }
            else{
                double fIndex = static_cast<double>(vert.MDXData.Weights.nWeightIndex[0].Valid() ? static_cast<int>(vert.MDXData.Weights.nWeightIndex[0]) : -1);
                Mdx->WriteFloat(&fIndex, 2, sMdx + "Weight " + std::to_string(d) + " > Index 0");
                fIndex = static_cast<double>(vert.MDXData.Weights.nWeightIndex[1].Valid() ? static_cast<int>(vert.MDXData.Weights.nWeightIndex[1]) : -1);
                Mdx->WriteFloat(&fIndex, 2, sMdx + "Weight " + std::to_string(d) + " > Index 1");
                fIndex = static_cast<double>(vert.MDXData.Weights.nWeightIndex[2].Valid() ? static_cast<int>(vert.MDXData.Weights.nWeightIndex[2]) : -1);
                Mdx->WriteFloat(&fIndex, 2, sMdx + "Weight " + std::to_string(d) + " > Index 2");
                fIndex = static_cast<double>(vert.MDXData.Weights.nWeightIndex[3].Valid() ? static_cast<int>(vert.MDXData.Weights.nWeightIndex[3]) : -1);
                Mdx->WriteFloat(&fIndex, 2, sMdx + "Weight " + std::to_string(d) + " > Index 3");
            }
        }
        Mdx->MarkDataBorder(Mdx->nPosition - 1);
    }

    /// We first need to save the current position for the padding afterwards
    unsigned nCurrentMdx = Mdx->nPosition;

    /// Also write the extra empty vert data
    sMdx = sPrefix + "Extra Data > ";
    node.Mesh.MDXData.nNameIndex = node.Head.nNameIndex;
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_VERTEX){
        if(node.Head.nType &NODE_SKIN) node.Mesh.MDXData.vVertex.Set(1000000.0, 1000000.0, 1000000.0);
        else node.Mesh.MDXData.vVertex.Set(10000000.0, 10000000.0, 10000000.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.vVertex.fX, 8, sMdx + "Vertex > X");
        Mdx->WriteFloat(&node.Mesh.MDXData.vVertex.fY, 8, sMdx + "Vertex > Y");
        Mdx->WriteFloat(&node.Mesh.MDXData.vVertex.fZ, 8, sMdx + "Vertex > Z");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_NORMAL){
        node.Mesh.MDXData.vNormal.Set(0.0, 0.0, 0.0);
        if(!bXbox){
            Mdx->WriteFloat(&node.Mesh.MDXData.vNormal.fX, 8, sMdx + "Normal > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vNormal.fY, 8, sMdx + "Normal > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vNormal.fZ, 8, sMdx + "Normal > Z");
        }
        else{
            unsigned nCompressed = CompressVector(node.Mesh.MDXData.vNormal);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Normal");
        }
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_COLOR){
        node.Mesh.MDXData.cColor.Set(0.0, 0.0, 0.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.cColor.fR, 8, sMdx + "Color > R");
        Mdx->WriteFloat(&node.Mesh.MDXData.cColor.fG, 8, sMdx + "Color > G");
        Mdx->WriteFloat(&node.Mesh.MDXData.cColor.fB, 8, sMdx + "Color > B");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV1){
        node.Mesh.MDXData.vUV1.Set(0.0, 0.0, 0.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV1.fX, 8, sMdx + "UVs 1 > U");
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV1.fY, 8, sMdx + "UVs 1 > V");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV2){
        node.Mesh.MDXData.vUV2.Set(0.0, 0.0, 0.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV2.fX, 8, sMdx + "UVs 2 > U");
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV2.fY, 8, sMdx + "UVs 2 > V");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV3){
        node.Mesh.MDXData.vUV3.Set(0.0, 0.0, 0.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV3.fX, 8, sMdx + "UVs 3 > U");
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV3.fY, 8, sMdx + "UVs 3 > V");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV4){
        node.Mesh.MDXData.vUV4.Set(0.0, 0.0, 0.0);
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV4.fX, 8, sMdx + "UVs 4 > U");
        Mdx->WriteFloat(&node.Mesh.MDXData.vUV4.fY, 8, sMdx + "UVs 4 > V");
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT1){
        node.Mesh.MDXData.vTangent1[0].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent1[1].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent1[2].Set(0.0, 0.0, 0.0);
        if(!bXbox){
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[0].fX, 8, sMdx + "Tangent Space 1 > Bitangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[0].fY, 8, sMdx + "Tangent Space 1 > Bitangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[0].fZ, 8, sMdx + "Tangent Space 1 > Bitangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[1].fX, 8, sMdx + "Tangent Space 1 > Tangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[1].fY, 8, sMdx + "Tangent Space 1 > Tangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[1].fZ, 8, sMdx + "Tangent Space 1 > Tangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[2].fX, 8, sMdx + "Tangent Space 1 > Normal > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[2].fY, 8, sMdx + "Tangent Space 1 > Normal > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent1[2].fZ, 8, sMdx + "Tangent Space 1 > Normal > Z");
        }
        else{
            unsigned nCompressed = CompressVector(node.Mesh.MDXData.vTangent1[0]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 1 > Bitangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent1[1]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 1 > Tangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent1[2]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 1 > Normal");
        }
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT2){
        node.Mesh.MDXData.vTangent2[0].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent2[1].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent2[2].Set(0.0, 0.0, 0.0);
        if(!bXbox){
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[0].fX, 8, sMdx + "Tangent Space 2 > Bitangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[0].fY, 8, sMdx + "Tangent Space 2 > Bitangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[0].fZ, 8, sMdx + "Tangent Space 2 > Bitangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[1].fX, 8, sMdx + "Tangent Space 2 > Tangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[1].fY, 8, sMdx + "Tangent Space 2 > Tangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[1].fZ, 8, sMdx + "Tangent Space 2 > Tangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[2].fX, 8, sMdx + "Tangent Space 2 > Normal > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[2].fY, 8, sMdx + "Tangent Space 2 > Normal > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent2[2].fZ, 8, sMdx + "Tangent Space 2 > Normal > Z");
        }
        else{
            unsigned nCompressed = CompressVector(node.Mesh.MDXData.vTangent2[0]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 2 > Bitangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent2[1]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 2 > Tangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent2[2]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 2 > Normal");
        }
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT3){
        node.Mesh.MDXData.vTangent3[0].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent3[1].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent3[2].Set(0.0, 0.0, 0.0);
        if(!bXbox){
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[0].fX, 8, sMdx + "Tangent Space 3 > Bitangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[0].fY, 8, sMdx + "Tangent Space 3 > Bitangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[0].fZ, 8, sMdx + "Tangent Space 3 > Bitangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[1].fX, 8, sMdx + "Tangent Space 3 > Tangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[1].fY, 8, sMdx + "Tangent Space 3 > Tangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[1].fZ, 8, sMdx + "Tangent Space 3 > Tangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[2].fX, 8, sMdx + "Tangent Space 3 > Normal > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[2].fY, 8, sMdx + "Tangent Space 3 > Normal > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent3[2].fZ, 8, sMdx + "Tangent Space 3 > Normal > Z");
        }
        else{
            unsigned nCompressed = CompressVector(node.Mesh.MDXData.vTangent3[0]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 3 > Bitangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent3[1]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 3 > Tangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent3[2]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 3 > Normal");
        }
    }
    if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT4){
        node.Mesh.MDXData.vTangent4[0].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent4[1].Set(0.0, 0.0, 0.0);
        node.Mesh.MDXData.vTangent4[2].Set(0.0, 0.0, 0.0);
        if(!bXbox){
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[0].fX, 8, sMdx + "Tangent Space 4 > Bitangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[0].fY, 8, sMdx + "Tangent Space 4 > Bitangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[0].fZ, 8, sMdx + "Tangent Space 4 > Bitangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[1].fX, 8, sMdx + "Tangent Space 4 > Tangent > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[1].fY, 8, sMdx + "Tangent Space 4 > Tangent > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[1].fZ, 8, sMdx + "Tangent Space 4 > Tangent > Z");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[2].fX, 8, sMdx + "Tangent Space 4 > Normal > X");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[2].fY, 8, sMdx + "Tangent Space 4 > Normal > Y");
            Mdx->WriteFloat(&node.Mesh.MDXData.vTangent4[2].fZ, 8, sMdx + "Tangent Space 4 > Normal > Z");
        }
        else{
            unsigned nCompressed = CompressVector(node.Mesh.MDXData.vTangent4[0]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 4 > Bitangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent4[1]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 4 > Tangent");
            nCompressed = CompressVector(node.Mesh.MDXData.vTangent4[2]);
            Mdx->WriteNumber(&nCompressed, 8, sMdx + "Tangent Space 4 > Normal");
        }
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
        Mdx->WriteFloat(&node.Mesh.MDXData.Weights.fWeightValue[0], 8, sMdx + "Weight > Value 0");
        Mdx->WriteFloat(&node.Mesh.MDXData.Weights.fWeightValue[1], 8, sMdx + "Weight > Value 1");
        Mdx->WriteFloat(&node.Mesh.MDXData.Weights.fWeightValue[2], 8, sMdx + "Weight > Value 2");
        Mdx->WriteFloat(&node.Mesh.MDXData.Weights.fWeightValue[3], 8, sMdx + "Weight > Value 3");
        if(bXbox){
            Mdx->WriteNumber(node.Mesh.MDXData.Weights.nWeightIndex[0].GetPtr(), 8, sMdx + "Weight > Index 0");
            Mdx->WriteNumber(node.Mesh.MDXData.Weights.nWeightIndex[1].GetPtr(), 8, sMdx + "Weight > Index 1");
            Mdx->WriteNumber(node.Mesh.MDXData.Weights.nWeightIndex[2].GetPtr(), 8, sMdx + "Weight > Index 2");
            Mdx->WriteNumber(node.Mesh.MDXData.Weights.nWeightIndex[3].GetPtr(), 8, sMdx + "Weight > Index 3");
        }
        else{
            double fIndex = static_cast<double>(node.Mesh.MDXData.Weights.nWeightIndex[0].Valid() ? static_cast<int>(node.Mesh.MDXData.Weights.nWeightIndex[0]) : -1);
            Mdx->WriteFloat(&fIndex, 8, sMdx + "Weight > Index 0");
            fIndex = static_cast<double>(node.Mesh.MDXData.Weights.nWeightIndex[1].Valid() ? static_cast<int>(node.Mesh.MDXData.Weights.nWeightIndex[1]) : -1);
            Mdx->WriteFloat(&fIndex, 8, sMdx + "Weight > Index 1");
            fIndex = static_cast<double>(node.Mesh.MDXData.Weights.nWeightIndex[2].Valid() ? static_cast<int>(node.Mesh.MDXData.Weights.nWeightIndex[2]) : -1);
            Mdx->WriteFloat(&fIndex, 8, sMdx + "Weight > Index 2");
            fIndex = static_cast<double>(node.Mesh.MDXData.Weights.nWeightIndex[3].Valid() ? static_cast<int>(node.Mesh.MDXData.Weights.nWeightIndex[3]) : -1);
            Mdx->WriteFloat(&fIndex, 8, sMdx + "Weight > Index 3");
        }
    }
    Mdx->MarkDataBorder(Mdx->nPosition - 1);

    /// Save the padding size for the next MDX item to use.
    nMdxPrevPadding = (node.Mesh.nMdxDataSize % 16 + nCurrentMdx % 16) % 16;
}

void MDL::WriteNodes(Node & node, std::vector<unsigned> indices, const std::string & sPrefix){
    FileHeader & Data = *FH;
    ReportObject ReportMdl(*this);
    node.nOffset = nPosition - 12;
    indices.push_back(node.Head.nNameIndex);

    /// Setting up the pointer to the data
    node.dataRegions.emplace_back("MDL", nPosition, node.GetSize());

    std::string sNode = sPrefix + std::string(GetNodeName(node).c_str()) + " (" + std::to_string(indices.size() - 1) + ") > ";

    if(Data.MH.bHeadLink && StringEqual(Data.MH.Names.at(node.Head.nNameIndex).sName, "neck_g")){
        unsigned PHnOffsetToHeadRootNode = Data.MH.nOffsetToHeadRootNode;
        Data.MH.nOffsetToHeadRootNode = node.nOffset;
        WriteNumber(&Data.MH.nOffsetToHeadRootNode, 0, "", &PHnOffsetToHeadRootNode);
    }
    WriteNumber(&node.Head.nType, 5, sNode + "Type");
    WriteNumber(&node.Head.nSupernodeNumber, 5, sNode + "Supernode Index");
    WriteNumber(&node.Head.nNameIndex, 5, sNode + "Name Index");
    WriteNumber(&node.Head.nPadding1, 8, sNode + "Padding");

    //Next is offset to root. For geo this is 0, for animations this is offset to animation
    if(!node.nAnimation.Valid()){
        node.Head.nOffsetToRoot = 0;
        WriteNumber(node.Head.nOffsetToRoot.GetPtr(), 6, sNode + "Offset To Root");
    }
    else{
        WriteNumber(&Data.MH.Animations[node.nAnimation].nOffset, 6, sNode + "Offset To Root");
        node.Head.nOffsetToRoot = Data.MH.Animations[node.nAnimation].nOffset;
    }

    //Next is offset to parent.
    if(!node.Head.nParentIndex.Valid()){
        node.Head.nOffsetToParent = 0;
        WriteNumber(node.Head.nOffsetToParent.GetPtr(), 6, sNode + "Offset To Parent");
    }
    else{
        //ReportMdl << "Found parent " << parent.Head.nNameIndex << ". His offset is " << parent.nOffset << ".\n";
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(node.Head.nParentIndex, node.nAnimation);
        if(!nNodeIndex.Valid()) throw mdlexception("converting something to binary error: could not find node with the specified parent name index.");
        std::vector<Node> * p_arrayofnodes = nullptr;
        if(!node.nAnimation.Valid()) p_arrayofnodes = &Data.MH.ArrayOfNodes;
        else p_arrayofnodes = &Data.MH.Animations.at(node.nAnimation).ArrayOfNodes;
        Node &parent = p_arrayofnodes->at(nNodeIndex);
        WriteNumber(&parent.nOffset, 6, sNode + "Offset To Parent");
        node.Head.nOffsetToParent = parent.nOffset;
    }

    //Write position
    WriteFloat(&node.Head.vPos.fX, 2, sNode + "Position > X");
    WriteFloat(&node.Head.vPos.fY, 2, sNode + "Position > Y");
    WriteFloat(&node.Head.vPos.fZ, 2, sNode + "Position > Z");

    //Write orientation - convert first
    WriteFloat(const_cast<double*>(&node.Head.oOrient.GetQuaternion().fW), 2, sNode + "Orientation > W");
    WriteFloat(const_cast<double*>(&node.Head.oOrient.GetQuaternion().vAxis.fX), 2, sNode + "Orientation > X");
    WriteFloat(const_cast<double*>(&node.Head.oOrient.GetQuaternion().vAxis.fY), 2, sNode + "Orientation > Y");
    WriteFloat(const_cast<double*>(&node.Head.oOrient.GetQuaternion().vAxis.fZ), 2, sNode + "Orientation > Z");

    /// Children Array
    unsigned PHnOffsetToChildren = WriteBytes(placeholder, 4, 6, sNode + "Child Array > Offset");
    node.Head.ChildrenArray.ResetToSize(node.Head.ChildIndices.size());
    WriteNumber(&node.Head.ChildrenArray.nCount, 1, sNode + "Child Array > Count 1");
    WriteNumber(&node.Head.ChildrenArray.nCount2, 1, sNode + "Child Array > Count 2");

    /// Controller Array
    unsigned PHnOffsetToControllers = WriteBytes(placeholder, 4, 6, sNode + "Controller Array > Offset");
    node.Head.ControllerArray.ResetToSize(node.Head.Controllers.size());
    WriteNumber(&node.Head.ControllerArray.nCount, 1, sNode + "Controller Array > Count 1");
    WriteNumber(&node.Head.ControllerArray.nCount2, 1, sNode + "Controller Array > Count 2");

    /// Controller Data Array
    unsigned PHnOffsetToControllerData = WriteBytes(placeholder, 4, 6, sNode + "Controller Data Array > Offset");
    node.Head.ControllerDataArray.ResetToSize(node.Head.ControllerData.size());
    WriteNumber(&node.Head.ControllerDataArray.nCount, 1, sNode + "Controller Data Array > Count 1");
    WriteNumber(&node.Head.ControllerDataArray.nCount2, 1, sNode + "Controller Data Array > Count 2");
    MarkDataBorder(nPosition - 1);


    //Here comes all the subheader bullshit.
    /// LIGHT HEADER
    std::string sLight = sNode + "Light > ";
    unsigned PHnOffsetToUnknownLightArray, PHnOffsetToFlareSizes, PHnOffsetToFlarePositions, PHnOffsetToFlareTextureNames, PHnOffsetToFlareColorShifts;
    if(node.Head.nType & NODE_LIGHT){
        WriteFloat(&node.Light.fFlareRadius, 2, sLight + "Flare Radius");
        WriteNumber(&node.Light.UnknownArray.nOffset, 8, sLight + "Unknown Array > Offset");
        WriteNumber(&node.Light.UnknownArray.nCount, 8, sLight + "Unknown Array > Count 1");
        WriteNumber(&node.Light.UnknownArray.nCount2, 8, sLight + "Unknown Array > Count 2");
        PHnOffsetToFlareSizes = WriteBytes(placeholder, 4, 6, sLight + "Flare Sizes > Offset");
        node.Light.FlareSizeArray.ResetToSize(node.Light.FlareSizes.size());
        WriteNumber(&node.Light.FlareSizeArray.nCount, 1, sLight + "Flare Sizes > Count 1");
        WriteNumber(&node.Light.FlareSizeArray.nCount2, 1, sLight + "Flare Sizes > Count 2");
        PHnOffsetToFlarePositions = WriteBytes(placeholder, 4, 6, sLight + "Flare Positions > Offset");
        node.Light.FlarePositionArray.ResetToSize(node.Light.FlarePositions.size());
        WriteNumber(&node.Light.FlarePositionArray.nCount, 1, sLight + "Flare Positions > Count 1");
        WriteNumber(&node.Light.FlarePositionArray.nCount2, 1, sLight + "Flare Positions > Count 2");
        PHnOffsetToFlareColorShifts = WriteBytes(placeholder, 4, 6, sLight + "Flare Color Shifts > Offset");
        node.Light.FlareColorShiftArray.ResetToSize(node.Light.FlareColorShifts.size());
        WriteNumber(&node.Light.FlareColorShiftArray.nCount, 1, sLight + "Flare Color Shifts > Count 1");
        WriteNumber(&node.Light.FlareColorShiftArray.nCount2, 1, sLight + "Flare Color Shifts > Count 2");
        PHnOffsetToFlareTextureNames = WriteBytes(placeholder, 4, 6, sLight + "Flare Texture Names > Offset");
        node.Light.FlareTextureNameArray.ResetToSize(node.Light.FlareTextureNames.size());
        WriteNumber(&node.Light.FlareTextureNameArray.nCount, 1, sLight + "Flare Texture Names > Count 1");
        WriteNumber(&node.Light.FlareTextureNameArray.nCount2, 1, sLight + "Flare Texture Names > Count 2");

        WriteNumber(&node.Light.nLightPriority, 4, sLight + "Light Priority");
        WriteNumber(&node.Light.nAmbientOnly, 4, sLight + "Ambient Only");
        WriteNumber(&node.Light.nDynamicType, 4, sLight + "Dynamic Type");
        WriteNumber(&node.Light.nAffectDynamic, 4, sLight + "Affect Dynamic");
        WriteNumber(&node.Light.nShadow, 4, sLight + "Shadow");
        WriteNumber(&node.Light.nFlare, 4, sLight + "Flare");
        WriteNumber(&node.Light.nFadingLight, 4, sLight + "Fading Light");
        MarkDataBorder(nPosition - 1);
    }

    /// EMITTER HEADER
    std::string sEmitter = sNode + "Emitter > ";
    if(node.Head.nType &NODE_EMITTER){
        WriteFloat(&node.Emitter.fDeadSpace, 2, sEmitter + "Dead Space");
        WriteFloat(&node.Emitter.fBlastRadius, 2, sEmitter + "Blast Radius");
        WriteFloat(&node.Emitter.fBlastLength, 2, sEmitter + "Blast Length");

        WriteNumber(&node.Emitter.nBranchCount, 1, sEmitter + "Branch Count");
        WriteFloat(&node.Emitter.fControlPointSmoothing, 2, sEmitter + "Control Point Smoothing");

        WriteNumber(&node.Emitter.nxGrid, 4, sEmitter + "xGrid");
        WriteNumber(&node.Emitter.nyGrid, 4, sEmitter + "yGrid");
        WriteNumber(&node.Emitter.nSpawnType, 4, sEmitter + "SpawnType");

        node.Emitter.cUpdate.resize(32);
        node.Emitter.cRender.resize(32);
        node.Emitter.cBlend.resize(32);
        node.Emitter.cTexture.resize(32);
        node.Emitter.cChunkName.resize(16);
        WriteString(&node.Emitter.cUpdate, 32, 3, sEmitter + "Update");
        WriteString(&node.Emitter.cRender, 32, 3, sEmitter + "Render");
        WriteString(&node.Emitter.cBlend, 32, 3, sEmitter + "Blend");
        WriteString(&node.Emitter.cTexture, 32, 3, sEmitter + "Texture");
        WriteString(&node.Emitter.cChunkName, 16, 3, sEmitter + "Chunk Name");

        WriteNumber(&node.Emitter.nTwosidedTex, 4, sEmitter + "Twosided Tex");
        WriteNumber(&node.Emitter.nLoop, 4, sEmitter + "Loop");
        WriteNumber(&node.Emitter.nRenderOrder, 5, sEmitter + "Render Order");
        WriteNumber(&node.Emitter.nFrameBlending, 7, sEmitter + "Frame Blending");

        node.Emitter.cDepthTextureName.resize(32);
        WriteString(&node.Emitter.cDepthTextureName, 32, 3, sEmitter + "Depth Texture Name");

        WriteNumber(&node.Emitter.nPadding1, 11, sEmitter + "Padding");
        WriteNumber(&node.Emitter.nFlags, 4, sEmitter + "Flags");
        MarkDataBorder(nPosition - 1);
    }

    /// REFERENCE HEADER
    std::string sReference = sNode + "Reference > ";
    if(node.Head.nType &NODE_REFERENCE){
        node.Reference.sRefModel.resize(32);
        WriteString(&node.Reference.sRefModel, 32, 3, sReference + "Reference Model");
        WriteNumber(&node.Reference.nReattachable, 4, sReference + "Reattachable");
        MarkDataBorder(nPosition - 1);
    }

    /// MESH HEADER
    std::string sMesh = sNode + "Mesh > ";
    unsigned PHnOffsetToFaces, PHnOffsetToIndexCount, PHnOffsetToIndexLocation, PHnOffsetToInvertedCounter, PHnOffsetIntoMdx, PHnOffsetToVertArray;
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
        WriteNumber(&node.Mesh.nFunctionPointer0, 9, sMesh + "Function Pointer 1");
        WriteNumber(&node.Mesh.nFunctionPointer1, 9, sMesh + "Function Pointer 2");
        node.Mesh.FaceArray.ResetToSize(node.Mesh.Faces.size());
        PHnOffsetToFaces = WriteNumber(&node.Mesh.FaceArray.nOffset, 6, sMesh + "Face Array > Offset"); /// This will remain 0 in case there are no faces
        WriteNumber(&node.Mesh.FaceArray.nCount, 1, sMesh + "Face Array > Count 1");
        WriteNumber(&node.Mesh.FaceArray.nCount2, 1, sMesh + "Face Array > Count 2");

        //Tons of floats
        WriteFloat(&node.Mesh.vBBmin.fX, 2, sMesh + "Bounding Box Min > X");
        WriteFloat(&node.Mesh.vBBmin.fY, 2, sMesh + "Bounding Box Min > Y");
        WriteFloat(&node.Mesh.vBBmin.fZ, 2, sMesh + "Bounding Box Min > Z");
        WriteFloat(&node.Mesh.vBBmax.fX, 2, sMesh + "Bounding Box Max > X");
        WriteFloat(&node.Mesh.vBBmax.fY, 2, sMesh + "Bounding Box Max > Y");
        WriteFloat(&node.Mesh.vBBmax.fZ, 2, sMesh + "Bounding Box Max > Z");
        WriteFloat(&node.Mesh.fRadius, 2, sMesh + "Radius");
        WriteFloat(&node.Mesh.vAverage.fX, 2, sMesh + "Average > X");
        WriteFloat(&node.Mesh.vAverage.fY, 2, sMesh + "Average > Y");
        WriteFloat(&node.Mesh.vAverage.fZ, 2, sMesh + "Average > Z");
        WriteFloat(&node.Mesh.fDiffuse.fR, 2, sMesh + "Diffuse > R");
        WriteFloat(&node.Mesh.fDiffuse.fG, 2, sMesh + "Diffuse > G");
        WriteFloat(&node.Mesh.fDiffuse.fB, 2, sMesh + "Diffuse > B");
        WriteFloat(&node.Mesh.fAmbient.fR, 2, sMesh + "Ambient > R");
        WriteFloat(&node.Mesh.fAmbient.fG, 2, sMesh + "Ambient > G");
        WriteFloat(&node.Mesh.fAmbient.fB, 2, sMesh + "Ambient > B");

        WriteNumber(&node.Mesh.nTransparencyHint, 4, sMesh + "Transparency Hint");

        node.Mesh.cTexture1.resize(32);
        node.Mesh.cTexture2.resize(32);
        node.Mesh.cTexture3.resize(12);
        node.Mesh.cTexture4.resize(12);
        WriteString(&node.Mesh.cTexture1, 32, 3, sMesh + "Bitmap 1");
        WriteString(&node.Mesh.cTexture2, 32, 3, sMesh + "Bitmap 2");
        WriteString(&node.Mesh.cTexture3, 12, 3, sMesh + "Texture 0");
        WriteString(&node.Mesh.cTexture4, 12, 3, sMesh + "Texture 1");

        int nSingleSize = (node.Mesh.Faces.size() == 0 || (node.Head.nType &NODE_SABER)) ? 0 : 1;
        node.Mesh.IndexCounterArray.ResetToSize(nSingleSize);
        node.Mesh.IndexLocationArray.ResetToSize(nSingleSize);
        node.Mesh.MeshInvertedCounterArray.ResetToSize(nSingleSize);
        PHnOffsetToIndexCount = WriteBytes(placeholder, 4, 6, sMesh + "Index Count Array > Offset"); /// This must always be updated to where the index count would be if it existed
        WriteNumber(&node.Mesh.IndexCounterArray.nCount, 1, sMesh + "Index Count Array > Count 1");
        WriteNumber(&node.Mesh.IndexCounterArray.nCount2, 1, sMesh + "Index Count Array > Count 2");
        PHnOffsetToIndexLocation = WriteNumber(&node.Mesh.IndexLocationArray.nOffset, 6, sMesh + "Index Loc Array > Offset"); /// This will remain 0 in case there are no faces
        WriteNumber(&node.Mesh.IndexLocationArray.nCount, 1, sMesh + "Index Loc Array > Count 1");
        WriteNumber(&node.Mesh.IndexLocationArray.nCount2, 1, sMesh + "Index Loc Array > Count 2");
        PHnOffsetToInvertedCounter = WriteNumber(&node.Mesh.MeshInvertedCounterArray.nOffset, 6, sMesh + "Inv Counter Array > Offset"); /// This will remain 0 in case there are no faces
        WriteNumber(&node.Mesh.MeshInvertedCounterArray.nCount, 1, sMesh + "Inv Counter Array > Count 1");
        WriteNumber(&node.Mesh.MeshInvertedCounterArray.nCount, 1, sMesh + "Inv Counter Array > Count 2");

        //Write unknown -1 -1 0
        WriteNumber(&node.Mesh.nUnknown3.at(0), 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nUnknown3.at(1), 8, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nUnknown3.at(2), 8, sMesh + "Unknown");

        WriteNumber(&node.Mesh.nSaberUnknown1, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown2, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown3, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown4, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown5, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown6, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown7, 11, sMesh + "Unknown");
        WriteNumber(&node.Mesh.nSaberUnknown8, 11, sMesh + "Unknown");

        //animated UV stuff
        WriteNumber(&node.Mesh.nAnimateUV, 4, sMesh + "Animate UV");
        WriteFloat(&node.Mesh.fUVDirectionX, 2, sMesh + "UV Direction X");
        WriteFloat(&node.Mesh.fUVDirectionY, 2, sMesh + "UV Direction Y");
        WriteFloat(&node.Mesh.fUVJitter, 2, sMesh + "UV Jitter");
        WriteFloat(&node.Mesh.fUVJitterSpeed, 2, sMesh + "UV Jitter Speed");

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
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_COLOR){
            node.Mesh.nOffsetToMdxColor = nMDXsize;
            nMDXsize += 12;
        }
        else node.Mesh.nOffsetToMdxColor = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV1){
            node.Mesh.nOffsetToMdxUV1 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV1 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV2){
            node.Mesh.nOffsetToMdxUV2 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV2 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV3){
            node.Mesh.nOffsetToMdxUV3 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV3 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_UV4){
            node.Mesh.nOffsetToMdxUV4 = nMDXsize;
            nMDXsize += 8;
        }
        else node.Mesh.nOffsetToMdxUV4 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT1){
            node.Mesh.nOffsetToMdxTangent1 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent1 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT2){
            node.Mesh.nOffsetToMdxTangent2 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent2 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT3){
            node.Mesh.nOffsetToMdxTangent3 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent3 = -1;
        if(node.Mesh.nMdxDataBitmap &MDX_FLAG_TANGENT4){
            node.Mesh.nOffsetToMdxTangent4 = nMDXsize;
            if(bXbox) nMDXsize += 12;
            else nMDXsize += 36;
        }
        else node.Mesh.nOffsetToMdxTangent4 = -1;
        if(node.Head.nType &NODE_SKIN){
            node.Skin.nOffsetToMdxWeightValues = nMDXsize;
            nMDXsize += 16;
            node.Skin.nOffsetToMdxBoneIndices = nMDXsize;
            if(bXbox) nMDXsize += 8;
            else nMDXsize += 16;
        }
        if(node.Mesh.Vertices.size() == 0) nMDXsize = -1;
        if(node.Head.nType &NODE_SABER) nMDXsize = 0;
        node.Mesh.nMdxDataSize = nMDXsize;

        WriteNumber(&node.Mesh.nMdxDataSize, 1, sMesh + "MDX Data Size");
        WriteNumber(&node.Mesh.nMdxDataBitmap, 4, sMesh + "MDX Data Bitmap");
        WriteNumber(&node.Mesh.nOffsetToMdxVertex, 6, sMesh + "Offset To MDX Verts");
        WriteNumber(&node.Mesh.nOffsetToMdxNormal, 6, sMesh + "Offset To MDX Normals");
        WriteNumber(&node.Mesh.nOffsetToMdxColor, 6, sMesh + "Offset To MDX Colors");
        WriteNumber(&node.Mesh.nOffsetToMdxUV1, 6, sMesh + "Offset To MDX UVs 1");
        WriteNumber(&node.Mesh.nOffsetToMdxUV2, 6, sMesh + "Offset To MDX UVs 2");
        WriteNumber(&node.Mesh.nOffsetToMdxUV3, 6, sMesh + "Offset To MDX UVs 3");
        WriteNumber(&node.Mesh.nOffsetToMdxUV4, 6, sMesh + "Offset To MDX UVs 4");
        WriteNumber(&node.Mesh.nOffsetToMdxTangent1, 6, sMesh + "Offset To MDX Tangent Space 1");
        WriteNumber(&node.Mesh.nOffsetToMdxTangent2, 6, sMesh + "Offset To MDX Tangent Space 2");
        WriteNumber(&node.Mesh.nOffsetToMdxTangent3, 6, sMesh + "Offset To MDX Tangent Space 3");
        WriteNumber(&node.Mesh.nOffsetToMdxTangent4, 6, sMesh + "Offset To MDX Tangent Space 4");

        node.Mesh.nNumberOfVerts = node.Mesh.Vertices.size();
        WriteNumber(&node.Mesh.nNumberOfVerts, 1, sMesh + "Vert Number");
        WriteNumber(&node.Mesh.nTextureNumber, 1, sMesh + "Texure Number");

        WriteNumber(&node.Mesh.nHasLightmap, 7, sMesh + "Has Lightmap");
        WriteNumber(&node.Mesh.nRotateTexture, 7, sMesh + "Rotate Texture");
        WriteNumber(&node.Mesh.nBackgroundGeometry, 7, sMesh + "Background Geometry");
        WriteNumber(&node.Mesh.nShadow, 7, sMesh + "Shadow");
        WriteNumber(&node.Mesh.nBeaming, 7, sMesh + "Beaming");
        WriteNumber(&node.Mesh.nRender, 7, sMesh + "Render");

        if(bK2) WriteNumber(&node.Mesh.nDirtEnabled, 7, sMesh + "Dirt Enabled");
        if(bK2) WriteNumber(&node.Mesh.nPadding1, 11, sMesh + "Padding");
        if(bK2) WriteNumber(&node.Mesh.nDirtTexture, 5, sMesh + "Dirt Texture");
        if(bK2) WriteNumber(&node.Mesh.nDirtCoordSpace, 5, sMesh + "Dirt Coord Space");
        if(bK2) WriteNumber(&node.Mesh.nHideInHolograms, 7, sMesh + "Hide In Holograms");
        if(bK2) WriteNumber(&node.Mesh.nPadding2, 11, sMesh + "Padding");

        WriteNumber(&node.Mesh.nPadding3, 11, sMesh + "Padding");

        WriteFloat(&node.Mesh.fTotalArea, 2, sMesh + "Total Area");
        WriteNumber(&node.Mesh.nPadding, 8, sMesh + "Padding");

        node.Mesh.nOffsetIntoMdx = 0;
        PHnOffsetIntoMdx = WriteNumber(&node.Mesh.nOffsetIntoMdx, 6, sMesh + "Offset Into MDX"); /// Will stay like this if there are no verts

        /// We moved the mdx writing to a separate function, so we need to preserve this value somehow
        node.Mesh.nOffsetIntoMdx = PHnOffsetIntoMdx;

        if(!bXbox){
            node.Mesh.nOffsetToVertArray = 0;
            PHnOffsetToVertArray = WriteNumber(&node.Mesh.nOffsetToVertArray, 6, sMesh + "Offset To Vert Array"); /// Will stay like this if there are no verts
        }
        MarkDataBorder(nPosition - 1);
    }

    /// SKIN HEADER
    std::string sSkin = sNode + "Skin > ";
    unsigned PHnOffsetToBonemap, PHnOffsetToQBones, PHnOffsetToTBones, PHnOffsetToArray8;
    if(node.Head.nType &NODE_SKIN){
        WriteNumber(&node.Skin.UnknownArray.nOffset, 8, sSkin + "Unknown Array > Offset"); //Unknown int32
        WriteNumber(&node.Skin.UnknownArray.nCount, 8, sSkin + "Unknown Array > Count 1"); //Unknown int32
        WriteNumber(&node.Skin.UnknownArray.nCount2, 8, sSkin + "Unknown Array > Count 2"); //Unknown int32
        WriteNumber(&node.Skin.nOffsetToMdxWeightValues, 6, sSkin + "Offset To MDX Weights");
        WriteNumber(&node.Skin.nOffsetToMdxBoneIndices, 6, sSkin + "Offset To MDX Indices");
        PHnOffsetToBonemap = WriteBytes(placeholder, 4, 6, sSkin + "Bonemap Offset");
        node.Skin.nNumberOfBonemap = node.Skin.Bones.size();
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "Bonemap Count");
        PHnOffsetToQBones = WriteBytes(placeholder, 4, 6, sSkin + "QBone Array > Offset");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "QBone Array > Count 1");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "QBone Array > Count 2");
        PHnOffsetToTBones = WriteBytes(placeholder, 4, 6, sSkin + "TBone Array > Offset");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "TBone Array > Count 1");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "TBone Array > Count 2");
        PHnOffsetToArray8 = WriteBytes(placeholder, 4, 6, sSkin + "Garbage Array > Offset");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "Garbage Array > Count 1");
        WriteNumber(&node.Skin.nNumberOfBonemap, 1, sSkin + "Garbage Array > Count 2");
        for(unsigned a = 0; a < 16; a++){
            WriteNumber(&node.Skin.nBoneIndices[a], 5, sSkin + "Bone Index " + std::to_string(a));
        }
        WriteNumber(&node.Skin.nPadding1, 11, sSkin + "Padding");
        WriteNumber(&node.Skin.nPadding2, 11, sSkin + "Padding");
        MarkDataBorder(nPosition - 1);
    }

    /// DANGLY HEADER
    std::string sDangly = sNode + "Dangly > ";
    unsigned PHnOffsetToConstraints, PHnOffsetToData2;
    if(node.Head.nType & NODE_DANGLY){
        PHnOffsetToConstraints = WriteBytes(placeholder, 4, 6, sDangly + "Constraint Array > Offset");
        node.Dangly.ConstraintArray.ResetToSize(node.Dangly.Constraints.size());
        WriteNumber(&node.Dangly.ConstraintArray.nCount, 1, sDangly + "Constraint Array > Count 1");
        WriteNumber(&node.Dangly.ConstraintArray.nCount2, 1, sDangly + "Constraint Array > Count 2");
        WriteFloat(&node.Dangly.fDisplacement, 2, sDangly + "Displacement");
        WriteFloat(&node.Dangly.fTightness, 2, sDangly + "Tightness");
        WriteFloat(&node.Dangly.fPeriod, 2, sDangly + "Period");
        PHnOffsetToData2 = WriteBytes(placeholder, 4, 6, sDangly + "Offset To Vert Data");
        MarkDataBorder(nPosition - 1);

    }

    /// AABB HEADER
    std::string sAabb = sNode + "AABB > ";
    unsigned PHnOffsetToAabb;
    if(node.Head.nType &NODE_AABB){
        PHnOffsetToAabb = WriteBytes(placeholder, 4, 6, sAabb + "Offset To Root AABB");
        MarkDataBorder(nPosition - 1);
    }

    /// SABER HEADER
    std::string sSaber = "Lightsaber > ";
    unsigned PHnOffsetToSaberVerts, PHnOffsetToSaberUVs, PHnOffsetToSaberNormals;
    if(node.Head.nType & NODE_SABER){
        PHnOffsetToSaberVerts = WriteBytes(placeholder, 4, 6, sSaber + "Offset To Verts");
        PHnOffsetToSaberUVs = WriteBytes(placeholder, 4, 6, sSaber + "Offset To UVs");
        PHnOffsetToSaberNormals = WriteBytes(placeholder, 4, 6, sSaber + "Offset To Normals");
        WriteNumber(&node.Saber.nInvCount1, 4, sSaber + "Inverted Counter 1");
        WriteNumber(&node.Saber.nInvCount2, 4, sSaber + "Inverted Counter 2");
        MarkDataBorder(nPosition - 1);
    }

    //now comes all the data in reversed order
    /// SABER DATA
    if(node.Head.nType & NODE_SABER){
        node.Saber.nOffsetToSaberVerts = nPosition - 12;
        WriteNumber(&node.Saber.nOffsetToSaberVerts, 0, "", &PHnOffsetToSaberVerts);
        for(unsigned d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(&node.Saber.SaberData[d].vVertex.fX, 2, sSaber + "Vertex " + std::to_string(d) + " > X");
            WriteFloat(&node.Saber.SaberData[d].vVertex.fY, 2, sSaber + "Vertex " + std::to_string(d) + " > Y");
            WriteFloat(&node.Saber.SaberData[d].vVertex.fZ, 2, sSaber + "Vertex " + std::to_string(d) + " > Z");
            MarkDataBorder(nPosition - 1);
        }
        node.Saber.nOffsetToSaberNormals = nPosition - 12;
        WriteNumber(&node.Saber.nOffsetToSaberNormals, 0, "", &PHnOffsetToSaberNormals);
        for(unsigned d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(&node.Saber.SaberData[d].vNormal.fX, 2, sSaber + "Normal " + std::to_string(d) + " > X");
            WriteFloat(&node.Saber.SaberData[d].vNormal.fY, 2, sSaber + "Normal " + std::to_string(d) + " > Y");
            WriteFloat(&node.Saber.SaberData[d].vNormal.fZ, 2, sSaber + "Normal " + std::to_string(d) + " > Z");
            MarkDataBorder(nPosition - 1);
        }
        node.Saber.nOffsetToSaberUVs = nPosition - 12;
        WriteNumber(&node.Saber.nOffsetToSaberUVs, 0, "", &PHnOffsetToSaberUVs);
        for(unsigned d = 0; d < node.Saber.SaberData.size(); d++){
            WriteFloat(&node.Saber.SaberData[d].vUV1.fX, 2, sSaber + "UV " + std::to_string(d) + " > U");
            WriteFloat(&node.Saber.SaberData[d].vUV1.fY, 2, sSaber + "UV " + std::to_string(d) + " > U");
            MarkDataBorder(nPosition - 1);
        }

    }

    /// AABB DATA
    if(node.Head.nType & NODE_AABB){
        node.Walkmesh.nOffsetToAabb = nPosition - 12;
        WriteNumber(&node.Walkmesh.nOffsetToAabb, 0, "", &PHnOffsetToAabb);
        nAabbCount = 0;
        WriteAabb(node.Walkmesh.RootAabb, sNode);
        MarkDataBorder(nPosition - 1);
    }

    /// DANGLY DATA
    if(node.Head.nType & NODE_DANGLY){
        node.Dangly.ConstraintArray.ResetToSize(node.Dangly.Constraints.size());
        if(node.Dangly.Constraints.size() > 0){
            node.Dangly.ConstraintArray.nOffset = nPosition - 12;
            WriteNumber(&node.Dangly.ConstraintArray.nOffset, 0, "", &PHnOffsetToConstraints);
        }
        else{
            node.Dangly.ConstraintArray.nOffset = 0;
            WriteNumber(&node.Dangly.ConstraintArray.nOffset, 0, "", &PHnOffsetToConstraints);
        }
        for(unsigned d = 0; d < node.Dangly.Constraints.size(); d++){
            WriteNumber(&node.Dangly.Constraints.at(d), 2, sDangly + "Constraint " + std::to_string(d));
            MarkDataBorder(nPosition - 1);
        }
        node.Dangly.nOffsetToData2 = nPosition - 12;
        WriteNumber(&node.Dangly.nOffsetToData2, 0, "", &PHnOffsetToData2);
        for(unsigned d = 0; d < node.Dangly.Data2.size(); d++){
            WriteFloat(&node.Dangly.Data2.at(d).fX, 2, sDangly + "Vertex " + std::to_string(d) + " > X");
            WriteFloat(&node.Dangly.Data2.at(d).fY, 2, sDangly + "Vertex " + std::to_string(d) + " > Y");
            WriteFloat(&node.Dangly.Data2.at(d).fZ, 2, sDangly + "Vertex " + std::to_string(d) + " > Z");
            MarkDataBorder(nPosition - 1);
        }
    }

    /// SKIN DATA
    if(node.Head.nType &NODE_SKIN){
        node.Skin.nNumberOfBonemap = node.Skin.Bones.size();
        if(node.Skin.Bones.size() > 0){
            node.Skin.nOffsetToBonemap = nPosition - 12;
            WriteNumber(&node.Skin.nOffsetToBonemap, 0, "", &PHnOffsetToBonemap);
        }
        else{
            node.Skin.nOffsetToBonemap = 0;
            WriteNumber(&node.Skin.nOffsetToBonemap, 0, "", &PHnOffsetToBonemap);
        }
        for(unsigned d = 0; d < node.Skin.Bones.size(); d++){
            if(bXbox) WriteNumber(node.Skin.Bones.at(d).nBonemap.GetPtr(), 5, sSkin + "Bonemap " + std::to_string(d));
            else{
                double fHelper = static_cast<double>(node.Skin.Bones.at(d).nBonemap.Valid() ? static_cast<int>(node.Skin.Bones.at(d).nBonemap) : -1);
                WriteFloat(&fHelper, 2, sSkin + "Bonemap " + std::to_string(d));
            }
            MarkDataBorder(nPosition - 1);
        }
        node.Skin.QBoneArray.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0){
            node.Skin.QBoneArray.nOffset = nPosition - 12;
            WriteNumber(&node.Skin.QBoneArray.nOffset, 0, "", &PHnOffsetToQBones);
        }
        else{
            node.Skin.QBoneArray.nOffset = 0;
            WriteNumber(&node.Skin.QBoneArray.nOffset, 0, "", &PHnOffsetToQBones);
        }
        for(unsigned d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).QBone.GetQuaternion().fW), 2, sSkin + "QBone " + std::to_string(d) + " > W");
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fX), 2, sSkin + "QBone " + std::to_string(d) + " > X");
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fY), 2, sSkin + "QBone " + std::to_string(d) + " > Y");
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).QBone.GetQuaternion().vAxis.fZ), 2, sSkin + "QBone " + std::to_string(d) + " > Z");
            MarkDataBorder(nPosition - 1);
        }
        node.Skin.TBoneArray.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0){
            node.Skin.TBoneArray.nOffset = nPosition - 12;
            WriteNumber(&node.Skin.TBoneArray.nOffset, 0, "", &PHnOffsetToTBones);
        }
        else{
            node.Skin.TBoneArray.nOffset = 0;
            WriteNumber(&node.Skin.TBoneArray.nOffset, 0, "", &PHnOffsetToTBones);
        }
        for(unsigned d = 0; d < node.Skin.Bones.size(); d++){
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).TBone.fX), 2, sSkin + "TBone " + std::to_string(d) + " > X");
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).TBone.fY), 2, sSkin + "TBone " + std::to_string(d) + " > Y");
            WriteFloat(const_cast<double*>(&node.Skin.Bones.at(d).TBone.fZ), 2, sSkin + "TBone " + std::to_string(d) + " > Z");
            MarkDataBorder(nPosition - 1);
        }
        node.Skin.Array8Array.ResetToSize(node.Skin.Bones.size());
        if(node.Skin.Bones.size() > 0){
            node.Skin.Array8Array.nOffset = nPosition - 12;
            WriteNumber(&node.Skin.Array8Array.nOffset, 0, "", &PHnOffsetToArray8);
        }
        else{
            node.Skin.Array8Array.nOffset = 0;
            WriteNumber(&node.Skin.Array8Array.nOffset, 0, "", &PHnOffsetToArray8);
        }
        for(unsigned d = 0; d < node.Skin.Bones.size(); d++){
            WriteNumber(&node.Skin.Bones.at(d).nPadding, 11, sSkin + "Garbage " + std::to_string(d)); //This array is not in use, might as well fill it with zeros
        }
        MarkDataBorder(nPosition - 1);
    }

    /// MESH DATA
    if(node.Head.nType &NODE_MESH){

        /// Write faces
        if(node.Mesh.Faces.size() > 0){
            node.Mesh.FaceArray.nOffset = nPosition - 12;
            WriteNumber(&node.Mesh.FaceArray.nOffset, 0, "", &PHnOffsetToFaces);
        }
        else{
            node.Mesh.FaceArray.nOffset = 0;
            WriteNumber(&node.Mesh.FaceArray.nOffset, 0, "", &PHnOffsetToFaces);
        }
        for(unsigned d = 0; d < node.Mesh.Faces.size(); d++){
            Face & face = node.Mesh.Faces.at(d);

            /// Setting up the pointer to the data
            face.dataRegions.emplace_back("MDL", nPosition, 32);

            WriteFloat(&face.vNormal.fX, 2, sMesh + "Face " + std::to_string(d) + " > Normal > X");
            WriteFloat(&face.vNormal.fY, 2, sMesh + "Face " + std::to_string(d) + " > Normal > Y");
            WriteFloat(&face.vNormal.fZ, 2, sMesh + "Face " + std::to_string(d) + " > Normal > Z");
            WriteFloat(&face.fDistance, 2, sMesh + "Face " + std::to_string(d) + " > Plane Distance");
            WriteNumber(&face.nMaterialID, 4, sMesh + "Face " + std::to_string(d) + " > Material ID");
            WriteNumber(face.nAdjacentFaces[0].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Adjacent Face 0");
            WriteNumber(face.nAdjacentFaces[1].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Adjacent Face 0");
            WriteNumber(face.nAdjacentFaces[2].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Adjacent Face 0");
            WriteNumber(face.nIndexVertex[0].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 0");
            WriteNumber(face.nIndexVertex[1].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 0");
            WriteNumber(face.nIndexVertex[2].GetPtr(), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 0");
            MarkDataBorder(nPosition - 1);
        }

        /// If SG recording is enabled, write SGs
        if(bWriteSmoothing){
            for(unsigned d = 0; d < node.Mesh.Faces.size(); d++){
                Face & face = node.Mesh.Faces.at(d);
                WriteNumber(&face.nSmoothingGroup, 4, sMesh + "Face " + std::to_string(d) + "Smoothing Groups");
                MarkDataBorder(nPosition - 1);
            }
        }

        if(node.Head.nType & NODE_SABER){
            node.Mesh.IndexCounterArray.ResetToSize(0);
            //WriteIntToPH(0, PHnOffsetToIndexCount, node.Mesh.IndexCounterArray.nOffset);
            node.Mesh.IndexCounterArray.nOffset = nPosition - 12;
            WriteNumber(&node.Mesh.IndexCounterArray.nOffset, 0, "", &PHnOffsetToIndexCount);

            /// This should possibly be deleted for the saber
            if(!bXbox){
                 node.Mesh.nOffsetToVertArray = nPosition - 12;
                WriteNumber(&node.Mesh.nOffsetToVertArray, 0, "", &PHnOffsetToVertArray);
                for(unsigned d = 0; d < node.Mesh.Vertices.size(); d++){
                    WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fX), 2, sMesh + "Vertex " + std::to_string(d) + " > X");
                    WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fY), 2, sMesh + "Vertex " + std::to_string(d) + " > Y");
                    WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fZ), 2, sMesh + "Vertex " + std::to_string(d) + " > Z");
                    MarkDataBorder(nPosition - 1);
                }
            }

            node.Mesh.IndexLocationArray.ResetToSize(0);
            //WriteIntToPH(0, PHnOffsetToIndexLocation, node.Mesh.IndexLocationArray.nOffset);
            node.Mesh.IndexLocationArray.nOffset = nPosition - 12;
            WriteNumber(&node.Mesh.IndexLocationArray.nOffset, 0, "", &PHnOffsetToIndexLocation);

            node.Mesh.MeshInvertedCounterArray.ResetToSize(0);
            //WriteIntToPH(0, PHnOffsetToInvertedCounter, node.Mesh.MeshInvertedCounterArray.nOffset);
            node.Mesh.MeshInvertedCounterArray.nOffset = nPosition - 12;
            WriteNumber(&node.Mesh.MeshInvertedCounterArray.nOffset, 0, "", &PHnOffsetToInvertedCounter);
            //WriteIntToPH(0, PHnPointerToIndexLocation, node.Mesh.nVertIndicesLocation);
        }
        else{
            /// The index counter offset is always set
            node.Mesh.IndexCounterArray.nOffset = nPosition - 12;
            WriteNumber(&node.Mesh.IndexCounterArray.nOffset, 0, "", &PHnOffsetToIndexCount);

            node.Mesh.nVertIndicesCount = node.Mesh.Faces.size() * 3;
            if(node.Mesh.Faces.size() > 0){
                WriteNumber(&node.Mesh.nVertIndicesCount, 1, sMesh + "Vert Indices Count");
                MarkDataBorder(nPosition - 1);

                if(!bXbox){
                    node.Mesh.nOffsetToVertArray = nPosition - 12;
                    WriteNumber(&node.Mesh.nOffsetToVertArray, 0, "", &PHnOffsetToVertArray);
                    for(unsigned d = 0; d < node.Mesh.Vertices.size(); d++){
                        WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fX), 2, sMesh + "Vertex " + std::to_string(d) + " > X");
                        WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fY), 2, sMesh + "Vertex " + std::to_string(d) + " > Y");
                        WriteFloat(const_cast<double*>(&node.Mesh.Vertices.at(d).fZ), 2, sMesh + "Vertex " + std::to_string(d) + " > Z");
                        MarkDataBorder(nPosition - 1);
                    }
                }

                node.Mesh.IndexLocationArray.ResetToSize(1);
                node.Mesh.IndexLocationArray.nOffset = nPosition - 12;
                WriteNumber(&node.Mesh.IndexLocationArray.nOffset, 0, "", &PHnOffsetToIndexLocation);

                unsigned PHnPointerToIndexLocation = WriteBytes(placeholder, 4, 6, sMesh + "Vert Indices Loc");
                MarkDataBorder(nPosition - 1);

                node.Mesh.MeshInvertedCounterArray.ResetToSize(1);
                node.Mesh.MeshInvertedCounterArray.nOffset = nPosition - 12;
                WriteNumber(&node.Mesh.MeshInvertedCounterArray.nOffset, 0, "", &PHnOffsetToInvertedCounter);
                WriteNumber(&node.Mesh.nMeshInvertedCounter, 4, sMesh + "Inverted Counter");
                MarkDataBorder(nPosition - 1);

                node.Mesh.nVertIndicesLocation = nPosition - 12;
                WriteNumber(&node.Mesh.nVertIndicesLocation, 0, "", &PHnPointerToIndexLocation);
                for(unsigned d = 0; d < node.Mesh.VertIndices.size(); d++){
                    WriteNumber(&node.Mesh.VertIndices.at(d).at(0), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 0");
                    WriteNumber(&node.Mesh.VertIndices.at(d).at(1), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 1");
                    WriteNumber(&node.Mesh.VertIndices.at(d).at(2), 5, sMesh + "Face " + std::to_string(d) + " > Vert Index 2");
                    MarkDataBorder(nPosition - 1);
                }
            }
        }
    }

    /// LIGHT DATA
    if(node.Head.nType &NODE_LIGHT){
        //WriteIntToPH(nPosition - 12, PHnOffsetToUnknownLightArray, node.Light.UnknownArray.nOffset);

        node.Light.FlareTextureNameArray.ResetToSize(node.Light.FlareTextureNames.size());
        if(node.Light.FlareTextureNames.size() > 0){
            node.Light.FlareTextureNameArray.nOffset = nPosition - 12;
            WriteNumber(&node.Light.FlareTextureNameArray.nOffset, 0, "", &PHnOffsetToFlareTextureNames);
        }
        else{
            node.Light.FlareTextureNameArray.nOffset = 0;
            WriteNumber(&node.Light.FlareTextureNameArray.nOffset, 0, "", &PHnOffsetToFlareTextureNames);
        }

        std::vector<unsigned> PHnTextureNameOffset;
        for(unsigned d = 0; d < node.Light.FlareTextureNames.size(); d++){
            PHnTextureNameOffset.push_back(WriteBytes(placeholder, 4, 6, sLight + "Flare Texture Name Offset " + std::to_string(d)));
            MarkDataBorder(nPosition - 1);
        }
        for(unsigned d = 0; d < node.Light.FlareTextureNames.size(); d++){
            node.Light.FlareTextureNames.at(d).nOffset = nPosition - 12;
            WriteNumber(&node.Light.FlareTextureNames.at(d).nOffset, 0, "", &PHnTextureNameOffset.at(d));
            WriteString(&node.Light.FlareTextureNames.at(d).sName, 0, 3, sLight + "Flare Texture Name String " + std::to_string(d));
            MarkDataBorder(nPosition - 1);
        }

        if(node.Light.FlareSizes.size() > 0){
            node.Light.FlareSizeArray.nOffset = nPosition - 12;
            WriteNumber(&node.Light.FlareSizeArray.nOffset, 0, "", &PHnOffsetToFlareSizes);
        }
        else{
            node.Light.FlareSizeArray.nOffset = 0;
            WriteNumber(&node.Light.FlareSizeArray.nOffset, 0, "", &PHnOffsetToFlareSizes);
        }

        for(unsigned d = 0; d < node.Light.FlareSizes.size(); d++){
            WriteFloat(&node.Light.FlareSizes.at(d), 2, sLight + "Flare Size " + std::to_string(d));
            MarkDataBorder(nPosition - 1);
        }

        if(node.Light.FlarePositions.size() > 0){
            node.Light.FlarePositionArray.nOffset = nPosition - 12;
            WriteNumber(&node.Light.FlarePositionArray.nOffset, 0, "", &PHnOffsetToFlarePositions);
        }
        else{
            node.Light.FlarePositionArray.nOffset = 0;
            WriteNumber(&node.Light.FlarePositionArray.nOffset, 0, "", &PHnOffsetToFlarePositions);
        }

        for(unsigned d = 0; d < node.Light.FlarePositions.size(); d++){
            WriteFloat(&node.Light.FlarePositions.at(d), 2, sLight + "Flare Position " + std::to_string(d));
            MarkDataBorder(nPosition - 1);
        }

        if(node.Light.FlareColorShifts.size() > 0){
            node.Light.FlareColorShiftArray.nOffset = nPosition - 12;
            WriteNumber(&node.Light.FlareColorShiftArray.nOffset, 0, "", &PHnOffsetToFlareColorShifts);
        }
        else{
            node.Light.FlareColorShiftArray.nOffset = 0;
            WriteNumber(&node.Light.FlareColorShiftArray.nOffset, 0, "", &PHnOffsetToFlareColorShifts);
        }

        for(unsigned d = 0; d < node.Light.FlareColorShifts.size(); d++){
            WriteFloat(&node.Light.FlareColorShifts.at(d).fR, 2, sLight + "Flare Color Shift " + std::to_string(d) + " > R");
            WriteFloat(&node.Light.FlareColorShifts.at(d).fG, 2, sLight + "Flare Color Shift " + std::to_string(d) + " > G");
            WriteFloat(&node.Light.FlareColorShifts.at(d).fB, 2, sLight + "Flare Color Shift " + std::to_string(d) + " > B");
            MarkDataBorder(nPosition - 1);
        }
    }
    /// DONE WITH SUBHEADERS

    //We get to the children array
    node.Head.ChildrenArray.ResetToSize(node.Head.ChildIndices.size());
    node.Head.ChildrenArray.nOffset = nPosition - 12;
    WriteNumber(&node.Head.ChildrenArray.nOffset, 0, "", &PHnOffsetToChildren);

    std::vector<unsigned> PHnOffsetToChild;
    for(unsigned a = 0; a < node.Head.ChildIndices.size(); a++){
        PHnOffsetToChild.push_back(WriteBytes(placeholder, 4, 6, sNode + "Child Offset" + std::to_string(a)));
        MarkDataBorder(nPosition - 1);
    }

    for(unsigned a = 0; a < node.Head.ChildIndices.size(); a++){
        MdlInteger<unsigned short> nNodeIndex = GetNodeIndexByNameIndex(node.Head.ChildIndices.at(a), node.nAnimation);
        if(!nNodeIndex.Valid()) throw mdlexception("To binary exception: Couldn't find child node index by name index " + std::to_string(node.Head.ChildIndices.at(a)) + " in animation " + std::to_string(node.nAnimation) + ", node " + GetNodeName(node) + ".");
        Node * p_node = nullptr;
        if(!node.nAnimation.Valid()) p_node = &Data.MH.ArrayOfNodes.at(nNodeIndex);
        else p_node = &Data.MH.Animations.at(node.nAnimation).ArrayOfNodes.at(nNodeIndex);
        if(std::find(indices.begin(), indices.end(), p_node->Head.nNameIndex) == indices.end()){
            p_node->nOffset = nPosition - 12;
            WriteNumber(&p_node->nOffset, 0, "", &PHnOffsetToChild.at(a));
            WriteNodes(*p_node, indices, sPrefix);
            //WriteNodes(node.Head.Children.at(a));
        }
        else{
             p_node->nOffset = p_node->nOffset;
            WriteNumber(&p_node->nOffset, 0, "", &PHnOffsetToChild.at(a));
        }
    }

    /// We get to the controller array
    node.Head.ControllerArray.ResetToSize(node.Head.Controllers.size());
    if(node.Head.Controllers.size() > 0){
        node.Head.ControllerArray.nOffset = nPosition-12;
        WriteNumber(&node.Head.ControllerArray.nOffset, 0, "", &PHnOffsetToControllers);
    }
    else{
        node.Head.ControllerArray.nOffset = 0;
        WriteNumber(&node.Head.ControllerArray.nOffset, 0, "", &PHnOffsetToControllers);
    }

    for(unsigned a = 0; a < node.Head.Controllers.size(); a++){
        Controller &ctrl = node.Head.Controllers.at(a);
        std::string sController = sNode + "Controller " + std::to_string(a) + " > ";
        WriteNumber(&ctrl.nControllerType, 4, sController + "Type");
        WriteNumber(&ctrl.nUnknown2, 10, sController + "Unknown");
        WriteNumber(&ctrl.nValueCount, 5, sController + "Value Count");
        WriteNumber(&ctrl.nTimekeyStart, 5, sController + "Timekey Start");
        WriteNumber(&ctrl.nDataStart, 5, sController + "Data Start");
        WriteNumber(&ctrl.nColumnCount, 7, sController + "Column Count");
        WriteNumber(&ctrl.nPadding[0], 11, sController + "Padding");
        WriteNumber(&ctrl.nPadding[1], 11, sController + "Padding");
        WriteNumber(&ctrl.nPadding[2], 11, sController + "Padding");
        MarkDataBorder(nPosition - 1);
    }

    /// We get to the controller data array
    node.Head.ControllerDataArray.ResetToSize(node.Head.ControllerData.size());
    if(node.Head.ControllerData.size() > 0){
        node.Head.ControllerDataArray.nOffset = nPosition - 12;
        WriteNumber(&node.Head.ControllerDataArray.nOffset, 0, "", &PHnOffsetToControllerData);
    }
    else{
        node.Head.ControllerDataArray.nOffset = 0;
        WriteNumber(&node.Head.ControllerDataArray.nOffset, 0, "", &PHnOffsetToControllerData);
    }

    for(unsigned a = 0; a < node.Head.ControllerData.size(); a++){
        WriteFloat(&node.Head.ControllerData.at(a), 2, sNode + "Controller Data " + std::to_string(a));
        MarkDataBorder(nPosition - 1);
    }

    /// Done you are, padawan
}

void BWM::Compile(){
    if(!Bwm) return;
    BWMHeader &data = *Bwm;
    nPosition = 0;
    sBuffer.resize(0);
    bKnown.resize(0);
    positions.clear();
    std::string sHeader ("Header > ");

    std::string sFileType = "BWM ";
    std::string sVersion = "V1.0";
    WriteString(&sFileType, 4, 3, sHeader + "File Type");
    WriteString(&sVersion, 4, 3, sHeader + "Version");
    WriteNumber(&data.nType, 4, sHeader + "Type");

    WriteFloat(&data.vUse1.fX, 2, sHeader + "Relative Use Vector 1 > X");
    WriteFloat(&data.vUse1.fY, 2, sHeader + "Relative Use Vector 1 > Y");
    WriteFloat(&data.vUse1.fZ, 2, sHeader + "Relative Use Vector 1 > Z");
    WriteFloat(&data.vUse2.fX, 2, sHeader + "Relative Use Vector 2 > X");
    WriteFloat(&data.vUse2.fY, 2, sHeader + "Relative Use Vector 2 > Y");
    WriteFloat(&data.vUse2.fZ, 2, sHeader + "Relative Use Vector 2 > Z");
    WriteFloat(&data.vDwk1.fX, 2, sHeader + "Absolute Use Vector 1 > X");
    WriteFloat(&data.vDwk1.fY, 2, sHeader + "Absolute Use Vector 1 > Y");
    WriteFloat(&data.vDwk1.fZ, 2, sHeader + "Absolute Use Vector 1 > Z");
    WriteFloat(&data.vDwk2.fX, 2, sHeader + "Absolute Use Vector 2 > X");
    WriteFloat(&data.vDwk2.fY, 2, sHeader + "Absolute Use Vector 2 > Y");
    WriteFloat(&data.vDwk2.fZ, 2, sHeader + "Absolute Use Vector 2 > Z");

    WriteFloat(&data.vPosition.fX, 2, sHeader + "Position > X");
    WriteFloat(&data.vPosition.fY, 2, sHeader + "Position > Y");
    WriteFloat(&data.vPosition.fZ, 2, sHeader + "Position > Z");

    data.nNumberOfVerts = data.verts.size();
    WriteNumber(&data.nNumberOfVerts, 1, sHeader + "Vert Count");
    unsigned nOffsetVerts = WriteBytes(placeholder, 4, 6, sHeader + "Vert Offset");

    data.nNumberOfFaces = data.faces.size();
    WriteNumber(&data.nNumberOfFaces, 1, sHeader + "Face Count");
    unsigned nOffsetVertIndices = WriteBytes(placeholder, 4, 6, sHeader + "Vert Indices Offset");
    unsigned nOffsetMaterials = WriteBytes(placeholder, 4, 6, sHeader + "Material IDs Offset");
    unsigned nOffsetNormals = WriteBytes(placeholder, 4, 6, sHeader + "Normals Offset");
    unsigned nOffsetDistances = WriteBytes(placeholder, 4, 6, sHeader + "Distances Offset");

    data.nNumberOfAabb = data.aabb.size();
    WriteNumber(&data.nNumberOfAabb, 1, sHeader + "AABB Count");
    unsigned nOffsetAabb = WriteBytes(placeholder, 4, 6, sHeader + "AABB Offset");

    WriteNumber(&data.nPadding, 11, sHeader + "Unknown");

    unsigned nCount = 0;
    for(unsigned f = 0; f < data.faces.size(); f++){
        if(IsMaterialWalkable(data.faces.at(f).nMaterialID)) nCount++;
    }
    data.nNumberOfAdjacentFaces = nCount;
    WriteNumber(&data.nNumberOfAdjacentFaces, 1, sHeader + "Adjacent Edges Count");
    unsigned nOffsetAdjacent = WriteBytes(placeholder, 4, 6, sHeader + "Adjacent Edges Offset");

    data.nNumberOfEdges = data.edges.size();
    WriteNumber(&data.nNumberOfEdges, 1, sHeader + "Outer Edges Count");
    unsigned nOffsetEdges = WriteBytes(placeholder, 4, 6, sHeader + "Outer Edges Offset");

    data.nNumberOfPerimeters = data.perimeters.size();
    WriteNumber(&data.nNumberOfPerimeters, 1, sHeader + "Perimeters Count");
    unsigned nOffsetPerimeters = WriteBytes(placeholder, 4, 6, sHeader + "Perimeters Offset");
    MarkDataBorder(nPosition - 1);

    if(data.verts.size() == 0){
        data.nOffsetToVerts = 0;
        WriteNumber(&data.nOffsetToVerts, 0, "", &nOffsetVerts);
    }
    else{
        data.nOffsetToVerts = nPosition;
        WriteNumber(&data.nOffsetToVerts, 0, "", &nOffsetVerts);
    }
    for(unsigned v = 0; v < data.verts.size(); v++){
        Vector &vert = data.verts.at(v);
        WriteFloat(&vert.fX, 2, "Vertex " + std::to_string(v) + " > X");
        WriteFloat(&vert.fY, 2, "Vertex " + std::to_string(v) + " > Y");
        WriteFloat(&vert.fZ, 2, "Vertex " + std::to_string(v) + " > Z");
        MarkDataBorder(nPosition - 1);
    }

    if(data.faces.size() == 0){
        data.nOffsetToIndices = 0;
        WriteNumber(&data.nOffsetToIndices, 0, "", &nOffsetVertIndices);
    }
    else{
        data.nOffsetToIndices = nPosition;
        WriteNumber(&data.nOffsetToIndices, 0, "", &nOffsetVertIndices);
    }
    for(unsigned f = 0; f < data.faces.size(); f++){
        Face & face = data.faces.at(f);

        int nTempIndex = face.nIndexVertex.at(0).Valid() ? static_cast<int>(face.nIndexVertex.at(0)) : -1;
        WriteNumber(&nTempIndex, 4, "Face " + std::to_string(f) + " > Vert Index 0");
        nTempIndex = face.nIndexVertex.at(1).Valid() ? static_cast<int>(face.nIndexVertex.at(1)) : -1;
        WriteNumber(&nTempIndex, 4, "Face " + std::to_string(f) + " > Vert Index 1");
        nTempIndex = face.nIndexVertex.at(2).Valid() ? static_cast<int>(face.nIndexVertex.at(2)) : -1;
        WriteNumber(&nTempIndex, 4, "Face " + std::to_string(f) + " > Vert Index 2");

        MarkDataBorder(nPosition - 1);
    }
    if(data.faces.size() == 0){
        data.nOffsetToMaterials = 0;
        WriteNumber(&data.nOffsetToMaterials, 0, "", &nOffsetMaterials);
    }
    else{
        data.nOffsetToMaterials = nPosition;
        WriteNumber(&data.nOffsetToMaterials, 0, "", &nOffsetMaterials);
    }
    for(unsigned f = 0; f < data.faces.size(); f++){
        Face &face = data.faces.at(f);
        WriteNumber(&face.nMaterialID, 4, "Face " + std::to_string(f) + " > Material ID");
        MarkDataBorder(nPosition - 1);
    }
    if(data.faces.size() == 0){
        data.nOffsetToNormals = 0;
        WriteNumber(&data.nOffsetToNormals, 0, "", &nOffsetNormals);
    }
    else{
        data.nOffsetToNormals = nPosition;
        WriteNumber(&data.nOffsetToNormals, 0, "", &nOffsetNormals);
    }
    for(unsigned f = 0; f < data.faces.size(); f++){
        Face &face = data.faces.at(f);
        WriteFloat(&face.vNormal.fX, 2, "Face " + std::to_string(f) + " > Normal > X");
        WriteFloat(&face.vNormal.fY, 2, "Face " + std::to_string(f) + " > Normal > Y");
        WriteFloat(&face.vNormal.fZ, 2, "Face " + std::to_string(f) + " > Normal > Z");
        MarkDataBorder(nPosition - 1);
    }
    if(data.faces.size() == 0){
        data.nOffsetToDistances = 0;
        WriteNumber(&data.nOffsetToDistances, 0, "", &nOffsetDistances);
    }
    else{
        data.nOffsetToDistances = nPosition;
        WriteNumber(&data.nOffsetToDistances, 0, "", &nOffsetDistances);
    }
    for(unsigned f = 0; f < data.faces.size(); f++){
        Face &face = data.faces.at(f);
        WriteFloat(&face.fDistance, 2, "Face " + std::to_string(f) + " > Plane Distance");
        MarkDataBorder(nPosition - 1);
    }

    if(data.aabb.size() == 0){
        data.nOffsetToAabb = 0;
        WriteNumber(&data.nOffsetToAabb, 0, "", &nOffsetAabb);
    }
    else{
        data.nOffsetToAabb = nPosition;
        WriteNumber(&data.nOffsetToAabb, 0, "", &nOffsetAabb);
    }
    for(unsigned v = 0; v < data.aabb.size(); v++){
        Aabb &aabb = data.aabb.at(v);
        WriteFloat(&aabb.vBBmin.fX, 2, "AABB " + std::to_string(v) + " > Bounding Box Min > X");
        WriteFloat(&aabb.vBBmin.fY, 2, "AABB " + std::to_string(v) + " > Bounding Box Min > Y");
        WriteFloat(&aabb.vBBmin.fZ, 2, "AABB " + std::to_string(v) + " > Bounding Box Min > Z");
        WriteFloat(&aabb.vBBmax.fX, 2, "AABB " + std::to_string(v) + " > Bounding Box Max > X");
        WriteFloat(&aabb.vBBmax.fY, 2, "AABB " + std::to_string(v) + " > Bounding Box Max > Y");
        WriteFloat(&aabb.vBBmax.fZ, 2, "AABB " + std::to_string(v) + " > Bounding Box Max > Z");
        int nTempIndex = aabb.nID.Valid() ? static_cast<signed int>(aabb.nID) : -1;
        WriteNumber(&nTempIndex, 4, "AABB " + std::to_string(v) + " > Face Index");
        WriteNumber(&aabb.nExtra, 4, "AABB " + std::to_string(v) + " > Unknown");
        WriteNumber(&aabb.nProperty, 4, "AABB " + std::to_string(v) + " > Second Child Property");
        WriteNumber(&aabb.nChild1, 4, "AABB " + std::to_string(v) + " > Child 1 Index");
        WriteNumber(&aabb.nChild2, 4, "AABB " + std::to_string(v) + " > Child 1 Index");
        MarkDataBorder(nPosition - 1);
    }

    if(data.faces.size() == 0 || data.nType == 0){
        data.nOffsetToAdjacentFaces = 0;
        WriteNumber(&data.nOffsetToAdjacentFaces, 0, "", &nOffsetAdjacent);
    }
    else{
        data.nOffsetToAdjacentFaces = nPosition;
        WriteNumber(&data.nOffsetToAdjacentFaces, 0, "", &nOffsetAdjacent);
    }
    for(int f = 0; f < data.faces.size() &&data.nType == 1; f++){
        Face &face = data.faces.at(f);
        if(IsMaterialWalkable(face.nMaterialID)){
            int nTempIndex = face.nAdjacentFaces.at(0).Valid() ? static_cast<int>(face.nAdjacentFaces.at(0)) : -1;
            WriteNumber(&nTempIndex, 4, "Walkable Face " + std::to_string(f) + " > Adjacent Edge 0");
            nTempIndex = face.nAdjacentFaces.at(1).Valid() ? static_cast<int>(face.nAdjacentFaces.at(1)) : -1;
            WriteNumber(&nTempIndex, 4, "Walkable Face " + std::to_string(f) + " > Adjacent Edge 1");
            nTempIndex = face.nAdjacentFaces.at(2).Valid() ? static_cast<int>(face.nAdjacentFaces.at(2)) : -1;
            WriteNumber(&nTempIndex, 4, "Walkable Face " + std::to_string(f) + " > Adjacent Edge 2");
            MarkDataBorder(nPosition - 1);
        }
    }
    if(data.edges.size() == 0 || data.nType == 0){
        data.nOffsetToEdges = 0;
        WriteNumber(&data.nOffsetToEdges, 0, "", &nOffsetEdges);
    }
    else{
        data.nOffsetToEdges = nPosition;
        WriteNumber(&data.nOffsetToEdges, 0, "", &nOffsetEdges);
    }
    for(unsigned f = 0; f < data.edges.size() && data.nType == 1; f++){
        Edge & edge = data.edges.at(f);
        WriteNumber(&edge.nIndex, 4, "Outer Edge " + std::to_string(f) + " > Index");
        WriteNumber(&edge.nTransition, 4, "Outer Edge " + std::to_string(f) + " > Transition");
        MarkDataBorder(nPosition - 1);
    }
    if(data.perimeters.size() == 0 || data.nType == 0){
        data.nOffsetToPerimeters = 0;
        WriteNumber(&data.nOffsetToPerimeters, 0, "", &nOffsetPerimeters);
    }
    else{
        data.nOffsetToPerimeters = nPosition;
        WriteNumber(&data.nOffsetToPerimeters, 0, "", &nOffsetPerimeters);
    }
    for(unsigned f = 0; f < data.perimeters.size() && data.nType == 1; f++){
        unsigned & perimeter = data.perimeters.at(f).nPerimeter;
        WriteNumber(&perimeter, 4, "Perimeter " + std::to_string(f));
        MarkDataBorder(nPosition - 1);
    }

    positions.shrink_to_fit();
}
