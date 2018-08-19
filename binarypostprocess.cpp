#include "MDL.h"
#include <iomanip>
#include <Shlwapi.h>
#include <algorithm>

/**
    Functions:
    MDL::BinaryPostProcess()
    MDL::ConsolidateSmoothingGroups()
    MDL::GenerateSmoothingNumber()
    Patch::FindNormal()
    Patch::FindTangentSpace()
/**/

extern bool bCancelSG;
extern std::vector<std::string> vsReport;

/// This function is the main binary decompilation post-processing function
void MDL::BinaryPostProcess(){
    FileHeader & Data = *FH;
    ReportObject ReportMdl (*this);
    Report("Post-processing decompiled model...");
    Timer tBinPostProcess;

    //Create file /stringstream)
    std::stringstream file, summary;
    std::stringstream fileaabb;

    /** In this part, I will go through all the nodes and calculate for all meshes:
        1. face IDs
        2. aabb node face centroids
        3. face area and face UV area
        4. If debug is on, calculate the aabb tree and compare it.
    **/
    for(Node & node : Data.MH.ArrayOfNodes){
        if(node.Head.nType & NODE_MESH && !(node.Head.nType & NODE_SABER)){
            for(int f = 0; f < node.Mesh.Faces.size(); f++){
                Face & face = node.Mesh.Faces.at(f);
                Vertex & v1 = node.Mesh.Vertices.at(face.nIndexVertex[0]);
                Vertex & v2 = node.Mesh.Vertices.at(face.nIndexVertex[1]);
                Vertex & v3 = node.Mesh.Vertices.at(face.nIndexVertex[2]);
                Vector & v1UV = v1.MDXData.vUV1;
                Vector & v2UV = v2.MDXData.vUV1;
                Vector & v3UV = v3.MDXData.vUV1;
                Vector Edge1 = v2 - v1;
                Vector Edge2 = v3 - v1;
                Vector Edge3 = v3 - v2;
                Vector EUV1 = v2UV - v1UV;
                Vector EUV2 = v3UV - v1UV;
                Vector EUV3 = v3UV - v2UV;

                /// Mark faces with their ID
                face.nID = f;

                /// Calculate Centroid
                if(node.Head.nType & NODE_AABB){
                    face.vBBmax = Vector(-10000.0, -10000.0, -10000.0);
                    face.vBBmin = Vector(10000.0, 10000.0, 10000.0);
                    face.vCentroid = Vector(0.0, 0.0, 0.0);
                    for(int i = 0; i < 3; i++){
                        face.vBBmax.fX = std::max(face.vBBmax.fX, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fX);
                        face.vBBmax.fY = std::max(face.vBBmax.fY, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fY);
                        face.vBBmax.fZ = std::max(face.vBBmax.fZ, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fZ);
                        face.vBBmin.fX = std::min(face.vBBmin.fX, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fX);
                        face.vBBmin.fY = std::min(face.vBBmin.fY, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fY);
                        face.vBBmin.fZ = std::min(face.vBBmin.fZ, node.Mesh.Vertices.at(face.nIndexVertex.at(i)).fZ);
                        face.vCentroid += node.Mesh.Vertices.at(face.nIndexVertex.at(i));
                    }
                    face.vCentroid /= 3.0;
                }

                /// Area calculation
                face.fArea = HeronFormulaEdge(Edge1, Edge2, Edge3);
                face.fAreaUV = HeronFormulaEdge(EUV1, EUV2, EUV3);
            }

            /// DEBUG: Verify aabb tree calculation
            if(bDebug && node.Head.nType & NODE_AABB){
                std::vector<Aabb> array1;
                std::vector<Aabb> array2;
                std::vector<Face*> faceptrs;
                for(int f = 0; f < node.Mesh.Faces.size(); f++){
                    faceptrs.push_back(&node.Mesh.Faces.at(f));
                }
                Aabb RecalculationAabb;
                std::stringstream ssTemp;
                BuildAabbTree(RecalculationAabb, faceptrs, &ssTemp);
                LinearizeAabbTree(RecalculationAabb, array1);
                Aabb VanillaCopy = node.Walkmesh.RootAabb;
                LinearizeAabbTree(VanillaCopy, array2);
                if(array1.size() != array2.size()) fileaabb << "ERROR! Aabb arrays have different sizes!";
                else{
                    fileaabb << "Vanilla vs New";
                    int nGood = 0;
                    int nTotal = 0;
                    std::stringstream ssTemp2;
                    for(int a = 0; a < array1.size(); a++){
                        bool bEq = (array1.at(a).nID == array2.at(a).nID);
                        if(array2.at(a).nID.Valid()){
                            nTotal++;
                            if(bEq) nGood++;
                        }
                        ssTemp2 << "\n (" << a << ") " << array2.at(a).nID.Print() << " " << (bEq? "==" : "!=") << " " << array1.at(a).nID.Print() << (bEq? "" : " DIFFERENT!!");
                    }
                    fileaabb << "\nCorrect: " << nGood << "/" << (nTotal) << " (" << std::setprecision(4) << ((double) nGood / (double) nTotal * 100.0) << "%)\n";
                    fileaabb << ssTemp2.str();
                    fileaabb << "\n\n\n" << ssTemp.str();
                }
            }
        }
    }

    bCancelSG = false; /// Reset the cancel here, we can only cancel if we're creating patches, recalculating vectors or calculating smoothing groups.

    //Create patches
    CreatePatches();

    if(bCancelSG){
        /// Canceled during CreatePatches(), clean up the patches and return this function
        Data.MH.PatchArrayPointers.clear();
        Data.MH.PatchArrayPointers.shrink_to_fit();
        return;
    }

    /// Statistics counters
    int nNumOfFoundNormals = 0;
    int nNumOfFoundTS = 0;
    int nNumOfFoundTSB = 0;
    int nNumOfFoundTST = 0;
    int nNumOfFoundTSN = 0;
    int nBadUV = 0;
    int nBadGeo = 0;
    int nTangentPerfect = 0;

    /// Patch vector compare precision
    if(bXbox) Patch::fDiff = 0.01;
    else Patch::fDiff = 0.0001;

    Timer tVectors;
    Report("Recalculating vectors...");
    ProgressSize(0, 100);
    unsigned long nStepper = 0;
    unsigned nUnit = std::max((unsigned long) 1, static_cast<unsigned long>(Data.MH.PatchArrayPointers.size()) / 100);
    ProgressPos(0);
    ProgressSetStep(1);

    /// Prepare the SG array here, because the SGs are the same across all meshes.
    std::vector<std::vector<unsigned long int>> nSmoothingGroupNumbers;
    nSmoothingGroupNumbers.resize(Data.MH.PatchArrayPointers.size());

    /// Go through all patch groups
    bool bReportModel = false;
    for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
        std::vector<Patch> & patchgroup = Data.MH.PatchArrayPointers.at(pg);
        if(patchgroup.size() == 0){
            ReportMdl << "Patch group " << pg << " contains no patches. This means that the algorithm is bugged and needs to be looked at.\n";
            continue;
        }

        if(bDebug) file << (pg > 0 ? "\r\n" : "");

        /// Get the coordinates of the first patch group from the vertex of the first patch
        Node & first_node = Data.MH.ArrayOfNodes.at(patchgroup.front().nNodeArrayIndex);
        Vertex & first_vert = first_node.Mesh.Vertices.at(patchgroup.front().nVertex);
        Vector vFirstNormal = first_vert.MDXData.vNormal;
        vFirstNormal.Rotate(first_node.Head.qFromRoot);

        /// This boolean will record if all more than one normals are equal
        bool bSingleNormal = true;
        if(patchgroup.size() < 2) bSingleNormal = false; /// If there is only one vert anyway, set it to false
        else for(Patch & patch : patchgroup){
            Node & patch_node = Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex);
            Vertex & patch_vert = patch_node.Mesh.Vertices.at(patch.nVertex);
            Vector vThisNormal = patch_vert.MDXData.vNormal;

            vThisNormal.Rotate(patch_node.Head.qFromRoot);

            if(bDebug) file << "Comparing first normal " << vFirstNormal.Print() << " to patch normal " << vThisNormal.Print() << "\r\n";
            if(!vFirstNormal.Compare(vThisNormal, Patch::fDiff)){
                bSingleNormal = false;
                break;
            }
        }

        /// bCombined marks whether the patch group is made up of more than one node
        bool bCombined = false;
        for(Patch & patch : patchgroup) if(patchgroup.front().nNodeArrayIndex != patch.nNodeArrayIndex){
            bCombined = true;
            break;
        }

        /// Report the patch group
        if(bDebug) file << (bCombined ? "Combined group " : "Group ") << pg << " " << first_vert.vFromRoot.Print() << " - " << patchgroup.size() << " patches.";
        if(bSingleNormal) if(bDebug) file << "\r\n     All normals equal. Expecting errors.";

        if(bDebug) summary << "\r\nPatch group " << pg;
        if(bCombined) if(bDebug) summary << " (combined)";

        /*****************************/
        /**** 1 - First Patch Loop ***/
        /**
                In this one, we will only calculate the candidates for every patch separately.
        **/
        /*****************************/
        for(int p = 0; p < patchgroup.size(); p++){
            Patch & patch = patchgroup.at(p);
            Node & patch_node = Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex);

            /// Report the patch
            if(bDebug) file << "\r\n" << "-> Patch " << p << "/" << patchgroup.size() - 1 << " - " << GetNodeName(patch_node) << " " << patch.nVertex << " - faces";
            if(bDebug) for(int face_ind : patch.FaceIndices) file << " " << face_ind;

            patch.CalculateWorld(true, true);
        }

        /*****************************/
        /*** 2 - Second Patch Loop ***/
        /**
                In this one, we will try to find the matches for our candidates
        **/
        /*****************************/
        for(int p = 0; p < patchgroup.size(); p++){
            Patch & patch = patchgroup.at(p);
            Node & patch_node = Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex);
            Vertex & patch_vert = patch_node.Mesh.Vertices.at(patch.nVertex);

            /// Report the patch
            if(bDebug) file << "\r\n" << "---> Patch " << p << "/" << patchgroup.size() - 1 << " - " << GetNodeName(patch_node) << " " << patch.nVertex << " - ";
            if(bDebug) file << "normal " << patch_vert.MDXData.vNormal.Print();
            std::string sOpener = "\r\n" + GetNodeName(patch_node) + ": vert " + std::to_string(patch.nVertex);
            if(bDebug) summary << sOpener;
            if(bDebug) for(int i = 0; i < (60 - sOpener.length()); i++) summary << " ";

            /// Clear the patch's smoothing arrays, we will be filling them soon
            patch.SmoothedPatches.reserve(patchgroup.size());
            patch.SmoothedPatches.clear();

            /// However, add this patch's index to the smoothed patches, because it is required for the algorithm later on.
            patch.SmoothedPatches.push_back(p);

            /*****************************/
            /***** 1 - Vertex Normal *****/
            /*****************************/

            /// Now we've got to construct a matching vertex normal
            /// Create a boolean to track this:
            bool bFoundVertexNormal = false; /// This boolean marks whether the matching vertex normal has been found

            /// This vector will be used for verification
            Vector vNormal = patch_vert.MDXData.vNormal;

            /// Before we dive into the search for this normal, we first need a way to convert it back to node coords.
            /// Because only the orientation of the normal is important, we only need a quaternion that we will use to rotate our normal.
            /// Create a new var for it:
            Quaternion qBack;

            /// The qFromRoot we calculated during decompilation is appropriate for this, but here's how you can calculate it alternatively:
            /** /
            /// Collect the name indices of the ancestors of the node in order from the node to the root
            short nIndex = patch_node.Head.nNameIndex; // Take the name index of the node
            std::vector<int> Indices; // This vector will contain the indices from the node to the root in order.
            while(nIndex != -1){ // The price we have to pay for not going recursive
                Indices.push_back(nIndex);
                int nNodeIndex = GetNodeIndexByNameIndex(nIndex);
                if(nNodeIndex == -1) throw mdlexception("Vertex normal calculation error: dealing with a name index that does not have a node in geometry.");
                nIndex = Data.MH.ArrayOfNodes.at(nNodeIndex).Head.nParentIndex;
            }

            /// Now we go from the root to the node...
            if(bDebug) file << "\r\n    Calculating conversion quaternion...";
            for(int a = Indices.size() - 1; a >= 0; a--){
                int nNodeIndex = GetNodeIndexByNameIndex(Indices.at(a)); /// Get the node index for the ancestor
                if(nNodeIndex == -1) throw mdlexception("Vertex normal calculation error: dealing with a name index that does not have a node in geometry.");
                Node & ancestor = Data.MH.ArrayOfNodes.at(nNodeIndex); /// Get the ancestor

                /// Add the orientation
                //file << "\r\n        " << qBack.Print() << " * " << ancestor.Head.oOrient.GetQuaternion().Print() << " = " << (qBack * ancestor.Head.oOrient.GetQuaternion()).Print();
                qBack *= ancestor.Head.oOrient.GetQuaternion();
            }

            /// This should be it
            if(bDebug) file << "\r\n      qBack:     " << qBack.Print();
            /**/
            /// But let's use qFromRoot instead
            qBack = patch_node.Head.qFromRoot;
            if(bDebug) file << "\r\n      qFromRoot: " << patch_node.Head.qFromRoot.Print() << " (using this)";

            /// For some reason, qBack's inverse is used in rotations.
            /// In max, this is done automatically when using the rotation function, but I don't know why.
            qBack = qBack.inverse();

            /// We now have a vector and quaternion that can be used to translate node coords into world coords.
            if(bDebug) file << "\r\n    Final conversion quaternion: " << qBack.Print();

            /// Find the match
            if(bSingleNormal){
                /// This is an optimization. If all the patches have the same normal,
                /// then they all must smooth between each other, so it makes no sense singling them out
                /// and looking for the right combination.
                if(bDebug) file << "\r\n    Patch group normals are uniform, apply smoothing to all!";
                bFoundVertexNormal = true;
                for(int p2 = 0; p2 < patchgroup.size(); p2++){
                    /// Don't add the index for this very patch because it has been added already above!
                    if(p2 != p) patch.SmoothedPatches.push_back(p2);
                }
            }
            else{
                /// First check if this patch's normal is enough.
                /// ! This calculation is correct only in the simple case that there is only one patch adding to this normal.
                Vector vCheck = patch.vWorldNormal;
                if(bDebug) file << "\r\n      Base world unnormalized " << vCheck.Print();
                vCheck.Normalize();
                if(bDebug) file << "\r\n      Base world normalized " << vCheck.Print();
                vCheck.Rotate(qBack);
                if(bDebug) file << "\r\n    Comparing to base local " << vCheck.Print();

                /// Non-verification version:
                patch.Calculate(true, false);
                if(bDebug) file << "\r\n    Non-verification normal " << patch.vVertexNormal.Print();

                if(patch.CompareNormal()){ //if(vCheck.Compare(vNormal, Patch::fDiff)){
                    bFoundVertexNormal = true;
                }
                else if(patchgroup.size() < 12){
                    /// If the number of patches isn't so high as to severely slow down the algorithm,
                    /// run the recursive function to find the matching combination of patches for the normal
                    bFoundVertexNormal = patch.FindNormal(0, patch.SmoothedPatches, file);
                }
            }

            if(bDebug){
                file << "\r\n              Target normal " << vNormal.Print();

                if(bFoundVertexNormal) summary << "found";
                else summary << ":(";
                if(bSingleNormal) summary << " (single)";
            }

            if(bFoundVertexNormal){
                if(bDebug) file << "\n:)    MATCH FOUND!";
                nNumOfFoundNormals++;
            }
            else if(patch.bBadGeo){
                if(bDebug) file << "\n:/    BAD GEOMETRY - NO MATCH FOUND!";
                nBadGeo++;
            }
            else{
                if(bDebug) file << "\n:(    NO MATCH FOUND!";
                if(!bReportModel && (!bCombined || patchgroup.size() < 3)) bReportModel = true;
            }


            /*****************************/
            /***** 2 - Tangent Space *****/
            /*****************************/

            /**
                This part is tricky. On one hand, we want to verify the existing tangent space vectors. On the other,
                we want to add the missing ones. The best solution is probably to put the whole thing under an if.
            **/

            if(patch_node.Mesh.nMdxDataBitmap & MDX_FLAG_TANGENT1){
                /// Do tangent space verification
                /// Create a boolean to track this:
                char cFoundTangentSpace = 0; /// This char marks whether the matching tangent space vectors have been found

                patch.Calculate(false, true);

                /// Now, let's verify
                if(bDebug) file << "\r\n   Comparing TS bitangent " << patch_vert.MDXData.vTangent1.at(0).Print() <<" to proposed " << patch.vVertexB.Print() << ".";
                if(bDebug) file << "\r\n   Comparing TS tangent " << patch_vert.MDXData.vTangent1.at(1).Print() <<" to proposed " << patch.vVertexT.Print() << ".";
                if(bDebug) file << "\r\n   Comparing TS normal " << patch_vert.MDXData.vTangent1.at(2).Print() <<" to proposed " << patch.vVertexN.Print() << ".";

                /// Report the vectors we got right
                if(bDebug) file << "\r\n   Correct:";
                if(patch.CompareTangentSpace(TS_BITANGENT)){
                    if(bDebug) file << " bitangent";
                    cFoundTangentSpace = cFoundTangentSpace | TS_BITANGENT;
                }
                if(patch.CompareTangentSpace(TS_TANGENT)){
                    if(bDebug) file << " tangent";
                    cFoundTangentSpace = cFoundTangentSpace | TS_TANGENT;
                }
                if(patch.CompareTangentSpace(TS_NORMAL)){
                    if(bDebug) file << " normal";
                    cFoundTangentSpace = cFoundTangentSpace | TS_NORMAL;
                }
                if(!cFoundTangentSpace) if(bDebug) file << " none";

                /// Increment the counters and report based on the result
                if(bFoundVertexNormal && cFoundTangentSpace == TS_ALL){
                    nTangentPerfect++; /// Increment this. This is when we have the vertex normal matching with tangent space vectors, the perfect match
                    if(bDebug) file << " (perfect match)";
                }
                else if(patch.bBadUV){ if(bDebug) file << " (bad UV)"; }
                else if(cFoundTangentSpace){ if(bDebug) file << " (incomplete)"; }

                /// If we didn't get a match with the vertex normal and we didn't get any match with the base and the number of patches isn't too big
                if(!bFoundVertexNormal && !cFoundTangentSpace && patchgroup.size() < 12){
                    /// Run recursive function to find the tangent space vector
                    cFoundTangentSpace = patch.FindTangentSpace(0, patch.SmoothedPatches, file);

                }

                if(cFoundTangentSpace & TS_BITANGENT) nNumOfFoundTSB++;
                if(cFoundTangentSpace & TS_TANGENT) nNumOfFoundTST++;
                if(cFoundTangentSpace & TS_NORMAL) nNumOfFoundTSN++;

                if(bCombined) if(bDebug) file << " (combined)";

                if(cFoundTangentSpace == TS_ALL){
                    if(bDebug) file << "\r\n:)    MATCH FOUND!";
                    nNumOfFoundTS++;
                }
                else if(patch.bGroupBadUV){
                    if(bDebug) file << "\r\n:/    BAD UV!";
                    nBadUV++;
                }
                else{
                    if(bDebug) file << "\r\n:(    NO MATCH FOUND!";
                }
            }
            else{
                /// Do tangent space calculation
                /// The new algorithm
                /// Go through our patches

                patch.Calculate(false, true);

                /// Add the vectors to the node
                patch_vert.MDXData.vTangent1.at(0) = patch.vVertexB;
                patch_vert.MDXData.vTangent1.at(1) = patch.vVertexT;
                patch_vert.MDXData.vTangent1.at(2) = patch.vVertexN;
            }
        }
        if(bDebug) summary << "\r\n";
        /** END SECOND PATCH LOOP **/

        /// When we get here all the data in the patch group has been worked over.
        /// Our patches should now contain the info about which patches they smooth to.
        /// Now we need to generate smoothing group numbers

        /// Make an array of smoothing group numbers for this patch group
        for(int n = 0; n < patchgroup.size()*(patchgroup.size() - 1)/2 + 1; n++){
            nSmoothingGroupNumbers.at(pg).push_back(0);
        }

        /// Report
        if(bDebug) file << "\r\n" << "Getting vertex smoothing groups.\n";

        /// The number of smoothing group numbers used inside this patch group
        int nSmoothingGroupCounter = 0;

        /// For every patch
        for(int p = 0; p < patchgroup.size(); p++){
            Patch & patch = patchgroup.at(p);

            /// Report
            if(bDebug) file << "  Patch " << p << " smooths to " << patch.SmoothedPatches.size() << " patches (including itself)...\n";

            /// Get smoothing group numbers from the array of patches that this patch smooths to
            while(patch.SmoothedPatches.size() > 0){
                /// This is the SmoothedPatchesGroup, which will contain the indices of the patches that all
                /// smooth between each other. This means that they can be assigned a common (single) smoothing group number.
                std::vector<int> SmoothedPatchesGroup;

                /// Get last smoothed patch index
                int p2 = patch.SmoothedPatches.back();

                /// If this is this very patch, simply pop it and continue
                if(p2 == p){
                    patch.SmoothedPatches.pop_back();
                    continue;
                }

                /// Get last smoothed patch
                Patch & patch2 = patchgroup.at(p2);

                /// Add the indices of both patches to the group
                SmoothedPatchesGroup.push_back(p);
                SmoothedPatchesGroup.push_back(p2);

                /// Add the pointers to smoothing group integers, both get the same integer
                patch.SmoothingGroupNumbers.push_back(&(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));
                patch2.SmoothingGroupNumbers.push_back(&(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));

                /// Delete both patches from each other's SmoothedPatches
                patch.SmoothedPatches.pop_back();
                for(int i2 = 0; i2 < patch2.SmoothedPatches.size(); i2++){
                    if(patch2.SmoothedPatches.at(i2) == p){
                        patch2.SmoothedPatches.erase(patch2.SmoothedPatches.begin() + i2);
                        break;
                    }
                }

                /// This function will go through all the patches in this patch group
                /// It will look at the patches that are not yet in SmoothedPatchesGroup and for each one
                /// it will check whether it smooths to all the patches currently in the smoothed patches group.
                /// If it does, then the patch will be added to the group.
                GenerateSmoothingNumber(SmoothedPatchesGroup, nSmoothingGroupNumbers.at(pg), nSmoothingGroupCounter, pg, file);

                /// Report
                if(bDebug) file << "  Added smoothing group " << nSmoothingGroupCounter+1 << " for patches:";
                for(int spg = 0; spg < SmoothedPatchesGroup.size(); spg++){
                    file << " " << SmoothedPatchesGroup.at(spg);
                }
                if(bDebug) file << "\n";

                /// Elevate smoothing group counter
                nSmoothingGroupCounter++;
            }
            patch.SmoothedPatches.clear();
            patch.SmoothedPatches.shrink_to_fit();

            /// Once we get here, we have checked (for patch p) all the patches we smooth over to and added their indices
            /// In case the patch doesn't smooth to any other patch, store an additional identity smoothing group for it
            if(patch.SmoothingGroupNumbers.size() == 0){
                if(bDebug) file << "  Patch " << p << " in patch group has no smoothing group, setting it to " << nSmoothingGroupCounter+1 << ".\n";
                patch.SmoothingGroupNumbers.push_back((unsigned long int*) &(nSmoothingGroupNumbers.at(pg).at(nSmoothingGroupCounter)));
                nSmoothingGroupCounter++;
            }

            /// Report
            if(bDebug) file << "  (patch " << p << " now has " << patch.SmoothingGroupNumbers.size() << " smoothing groups)\n";
        }
        nStepper++;
        if(nStepper % nUnit == 0) ProgressStepIt();
    }
    ProgressPos(100);
    ReportMdl << "Recalculated vectors in " << tVectors.GetTime() << ".\n";
    if(bDebug) file << "\r\n" << summary.str();
    //vsReport.push_back(std::string(Data.MH.GH.sName.c_str()) + (!bReportModel ? " good" : " bad"));
    /**/

    /// Report results
    double fPercentage = ((double)nNumOfFoundNormals / (double)(Data.MH.nTotalVertCount - Data.MH.nExcludedVerts)) * 100.0;
    if(nNumOfFoundNormals != (Data.MH.nTotalVertCount - Data.MH.nExcludedVerts)) fPercentage = std::min(fPercentage, 99.99);
    double fPercentage2 = ((double) nBadGeo / (double)(Data.MH.nTotalVertCount - Data.MH.nExcludedVerts)) * 100.0;
    if(nBadGeo != (Data.MH.nTotalVertCount - Data.MH.nExcludedVerts)) fPercentage2 = std::min(fPercentage2, 99.99);
    bool bGoodEnough = (fPercentage > 80.0 || (Data.MH.nTotalVertCount - Data.MH.nExcludedVerts) == 0);
    ReportMdl << "Found normals: " << nNumOfFoundNormals << "/" << (Data.MH.nTotalVertCount - Data.MH.nExcludedVerts);
    if(Data.MH.nTotalVertCount > 0) ReportMdl << " (" << round(fPercentage * 100.0) / 100.0 << "%)";
    ReportMdl << "\n";
    ReportMdl << "  Bad geometry: " << nBadGeo << "/" << (Data.MH.nTotalVertCount - Data.MH.nExcludedVerts);
    ReportMdl << " (" << round(fPercentage2 * 100.0) / 100.0 << "%)";
    ReportMdl << "\n";

    if(Data.MH.nTotalTangent1Count > 0){
        fPercentage = ((double)nNumOfFoundTS / (double)Data.MH.nTotalTangent1Count) * 100.0;
        if(nNumOfFoundTS != Data.MH.nTotalTangent1Count) fPercentage = std::min(fPercentage, 99.99);
        //fPercentage2 = ((double)nNumOfFoundTS / (double)(Data.MH.nTotalTangent1Count - nBadUV)) * 100.0;
        ReportMdl << "Found tangent spaces: " << nNumOfFoundTS << "/" << Data.MH.nTotalTangent1Count << " (" << round(fPercentage * 100.0) / 100.0 << "%)\n";
        //ReportMdl << "  Without bad UVs: " << nNumOfFoundTS << "/" << (Data.MH.nTotalTangent1Count - nBadUV) << " (" << std::setprecision(4) << fPercentage2 << "%)\n";
        fPercentage2 = ((double)nTangentPerfect / (double)(Data.MH.nTotalTangent1Count)) * 100.0;
        if(nTangentPerfect != Data.MH.nTotalTangent1Count) fPercentage2 = std::min(fPercentage2, 99.99);
        ReportMdl << "  Perfect matches: " << nTangentPerfect << "/" << (Data.MH.nTotalTangent1Count) << " (" << round(fPercentage2 * 100.0) / 100.0 << "%)\n";
        fPercentage = ((double)nNumOfFoundTSB / (double)Data.MH.nTotalTangent1Count) * 100.0;
        if(nNumOfFoundTSB != Data.MH.nTotalTangent1Count) fPercentage = std::min(fPercentage, 99.99);
        //fPercentage2 = ((double)nNumOfFoundTSB / (double)(Data.MH.nTotalTangent1Count - nBadUV)) * 100.0;
        ReportMdl << "  Found bitangents: " << nNumOfFoundTSB << "/" << Data.MH.nTotalTangent1Count << " (" << round(fPercentage * 100.0) / 100.0 << "%)\n";
        //ReportMdl << "    Without bad UVs: " << nNumOfFoundTSB << "/" << (Data.MH.nTotalTangent1Count - nBadUV) << " (" << std::setprecision(4) << fPercentage2 << "%)\n";
        fPercentage = ((double)nNumOfFoundTST / (double)Data.MH.nTotalTangent1Count) * 100.0;
        if(nNumOfFoundTST != Data.MH.nTotalTangent1Count) fPercentage = std::min(fPercentage, 99.99);
        //fPercentage2 = ((double)nNumOfFoundTST / (double)(Data.MH.nTotalTangent1Count - nBadUV)) * 100.0;
        ReportMdl << "  Found tangents: " << nNumOfFoundTST << "/" << Data.MH.nTotalTangent1Count << " (" << round(fPercentage * 100.0) / 100.0 << "%)\n";
        //ReportMdl << "    Without bad UVs: " << nNumOfFoundTST << "/" << (Data.MH.nTotalTangent1Count - nBadUV) << " (" << std::setprecision(4) << fPercentage2 << "%)\n";
        fPercentage = ((double)nNumOfFoundTSN / (double)Data.MH.nTotalTangent1Count) * 100.0;
        if(nNumOfFoundTSN != Data.MH.nTotalTangent1Count) fPercentage = std::min(fPercentage, 99.99);
        //fPercentage2 = ((double)nNumOfFoundTSN / (double)(Data.MH.nTotalTangent1Count - nBadUV)) * 100.0;
        ReportMdl << "  Found normals: " << nNumOfFoundTSN << "/" << Data.MH.nTotalTangent1Count << " (" << round(fPercentage * 100.0) / 100.0 << "%)\n";
        //ReportMdl << "    Without bad UVs: " << nNumOfFoundTSN << "/" << (Data.MH.nTotalTangent1Count - nBadUV) << " (" << std::setprecision(4) << fPercentage2 << "%)\n";
        fPercentage2 = ((double) nBadUV / (double)(Data.MH.nTotalTangent1Count)) * 100.0;
        if(nBadUV != Data.MH.nTotalTangent1Count) fPercentage2 = std::min(fPercentage2, 99.99);
        ReportMdl << "  Bad UVs: " << nBadUV << "/" << Data.MH.nTotalTangent1Count << " (" << round(fPercentage2 * 100.0) / 100.0 << "%)\n";
    }

    /// Check if the vector recalculation was good enough for SG application
    if(!bGoodEnough) Error("The vector recalculations were off by too much to be able to determine the smoothing groups. Try decompiling with different vertex normal calculation settings.");
    else{
    //if(bGoodEnough){
        Report("Calculating smoothing groups...");

        /// This array will keep track of which patch groups we've processed already
        std::vector<bool> DonePatches(Data.MH.PatchArrayPointers.size(), false);

        /// Go through all the patch groups
        //file << "\n\n== Consolidation ==";
        for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
            ConsolidateSmoothingGroups(pg, nSmoothingGroupNumbers, DonePatches);
        }

        /// And finally finally, we merge the numbers for every face
        for(std::vector<Patch> & patchgroup : Data.MH.PatchArrayPointers){
            for(Patch & patch : patchgroup){
                unsigned long int nExistingSG = 0;
                for(int i = 0; i < patch.SmoothingGroupNumbers.size(); i++){
                    nExistingSG |= *patch.SmoothingGroupNumbers.at(i);
                }
                patch.nSmoothingGroups = (unsigned int) nExistingSG;
                for(int face_ind : patch.FaceIndices){
                    Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex).Mesh.Faces.at(face_ind).nSmoothingGroup |= nExistingSG;
                }
            }
        }
    }

    /// Get rid of the patch array.
    Data.MH.PatchArrayPointers.clear();
    Data.MH.PatchArrayPointers.shrink_to_fit();

    /// If debug is on, print out all the debug info we've gathered
    if(bDebug){
        std::wstring sDir;
        sDir = path.GetFullPath();
        sDir.reserve(MAX_PATH);
        PathRemoveFileSpecW(&sDir[0]);
        sDir.resize(wcslen(sDir.c_str()));
        std::wstring sDir2 (sDir);
        sDir += L"\\debug.txt";
        sDir2 += L"\\debug_aabb_comp.txt";
        ReportMdl << "Writing smoothing debug: " << to_ansi(std::wstring(sDir.c_str())) << "\n";
        ReportMdl << "Writing aabb debug: " << to_ansi(std::wstring(sDir2.c_str())) << "\n";
        HANDLE hFile = bead_CreateWriteFile(sDir);
        HANDLE hFile2 = bead_CreateWriteFile(sDir2);

        if(hFile == INVALID_HANDLE_VALUE){
            ReportMdl << "'debug.txt' does not exist. No debug will be written.\n";
        }
        else{
            bead_WriteFile(hFile, file.str());
            CloseHandle(hFile);
        }

        if(hFile2 == INVALID_HANDLE_VALUE){
            ReportMdl << "'debug_aabb_comp.txt' does not exist. No debug will be written.\n";
        }
        else{
            bead_WriteFile(hFile2, fileaabb.str());
            CloseHandle(hFile2);
        }
    }

    ReportMdl << "Done post-processing model (" << tBinPostProcess.GetTime() << ")!\n";
}

/** Algorithm
  1. Go through all of the patches in the patch group.
  2. If number is not single, skip it. Else if there is no assigned number for the patch
     make the next number in sequence on the patch.
  3. Go through all the faces and assign the same
     number to all the other single corners, then mark the face as processed.
  4. now we go through the processed faces and apply the same algorithm
/**/
void MDL::ConsolidateSmoothingGroups(int nPatchGroup, std::vector<std::vector<unsigned long int>> & Numbers, std::vector<bool> & DoneGroups){
    if(bCancelSG) return;
    FileHeader & Data = *FH;
    for(int p = 0; p < Data.MH.PatchArrayPointers.at(nPatchGroup).size(); p++){
        Patch & patch = Data.MH.PatchArrayPointers.at(nPatchGroup).at(p);
        if(patch.SmoothingGroupNumbers.size() == 1){
            if(*patch.SmoothingGroupNumbers.front() == 0){
                unsigned long int nBitflag = 0;

                /// First get all the used numbers
                for(int num = 0; num < Numbers.at(nPatchGroup).size(); num++){
                    nBitflag = nBitflag | Numbers.at(nPatchGroup).at(num);
                }

                /// Now find the first unused one and use it.
                for(int n = 0; *patch.SmoothingGroupNumbers.front() == 0 && n < 32; n++){
                    if(!(nBitflag & pown(2, n))) *patch.SmoothingGroupNumbers.front() = pown(2, n);
                }
            }

            for(int f = 0; f < patch.FaceIndices.size(); f++){
                Node & node = Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex);
                Face & face = node.Mesh.Faces.at(patch.FaceIndices.at(f));

                /// Skip if processed
                if(face.bProcessedSG) continue;

                /// Okay, we've got one corner, now we need to check the other two corners if they're single
                for(int v = 0; v < 3; v++){
                    Vertex & vert = node.Mesh.Vertices.at(face.nIndexVertex[v]);

                    /// Don't process yourself
                    if(face.nIndexVertex[v] == patch.nVertex) continue;

                    /// We get every vert's patch group
                    std::vector<Patch> & PatchVector = Data.MH.PatchArrayPointers.at(vert.nLinkedFacesIndex);
                    bool bFound = false;
                    int p2 = 0;
                    while(!bFound && p2 < PatchVector.size()){
                        /// Find the patch with the same vert index as stored with the face
                        if(patch.nNodeArrayIndex == PatchVector.at(p2).nNodeArrayIndex && PatchVector.at(p2).nVertex == face.nIndexVertex[v]){
                            bFound = true;
                        }
                        else p2++;
                    }
                    if(p2 == PatchVector.size()) break;
                    /// We get the patch that contains the current vert
                    Patch & vertpatch = PatchVector.at(p2);
                    if(vertpatch.SmoothingGroupNumbers.size() == 1){
                        /// Finally, copy the number to a single corner
                        *vertpatch.SmoothingGroupNumbers.front() = *patch.SmoothingGroupNumbers.front();
                    }
                }
                face.bProcessedSG = true;
            }
        }
    }

    DoneGroups.at(nPatchGroup) = true;

    for(int p = 0; p < Data.MH.PatchArrayPointers.at(nPatchGroup).size(); p++){
        Patch & patch = Data.MH.PatchArrayPointers.at(nPatchGroup).at(p);
        for(int f = 0; f < patch.FaceIndices.size(); f++){
            Node & node = Data.MH.ArrayOfNodes.at(patch.nNodeArrayIndex);
            Face & face = node.Mesh.Faces.at(patch.FaceIndices.at(f));

            /// Only processed faces
            if(!face.bProcessedSG) continue;

            for(int v = 0; v < 3; v++){
                Vertex & vert = node.Mesh.Vertices.at(face.nIndexVertex[v]);

                /// Don't process yourself
                if(face.nIndexVertex[v] == patch.nVertex) continue;

                /// AND NOW... DO RECURSION!!!
                if(!DoneGroups.at(vert.nLinkedFacesIndex)) ConsolidateSmoothingGroups(vert.nLinkedFacesIndex, Numbers, DoneGroups);
            }
        }
    }
}

void MDL::GenerateSmoothingNumber(std::vector<int> & SmoothedPatchesGroup, const std::vector<unsigned long int> & nSmoothingGroupNumbers, const int & nSmoothingGroupCounter, const int & pg, std::stringstream & file){
    if(bCancelSG) return;
    FileHeader & Data = *FH;

    std::vector<Patch> & patchgroup = Data.MH.PatchArrayPointers.at(pg);
    for(int p = 0; p < patchgroup.size(); p++){
        Patch & patch = patchgroup.at(p);

        /// Check if patch p already exists in the smoothed patches group, if so skip it
        bool bExists = false;
        for(int sp = 0; sp < SmoothedPatchesGroup.size() && !bExists; sp++){
            if(SmoothedPatchesGroup.at(sp) == p){
                bExists = true;
            }
        }
        if(bExists) continue;

        /// Go through the smoothed patches group; check that all patches in
        /// the smoothed patch group are linked in this patch's smoothed patches as well
        bool bFound = true;
        for(int sp = 0; sp < SmoothedPatchesGroup.size() && bFound; sp++){
            bFound = false;

            /// Go through the current patch's smoothed patches
            for(int i = 0; i < patch.SmoothedPatches.size() && !bFound; i++){
                /// If this patch smooths over to the current patch in the smoothed patches group
                if(patch.SmoothedPatches.at(i) == SmoothedPatchesGroup.at(sp)){
                    bFound = true;
                }
            }
        }
        if(bFound){
            /// Add the patch to the smoothed patches group
            SmoothedPatchesGroup.push_back(p);

            ///Add the same smoothing group number to the smoothing group numbers of the current patch.
            patch.SmoothingGroupNumbers.push_back((long unsigned int*)&(nSmoothingGroupNumbers.at(nSmoothingGroupCounter)));

            ///Make sure we delete the smoothed patch numbers on everyone
            for(int spg = 0; spg < SmoothedPatchesGroup.size(); spg++){
                Patch & spgpatch = patchgroup.at(SmoothedPatchesGroup.at(spg));
                for(int sp = 0; sp < spgpatch.SmoothedPatches.size(); sp++){
                    for(int spg2 = 0; spg2 < SmoothedPatchesGroup.size(); spg2++){
                        if(spgpatch.SmoothedPatches.at(sp) == spg2){
                            spgpatch.SmoothedPatches.erase(spgpatch.SmoothedPatches.begin() + sp);
                            sp--;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool Patch::FindNormal(unsigned int nCheckFrom,                              /// Check from this patch, needed for recursion
                     std::vector<MdlInteger<unsigned int>> & CurrentlySmoothedPatches,   /// The vector of indices of patches that are being considered, needed for recursion
                     std::stringstream & file                       /// Report string stream
                     )
{
    if(bCancelSG) return false;
    std::vector<Patch> & patch_group = GetPatchGroup();

    if(patch_group.size() == 0) throw mdlexception("Patch::FindNormal() error: patch_group contains no patches.");

    /// Go through all patches in reverse order
    for(MdlInteger<unsigned> nCount = patch_group.size() - 1; nCount.Valid() && nCount >= nCheckFrom; nCount = nCount - 1){

        if(!nCount.Valid()) throw mdlexception("Patch::FindNormal() error: nCount somehow managed to be negative (nCheckFrom = " + std::to_string(nCheckFrom) + ").");

        /// Skip the current patch
        if(&patch_group.at(nCount) == this) continue;

        /// Make a copy of the currently smoothed patches
        std::vector<MdlInteger<unsigned int>> OurSmoothedPatches = CurrentlySmoothedPatches;

        /// We are now smoothing also for this patch in this run, so add it to the lot
        OurSmoothedPatches.push_back(nCount);

        /// Calculate the vertex normal candidate for this set of smoothed patches
        Calculate(true, false, &OurSmoothedPatches);

        /// Report candidate
        if(ptr_mdl->bDebug) file << "\n    Comparing to proposed   " << vVertexNormal.Print() << ". Included patches:";
        for(int g = 0; g < OurSmoothedPatches.size(); g++){
            if(&patch_group.at(OurSmoothedPatches.at(g)) != this) if(ptr_mdl->bDebug) file << " " << OurSmoothedPatches.at(g);
        }
        //file << "\n";

        /// Compare
        if(CompareNormal()){
            SmoothedPatches = OurSmoothedPatches;
            return true;
        }

        if(FindNormal(nCount+1, OurSmoothedPatches, file)){
            return true;
        }
    }
    return false;
}

char Patch::FindTangentSpace(unsigned int nCheckFrom, std::vector<MdlInteger<unsigned int>> & CurrentlySmoothedPatches, std::stringstream & file){
    if(bCancelSG) return false;
    std::vector<Patch> & patch_group = GetPatchGroup();

    if(patch_group.size() == 0) throw mdlexception("Patch::FindTangentSpace() error: patch_group contains no patches.");

    /// Go through all patches in reverse order
    for(MdlInteger<unsigned> nCount = patch_group.size() - 1; nCount.Valid() && nCount >= nCheckFrom; nCount = nCount - 1){
        /// Skip the current patch
        if(&patch_group.at(nCount) == this) continue;

        /// Make a copy of the currently smoothed patches
        std::vector<MdlInteger<unsigned int>> OurSmoothedPatches = CurrentlySmoothedPatches;

        /// We are now smoothing also for this patch in this run, so add it to the lot
        OurSmoothedPatches.push_back(nCount);

        /// Calculate the tangent space candidate for this set of smoothed patches
        Calculate(false, true, &OurSmoothedPatches);

        /// Report
        if(ptr_mdl->bDebug) file << "\r\n   Comparing to proposed " << vVertexB.Print() << ".";
        if(ptr_mdl->bDebug) file << "\r\n   Comparing to proposed " << vVertexT.Print() << ".";
        if(ptr_mdl->bDebug) file << "\r\n   Comparing to proposed " << vVertexN.Print() << ".";
        if(ptr_mdl->bDebug) file << "\r\n   Included patches:";
        for(int g = 0; g < OurSmoothedPatches.size(); g++){
            if(&patch_group.at(OurSmoothedPatches.at(g)) != this) if(ptr_mdl->bDebug) file << " " << OurSmoothedPatches.at(g);
        }
        if(ptr_mdl->bDebug) file << "\r\n   Correct:";
        char nReturn = 0;
        if(CompareTangentSpace(TS_BITANGENT)){
            if(ptr_mdl->bDebug) file << " bitangent";
            nReturn = (nReturn | TS_BITANGENT);
        }
        if(CompareTangentSpace(TS_TANGENT)){
            if(ptr_mdl->bDebug) file << " tangent";
            nReturn = (nReturn | TS_TANGENT);
        }
        if(CompareTangentSpace(TS_NORMAL)){
            if(ptr_mdl->bDebug) file << " normal";
            nReturn = (nReturn | TS_NORMAL);
        }
        if(bGroupBadUV){ if(ptr_mdl->bDebug) file << " (bad UV)"; }
        if(bGroupBadGeo){ if(ptr_mdl->bDebug) file << " (bad geo)"; }

        if(nReturn == TS_ALL){
            if(ptr_mdl->bDebug) file << " (match found)";
            SmoothedPatches = OurSmoothedPatches;
            return nReturn;
        }
        else{
            if(ptr_mdl->bDebug){
                if(nReturn) file << " (incomplete)";
                else file << " none";
            }
            nReturn = FindTangentSpace(nCount+1, OurSmoothedPatches, file);
            if(nReturn == TS_ALL) return nReturn;
        }
    }
    return 0;
}
