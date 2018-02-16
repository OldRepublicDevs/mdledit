#include "MDL.h"
#include <algorithm>
#include "shlwapi.h"
#include <fstream>

/**
    Functions:
    ASCII::Read()
    MDL::GatherChildren()
    MDL::AsciiPostProcess()
/**/

/// Utility function, gets triangular face normalized normal from vert vectors
Vector GetNormal(Vector v1, Vector v2, Vector v3){
    Vector vNormal = (v2 - v1) / (v3 - v1);
    vNormal.Normalize();
    return vNormal;
}

/// Utility function for mesh->saber conversion
/// Loops through the faces, until it finds one where both of the vert indices exits, then it returns the third index (unless it's being ignored), otherwise -1
int FindThirdIndex(const std::vector<Face> & faces, int ind1, int ind2, int ignore = -1){
    for(int f = 0; f < faces.size(); f++){
        const Face & face = faces.at(f);
        int nFound = 0;
        for(int i = 0; i < 3; i++){
            if(face.nIndexVertex.at(i) == ind1 || face.nIndexVertex.at(i) == ind2) nFound++;
        }
        if(nFound == 2){
            for(int i = 0; i < 3; i++){
                if(face.nIndexVertex.at(i) != ind1 && face.nIndexVertex.at(i) != ind2 && face.nIndexVertex.at(i) != ignore) return face.nIndexVertex.at(i);
            }
        }
    }
    return -1;
}

bool ASCII::Read(MDL & Mdl){
    //std::cout << "We made it into ASCII::Read.\n";
    ReportObject ReportMdl(Mdl);

    //CreateDataStructure
    Mdl.GetFileData().reset(new FileHeader);
    std::unique_ptr<FileHeader> & FH = Mdl.GetFileData();
    std::string sID;

    Mdl.src = AsciiSource;
    Mdl.Report("Reading ASCII...");

    //Set stuff to zero
    nPosition = 0;
    FH->MH.Animations.resize(0);
    FH->MH.Names.resize(0);

    int nConvert;
    unsigned int uConvert;
    double fConvert;
    bool bFound = false;
    bool bEnd = false;
    bool bGeometry = false;
    int nNode = 0;
    int nAnimation = -1;
    bool bAnimation = false;
    bool bEventlist = false;
    bool bVerts = false;
    bool bFaces = false;
    bool bTverts = false;
    bool bTexIndices1 = false;
    bool bTverts1 = false;
    bool bTexIndices2 = false;
    bool bTverts2 = false;
    bool bTexIndices3 = false;
    bool bTverts3 = false;
    bool bWeights = false;
    bool bAabb = false;
    bool bConstraints = false;
    bool bKeys = false;
    bool bTextureNames = false;
    bool bFlarePositions = false;
    bool bFlareSizes = false;
    bool bFlareColorShifts = false;
    bool bRoomLinks = false;
    bool bColors = false;
    bool bColorIndices = false;
    bool bMagnusll = false;
    unsigned int nNodeCounter = 0;
    Node * PreviousNode;
    unsigned int nDataMax;
    unsigned int nDataCounter;
    int nCurrentIndex = -1;

    std::vector<std::string> sBumpmapped;

    ///This first loop is for building the name array, which we need for the weights
    /// Actually, it seems that the ascii is potentially ambiguous. In binary, two objects may have the same name,
    /// and they will still be distinguished in all cases, because they are always referred to by their name index.
    /// In ascii, the name replaces the name index. This means that weights, which do not rely on a bonemap, will be
    /// ambiguous when it comes to two objects with the same name. Actually the same applies for the parent field,
    /// it is potentially possible for either of the objects to be the real parent. This should be addressed in the
    /// conversion to ascii, so that the names are disambiguated. The user then only needs to make sure that the ascii
    /// models they provide for reading are well-formed in that respect.
    /// In short, we will now assume that all names are unique.
    sID = "";
    nPosition = 0;
    bool bStop = false;
    try{
        while(nPosition < sBuffer.size() && !bStop){
            ReadUntilText(sID);
            while(sID != "node" && sID != "name" && nPosition < sBuffer.size() && !bStop){
                SkipLine();
                ReadUntilText(sID);
                if(sID == "endmodelgeom") bStop = true;
            }
            if(nPosition < sBuffer.size() && !bStop){
                //Once we get here, we have found a node. The name is only a keyword's strlen away!
                if(sID == "node") ReadUntilText(sID); //Get keyword
                ReadUntilText(sID, false); //Get name

                //Got it! Now to save it the name array.
                Name name; //our new name
                name.sName = sID;
                FH->MH.Names.push_back(name);
            }
        }
        ReportMdl << "Done indexing names (" << FH->MH.Names.size() << ").\n";

        nPosition = 0;
        ///Loops for every row
        bool * lpbList = nullptr;
        while(nPosition < sBuffer.size() && !bEnd){
            //First set our current list
            if(bKeys) lpbList = &bKeys;
            else if(bWeights) lpbList = &bWeights;
            else if(bEventlist) lpbList = &bEventlist;
            else if(bAabb) lpbList = &bAabb;
            else if(bVerts) lpbList = &bVerts;
            else if(bFaces) lpbList = &bFaces;
            else if(bTverts) lpbList = &bTverts;
            else if(bTexIndices1) lpbList = &bTexIndices1;
            else if(bTverts1) lpbList = &bTverts1;
            else if(bTexIndices2) lpbList = &bTexIndices2;
            else if(bTverts2) lpbList = &bTverts2;
            else if(bTexIndices3) lpbList = &bTexIndices3;
            else if(bTverts3) lpbList = &bTverts3;
            else if(bConstraints) lpbList = &bConstraints;
            else if(bFlarePositions) lpbList = &bFlarePositions;
            else if(bFlareSizes) lpbList = &bFlareSizes;
            else if(bFlareColorShifts) lpbList = &bFlareColorShifts;
            else if(bTextureNames) lpbList = &bTextureNames;
            else if(bRoomLinks) lpbList = &bRoomLinks;
            else if(bColors) lpbList = &bColors;
            else if(bColorIndices) lpbList = &bColorIndices;
            else lpbList = nullptr;

            //First, check if we have a blank line, we'll just skip it here.
            if(EmptyRow()){
                SkipLine();
            }
            //Second, check if we are in some list of data. We should not look for a keyword then.
            else if(lpbList != nullptr){
                //First, check if we're done. Save your position, cuz we'll need to revert if we're not done
                int nSavePos = nPosition;
                ReadUntilText(sID);
                if(sID == "endlist" || sID == "endnode" || nDataMax == 0){
                    *lpbList = false;
                    if(sID == "endnode") nNode = 0;
                    SkipLine();
                }
                else{
                    //Revert back to old position
                    nPosition = nSavePos;

                    /// Read the data
                    try{
                    if(bKeys){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading key data " << nDataCounter << ".\n";
                        //We've already read the timekeys, we're left with the values
                        Animation & anim = FH->MH.Animations.back();
                        Node & node = anim.ArrayOfNodes.back();
                        Node & geonode = Mdl.GetNodeByNameIndex(node.Head.nNodeNumber);
                        Location loc = geonode.GetLocation();
                        Controller & ctrl = node.Head.Controllers.back();

                        if(!ReadFloat(fConvert)) throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'."); //First read the timekey, also check that we're valid

                        if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION && ctrl.nColumnCount < 16){
                            double fX, fY, fZ, fA;
                            if(ReadFloat(fConvert)) fX = fConvert;
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fY = fConvert;
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fZ = fConvert;
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fA = fConvert;
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            Orientation NewOrientKey;
                            NewOrientKey.SetAxisAngle(fX, fY, fZ, fA);
                            if(FH->MH.bCompressQuaternions){
                                ByteBlock4.ui = CompressQuaternion(NewOrientKey.GetQuaternion());
                                node.Head.ControllerData.push_back(ByteBlock4.f);
                            }
                            else{
                                node.Head.ControllerData.push_back(NewOrientKey.GetQuaternion().vAxis.fX);
                                node.Head.ControllerData.push_back(NewOrientKey.GetQuaternion().vAxis.fY);
                                node.Head.ControllerData.push_back(NewOrientKey.GetQuaternion().vAxis.fZ);
                                node.Head.ControllerData.push_back(NewOrientKey.GetQuaternion().fW);
                            }
                        }
                        else if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fX);
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fY);
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fZ);
                            else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ctrl.nColumnCount > 16){
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            }
                        }
                        else{
                            bFound = true;
                            while(bFound){
                                if(ReadFloat(fConvert)){
                                    //ReportMdl << "Reading non-position keys...\n";
                                    node.Head.ControllerData.push_back(fConvert);
                                }
                                else bFound = false;
                            }
                        }
                    }
                    else if(bEventlist){
                        Animation & anim = FH->MH.Animations.back();
                        Event sound; //New sound
                        if(ReadFloat(fConvert)) sound.fTime = fConvert;
                        else throw mdlexception("Error reading eventlist data for animation '" + anim.sName + "'. Event time missing.");
                        bFound = ReadUntilText(sID, false);
                        if(!bFound){
                            ReportMdl << "ReadUntilText() Event name is missing!\n";
                            throw mdlexception("Error reading eventlist data for animation '" + anim.sName + "'. Event name missing");
                        }
                        else if(sID.size() > 32){
                            Error("Event name larger than the limit, 32 characters! Will truncate and continue.");
                            sID.resize(32);
                        }
                        else if(sID.size() > 16){
                            Warning("Event name larger than 16 characters! This may cause problems in the game.");
                        }
                        sound.sName = sID;
                        if(bFound){
                            anim.Events.push_back(sound);
                        }
                        else throw mdlexception("Error reading eventlist data for node '" + anim.sName + "'.");
                    }
                    else if(bAabb){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading aabb data " << nDataCounter << ".\n";
                        //SkipLine();
                        int nSaveCurrentPosition = nPosition;
                        bool bContinue = true;
                        while(bContinue){
                            try{
                                bContinue = ReadFloat(fConvert);
                            }
                            catch(const std::exception & e){
                                bContinue = false;
                            }
                            if(!bContinue){
                                //Revert, because we still have to read this non-float in the next turn
                                nPosition = nSaveCurrentPosition;

                                //Setting these guys like this should immediately end the loop.
                                nDataCounter = 1;
                                nDataMax = 1;
                            }
                            else{
                                nSaveCurrentPosition = nPosition;
                                SkipLine();
                            }
                        }

                        /*
                        Aabb aabb; //New aabb
                        bFound = true;
                        if(ReadFloat(fConvert)) aabb.vBBmin.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) aabb.vBBmin.fY = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) aabb.vBBmin.fZ = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) aabb.vBBmax.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) aabb.vBBmax.fY = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) aabb.vBBmax.fZ = fConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) aabb.nID = nConvert;
                        else bFound = false;

                        if(bFound){
                            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                            node.Walkmesh.ArrayOfAabb.push_back(aabb);
                        }
                        */
                    }
                    else if(bVerts){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading verts data " << nDataCounter << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Vector vert;
                        if(ReadFloat(fConvert)) vert.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vert.fY = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vert.fZ = fConvert;
                        else bFound = false;
                        if(bFound){
                            node.Mesh.TempVerts.push_back(std::move(vert));
                        }
                        else throw mdlexception("Error reading vert data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bFaces){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading faces data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Face face;
                        face.nNodeNumber = nCurrentIndex;

                        //Currently we read the regular NWMax version with only a single set of tvert indices
                        if(ReadInt(nConvert)) face.nIndexVertex[0] = nConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) face.nIndexVertex[1] = nConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) face.nIndexVertex[2] = nConvert;
                        else bFound = false;
                        if(ReadUInt(uConvert)) face.nSmoothingGroup = uConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) face.nIndexTvert[0] = nConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) face.nIndexTvert[1] = nConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) face.nIndexTvert[2] = nConvert;
                        else bFound = false;
                        face.nTextureCount++;
                        if(ReadInt(nConvert)) face.nMaterialID = nConvert;
                        else bFound = false;

                        if(bMagnusll){ //read tverts magnusll style
                            int ntv1ind1, ntv1ind2, ntv1ind3;
                            //bitmap2
                            bool bTry = true;
                            if(ReadInt(nConvert)) ntv1ind1 = nConvert;
                            else bTry = false;
                            if(ReadInt(nConvert)) ntv1ind2 = nConvert;
                            else bTry = false;
                            if(ReadInt(nConvert)) ntv1ind3 = nConvert;
                            else bTry = false;
                            if(bTry){
                                face.nIndexTvert1[0] = ntv1ind1;
                                face.nIndexTvert1[1] = ntv1ind2;
                                face.nIndexTvert1[2] = ntv1ind3;
                                face.nTextureCount++;

                                //texture0
                                if(ReadInt(nConvert)) ntv1ind1 = nConvert;
                                else bTry = false;
                                if(ReadInt(nConvert)) ntv1ind2 = nConvert;
                                else bTry = false;
                                if(ReadInt(nConvert)) ntv1ind3 = nConvert;
                                else bTry = false;
                                if(bTry){
                                    face.nIndexTvert2[0] = ntv1ind1;
                                    face.nIndexTvert2[1] = ntv1ind2;
                                    face.nIndexTvert2[2] = ntv1ind3;
                                    face.nTextureCount++;

                                    //texture1
                                    if(ReadInt(nConvert)) ntv1ind1 = nConvert;
                                    else bTry = false;
                                    if(ReadInt(nConvert)) ntv1ind2 = nConvert;
                                    else bTry = false;
                                    if(ReadInt(nConvert)) ntv1ind3 = nConvert;
                                    else bTry = false;
                                    if(bTry){
                                        face.nIndexTvert3[0] = ntv1ind1;
                                        face.nIndexTvert3[1] = ntv1ind2;
                                        face.nIndexTvert3[2] = ntv1ind3;
                                        face.nTextureCount++;
                                    }
                                }
                            }
                        }

                        if(bFound){
                            node.Mesh.Faces.push_back(face);
                        }
                        else throw mdlexception("Error reading face data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTverts){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading tverts data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Vector vUV;
                        if(ReadFloat(fConvert)) vUV.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vUV.fY = fConvert;
                        else bFound = false;

                        if(bFound) node.Mesh.TempTverts.push_back(vUV);
                        else throw mdlexception("Error reading tvert data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTexIndices1){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        if(nDataCounter < node.Mesh.Faces.size()){
                            Face & face = node.Mesh.Faces.at(nDataCounter);

                            if(ReadInt(nConvert)) face.nIndexTvert1[0] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert1[1] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert1[2] = nConvert;
                            else bFound = false;
                            face.nTextureCount++;
                        }
                        else{
                            ReportMdl << "Warning! There are more texindices1 than faces!\n";
                            nDataCounter = nDataMax;
                        }

                        if(!bFound) throw mdlexception("Error reading texindices1 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTverts1){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading tverts data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Vector vUV;
                        if(ReadFloat(fConvert)) vUV.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vUV.fY = fConvert;
                        else bFound = false;

                        if(bFound) node.Mesh.TempTverts1.push_back(vUV);
                        else throw mdlexception("Error reading tverts1 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTexIndices2){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Face & face = node.Mesh.Faces.at(nDataCounter);

                        if(nDataCounter < node.Mesh.Faces.size()){
                            if(ReadInt(nConvert)) face.nIndexTvert2[0] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert2[1] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert2[2] = nConvert;
                            else bFound = false;
                            face.nTextureCount++;
                        }
                        else{
                            ReportMdl << "Warning! There are more texindices2 than faces!\n";
                            nDataCounter = nDataMax;
                        }

                        if(!bFound) throw mdlexception("Error reading texindices2 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTverts2){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading tverts data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Vector vUV;
                        if(ReadFloat(fConvert)) vUV.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vUV.fY = fConvert;
                        else bFound = false;

                        if(bFound) node.Mesh.TempTverts2.push_back(vUV);
                        else throw mdlexception("Error reading tverts2 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTexIndices3){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Face & face = node.Mesh.Faces.at(nDataCounter);

                        if(nDataCounter < node.Mesh.Faces.size()){
                            if(ReadInt(nConvert)) face.nIndexTvert3[0] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert3[1] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexTvert3[2] = nConvert;
                            else bFound = false;
                            face.nTextureCount++;
                        }
                        else{
                            ReportMdl << "Warning! There are more texindices3 than faces!\n";
                            nDataCounter = nDataMax;
                        }

                        if(!bFound) throw mdlexception("Error reading texindices3 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bTverts3){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading tverts data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Vector vUV;
                        if(ReadFloat(fConvert)) vUV.fX = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) vUV.fY = fConvert;
                        else bFound = false;

                        if(bFound) node.Mesh.TempTverts3.push_back(vUV);
                        else throw mdlexception("Error reading tvert3 data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bColorIndices){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        if(nDataCounter < node.Mesh.Faces.size()){
                            Face & face = node.Mesh.Faces.at(nDataCounter);

                            if(ReadInt(nConvert)) face.nIndexColor[0] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexColor[1] = nConvert;
                            else bFound = false;
                            if(ReadInt(nConvert)) face.nIndexColor[2] = nConvert;
                            else bFound = false;
                            face.nTextureCount++;
                        }
                        else{
                            ReportMdl << "Warning! There are more color indices than faces!\n";
                            nDataCounter = nDataMax;
                        }

                        if(!bFound) throw mdlexception("Error reading color indices data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bColors){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading color data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = true;
                        Color cColor;
                        if(ReadFloat(fConvert)) cColor.fR = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) cColor.fG = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) cColor.fB = fConvert;
                        else bFound = false;

                        if(bFound) node.Mesh.TempColors.push_back(std::move(cColor));
                        else throw mdlexception("Error reading color data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bConstraints){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading constraints data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Dangly.TempConstraints.push_back(fConvert);
                        else throw mdlexception("Error reading constraint data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bWeights){
                        //if(DEBUG_LEVEL > 3) ReportMdl << "Reading weights data" << "" << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        node.Skin.Bones[nCurrentIndex].nNodeNumber = nCurrentIndex;
                        bFound = true;
                        bool bPresent = false;
                        int z = 0;
                        int nBoneIndex = 0;
                        int nNodeNumber = 0;
                        Weight weight;
                        std::vector<int> & nWeightIndices = node.Skin.BoneBinaryOrderIndices;
                        while(z < 4){
                            //Get first name
                            if(!ReadUntilText(sID, false, true)) break;
                            std::string sCheck (sID);
                            std::transform(sCheck.begin(), sCheck.end(), sCheck.begin(), ::tolower);

                            if(sCheck == "root"){
                                if(!ReadFloat(fConvert)) throw mdlexception("Error reading weight data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. No weight value.");
                            }
                            else{
                                //if we found a name, loop through the name array to find our name index
                                for(nNodeNumber = 0; nNodeNumber < FH->MH.Names.size(); nNodeNumber++){
                                    //check if there is a match
                                    if(StringEqual(FH->MH.Names[nNodeNumber].sName, sID)){
                                        //We have found the name index for the current name, now we need to make sure this name has been indexed in the skin
                                        //Check if we already have this name indexed in the skin
                                        bPresent = false;
                                        for(nBoneIndex = 0; nBoneIndex < nWeightIndices.size() && !bPresent; ){
                                            if(nWeightIndices[nBoneIndex] == nNodeNumber){
                                                bPresent = true;
                                            }
                                            else nBoneIndex++;
                                        }
                                        if(!bPresent){
                                            //This is a new name index, so we need to add it to the skin's index list
                                            nWeightIndices.push_back(nNodeNumber);
                                            nBoneIndex = nWeightIndices.size() - 1; //Update nBoneIndex so it always points to the correct bone

                                            //We also add it to the bonemap.
                                            node.Skin.Bones[nNodeNumber].nBonemap = nBoneIndex;

                                            //We can just add it into the mdl's bone index list as well, what the heck
                                            if(nBoneIndex < 16){
                                                node.Skin.nBoneIndices[nBoneIndex] = nNodeNumber;
                                            }
                                            else Warning("Warning! A skin has more than 16 bones, which is the number of available slots in one of the lists. I do not know how this affects the game.");
                                        }
                                        //By here, we have gotten our nNodeNumber and nBoneIndex, and everything is indexed properly
                                        weight.nWeightIndex[z] = nBoneIndex;

                                        //Since we found the name, we don't need to keep looping anymore
                                        break;
                                    }
                                }
                                if(nNodeNumber == FH->MH.Names.size()){
                                    //we failed to find the name in the name array. This data is broken.
                                    ReportMdl << "Reading weights data: failed to find name in name array! Name: " << sID << ".\n";
                                    throw mdlexception("Error reading weight data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. Could not find bone '" + sID + "' by name.");
                                }
                                else{
                                    //We found the name in the name array. We are therefore ready to read the value as well.
                                    if(ReadFloat(fConvert)) weight.fWeightValue[z] = fConvert;
                                }
                            }
                            z++;
                        }
                        if(z < 1){
                            //This means we exited before writing a single piece of data
                            ReportMdl << "Didn't even find one name" << "" << ".\n";
                            ReportMdl << "DataCounter: " << nDataCounter << ". DataMax: " << nDataMax << ".\n";
                            throw mdlexception("Error reading weight data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. No weights specified for a vertex.");
                        }
                        else{
                            /// Here we assume that everything went fine, so we add the weight to our list
                            node.Skin.TempWeights.push_back(std::move(weight));
                        }
                    }
                    else if(bTextureNames){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false, true);
                        if(bFound){
                            Name name;
                            name.sName = sID;
                            node.Light.FlareTextureNames.push_back(name);
                        }
                        else throw mdlexception("Error reading flare texture names data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bFlareSizes){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Light.FlareSizes.push_back(fConvert);
                        else throw mdlexception("Error reading flare sizes data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bFlarePositions){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Light.FlarePositions.push_back(fConvert);
                        else throw mdlexception("Error reading flare positions data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bFlareColorShifts){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        float f1, f2, f3;
                        bFound = true;
                        if(ReadFloat(fConvert)) f1 = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) f2 = fConvert;
                        else bFound = false;
                        if(ReadFloat(fConvert)) f3 = fConvert;
                        else bFound = false;
                        if(bFound) node.Light.FlareColorShifts.push_back(Color(f1, f2, f3));
                        else throw mdlexception("Error reading flare color shifts data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    else if(bRoomLinks){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        Edge edge;
                        bFound = true;
                        if(ReadInt(nConvert)) edge.nIndex = nConvert;
                        else bFound = false;
                        if(ReadInt(nConvert)) edge.nTransition = nConvert;
                        else bFound = false;
                        if(bFound){
                            node.Mesh.Faces.at(edge.nIndex/3).nEdgeTransitions.at(edge.nIndex%3) = edge.nTransition;
                        }
                        else throw mdlexception("Error reading roomlink data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                    }
                    }
                    catch(const std::exception & e){
                        std::string sData;
                        if(bRoomLinks) sData = "room link";
                        else if(bVerts) sData = "vert";
                        else if(bFaces) sData = "face";
                        else if(bTverts) sData = "tvert";
                        else if(bTverts1) sData = "tvert1";
                        else if(bTverts2) sData = "tvert2";
                        else if(bTverts3) sData = "tvert3";
                        else if(bAabb) sData = "aabb";
                        else if(bWeights) sData = "weight";
                        else if(bConstraints) sData = "constraint";
                        else if(bEventlist) sData = "eventlist";
                        else if(bKeys) sData = "animation key";
                        else if(bTexIndices1) sData = "texindices1";
                        else if(bTexIndices2) sData = "texindices2";
                        else if(bTexIndices3) sData = "texindices3";
                        else if(bTextureNames) sData = "flare texture name";
                        else if(bFlareColorShifts) sData = "flare color shift";
                        else if(bFlarePositions) sData = "flare position";
                        else if(bFlareSizes) sData = "flare size";
                        throw mdlexception("Exception while reading " + sData + " data for node '" + FH->MH.Names.at(nCurrentIndex).sName + "': " + e.what());
                    }
                    nDataCounter++;
                    if(nDataCounter >= nDataMax && nDataMax != -1){
                        *lpbList = false;
                    }
                    SkipLine();
                }
            }
            //So no data. Find the next keyword then.
            else{
                //ReportMdl << "No data to read. Read keyword instead" << "" << ".\n";
                bFound = ReadUntilText(sID);
                std::string sLastThree;
                if(sID.length()> 3) sLastThree = sID.substr(sID.length()-3);
                else sLastThree = sID;
                if(!bFound) SkipLine(); //This will have already been done above, no need to look for it again
                else{
                    /// Main header stuff
                    if(sID == "newmodel"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        //Read the model name
                        bFound = ReadUntilText(sID, false);
                        if(!bFound){
                            ReportMdl << "ReadUntilText() Error! Model name is missing!\n";
                            throw mdlexception("No specified model name.");
                        }
                        else if(sID.size() > 32){
                            Error("Model name larger than the limit, 32 characters! Will truncate and continue.");
                            sID.resize(32);
                        }
                        else if(sID.size() > 16){
                            Warning("Model name larger than 16 characters! This may cause problems in the game.");
                        }
                        if(bFound) FH->MH.GH.sName = sID;

                        //Initialize all header information
                        FH->MH.vBBmin.fX = -5.0;
                        FH->MH.vBBmin.fY = -5.0;
                        FH->MH.vBBmin.fZ = -1.0;
                        FH->MH.vBBmax.fX = 5.0;
                        FH->MH.vBBmax.fY = 5.0;
                        FH->MH.vBBmax.fZ = 10.0;
                        FH->MH.fRadius = 7.0;
                        FH->MH.fScale = 1.0;
                        FH->MH.cSupermodelName = "NULL";
                        FH->MH.ArrayOfNodes.resize(FH->MH.Names.size());

                        SkipLine();
                    }
                    else if(sID == "setsupermodel"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        ReadUntilText(sID, false); //This should get us the model name first. I won't verify at this point.
                        bFound = ReadUntilText(sID, false);
                        if(!bFound){
                            ReportMdl << "ReadUntilText() Supermodel name is missing!\n";
                        }
                        else if(sID.size() > 32){
                            Error("Supermodel name larger than the limit, 32 characters! Will truncate and continue.");
                            sID.resize(32);
                        }
                        else if(sID.size() > 16){
                            Warning("Supermodel name larger than 16 characters! This may or may not cause problems in the game.");
                        }
                        if(bFound) FH->MH.cSupermodelName = sID;
                        SkipLine();
                    }
                    else if(sID == "classification"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFound = ReadUntilText(sID);
                        if(!bFound){
                            ReportMdl << "ReadUntilText() Classification is missing!\n";
                            throw mdlexception("No specified classification.");
                        }
                        if(sID == "other") FH->MH.nClassification = CLASS_OTHER;
                        else if(sID == "unknown"){
                            ReportMdl << "Corrected classification '" << sID << "' to classification 'other'.\n";
                            FH->MH.nClassification = CLASS_OTHER;
                        }
                        else if(sID == "effect") FH->MH.nClassification = CLASS_EFFECT;
                        else if(sID == "tile") FH->MH.nClassification = CLASS_TILE;
                        else if(sID == "character") FH->MH.nClassification = CLASS_CHARACTER;
                        else if(sID == "door") FH->MH.nClassification = CLASS_DOOR;
                        else if(sID == "placeable") FH->MH.nClassification = CLASS_PLACEABLE;
                        else if(sID == "lightsaber") FH->MH.nClassification = CLASS_LIGHTSABER;
                        else if(sID == "flyer") FH->MH.nClassification = CLASS_FLYER;
                        else if(bFound){
                            ReportMdl << "ReadUntilText() has found a classification token that we cannot interpret: " << sID << "\n";
                            throw mdlexception("Unknown classification '" + sID + "'.");
                        }
                        SkipLine();
                    }
                    else if(sID == "classification_unk1"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadInt(nConvert)) FH->MH.nSubclassification = nConvert;
                        SkipLine();
                    }
                    else if(sID == "ignorefog"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadInt(nConvert)) FH->MH.nAffectedByFog = nConvert ? 0 : 1;
                        SkipLine();
                    }
                    else if(sID == "setanimationscale"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadFloat(fConvert)) FH->MH.fScale = fConvert;
                        SkipLine();
                    }
                    else if(sID == "compress_quaternions"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadInt(nConvert)) FH->MH.bCompressQuaternions = (nConvert == 0) ? false : true;
                        SkipLine();
                    }
                    else if(sID == "headlink"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadInt(nConvert)) FH->MH.bHeadLink = (nConvert == 0) ? false : true;
                        SkipLine();
                    }
                    else if(sID == "beginmodelgeom"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bGeometry = true;
                        SkipLine();
                    }
                    else if(sID == "layoutposition" && bGeometry && nNode == 0){
                        if(ReadFloat(fConvert)) FH->MH.vLytPosition.fX = fConvert;
                        else throw mdlexception("Error reading layoutposition.");
                        if(ReadFloat(fConvert)) FH->MH.vLytPosition.fY = fConvert;
                        else throw mdlexception("Error reading layoutposition.");
                        if(ReadFloat(fConvert)) FH->MH.vLytPosition.fZ = fConvert;
                        else throw mdlexception("Error reading layoutposition.");
                        SkipLine();
                    }
                    else if(sID == "bmin" && bGeometry && nNode == 0){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadFloat(fConvert)) FH->MH.vBBmin.fX = fConvert;
                        else throw mdlexception("Error reading bmin.");
                        if(ReadFloat(fConvert)) FH->MH.vBBmin.fY = fConvert;
                        else throw mdlexception("Error reading bmin.");
                        if(ReadFloat(fConvert)) FH->MH.vBBmin.fZ = fConvert;
                        else throw mdlexception("Error reading bmin.");
                        SkipLine();
                    }
                    else if(sID == "bmax" && bGeometry && nNode == 0){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadFloat(fConvert)) FH->MH.vBBmax.fX = fConvert;
                        else throw mdlexception("Error reading bmax.");
                        if(ReadFloat(fConvert)) FH->MH.vBBmax.fY = fConvert;
                        else throw mdlexception("Error reading bmax.");
                        if(ReadFloat(fConvert)) FH->MH.vBBmax.fZ = fConvert;
                        else throw mdlexception("Error reading bmax.");
                        SkipLine();
                    }
                    else if(sID == "radius" && bGeometry && nNode == 0){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        if(ReadFloat(fConvert)) FH->MH.fRadius = fConvert;
                        SkipLine();
                    }
                    /// Common case for nodes, also for animation nodes
                    else if(sID == "node" && (bGeometry || bAnimation) && nNode == 0){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node node; //our new node

                        //Read type
                        int nType;
                        bFound = ReadUntilText(sID); //Get type
                        if(!bFound){
                            ReportMdl << "ReadUntilText() ERROR: a node is without any other specification.\n";
                            throw mdlexception("Error reading node. No specified type.");
                        }
                        if(sID == "dummy") nType = NODE_HEADER;
                        else if(sID == "light") nType = NODE_HEADER | NODE_LIGHT;
                        else if(sID == "emitter") nType = NODE_HEADER | NODE_EMITTER;
                        else if(sID == "reference") nType = NODE_HEADER | NODE_REFERENCE;
                        else if(sID == "trimesh") nType = NODE_HEADER | NODE_MESH;
                        else if(sID == "skin") nType = NODE_HEADER | NODE_MESH | NODE_SKIN;
                        else if(sID == "danglymesh") nType = NODE_HEADER | NODE_MESH | NODE_DANGLY;
                        else if(sID == "aabb") nType = NODE_HEADER | NODE_MESH | NODE_AABB;
                        else if(sID == "lightsaber") nType = NODE_HEADER | NODE_MESH | NODE_SABER;
                        else if(bFound){
                            ReportMdl << "ReadUntilText() has found some text (type?) that we cannot interpret: " << sID << "\n";
                            throw mdlexception("Error reading node. Invalid type '" + sID + "'." + sID + "'.");
                        }
                        if(bFound) node.Head.nType = nType;

                        //Read name
                        bFound = ReadUntilText(sID, false); //Name
                        if(!bFound){
                            ReportMdl << "ReadUntilText() ERROR: a node is without a name.\n";
                            throw mdlexception("Error reading node. No specified name.");
                        }
                        else{
                            node.Head.nNodeNumber = Mdl.GetNameIndex(sID);
                            if(node.Head.nNodeNumber == -1){
                                std::string sType = "unknown";
                                if(bGeometry) sType = "geometry";
                                else if(bAnimation) sType = "animation";
                                throw mdlexception("Error reading " + sType + " node. Name " + sID + " not found!");
                            }
                            node.Head.nSupernodeNumber = node.Head.nNodeNumber;
                        }

                        //Get animation number (automatically -1 if geo)
                        node.nAnimation = nAnimation;

                        //Initialize node <-- This is now mostly taken care of in the struct definitions, so no need for anything but the default values here.
                        if(nType & NODE_HEADER){
                            node.Head.vPos.Set(0.0, 0.0, 0.0);
                            node.Head.oOrient.SetQuaternion(0.0, 0.0, 0.0, 1.0);
                        }
                        if(nType & NODE_EMITTER){
                            node.Emitter.cDepthTextureName = "NULL";
                        }
                        if(nType & NODE_MESH){
                            node.Mesh.nSaberUnknown1 = 3;
                        }

                        //Finish up
                        nNode = nType;
                        if(bGeometry){
                            nCurrentIndex = node.Head.nNodeNumber;
                            FH->MH.ArrayOfNodes.at(node.Head.nNodeNumber) = std::move(node);
                        }
                        else if(bAnimation){
                            Animation & anim = FH->MH.Animations.back();
                            anim.ArrayOfNodes.push_back(std::move(node));
                        }
                        nNodeCounter++;
                        SkipLine();
                    }
                    else if(sID == "parent" && nNode & NODE_HEADER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFound = ReadUntilText(sID, false);
                        if(bGeometry){
                            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                            if(sID == "NULL") node.Head.nParentIndex = -1;
                            else if(bFound){
                                int nNodeNumber = 0;
                                //if we found a name, loop through the name array to find our name index
                                bFound = false;
                                while(nNodeNumber < FH->MH.Names.size() && !bFound){
                                    //check if there is a match
                                    if(StringEqual(FH->MH.Names[nNodeNumber].sName, sID)){
                                        //We have found the name index for the current name
                                        bFound = true;
                                    }
                                    else nNodeNumber++;
                                }
                                if(nNodeNumber == FH->MH.Names.size()){
                                    ReportMdl << "Failed to find parent.\n";
                                    throw mdlexception("Error reading parent name for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. The specified parent does not exist.");
                                }
                                else node.Head.nParentIndex = nNodeNumber;
                            }
                            else throw mdlexception("Error reading parent name for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. Parent name missing.");
                        }
                        else if(bAnimation){
                            Animation & anim = FH->MH.Animations.back();
                            Node & animnode = anim.ArrayOfNodes.back();
                            if(sID == "NULL") animnode.Head.nParentIndex = -1;
                            else if(bFound){
                                int nNodeNumber = 0;
                                //if we found a name, loop through the name array to find our name index
                                bFound = false;
                                while(nNodeNumber < FH->MH.Names.size() && !bFound){
                                    //check if there is a match
                                    if(StringEqual(FH->MH.Names[nNodeNumber].sName, sID)){
                                        //We have found the name index for the current name
                                        bFound = true;
                                    }
                                    else nNodeNumber++;
                                }
                                if(nNodeNumber == FH->MH.Names.size()){
                                    ReportMdl << "Failed to find parent.\n";
                                    throw mdlexception("Error reading parent name for node '" + FH->MH.Names.at(animnode.Head.nNodeNumber).sName + "'. The specified parent does not exist.");
                                }
                                else animnode.Head.nParentIndex = nNodeNumber;
                            }
                            else throw mdlexception("Error reading parent name for node '" + FH->MH.Names.at(animnode.Head.nNodeNumber).sName + "'. Parent name missing.");
                        }
                        SkipLine();
                    }
                    /// Now come the various node fields
                    /// For LIGHT
                    else if(sID == "lightpriority" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nLightPriority = nConvert;
                        SkipLine();
                    }
                    else if(sID == "shadow" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nShadow = nConvert;
                        SkipLine();
                    }
                    else if(sID == "affectdynamic" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nAffectDynamic = nConvert;
                        SkipLine();
                    }
                    else if(sID == "ndynamictype" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nDynamicType = nConvert;
                        SkipLine();
                    }
                    else if(sID == "ambientonly" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nAmbientOnly = nConvert;
                        SkipLine();
                    }
                    else if(sID == "fadinglight" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nFadingLight = nConvert;
                        SkipLine();
                    }
                    else if((sID == "lensflares" || sID == "flare") && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Light.nFlare = nConvert;
                        SkipLine();
                    }
                    else if(sID == "flareradius" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Light.fFlareRadius = fConvert;
                        SkipLine();
                    }
                    /// For EMITTER
                    else if(sID == "deadspace" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Emitter.fDeadSpace = fConvert;
                        SkipLine();
                    }
                    else if(sID == "blastlength" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Emitter.fBlastLength = fConvert;
                        SkipLine();
                    }
                    else if(sID == "blastradius" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Emitter.fBlastRadius = fConvert;
                        SkipLine();
                    }
                    else if(sID == "numbranches" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nBranchCount = nConvert;
                        SkipLine();
                    }
                    else if(sID == "controlptsmoothing" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Emitter.fControlPointSmoothing = fConvert;
                        SkipLine();
                    }
                    else if(sID == "xgrid" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nxGrid = nConvert;
                        SkipLine();
                    }
                    else if(sID == "ygrid" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nyGrid = nConvert;
                        SkipLine();
                    }
                    else if(sID == "spawntype" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nSpawnType = nConvert;
                        SkipLine();
                    }
                    else if(sID == "twosidedtex" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nTwosidedTex = nConvert;
                        SkipLine();
                    }
                    else if(sID == "loop" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nLoop = nConvert;
                        SkipLine();
                    }
                    else if(sID == "renderorder" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nRenderOrder = nConvert;
                        SkipLine();
                    }
                    else if(sID == "m_bframeblending" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Emitter.nFrameBlending = nConvert;
                        SkipLine();
                    }
                    else if(sID == "m_sdepthtexturename" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Depth texture name (Emitter) larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            node.Emitter.cDepthTextureName = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "update" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Update name (Emitter) larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            node.Emitter.cUpdate = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "render" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Render name (Emitter) larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            node.Emitter.cRender = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "blend" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Blend name (Emitter) larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            node.Emitter.cBlend = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "texture" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Texture name (Emitter) larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Texture name (Emitter) larger than 16 characters! This may cause problems in the game.");
                            }
                            node.Emitter.cTexture = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "chunkname" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 16){
                                Error("ChunkName larger than the limit, 16 characters! Will truncate and continue.");
                                sID.resize(16);
                            }
                            if(sID.c_str() != std::string("NULL")) node.Emitter.cChunkName = sID;
                            else node.Emitter.cChunkName = "";
                        }
                        SkipLine();
                    }
                    else if(sID == "p2p" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P;
                        }
                        SkipLine();
                    }
                    else if(sID == "p2p_sel" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P_SEL;
                        }
                        SkipLine();
                    }
                    else if(sID == "affectedbywind" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_AFFECTED_WIND;
                        }
                        SkipLine();
                    }
                    else if(sID == "m_istinted" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_TINTED;
                        }
                        SkipLine();
                    }
                    else if(sID == "bounce" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_BOUNCE;
                        }
                        SkipLine();
                    }
                    else if(sID == "random" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_RANDOM;
                        }
                        SkipLine();
                    }
                    else if(sID == "inherit" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT;
                        }
                        SkipLine();
                    }
                    else if(sID == "inheritvel" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_VEL;
                        }
                        SkipLine();
                    }
                    else if(sID == "inherit_local" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_LOCAL;
                        }
                        SkipLine();
                    }
                    else if(sID == "splat" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_SPLAT;
                        }
                        SkipLine();
                    }
                    else if(sID == "inherit_part" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_PART;
                        }
                        SkipLine();
                    }
                    else if(sID == "depth_texture" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_DEPTH_TEXTURE;
                        }
                        SkipLine();
                    }
                    else if(sID == "emitterflag13" && nNode & NODE_EMITTER){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_13;
                        }
                        SkipLine();
                    }
                    /// For REFERENCE
                    else if(sID == "refmodel" && nNode & NODE_REFERENCE){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID.size() > 32){
                                Error("Reference model name larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Reference model name larger than 16 characters! This may cause problems in the game.");
                            }
                            node.Reference.sRefModel = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "reattachable" && nNode & NODE_REFERENCE){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Reference.nReattachable = nConvert;
                        SkipLine();
                    }
                    /// For MESH
                    else if(sID == "bitmap" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID != "") node.Mesh.nTextureNumber++;
                            if(sID.size() > 32){
                                Error("Bitmap name larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Bitmap name larger than 16 characters! This may cause problems in the game.");
                            }
                            node.Mesh.cTexture1 = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "bitmap2" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID != "") node.Mesh.nTextureNumber++;
                            if(sID.size() > 32){
                                Error("Bitmap2 name larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Bitmap2 name larger than 16 characters! This may cause problems in the game.");
                            }
                            node.Mesh.cTexture2 = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "texture0" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID != "") node.Mesh.nTextureNumber++;
                            if(sID.size() > 12){
                                Error("Texture0 name larger than the limit, 12 characters! Will truncate and continue.");
                                sID.resize(12);
                            }
                            node.Mesh.cTexture3 = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "texture1" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            if(sID != "") node.Mesh.nTextureNumber++;
                            if(sID.size() > 12){
                                Error("Texture1 name larger than the limit, 12 characters! Will truncate and continue.");
                                sID.resize(12);
                            }
                            node.Mesh.cTexture4 = sID;
                        }
                        SkipLine();
                    }
                    else if(sID == "lightmap" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        bFound = ReadUntilText(sID, false);
                        if(bFound){
                            bMagnusll = true;
                            if(sID != "") node.Mesh.nTextureNumber++;
                            if(sID.size() > 32){
                                Error("Lightmap name larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Lightmap name larger than 16 characters! This may cause problems in the game.");
                            }
                            node.Mesh.cTexture2 = sID;
                            node.Mesh.nHasLightmap = 1; //Do this if we're using magnusII's version, cuz we won't have it separately
                        }
                        SkipLine();
                    }
                    else if(sID == "diffuse" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fR = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fG = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fB = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        SkipLine();
                    }
                    else if(sID == "ambient" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fAmbient.fR = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        if(ReadFloat(fConvert)) node.Mesh.fAmbient.fG = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        if(ReadFloat(fConvert)) node.Mesh.fAmbient.fB = fConvert;
                        else throw mdlexception("Error reading '" + sID + "' for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        SkipLine();
                    }
                    else if(sID == "tangentspace" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)){
                            node.Mesh.TangentSpace.at(0) = (nConvert != 0);
                            if(ReadInt(nConvert)){
                                node.Mesh.TangentSpace.at(1) = (nConvert != 0);
                                if(ReadInt(nConvert)){
                                    node.Mesh.TangentSpace.at(2) = (nConvert != 0);
                                    if(ReadInt(nConvert)){
                                        node.Mesh.TangentSpace.at(3) = (nConvert != 0);
                                    }
                                }
                            }
                        }
                        SkipLine();
                    }
                    else if(sID == "bumpmapped_texture"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFound = ReadUntilText(sID, false);
                        if(bFound) sBumpmapped.push_back(sID);
                        SkipLine();
                    }
                    else if(sID == "lightmapped" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nHasLightmap = nConvert;
                        SkipLine();
                    }
                    else if(sID == "rotatetexture" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nRotateTexture = nConvert;
                        SkipLine();
                    }
                    else if(sID == "m_bisbackgroundgeometry" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nBackgroundGeometry = nConvert;
                        SkipLine();
                    }
                    else if(sID == "dirt_enabled" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nDirtEnabled = nConvert;
                        SkipLine();
                    }
                    else if(sID == "dirt_texture" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nDirtTexture = nConvert;
                        SkipLine();
                    }
                    else if(sID == "dirt_worldspace" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nDirtCoordSpace = nConvert;
                        SkipLine();
                    }
                    else if(sID == "hologram_donotdraw" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nHideInHolograms = nConvert;
                        SkipLine();
                    }
                    else if(sID == "beaming" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nBeaming = nConvert;
                        SkipLine();
                    }
                    else if(sID == "render" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nRender = nConvert;
                        SkipLine();
                    }
                    else if(sID == "shadow" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nShadow = nConvert;
                        SkipLine();
                    }
                    else if(sID == "transparencyhint" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nTransparencyHint = nConvert;
                        SkipLine();
                    }
                    else if(sID == "inv_count" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(nNode & NODE_SABER){
                            if(ReadInt(nConvert)) node.Saber.nInvCount1 = nConvert;
                            if(ReadInt(nConvert)) node.Saber.nInvCount2 = nConvert;
                        }
                        else if(ReadInt(nConvert)) node.Mesh.nMeshInvertedCounter = nConvert;
                        SkipLine();
                    }
                    else if(sID == "animateuv" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) node.Mesh.nAnimateUV = nConvert;
                        SkipLine();
                    }
                    else if(sID == "uvdirectionx" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fUVDirectionX = fConvert;
                        SkipLine();
                    }
                    else if(sID == "uvdirectiony" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fUVDirectionY = fConvert;
                        SkipLine();
                    }
                    else if(sID == "uvjitter" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fUVJitter = fConvert;
                        SkipLine();
                    }
                    else if(sID == "uvjitterspeed" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Mesh.fUVJitterSpeed = fConvert;
                        SkipLine();
                    }
                    /// For DANGLY
                    else if(sID == "displacement" && nNode & NODE_DANGLY){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Dangly.fDisplacement = fConvert;
                        SkipLine();
                    }
                    else if(sID == "tightness" && nNode & NODE_DANGLY){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Dangly.fTightness = fConvert;
                        SkipLine();
                    }
                    else if(sID == "period" && nNode & NODE_DANGLY){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadFloat(fConvert)) node.Dangly.fPeriod = fConvert;
                        SkipLine();
                    }
                    /// Next we have the DATA LISTS
                    else if(sID == "verts" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bVerts = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "faces" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFaces = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "tverts" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTverts = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "texindices1" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTexIndices1 = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "lightmaptverts" && nNode & NODE_MESH || sID == "tverts1" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTverts1 = true;
                        if(sID == "lightmaptverts") bMagnusll = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "texindices2" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTexIndices2 = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "tverts2" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTverts2 = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "texindices3" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTexIndices3 = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "tverts3" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTverts3 = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "colorindices" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bColorIndices = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "colors" && nNode & NODE_MESH){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bColors = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "weights" && nNode & NODE_SKIN){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bWeights = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        node.Skin.Bones.resize(FH->MH.Names.size());
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "aabb" && nNode & NODE_AABB){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bAabb = true;
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        int nSavePos = nPosition; //Save position
                        nDataMax = node.Mesh.Faces.size() * 2 - 1;
                        nDataCounter = 0;
                        if(ReadFloat(fConvert)) nPosition = nSavePos; //The data starts in this line, revert position
                        else SkipLine(); //if not, then data starts next line, so it is okay to skip
                    }
                    else if(sID == "roomlinks" && nNode & NODE_AABB){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bRoomLinks = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "constraints" && nNode & NODE_DANGLY){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bConstraints = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "texturenames" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bTextureNames = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "flaresizes" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFlareSizes = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "flarepositions" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFlarePositions = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "flarecolorshifts" && nNode & NODE_LIGHT){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFlareColorShifts = true;
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        SkipLine();
                    }
                    /// Extra (controllerless) data
                    else if(sID == "extra_data" && nAnimation >= 0){
                        Animation & anim = FH->MH.Animations.back();
                        Node & node = anim.ArrayOfNodes.back();
                        int nData = 0;
                        if(ReadInt(nConvert)) nData = nConvert;
                        node.Head.ControllerData.resize(nData, 0.0);
                        SkipLine();
                    }
                    /// First, controllerless controller data
                    else if(safesubstr(sID, sID.length()-3) == "key" && safesubstr(sID, 0, 15) == "controllerless_"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Animation & anim = FH->MH.Animations.back();
                        Node & node = anim.ArrayOfNodes.back();
                        Node & geonode = Mdl.GetNodeByNameIndex(node.Head.nNodeNumber);
                        Location loc = geonode.GetLocation();

                        int nControllerType = ReturnController(safesubstr(sID, 15, sID.length()-18), nNode);
                        int nTimekeyStart = node.Head.ControllerData.size();
                        int nColumnCount = 0;
                        if(nControllerType == CONTROLLER_HEADER_POSITION) nColumnCount = 3;
                        else if(nControllerType == CONTROLLER_HEADER_ORIENTATION) nColumnCount = 4;
                        else throw mdlexception("Invalid controllerless controller data type!");

                        //This is all we can tell right now.
                        int nSavePos = nPosition; //Save position

                        //Now let's get the row count. It is actually best to also read the timekeys in this step
                        SkipLine();
                        nDataCounter = 0;
                        bFound = true;
                        int nSavePos2;
                        while(bFound && (nDataMax != 0 || nDataCounter < nDataMax)){
                            if(EmptyRow()) SkipLine();
                            else{
                                nSavePos2 = nPosition;
                                ReadUntilText(sID);
                                if(sID=="endlist") bFound = false;
                                else{
                                    nPosition = nSavePos2;
                                    if(!EmptyRow()){
                                        if(ReadFloat(fConvert)){
                                            node.Head.ControllerData.push_back(fConvert);
                                        }
                                        else{
                                            ReportMdl << "This is not a float: " << sID << ".\n";
                                            throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. This is not a float: " + sID + ".");
                                        }
                                        nDataCounter++;
                                    }
                                    SkipLine();
                                }
                            }
                        }
                        if(nDataCounter == 0){
                            ReportMdl << "keyed controller error: no data at all in the first line after the token line.\n";
                            throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        }
                        int nValueCount = nDataCounter;
                        int nDataStart = node.Head.ControllerData.size();

                        //We now have all the data, reset the position and prepare for actually reading the keys.
                        nPosition = nSavePos;
                        for(int dc = 0; dc < nValueCount; dc++){
                            SkipLine();

                            /// First read the time
                            if(!ReadFloat(fConvert)) throw mdlexception("Controllerless controller data reading error, time not a valid float!");

                            if(nControllerType == CONTROLLER_HEADER_ORIENTATION){
                                double fX, fY, fZ, fA;
                                if(ReadFloat(fConvert)) fX = fConvert;
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) fY = fConvert;
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) fZ = fConvert;
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) fA = fConvert;
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                Orientation NewOrientKey;
                                NewOrientKey.SetAxisAngle(fX, fY, fZ, fA);
                                ByteBlock4.ui = CompressQuaternion(NewOrientKey.GetQuaternion());
                                node.Head.ControllerData.push_back(ByteBlock4.f);
                            }
                            else if(nControllerType == CONTROLLER_HEADER_POSITION){
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fX);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fY);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                                if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fZ);
                                else throw mdlexception("Error reading animation key data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            }
                        }

                        SkipLine();
                    }
                    /// Next we have bezier controllers
                    else if(safesubstr(sID, sID.length()-9) == "bezierkey" && ReturnController(safesubstr(sID, 0, sID.length()-9), nNode)){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Animation & anim = FH->MH.Animations.back();
                        Node & node = anim.ArrayOfNodes.back();
                        Controller ctrl;
                        ctrl.nAnimation = nAnimation;
                        ctrl.nNodeNumber = node.Head.nNodeNumber;
                        ctrl.nControllerType = ReturnController(safesubstr(sID, 0, sID.length()-9), nNode);
                        ctrl.nTimekeyStart = node.Head.ControllerData.size();
                        ctrl.nUnknown2 = -1;
                        if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                           ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                            ctrl.nUnknown2 = ctrl.nControllerType + 8;
                        }

                        //This is all we can tell right now.
                        int nSavePos = nPosition; //Save position

                        //To continue let's first get the column count
                        nDataMax = -1;
                        if(!EmptyRow()){
                            if(ReadInt(nConvert)) nDataMax = nConvert;
                            else{
                                ReportMdl << "Something weird is going on after the controller keyword.\n";
                                throw mdlexception("Error reading bezier keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. The expected count after the controller name is not an integer.");
                            }
                        }
                        SkipLine();
                        nDataCounter = 0;
                        bFound = true;
                        while(bFound && nDataMax != 0){
                            if(!EmptyRow() || nDataCounter > 0){
                                if(ReadFloat(fConvert, &sID)){
                                    nDataCounter++;
                                }
                                else{
                                    ReportMdl << "This is not a float: " << sID << ".\n";
                                    throw mdlexception("Error reading bezier keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. This is not a float: " + sID + ".");
                                }
                                if(EmptyRow()) bFound = false;
                            }
                            else SkipLine();
                        }
                        if(nDataCounter == 0){
                            ReportMdl << "keyed controller error: no data at all in the first line after the token line.\n";
                            throw mdlexception("Error reading bezier keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. No data.");
                        }

                        ctrl.nColumnCount = 16 + (nDataCounter - 1) / 3;
                        //ReportMdl << "Column count for " << sID << " is " << nDataCounter - 1 << "\n";

                        //Now let's get the row count. It is actually best to also read the timekeys in this step
                        nPosition = nSavePos;
                        SkipLine();
                        nDataCounter = 0;
                        bFound = true;
                        int nSavePos2;
                        while(bFound && (nDataMax < 0 || nDataCounter < nDataMax)){
                            //ReportMdl << "Looking.. Position=" << nPosition << ".\n";
                            if(EmptyRow()) SkipLine();
                            else{
                                nSavePos2 = nPosition;
                                ReadUntilText(sID);
                                if(sID=="endlist") bFound = false;
                                else{
                                    nPosition = nSavePos2;
                                    if(!EmptyRow()){
                                        if(ReadFloat(fConvert)){
                                            node.Head.ControllerData.push_back(fConvert);
                                        }
                                        else{
                                            ReportMdl << "This is not a float: " << sID << ".\n";
                                            throw mdlexception("Error reading bezier keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. This is not a float: " + sID + ".");

                                        }
                                        nDataCounter++;
                                    }
                                    SkipLine();
                                }
                            }
                        }
                        if(nDataCounter == 0){
                            ReportMdl << "keyed controller error: no data at all in the first line after the token line.\n";
                            throw mdlexception("Error reading bezier keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. no data.");
                        }
                        ctrl.nValueCount = nDataCounter;
                        ctrl.nDataStart = node.Head.ControllerData.size();

                        //We now have all the data, append the controller, reset the position and prepare for actually reading the keys.
                        nPosition = nSavePos;
                        node.Head.Controllers.push_back(ctrl);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        bKeys = true;
                        SkipLine();
                    }
                    /// Next we have keyed controllers
                    else if(safesubstr(sID, sID.length()-3) == "key" && ReturnController(safesubstr(sID, 0, sID.length()-3), nNode)){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Animation & anim = FH->MH.Animations.back();
                        Node & node = anim.ArrayOfNodes.back();
                        Controller ctrl;
                        ctrl.nAnimation = nAnimation;
                        ctrl.nNodeNumber = node.Head.nNodeNumber;
                        ctrl.nControllerType = ReturnController(safesubstr(sID, 0, sID.length()-3), nNode);
                        ctrl.nTimekeyStart = node.Head.ControllerData.size();
                        ctrl.nUnknown2 = -1;
                        if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                           ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                            ctrl.nUnknown2 = ctrl.nControllerType + 8;
                        }

                        //This is all we can tell right now.
                        int nSavePos = nPosition; //Save position

                        //To continue let's first get the column count
                        nDataMax = -1;
                        if(!EmptyRow()){
                            if(ReadInt(nConvert)) nDataMax = nConvert;
                            else{
                                ReportMdl << "Something weird is going on after the controller keyword.\n";
                                throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. The expected count after the controller name is not an integer.");
                            }
                        }
                        SkipLine();
                        nDataCounter = 0;
                        bFound = true;
                        while(bFound && nDataMax != 0){
                            if(!EmptyRow() || nDataCounter > 0){
                                if(ReadFloat(fConvert, &sID)){
                                    nDataCounter++;
                                }
                                else{
                                    ReportMdl << "This is not a float: " << sID << ".\n";
                                    throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. This is not a float: " + sID + ".");
                                }
                                if(EmptyRow()) bFound = false;
                            }
                            else SkipLine();
                        }
                        if(nDataCounter == 0){
                            ReportMdl << "keyed controller error: no data at all in the first line after the token line.\n";
                            throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. No data.");
                        }

                        ctrl.nColumnCount = nDataCounter - 1;
                        if(FH->MH.bCompressQuaternions && ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION) ctrl.nColumnCount = 2;
                        //ReportMdl << "Column count for " << sID << " is " << nDataCounter - 1 << "\n";

                        //Now let's get the row count. It is actually best to also read the timekeys in this step
                        nPosition = nSavePos;
                        SkipLine();
                        nDataCounter = 0;
                        bFound = true;
                        int nSavePos2;
                        while(bFound && (nDataMax < 0 || nDataCounter < nDataMax)){
                            if(EmptyRow()) SkipLine();
                            else{
                                nSavePos2 = nPosition;
                                ReadUntilText(sID);
                                if(sID=="endlist") bFound = false;
                                else{
                                    nPosition = nSavePos2;
                                    if(!EmptyRow()){
                                        if(ReadFloat(fConvert)){
                                            node.Head.ControllerData.push_back(fConvert);
                                        }
                                        else{
                                            ReportMdl << "This is not a float: " << sID << ".\n";
                                            throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'. This is not a float: " + sID + ".");
                                        }
                                        nDataCounter++;
                                    }
                                    SkipLine();
                                }
                            }
                        }
                        if(nDataCounter == 0){
                            ReportMdl << "keyed controller error: no data at all in the first line after the token line.\n";
                            throw mdlexception("Error reading keyed controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                        }
                        ctrl.nValueCount = nDataCounter;
                        ctrl.nDataStart = node.Head.ControllerData.size();

                        //We now have all the data, append the controller, reset the position and prepare for actually reading the keys.
                        nPosition = nSavePos;
                        node.Head.Controllers.push_back(ctrl);
                        if(ReadInt(nConvert)) nDataMax = nConvert;
                        else nDataMax = -1;
                        nDataCounter = 0;
                        bKeys = true;
                        SkipLine();
                    }
                    /// Next we have single controllers
                    else if(ReturnController(sID, nNode)){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";

                        Node & node = nAnimation > -1 ? FH->MH.Animations.back().ArrayOfNodes.back() : FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        Controller ctrl;
                        ctrl.nControllerType = ReturnController(sID, nNode);
                        ctrl.nUnknown2 = -1;
                        if(nAnimation > -1 &&
                           ( ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                             ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION ) ){
                            ctrl.nUnknown2 = ctrl.nControllerType + 8;
                        }
                        ctrl.nTimekeyStart = node.Head.ControllerData.size();
                        ctrl.nDataStart = node.Head.ControllerData.size() + 1;
                        ctrl.nValueCount = 1;
                        ctrl.nNodeNumber = node.Head.nNodeNumber;
                        ctrl.nAnimation = nAnimation;

                        //First put in the 0.0 to fill the required timekey
                        node.Head.ControllerData.push_back(0.0);

                        //Now fill values
                        if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                            double fX, fY, fZ, fAngle;
                            if(ReadFloat(fConvert)) fX = fConvert;
                            else throw mdlexception("Error reading single orientation controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fY = fConvert;
                            else throw mdlexception("Error reading single orientation controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fZ = fConvert;
                            else throw mdlexception("Error reading single orientation controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            if(ReadFloat(fConvert)) fAngle = fConvert;
                            else throw mdlexception("Error reading single orientation controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            node.Head.oOrient.SetAxisAngle(fX, fY, fZ, fAngle);
                            node.Head.ControllerData.push_back(node.Head.oOrient.GetQuaternion().vAxis.fX);
                            node.Head.ControllerData.push_back(node.Head.oOrient.GetQuaternion().vAxis.fY);
                            node.Head.ControllerData.push_back(node.Head.oOrient.GetQuaternion().vAxis.fZ);
                            node.Head.ControllerData.push_back(node.Head.oOrient.GetQuaternion().fW);
                            ctrl.nColumnCount = 4;
                        }
                        else{
                            nDataCounter = 0;
                            bFound = true;
                            while(bFound){
                                if(ReadFloat(fConvert)){
                                    nDataCounter++;
                                    node.Head.ControllerData.push_back(fConvert);
                                }
                                else bFound = false;
                            }
                            if(nDataCounter == 0){
                                ReportMdl << "Single controller error: no data at all.\n";
                                throw mdlexception("Error reading single controller data for node '" + FH->MH.Names.at(node.Head.nNodeNumber).sName + "'.");
                            }
                            ctrl.nColumnCount = nDataCounter;

                            if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                                node.Head.vPos.fX = node.Head.ControllerData.at(ctrl.nDataStart + 0);
                                node.Head.vPos.fY = node.Head.ControllerData.at(ctrl.nDataStart + 1);
                                node.Head.vPos.fZ = node.Head.ControllerData.at(ctrl.nDataStart + 2);
                            }
                        }

                        node.Head.Controllers.push_back(ctrl);
                        SkipLine();
                    }
                    /// General ending tokens
                    else if(sID == "endnode" && nNode > 0){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bMagnusll = false;
                        nNode = 0;
                        nCurrentIndex = -1;
                        SkipLine();
                    }
                    else if(sID == "endmodelgeom"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bGeometry = false;
                        FH->MH.nNodeCount = nNodeCounter;
                        nNodeCounter = 0;
                        SkipLine();

                    }
                    /// Animation tokens
                    else if(sID == "newanim" && !bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bAnimation = true;
                        nAnimation++;

                        //Get Animation Name
                        bFound = ReadUntilText(sID, false); //Name
                        if(!bFound){
                            ReportMdl << "ReadUntilText() ERROR: an animation name is missing!\n";
                            throw mdlexception("No animation name has been found after a 'newanim' token.");
                        }
                        else if(sID.size() > 32){
                            Error("Animation name larger than the limit, 32 characters! Will truncate and continue.");
                            sID.resize(32);
                        }
                        else if(sID.size() > 16){
                            Warning("Animation name larger than 16 characters! This may cause problems in the game.");
                        }
                        Animation anim;
                        if(bFound) anim.sName = sID;

                        //Initialize animation in case something is left undefined
                        anim.sAnimRoot = FH->MH.Names.front().sName;
                        anim.fLength = 0.0;
                        anim.fTransition = 0.0;
                        anim.EventArray.nCount = 0;
                        anim.Events.resize(0);
                        anim.ArrayOfNodes.reserve(FH->MH.Names.size());

                        //Finish up
                        if(bFound) FH->MH.Animations.push_back(anim);
                        SkipLine();
                    }
                    else if(sID == "length" && bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Animation & anim = FH->MH.Animations.back();
                        if(ReadFloat(fConvert)) anim.fLength = fConvert;
                        SkipLine();
                    }
                    else if(sID == "transtime" && bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        Animation & anim = FH->MH.Animations.back();
                        if(ReadFloat(fConvert)) anim.fTransition = fConvert;
                        SkipLine();
                    }
                    else if(sID == "animroot" && bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bFound = ReadUntilText(sID, false);
                        if(!bFound){
                            ReportMdl << "ReadUntilText() ERROR: an animation's AnimRoot name is missing.\n";
                        }
                        else if(sID.size() > 32){
                            Error("AnimRoot name larger than the limit, 32 characters! Will truncate and continue.");
                            sID.resize(32);
                        }
                        else if(sID.size() > 16){
                            Warning("AnimRoot name larger than 16 characters! This may or may not cause problems in the game.");
                        }
                        Animation & anim = FH->MH.Animations.back();
                        if(bFound) anim.sAnimRoot = sID;
                        SkipLine();
                    }
                    else if(sID == "eventlist" && bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bEventlist = true;
                        SkipLine();
                    }
                    else if(sID == "event" && bAnimation){
                        Event sound; //New sound
                        if(ReadFloat(fConvert)){
                            sound.fTime = fConvert;
                            bFound = ReadUntilText(sID, false);
                            if(!bFound){
                                ReportMdl << "ReadUntilText() Event name is missing!\n";
                            }
                            else if(sID.size() > 32){
                                Error("Event name larger than the limit, 32 characters! Will truncate and continue.");
                                sID.resize(32);
                            }
                            else if(sID.size() > 16){
                                Warning("Event name larger than 16 characters! This may cause problems in the game.");
                            }
                            sound.sName = sID;
                            if(bFound){
                                Animation & anim = FH->MH.Animations.back();
                                anim.Events.push_back(sound);
                            }
                        }
                        SkipLine();
                    }
                    else if(sID == "doneanim" && bAnimation){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bAnimation = false;
                        nNodeCounter = 0;
                        SkipLine();
                    }
                    else if(sID == "donemodel"){
                        if(DEBUG_LEVEL > 3) ReportMdl << "Reading " << sID << ".\n";
                        bEnd = true;
                    }
                    //These are the names we should ignore, because we expect them to appear, but we have no use for them
                    else if(sID == "multimaterial"){
                        if(ReadInt(nConvert)){
                            SkipLine();
                            for(int nj = 0; nj < nConvert; nj++){
                                SkipLine();
                            }
                        }
                        else SkipLine();
                    }
                    else if(sID == "filedependancy") SkipLine();
                    else if(sID == "specular") SkipLine();
                    else if(sID == "wirecolor") SkipLine();
                    else if(sID == "shininess") SkipLine();
                    else if(sID == "name") SkipLine();
                    else if(sID == "inheritcolor") SkipLine();
                    else if(sID == "tilefade") SkipLine();
                    else if(sID == "center") SkipLine();
                    else if(sID == "bmin") SkipLine();
                    else if(sID == "bmax") SkipLine();
                    else if(sID == "radius") SkipLine();
                    else if(sID == "average") SkipLine();
                    else{
                        ReportMdl << "ReadUntilText() has found some text that we cannot interpret: " << sID << "\n";
                        SkipLine();
                    }
                }
            }
        }
        ReportMdl << "Done reading ascii, checking for errors...\n";
    }
    catch(const std::exception & e){
        Error("An exception has occurred while reading the ascii model:\n\n" + std::string(e.what()) + "\n\nThe program will now cleanup what it has read since the data is now broken.");
        return false;
    }
    catch(...){
        Error("An unknown exception has occurred while reading the ascii model!\n\nThe program will now cleanup what it has read since the data is now broken.");
        return false;
    }

    /// Implementation of 'bumpmapped_texture'
    FileHeader & Data = *FH;
    for(int s = 0; s < sBumpmapped.size(); s++){
        for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
            //ReportMdl << "Checking node\n";
            Node & node = Data.MH.ArrayOfNodes.at(n);
            if(node.Head.nType & NODE_MESH && !(node.Head.nType & NODE_AABB) && !(node.Head.nType & NODE_SABER)){
                if(std::string(node.Mesh.cTexture1.c_str()) != "" && std::string(node.Mesh.cTexture1.c_str()) != "NULL"){
                    if(sBumpmapped.at(s) == node.Mesh.cTexture1.c_str()){
                        node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_TANGENT1;
                    }
                }
                if(node.Mesh.cTexture2.c_str() != std::string("") && node.Mesh.cTexture2.c_str() != std::string("NULL")){
                    if(sBumpmapped.at(s) == node.Mesh.cTexture2.c_str()){
                        node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_TANGENT2;
                    }
                }
                if(node.Mesh.cTexture3.c_str() != std::string("") && node.Mesh.cTexture3.c_str() != std::string("NULL")){
                    if(sBumpmapped.at(s) == node.Mesh.cTexture3.c_str()){
                        node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_TANGENT3;
                    }
                }
                if(node.Mesh.cTexture4.c_str() != std::string("") && node.Mesh.cTexture4.c_str() != std::string("NULL")){
                    if(sBumpmapped.at(s) == node.Mesh.cTexture4.c_str()){
                        node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_TANGENT4;
                    }
                }
            }
        }
    }

    Mdl.AsciiPostProcess();
    return true;
}

void MDL::GatherChildren(Node & node, std::vector<Node> & ArrayOfNodes, Vector vFromRoot){
    node.Head.ChildIndices.resize(0); //Reset child array
    node.Head.ChildIndices.reserve(ArrayOfNodes.size());

    if(node.Head.nType & NODE_MESH){
        /// Let's do the transformations/translations here. First orientation, then translation.
        Location loc = node.GetLocation();
        vFromRoot.Rotate(loc.oOrientation.GetQuaternion());
        vFromRoot += loc.vPosition;

        node.Head.vFromRoot = vFromRoot;
    }

    /*
    //Update the animation node IDs
    if(node.nAnimation != -1){
        node.Head.nSupernodeNumber = GetNodeByNameIndex(node.Head.nNodeNumber).Head.nSupernodeNumber;
        //if(node.Head.nSupernodeNumber == -1) node.Head.nSupernodeNumber = node.Head.nNodeNumber;
    }
    */

    for(int n = 0; n < ArrayOfNodes.size(); n++){
        if(ArrayOfNodes[n].Head.nParentIndex == node.Head.nNodeNumber){
            //The nodes with this index is a child, adopt it
            //node.Head.Children.push_back(ArrayOfNodes[n]);
            node.Head.ChildIndices.push_back(ArrayOfNodes[n].Head.nNodeNumber);
            GatherChildren(ArrayOfNodes[n], ArrayOfNodes, vFromRoot);
        }
    }
    node.Head.ChildIndices.shrink_to_fit();
}

void GetSupernodes(ModelHeader & MH, ModelHeader & superMH, int & nHighest, int & nTotalSupermodelNodes, int nNodeCurrent, int nSupernodeCurrent){
    if(nNodeCurrent < 0) return;
    Node & node = MH.ArrayOfNodes.at(nNodeCurrent);

    if(nSupernodeCurrent == -1){
        node.Head.nSupernodeNumber = nHighest;
        nHighest++;
        for(int n = 0; n < node.Head.ChildIndices.size(); n++){
            GetSupernodes(MH, superMH, nHighest, nTotalSupermodelNodes, node.Head.ChildIndices.at(n), -2);
        }
    }
    else if(nSupernodeCurrent == -2){
        node.Head.nSupernodeNumber += nTotalSupermodelNodes + 1;
        for(int n = 0; n < node.Head.ChildIndices.size(); n++){
            GetSupernodes(MH, superMH, nHighest, nTotalSupermodelNodes, node.Head.ChildIndices.at(n), -2);
        }
    }
    else{
        Node & supernode = superMH.ArrayOfNodes.at(nSupernodeCurrent);
        node.Head.nSupernodeNumber = supernode.Head.nSupernodeNumber;
        for(int n = 0; n < node.Head.ChildIndices.size(); n++){
            bool bFound = false;
            std::string sNodeName = MH.Names.at(node.Head.ChildIndices.at(n)).sName.c_str();
            std::transform(sNodeName.begin(), sNodeName.end(), sNodeName.begin(), ::tolower);
            for(int n2 = 0; n2 < supernode.Head.ChildIndices.size() && !bFound; n2++){
                std::string sSupernodeName = superMH.Names.at(supernode.Head.ChildIndices.at(n2)).sName.c_str();
                std::transform(sSupernodeName.begin(), sSupernodeName.end(), sSupernodeName.begin(), ::tolower);
                if(sNodeName == sSupernodeName){
                    bFound = true;
                    GetSupernodes(MH, superMH, nHighest, nTotalSupermodelNodes, node.Head.ChildIndices.at(n), supernode.Head.ChildIndices.at(n2));
                }
            }
            if(!bFound) GetSupernodes(MH, superMH, nHighest, nTotalSupermodelNodes, node.Head.ChildIndices.at(n), -1);
        }
    }
}

void MDL::AsciiPostProcess(){
    ReportObject ReportMdl(*this);
    ReportMdl << "Ascii post-processing...\n";
    Report("Post-processing imported ASCII...");
    FileHeader & Data = *FH;

    /// PART 0 ///
    /// Get rid of the duplication marks
    for(int n = 0; n < Data.MH.Names.size(); n++){
        std::string & sNode = Data.MH.Names.at(n).sName;
        if(sNode.find("__dpl") != std::string::npos){
            sNode.resize(sNode.find("__dpl"));
        }
    }

    /// PART 1 ///
    /// Gather all the children (the indices!!!)
    /// This part means going from every node only specifying its parent to every node also specifying its children
    // 1. Gather children for animations
    for(int i = 0; i < Data.MH.Animations.size(); i++){
        Animation & anim = Data.MH.Animations[i];
        for(int n = 0; n < anim.ArrayOfNodes.size(); n++){
            if(anim.ArrayOfNodes[n].Head.nParentIndex == -1){
                //anim.RootAnimationNode = anim.ArrayOfNodes[n];
                GatherChildren(anim.ArrayOfNodes[n], anim.ArrayOfNodes, Vector());
                n = anim.ArrayOfNodes.size();
            }
        }
    }
    // 2. Gather children for geometry
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        if(Data.MH.ArrayOfNodes[n].Head.nParentIndex == -1){
            //Data.MH.RootNode = Data.MH.ArrayOfNodes[n];
            GatherChildren(Data.MH.ArrayOfNodes[n], Data.MH.ArrayOfNodes, Vector());
            n = Data.MH.ArrayOfNodes.size();
        }
    }

    /// PART 2 ///
    /// Do supernodes
    /// This loads up all the supermodels and calculates the supernode numbers
    Data.MH.GH.nTotalNumberOfNodes = Data.MH.nNodeCount;
    if(Data.MH.cSupermodelName != "NULL" && Data.MH.cSupermodelName != ""){
        std::unique_ptr<MDL> Supermodel;
        LoadSupermodel(*this, Supermodel);
        //First, update the TotalNodeCount
        if(Supermodel){
            int nTotalSupermodelNodes = Supermodel->GetFileData()->MH.GH.nTotalNumberOfNodes;
            ReportMdl << "Total Supermodel Nodes: " << nTotalSupermodelNodes << "\n";
            if(nTotalSupermodelNodes > 0)
                Data.MH.GH.nTotalNumberOfNodes += 1 + nTotalSupermodelNodes;

            //Next we need the largest supernode number.
            int nMaxSupernode = 0;
            for(int n = 0; n < Supermodel->GetFileData()->MH.ArrayOfNodes.size(); n++){
                nMaxSupernode = std::max(nMaxSupernode, (int) Supermodel->GetFileData()->MH.ArrayOfNodes.at(n).Head.nSupernodeNumber);
            }
            int nCurrentSupernode = nMaxSupernode + 1;

            GetSupernodes(Data.MH, Supermodel->GetFileData()->MH, nCurrentSupernode, nTotalSupermodelNodes, 0, 0);
        }
    }
    /// Apply supernode numbers to anim nodes
    for(int an = 0; an < Data.MH.Animations.size(); an++){
        Animation & anim = Data.MH.Animations.at(an);
        for(int ann = 0; ann < anim.ArrayOfNodes.size(); ann++){
            Node & anim_node = anim.ArrayOfNodes.at(ann);
            anim_node.Head.nSupernodeNumber = GetNodeByNameIndex(anim_node.Head.nNodeNumber).Head.nSupernodeNumber;
        }
    }

    /// Build Array of Indices By Offset Order
    Data.MH.NameIndicesInBinaryOrder.reserve(Data.MH.ArrayOfNodes.size());
    for(Node & node : Data.MH.ArrayOfNodes){
        Data.MH.NameIndicesInBinaryOrder.push_back(node.Head.nNodeNumber);
    }

    /// PART 3 ///
    /// Interpret ascii data
    /// This constructs the Mesh.Vertices, Mesh.VertIndices, Dangly.Data2, Dangly.Constraints and Saber.SaberData structures.
    /// And not to forget the weights. Also face normals, average, aabb tree .... everything.
    Report("Interpreting ascii data...");
    ProgressSize(0, Data.MH.ArrayOfNodes.size());
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);
        if(node.Head.nType & NODE_SABER){
            /// Saber interpretation goes here.
            if((node.Mesh.TempVerts.size() == 16 && node.Mesh.TempTverts.size() == 16) ||
                (node.Mesh.TempVerts.size() == 176 && node.Mesh.TempTverts.size() == 176)){

                int nBase = 8;
                if (node.Mesh.TempVerts.size() == 176) nBase = 88;

                Vector v0 = node.Mesh.TempVerts.at(0);
                Vector v1 = node.Mesh.TempVerts.at(1);
                Vector v2 = node.Mesh.TempVerts.at(2);
                Vector v3 = node.Mesh.TempVerts.at(3);
                Vector v4 = node.Mesh.TempVerts.at(4);
                Vector v5 = node.Mesh.TempVerts.at(5);
                Vector v6 = node.Mesh.TempVerts.at(6);
                Vector v7 = node.Mesh.TempVerts.at(7);
                Vector v8 = node.Mesh.TempVerts.at(nBase+0);
                Vector v9 = node.Mesh.TempVerts.at(nBase+1);
                Vector v10 = node.Mesh.TempVerts.at(nBase+2);
                Vector v11 = node.Mesh.TempVerts.at(nBase+3);
                Vector v12 = node.Mesh.TempVerts.at(nBase+4);
                Vector v13 = node.Mesh.TempVerts.at(nBase+5);
                Vector v14 = node.Mesh.TempVerts.at(nBase+6);
                Vector v15 = node.Mesh.TempVerts.at(nBase+7);

                node.Saber.SaberData.reserve(50);
                node.Saber.SaberData.push_back(VertexData(v0, node.Mesh.TempTverts.at(0)));
                node.Saber.SaberData.push_back(VertexData(v1, node.Mesh.TempTverts.at(1)));
                node.Saber.SaberData.push_back(VertexData(v2, node.Mesh.TempTverts.at(2)));
                node.Saber.SaberData.push_back(VertexData(v3, node.Mesh.TempTverts.at(3)));
                node.Saber.SaberData.push_back(VertexData(v4, node.Mesh.TempTverts.at(4)));
                node.Saber.SaberData.push_back(VertexData(v5, node.Mesh.TempTverts.at(5)));
                node.Saber.SaberData.push_back(VertexData(v6, node.Mesh.TempTverts.at(6)));
                node.Saber.SaberData.push_back(VertexData(v7, node.Mesh.TempTverts.at(7)));
                for(int r = 0; r < 20; r++){
                    node.Saber.SaberData.push_back(VertexData(v0, node.Mesh.TempTverts.at(0)));
                    node.Saber.SaberData.push_back(VertexData(v1, node.Mesh.TempTverts.at(1)));
                    node.Saber.SaberData.push_back(VertexData(v2, node.Mesh.TempTverts.at(2)));
                    node.Saber.SaberData.push_back(VertexData(v3, node.Mesh.TempTverts.at(3)));
                }

                node.Saber.SaberData.push_back(VertexData(v8, node.Mesh.TempTverts.at(nBase+0)));
                node.Saber.SaberData.push_back(VertexData(v9, node.Mesh.TempTverts.at(nBase+1)));
                node.Saber.SaberData.push_back(VertexData(v10, node.Mesh.TempTverts.at(nBase+2)));
                node.Saber.SaberData.push_back(VertexData(v11, node.Mesh.TempTverts.at(nBase+3)));
                node.Saber.SaberData.push_back(VertexData(v12, node.Mesh.TempTverts.at(nBase+4)));
                node.Saber.SaberData.push_back(VertexData(v13, node.Mesh.TempTverts.at(nBase+5)));
                node.Saber.SaberData.push_back(VertexData(v14, node.Mesh.TempTverts.at(nBase+6)));
                node.Saber.SaberData.push_back(VertexData(v15, node.Mesh.TempTverts.at(nBase+7)));
                for(int r = 0; r < 20; r++){
                    node.Saber.SaberData.push_back(VertexData(v8, node.Mesh.TempTverts.at(nBase+0)));
                    node.Saber.SaberData.push_back(VertexData(v9, node.Mesh.TempTverts.at(nBase+1)));
                    node.Saber.SaberData.push_back(VertexData(v10, node.Mesh.TempTverts.at(nBase+2)));
                    node.Saber.SaberData.push_back(VertexData(v11, node.Mesh.TempTverts.at(nBase+3)));
                }
                node.Mesh.Faces.resize(0);
                node.Mesh.Faces.shrink_to_fit();
            }
            else{
                ReportMdl << "Warning! Requirements for saber mesh not met for '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'! Converting to trimesh...\n";
                node.Head.nType = NODE_HEADER | NODE_MESH;
            }
        }

        if(node.Head.nType & NODE_MESH && !(node.Head.nType & NODE_SABER)){
            std::vector<Vector> vectorarray;
            vectorarray.reserve(node.Mesh.Faces.size()*3);
            node.Mesh.fTotalArea = 0.0;

            /// Build mdx bitmap
            if(node.Mesh.TempVerts.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_VERTEX | MDX_FLAG_NORMAL);
            if(node.Mesh.TempColors.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_COLOR);
            if(node.Mesh.TempTverts.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_UV1);
            if(node.Mesh.TempTverts1.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_UV2);
            if(node.Mesh.TempTverts2.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_UV3);
            if(node.Mesh.TempTverts3.size() > 0) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_UV4);
            if(node.Mesh.TangentSpace.at(0)) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_TANGENT1);
            if(node.Mesh.TangentSpace.at(1)) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_TANGENT2);
            if(node.Mesh.TangentSpace.at(2)) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_TANGENT3);
            if(node.Mesh.TangentSpace.at(3)) node.Mesh.nMdxDataBitmap |= (MDX_FLAG_TANGENT4);

            for(int f = 0; f < node.Mesh.Faces.size(); f++){
                Face & face = node.Mesh.Faces.at(f);
                face.nID = f; /// Why is this necessary?

                /// MDLOps may leave out texindicesX arrays, I need to check for unset indices and make them use the diffuse ones instead.
                for(int i = 0; i < 3; i++){
                    if(node.Mesh.TempTverts1.size() > 0 && face.nIndexTvert1.at(i) < 0){
                        for(int i2 = 0; i2 < 3; i2++) face.nIndexTvert1.at(i2) = face.nIndexTvert.at(i2);
                    }
                    if(node.Mesh.TempTverts2.size() > 0 && face.nIndexTvert2.at(i) < 0){
                        for(int i2 = 0; i2 < 3; i2++) face.nIndexTvert2.at(i2) = face.nIndexTvert.at(i2);
                    }
                    if(node.Mesh.TempTverts3.size() > 0 && face.nIndexTvert3.at(i) < 0){
                        for(int i2 = 0; i2 < 3; i2++) face.nIndexTvert3.at(i2) = face.nIndexTvert.at(i2);
                    }
                }

                for(int i = 0; i < 3; i++){
                    if(!face.bProcessed[i]){
                        bool bIgnoreVert = true, bIgnoreTvert = true, bIgnoreTvert1 = true, bIgnoreTvert2 = true, bIgnoreTvert3 = true, bIgnoreColor = true;
                        Vertex vert;
                        vert.MDXData.nNodeNumber = node.Head.nNodeNumber;

                        if(node.Mesh.TempVerts.size() > 0){
                            bIgnoreVert = false;
                            vert.assign(node.Mesh.TempVerts.at(face.nIndexVertex[i]));

                            //Add to vectorarray if no identical
                            bool bAdd = true;
                            for(int v = 0; v < vectorarray.size() && bAdd; v++){
                                if(vectorarray.at(v).Compare(node.Mesh.TempVerts.at(face.nIndexVertex[i]))) bAdd = false;
                            }
                            if(bAdd) vectorarray.push_back(node.Mesh.TempVerts.at(face.nIndexVertex[i]));

                            vert.vFromRoot = node.Mesh.TempVerts.at(face.nIndexVertex[i]);
                            vert.vFromRoot.Rotate(node.GetLocation().oOrientation.GetQuaternion());
                            vert.vFromRoot += node.Head.vFromRoot;

                            vert.MDXData.vVertex = node.Mesh.TempVerts.at(face.nIndexVertex[i]);

                            if(node.Head.nType & NODE_DANGLY){
                                node.Dangly.Data2.push_back(node.Mesh.TempVerts.at(face.nIndexVertex[i]));
                                node.Dangly.Constraints.push_back(node.Dangly.TempConstraints.at(face.nIndexVertex[i]));
                            }

                            if(node.Head.nType & NODE_SKIN){
                                vert.MDXData.Weights = node.Skin.TempWeights.at(face.nIndexVertex[i]);
                                double fTotalWeight = 0.0;
                                fTotalWeight += vert.MDXData.Weights.fWeightValue.at(0);
                                fTotalWeight += vert.MDXData.Weights.fWeightValue.at(1);
                                fTotalWeight += vert.MDXData.Weights.fWeightValue.at(2);
                                fTotalWeight += vert.MDXData.Weights.fWeightValue.at(3);
                                if(abs(fTotalWeight - 1.0) >= 0.0001) ReportMdl << "Warning! Skin weights for ascii vertex " << face.nIndexVertex[i] << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "' do not equal 1.0, instead they equal " << fTotalWeight << ". This may cause problems in the game.\n";
                            }
                        }
                        if(node.Mesh.TempTverts.size() > 0){
                            bIgnoreTvert = false;
                            vert.MDXData.vUV1 = node.Mesh.TempTverts.at(face.nIndexTvert[i]);
                        }
                        if(node.Mesh.TempTverts1.size() > 0){
                            bIgnoreTvert1 = false;
                            vert.MDXData.vUV2 = node.Mesh.TempTverts1.at(face.nIndexTvert1[i]);
                        }
                        if(node.Mesh.TempTverts2.size() > 0){
                            bIgnoreTvert2 = false;
                            vert.MDXData.vUV3 = node.Mesh.TempTverts2.at(face.nIndexTvert2[i]);
                        }
                        if(node.Mesh.TempTverts3.size() > 0){
                            bIgnoreTvert3 = false;
                            vert.MDXData.vUV4 = node.Mesh.TempTverts3.at(face.nIndexTvert3[i]);
                        }
                        if(node.Mesh.TempColors.size() > 0){
                            bIgnoreColor = false;
                            vert.MDXData.cColor = node.Mesh.TempColors.at(face.nIndexColor[i]);
                        }

                        //Find identical verts
                        for(int f2 = f; f2 < node.Mesh.Faces.size(); f2++){
                            Face & face2 = node.Mesh.Faces.at(f2);
                            for(int i2 = 0; i2 < 3; i2++){
                                //Make sure that we're only changing what's past our current position if we are in the same face.
                                if(f2 != f || i2 > i){
                                    if(bMinimizeVerts){
                                        try{
                                            if( !face2.bProcessed[i2] &&
                                                (bIgnoreVert || node.Mesh.TempVerts.at(face2.nIndexVertex[i2]) == node.Mesh.TempVerts.at(face.nIndexVertex[i]) ) &&
                                                (bIgnoreVert || !(node.Head.nType & NODE_DANGLY) || node.Dangly.TempConstraints.at(face2.nIndexVertex[i2]) == node.Dangly.TempConstraints.at(face.nIndexVertex[i]) ) &&
                                                (bIgnoreVert || !(node.Head.nType & NODE_SKIN) || node.Skin.TempWeights.at(face2.nIndexVertex[i2]) == node.Skin.TempWeights.at(face.nIndexVertex[i]) ) &&
                                                (bIgnoreTvert || node.Mesh.TempTverts.at(face2.nIndexTvert[i2]) == node.Mesh.TempTverts.at(face.nIndexTvert[i]) ) &&
                                                (bIgnoreTvert1 || node.Mesh.TempTverts1.at(face2.nIndexTvert1[i2]) == node.Mesh.TempTverts1.at(face.nIndexTvert1[i]) ) &&
                                                (bIgnoreTvert2 || node.Mesh.TempTverts2.at(face2.nIndexTvert2[i2]) == node.Mesh.TempTverts2.at(face.nIndexTvert2[i]) ) &&
                                                (bIgnoreTvert3 || node.Mesh.TempTverts3.at(face2.nIndexTvert3[i2]) == node.Mesh.TempTverts3.at(face.nIndexTvert3[i]) ) &&
                                                (bIgnoreColor || node.Mesh.TempColors.at(face2.nIndexColor[i2]) == node.Mesh.TempColors.at(face.nIndexColor[i]) ) &&
                                                face.nSmoothingGroup & face2.nSmoothingGroup)
                                            {
                                                //If we find a reference to the exact same vert, we have to link to it
                                                //Actually we only need to link vert indices, the correct UV are now already included in the Vertex struct
                                                face2.nIndexVertex[i2] = node.Mesh.Vertices.size();
                                                face2.bProcessed[i2] = true;
                                            }
                                        }
                                        catch(const std::exception & e){
                                            throw mdlexception("Exception while handling temp arrays (face2=" + std::to_string(f2) + ", i2=" + std::to_string(i2) + ") node '" + Data.MH.Names.at(node.Head.nNodeNumber).sName + "':\n" + e.what());
                                        }
                                    }
                                    else{
                                        if( (bIgnoreVert || face2.nIndexVertex[i2] == face.nIndexVertex[i] ) &&
                                            (bIgnoreTvert || face2.nIndexTvert[i2] == face.nIndexTvert[i] ) &&
                                            (bIgnoreTvert1 || face2.nIndexTvert1[i2] == face.nIndexTvert1[i] ) &&
                                            (bIgnoreTvert2 || face2.nIndexTvert2[i2] == face.nIndexTvert2[i] ) &&
                                            (bIgnoreTvert3 || face2.nIndexTvert3[i2] == face.nIndexTvert3[i] ) &&
                                            (bIgnoreColor || face2.nIndexColor[i2] == face.nIndexColor[i] ) &&
                                            !face2.bProcessed[i2] &&
                                            face.nSmoothingGroup & face2.nSmoothingGroup)
                                        {
                                            //If we find a reference to the exact same vert, we have to link to it
                                            //Actually we only need to link vert indices, the correct UV are now already included in the Vertex struct
                                            face2.nIndexVertex[i2] = node.Mesh.Vertices.size();
                                            face2.bProcessed[i2] = true;
                                        }
                                    }
                                }
                            }
                        }

                        //Now we're allowed to link the original vert as well
                        face.nIndexVertex[i] = node.Mesh.Vertices.size();
                        face.bProcessed[i] = true;

                        //Put the new vert into the array
                        node.Mesh.Vertices.push_back(std::move(vert));
                    }
                }

                std::array<short, 3> vertindicesarray = {face.nIndexVertex[0], face.nIndexVertex[1], face.nIndexVertex[2]};
                node.Mesh.VertIndices.push_back(std::move(vertindicesarray));

                /// Surprise! Face normal calculation! Moved here so it can be used by BuildAABB
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

                /// This is for the face normal
                face.vNormal = cross(Edge1, Edge2); //Cross product, unnormalized
                face.vNormal.Normalize();

                /// This is for the distance.
                face.fDistance = - (face.vNormal.fX * v1.fX +
                                    face.vNormal.fY * v1.fY +
                                    face.vNormal.fZ * v1.fZ);

                /// Area calculation
                face.fArea = HeronFormulaEdge(Edge1, Edge2, Edge3);
                face.fAreaUV = HeronFormulaEdge(EUV1, EUV2, EUV3);
                if(face.fArea != -1.0) node.Mesh.fTotalArea += face.fArea;

                /// Tangent space vectors
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
                    /**/
                    //ndix UR's magic factor
                    r = 2406.6388;
                }
                face.vTangent = r * (Edge1 * EUV2.fY - Edge2 * EUV1.fY);
                face.vBitangent = r * (Edge2 * EUV1.fX - Edge1 * EUV2.fX);
                face.vTangent.Normalize();
                face.vBitangent.Normalize();
                if(face.vTangent.Null()) face.vTangent = Vector(1.0, 0.0, 0.0);
                if(face.vBitangent.Null()) face.vBitangent = Vector(1.0, 0.0, 0.0);

                //Handedness
                Vector vCross = cross(face.vNormal, face.vTangent);
                double fDot = dot(vCross, face.vBitangent);
                if(fDot > 0.0000000001) face.vTangent *= -1.0;

                //Now check if we need to invert  T and B. But first we need a UV normal
                Vector vNormalUV = cross(EUV1, EUV2); //cross product
                if(vNormalUV.fZ < 0.0){
                    face.vTangent *= -1.0;
                    face.vBitangent *= -1.0;
                }

                /// Face Bounding Box calculation for AABB tree
                if(node.Head.nType & NODE_AABB){
                    face.vBBmax = Vector(-10000.0, -10000.0, -10000.0);
                    face.vBBmin = Vector( 10000.0,  10000.0,  10000.0);
                    face.vCentroid = Vector(0.0, 0.0, 0.0);
                    for(int i = 0; i < 3; i++){
                        face.vBBmax.fX = std::max(face.vBBmax.fX, node.Mesh.Vertices.at(face.nIndexVertex[i]).fX);
                        face.vBBmax.fY = std::max(face.vBBmax.fY, node.Mesh.Vertices.at(face.nIndexVertex[i]).fY);
                        face.vBBmax.fZ = std::max(face.vBBmax.fZ, node.Mesh.Vertices.at(face.nIndexVertex[i]).fZ);
                        face.vBBmin.fX = std::min(face.vBBmin.fX, node.Mesh.Vertices.at(face.nIndexVertex[i]).fX);
                        face.vBBmin.fY = std::min(face.vBBmin.fY, node.Mesh.Vertices.at(face.nIndexVertex[i]).fY);
                        face.vBBmin.fZ = std::min(face.vBBmin.fZ, node.Mesh.Vertices.at(face.nIndexVertex[i]).fZ);
                        face.vCentroid += node.Mesh.Vertices.at(face.nIndexVertex[i]);
                    }
                    face.vCentroid /= 3.0;
                }
            }

            /// Surprise 2!! Average and BB calculation!
            node.Mesh.vAverage = Vector(0.0, 0.0, 0.0);
            node.Mesh.vBBmin = Vector(0.0, 0.0, 0.0); /// Wrong, but Bioware-correct
            node.Mesh.vBBmax = Vector(0.0, 0.0, 0.0); /// Wrong, but Bioware-correct
            for(int v = 0; v < vectorarray.size(); v++){
                node.Mesh.vBBmin.fX = std::min(node.Mesh.vBBmin.fX, vectorarray.at(v).fX);
                node.Mesh.vBBmin.fY = std::min(node.Mesh.vBBmin.fY, vectorarray.at(v).fY);
                node.Mesh.vBBmin.fZ = std::min(node.Mesh.vBBmin.fZ, vectorarray.at(v).fZ);
                node.Mesh.vBBmax.fX = std::max(node.Mesh.vBBmax.fX, vectorarray.at(v).fX);
                node.Mesh.vBBmax.fY = std::max(node.Mesh.vBBmax.fY, vectorarray.at(v).fY);
                node.Mesh.vBBmax.fZ = std::max(node.Mesh.vBBmax.fZ, vectorarray.at(v).fZ);
                node.Mesh.vAverage += vectorarray.at(v);
            }
            node.Mesh.vAverage /= (double) vectorarray.size();

            /// Now find the radius as well!
            node.Mesh.fRadius = 0.0;
            for(int v = 0; v < vectorarray.size(); v++){
                node.Mesh.fRadius = std::max(node.Mesh.fRadius, Vector(vectorarray.at(v) - node.Mesh.vAverage).GetLength());
            }

            //Calculate adjacent faces
            for(int f = 0; f < node.Mesh.Faces.size(); f++){
                Face & face = node.Mesh.Faces.at(f);

                // Skip if none is -1
                if(face.nAdjacentFaces[0]!=-1 &&
                   face.nAdjacentFaces[1]!=-1 &&
                   face.nAdjacentFaces[2]!=-1 ) continue;

                //Go through all the faces coming after this one
                for(int f2 = f+1; f2 < node.Mesh.Faces.size(); f2++){
                    Face & compareface = node.Mesh.Faces.at(f2);
                    std::vector<bool> VertMatches(3, false);
                    std::vector<bool> VertMatchesCompare(3, false);
                    for(int i3 = 0; i3 < 3; i3++){
                        int nVertIndex = face.nIndexVertex[i3];
                        Vector & ourvect = node.Mesh.Vertices.at(nVertIndex).vFromRoot;
                        for(int i4 = 0; i4 < 3; i4++){
                            Vector & othervect = node.Mesh.Vertices.at(compareface.nIndexVertex[i4]).vFromRoot;
                            if(ourvect.Compare(othervect)){
                                VertMatches.at(i3) = true;
                                VertMatchesCompare.at(i4) = true;
                                i4 = 3; // we can only have one matching vert in a face per vert. Once we find a match, we're done.
                            }
                        }
                    }
                    if(VertMatches.at(0) && VertMatches.at(1)){
                        if(face.nAdjacentFaces[0] != -1) ReportMdl<<"Found an additional adjacent face on edge 0 for face " << f << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else face.nAdjacentFaces[0] = f2;
                    }
                    else if(VertMatches.at(1) && VertMatches.at(2)){
                        if(face.nAdjacentFaces[1] != -1) ReportMdl<<"Found an additional adjacent face on edge 1 for face " << f << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else face.nAdjacentFaces[1] = f2;
                    }
                    else if(VertMatches.at(2) && VertMatches.at(0)){
                        if(face.nAdjacentFaces[2] != -1) ReportMdl<<"Found an additional adjacent face on edge 2 for face " << f << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else face.nAdjacentFaces[2] = f2;
                    }
                    if(VertMatchesCompare.at(0) && VertMatchesCompare.at(1)){
                        if(compareface.nAdjacentFaces[0] != -1) ReportMdl<<"Found an additional adjacent face on edge 0 for face " << f2 << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else compareface.nAdjacentFaces[0] = f;
                    }
                    else if(VertMatchesCompare.at(1) && VertMatchesCompare.at(2)){
                        if(compareface.nAdjacentFaces[1] != -1) ReportMdl<<"Found an additional adjacent face on edge 1 for face " << f2 << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else compareface.nAdjacentFaces[1] = f;
                    }
                    else if(VertMatchesCompare.at(2) && VertMatchesCompare.at(0)){
                        if(compareface.nAdjacentFaces[2] != -1) ReportMdl<<"Found an additional adjacent face on edge 2 for face " << f2 << " on '" << Data.MH.Names.at(node.Head.nNodeNumber).sName << "'...\n";
                        else compareface.nAdjacentFaces[2] = f;
                    }
                    if(face.nAdjacentFaces[0]!=-1 &&
                       face.nAdjacentFaces[1]!=-1 &&
                       face.nAdjacentFaces[2]!=-1 ){
                        f2 = node.Mesh.Faces.size(); //Found them all, maybe I finish early?
                    }
                }
            }

            /// Texture count depends on the UVs.
            node.Mesh.nTextureNumber = (node.Mesh.nMdxDataBitmap & MDX_FLAG_UV1 ? 1 : 0) +
                                       (node.Mesh.nMdxDataBitmap & MDX_FLAG_UV2 ? 1 : 0) +
                                       (node.Mesh.nMdxDataBitmap & MDX_FLAG_UV3 ? 1 : 0) +
                                       (node.Mesh.nMdxDataBitmap & MDX_FLAG_UV4 ? 1 : 0);

            node.Mesh.TempVerts.resize(0);
            node.Mesh.TempTverts.resize(0);
            node.Mesh.TempTverts1.resize(0);
            node.Mesh.TempTverts2.resize(0);
            node.Mesh.TempTverts3.resize(0);
            node.Dangly.TempConstraints.resize(0);
            node.Skin.TempWeights.resize(0);
            node.Mesh.TempVerts.shrink_to_fit();
            node.Mesh.TempTverts.shrink_to_fit();
            node.Mesh.TempTverts1.shrink_to_fit();
            node.Mesh.TempTverts2.shrink_to_fit();
            node.Mesh.TempTverts3.shrink_to_fit();
            node.Dangly.TempConstraints.shrink_to_fit();
            node.Skin.TempWeights.shrink_to_fit();
        }

        if(node.Head.nType & NODE_MESH &&
           !(node.Head.nType & NODE_SABER) &&
           Data.MH.Names.at(node.Head.nNodeNumber).sName.substr(0, 6) == "2081__" &&
           (node.Mesh.Faces.size() == 12  /*|| node.Mesh.Faces.size() == 24*/) &&
           node.Mesh.Vertices.size() == 16 )
        {
            bool bAbort = false; /// If this is set to true at any time, we will abort the conversion to saber

            std::array<int, 16> VertRefsArray = {0, 0, 0, 0, 0, 0, 0, 0,
                                                 0, 0, 0, 0, 0, 0, 0, 0};

            /// Go through the faces and build the VertRefsArray
            /// If a reference to a higher than 16th vertex, abort
            /// If the number of references to a vertex is greater than 4, abort
            for(int f = 0; f < 12 && !bAbort; f++){
                Face & face = node.Mesh.Faces.at(f);
                for(int i = 0; i < 3; i++){
                    if(face.nIndexVertex.at(i) < 16){
                        VertRefsArray.at(face.nIndexVertex.at(i)) += 1;
                        if ( VertRefsArray.at(face.nIndexVertex.at(i)) > 4) bAbort = true;
                    }
                    else bAbort = true;
                }
            }

            /// Now we need to find these guys
            int nOuter1 = -1;
            int nOuter2 = -1;
            int nCorner1 = -1;
            int nCorner2 = -1;

            /// Go through the verts, find the two that are referenced by 4 faces
            /// Put them into outer1 and outer2
            /// If there's more than two of those, abort
            int nFound = 0;
            for(int v = 0; v < 16 && !bAbort; v++){
                if(VertRefsArray.at(v) == 4){
                    if(nFound == 0){
                        nOuter1 = v;
                        nFound++;
                    }
                    else if(nFound == 1){
                        nOuter2 = v;
                        nFound++;
                    }
                    else bAbort = true;
                }
            }

            /// Go through faces again and find the corners – the only ones adjacent to outer1 & 2 that have 1 ref
            for(int f = 0; f < 12 && !bAbort; f++){
                Face & face = node.Mesh.Faces.at(f);
                /// Go through the indices
                for(int i = 0; i < 3; i++){
                    if( face.nIndexVertex.at(i) == nOuter1){
                        /// Go through the same indices again
                        for(int i2 = 0; i2 < 3; i2++){
                            if(nCorner1 == -1 && VertRefsArray.at(face.nIndexVertex.at(i2)) == 1){
                                nCorner1 = face.nIndexVertex.at(i2);
                                break;
                            }
                            else if(nCorner1 != -1 && VertRefsArray.at(face.nIndexVertex.at(i2)) == 1){
                                ReportMdl << "Error! nCorner1 found several times, this shouldn't be happening!!\n";
                                bAbort = true;
                                break;
                            }
                        }
                        break;
                    }
                    else if( face.nIndexVertex.at(i) == nOuter2){
                        /// Go through the same indices again
                        for(int i2 = 0; i2 < 3; i2++){
                            if(nCorner2 == -1 && VertRefsArray.at(face.nIndexVertex.at(i2)) == 1){
                                nCorner2 = face.nIndexVertex.at(i2);
                                break;
                            }
                            else if(nCorner2 != -1 && VertRefsArray.at(face.nIndexVertex.at(i2)) == 1){
                                ReportMdl << "Error! nCorner2 found several times, this shouldn't be happening!!\n";
                                bAbort = true;
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            if(nOuter1!=-1 && nOuter2!=-1 && nCorner1!=-1 && nCorner2!=-1 && !bAbort){
                /// Build blade vert arrays
                std::array<int, 8> Blade1VertArray = {-1, -1, -1, -1, -1, -1, -1, -1};
                std::array<int, 8> Blade2VertArray = {-1, -1, -1, -1, -1, -1, -1, -1};

                Blade1VertArray.at(6) = nOuter1;
                Blade1VertArray.at(7) = nCorner1;
                Blade1VertArray.at(3) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(6), Blade1VertArray.at(7));
                Blade1VertArray.at(2) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(3), Blade1VertArray.at(6), Blade1VertArray.at(7));
                Blade1VertArray.at(1) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(2), Blade1VertArray.at(6), Blade1VertArray.at(3));
                Blade1VertArray.at(5) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(1), Blade1VertArray.at(6), Blade1VertArray.at(2));
                Blade1VertArray.at(0) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(1), Blade1VertArray.at(5), Blade1VertArray.at(6));
                Blade1VertArray.at(4) = FindThirdIndex(node.Mesh.Faces, Blade1VertArray.at(0), Blade1VertArray.at(5), Blade1VertArray.at(1));

                Blade2VertArray.at(6) = nOuter2;
                Blade2VertArray.at(7) = nCorner2;
                Blade2VertArray.at(3) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(6), Blade2VertArray.at(7));
                Blade2VertArray.at(2) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(3), Blade2VertArray.at(6), Blade2VertArray.at(7));
                Blade2VertArray.at(1) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(2), Blade2VertArray.at(6), Blade2VertArray.at(3));
                Blade2VertArray.at(5) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(1), Blade2VertArray.at(6), Blade2VertArray.at(2));
                Blade2VertArray.at(0) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(1), Blade2VertArray.at(5), Blade2VertArray.at(6));
                Blade2VertArray.at(4) = FindThirdIndex(node.Mesh.Faces, Blade2VertArray.at(0), Blade2VertArray.at(5), Blade2VertArray.at(1));

                /// if there is a -1 in any of the two arrays, abort
                if(std::find(Blade1VertArray.begin(), Blade1VertArray.end(), -1) != Blade1VertArray.end() ||
                   std::find(Blade2VertArray.begin(), Blade2VertArray.end(), -1) != Blade2VertArray.end()) bAbort = true;

                if(!bAbort){
                    ///Now all we need to do is decide which of the two blades to invert.
                    std::array<int, 8> Blade1, Blade2;
                    if(node.Mesh.Vertices.at(Blade1VertArray.at(6)).fZ - node.Mesh.Vertices.at(Blade1VertArray.at(5)).fZ >
                       node.Mesh.Vertices.at(Blade2VertArray.at(6)).fZ - node.Mesh.Vertices.at(Blade2VertArray.at(5)).fZ )
                    {
                        Blade1 = Blade1VertArray;
                        Blade2 = {Blade2VertArray[3], Blade2VertArray[2], Blade2VertArray[1], Blade2VertArray[0],
                                  Blade2VertArray[7], Blade2VertArray[6], Blade2VertArray[5], Blade2VertArray[4]};
                    }
                    else{
                        Blade1 = Blade2VertArray;
                        Blade2 = {Blade1VertArray[3], Blade1VertArray[2], Blade1VertArray[1], Blade1VertArray[0],
                                  Blade1VertArray[7], Blade1VertArray[6], Blade1VertArray[5], Blade1VertArray[4]};
                    }

                    node.Saber.SaberData.reserve(50);
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(0)), node.Mesh.Vertices.at(Blade1.at(0)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(1)), node.Mesh.Vertices.at(Blade1.at(1)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(2)), node.Mesh.Vertices.at(Blade1.at(2)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(3)), node.Mesh.Vertices.at(Blade1.at(3)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(4)), node.Mesh.Vertices.at(Blade1.at(4)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(5)), node.Mesh.Vertices.at(Blade1.at(5)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(6)), node.Mesh.Vertices.at(Blade1.at(6)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(7)), node.Mesh.Vertices.at(Blade1.at(7)).MDXData.vUV1));
                    for(int r = 0; r < 20; r++){
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(0)), node.Mesh.Vertices.at(Blade1.at(0)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(1)), node.Mesh.Vertices.at(Blade1.at(1)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(2)), node.Mesh.Vertices.at(Blade1.at(2)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade1.at(3)), node.Mesh.Vertices.at(Blade1.at(3)).MDXData.vUV1));
                    }

                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(0)), node.Mesh.Vertices.at(Blade2.at(0)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(1)), node.Mesh.Vertices.at(Blade2.at(1)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(2)), node.Mesh.Vertices.at(Blade2.at(2)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(3)), node.Mesh.Vertices.at(Blade2.at(3)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(4)), node.Mesh.Vertices.at(Blade2.at(4)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(5)), node.Mesh.Vertices.at(Blade2.at(5)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(6)), node.Mesh.Vertices.at(Blade2.at(6)).MDXData.vUV1));
                    node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(7)), node.Mesh.Vertices.at(Blade2.at(7)).MDXData.vUV1));
                    for(int r = 0; r < 20; r++){
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(0)), node.Mesh.Vertices.at(Blade2.at(0)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(1)), node.Mesh.Vertices.at(Blade2.at(1)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(2)), node.Mesh.Vertices.at(Blade2.at(2)).MDXData.vUV1));
                        node.Saber.SaberData.push_back(VertexData(node.Mesh.Vertices.at(Blade2.at(3)), node.Mesh.Vertices.at(Blade2.at(3)).MDXData.vUV1));
                    }

                    /// Convert trimesh to lightsaber here
                    node.Head.nType = node.Head.nType | NODE_SABER;
                    Data.MH.Names.at(node.Head.nNodeNumber).sName = Data.MH.Names.at(node.Head.nNodeNumber).sName.substr(6);
                }
            }
        }

        if(node.Head.nType & NODE_SABER){
            std::array<std::array<int, 3>, 12> FaceIndices = {{{0,4,5},{1,0,5},{1,5,2},
                                                              {2,5,6},{3,2,6},{3,6,7},
                                                              {88+4,88+0,88+5},{88+0,88+1,88+5},{88+5,88+1,88+2},
                                                              {88+5,88+2,88+6},{88+2,88+3,88+6},{88+6,88+3,88+7}}};

            std::array<Vector, 12> vFaceNormals;
            for(int v = 0; v < 12; v++){
                vFaceNormals[v] = GetNormal(node.Saber.SaberData.at(FaceIndices[v][0]).vVertex,
                                            node.Saber.SaberData.at(FaceIndices[v][1]).vVertex,
                                            node.Saber.SaberData.at(FaceIndices[v][2]).vVertex);
            }

            std::array<double, 12> fFaceAreas;
            for(int v = 0; v < 12; v++){
                fFaceAreas[v] = HeronFormulaVert(node.Saber.SaberData.at(FaceIndices[v][0]).vVertex,
                                                 node.Saber.SaberData.at(FaceIndices[v][1]).vVertex,
                                                 node.Saber.SaberData.at(FaceIndices[v][2]).vVertex);
            }

            std::array<Vector, 8> vVertNormals;

            for(int v = 0; v < 8; v++){
                Vector & vCurrent = vVertNormals.at(v);
                int nCurrent = v;
                if(nCurrent > 3) nCurrent += 84;
                for(int f = 0; f < 12; f++){
                    for(int i = 0; i < 3; i++){
                        if(FaceIndices[f][i] == nCurrent){
                            Vector vAdd = vFaceNormals[f];
                            if(bSmoothAreaWeighting) vAdd *= fFaceAreas[f];
                            if(bSmoothAngleWeighting){
                                if(i == 0){
                                    vAdd *= Angle(node.Saber.SaberData.at(FaceIndices[f][2]).vVertex - node.Saber.SaberData.at(FaceIndices[f][0]).vVertex,
                                                  node.Saber.SaberData.at(FaceIndices[f][1]).vVertex - node.Saber.SaberData.at(FaceIndices[f][0]).vVertex);
                                }
                                else if(i == 1){
                                    vAdd *= Angle(node.Saber.SaberData.at(FaceIndices[f][2]).vVertex - node.Saber.SaberData.at(FaceIndices[f][1]).vVertex,
                                                  node.Saber.SaberData.at(FaceIndices[f][0]).vVertex - node.Saber.SaberData.at(FaceIndices[f][1]).vVertex);
                                }
                                else if(i == 2){
                                    vAdd *= Angle(node.Saber.SaberData.at(FaceIndices[f][1]).vVertex - node.Saber.SaberData.at(FaceIndices[f][2]).vVertex,
                                                  node.Saber.SaberData.at(FaceIndices[f][0]).vVertex - node.Saber.SaberData.at(FaceIndices[f][2]).vVertex);
                                }
                            }
                            vCurrent += vAdd;
                        }
                    }
                }
                vCurrent.Normalize();
            }
            for(int v = 0; v < node.Saber.SaberData.size(); v++){
                if(v < node.Saber.SaberData.size()/2) node.Saber.SaberData.at(v).vNormal = vVertNormals.at(v%4);
                else node.Saber.SaberData.at(v).vNormal = vVertNormals.at(4 + v%4);
            }
            node.Mesh.Vertices.reserve(node.Saber.SaberData.size());
            for(VertexData & sd : node.Saber.SaberData) node.Mesh.Vertices.push_back(Vertex().assign(sd.vVertex));
            node.Mesh.Faces.resize(12);
            node.Mesh.Faces.at(0).nIndexVertex = {0, 1, 2};
            node.Mesh.Faces.at(1).nIndexVertex = {3, 4, 5};
            node.Mesh.Faces.at(2).nIndexVertex = {6, 7, 8};
            node.Mesh.Faces.at(3).nIndexVertex = {9, 10, 11};
            node.Mesh.Faces.at(4).nIndexVertex = {12, 13, 14};
            node.Mesh.Faces.at(5).nIndexVertex = {15, 16, 17};
            node.Mesh.Faces.at(6).nIndexVertex = {18, 19, 20};
            node.Mesh.Faces.at(7).nIndexVertex = {21, 22, 23};
            node.Mesh.Faces.at(8).nIndexVertex = {24, 25, 26};
            node.Mesh.Faces.at(9).nIndexVertex = {27, 28, 29};
            node.Mesh.Faces.at(10).nIndexVertex = {30, 31, 32};
            node.Mesh.Faces.at(11).nIndexVertex = {33, 32, 31};
        }

        if(node.Head.nType & NODE_AABB){
            if(Wok) Warning("Found an aabb node, but Wok already exists! Skipping this node...");
            else{
                Wok.reset(new WOK());
                std::vector<Face*> allfaces;
                for(int f = 0; f < node.Mesh.Faces.size(); f++){
                    allfaces.push_back(&node.Mesh.Faces.at(f));
                }
                std::stringstream file2;
                BuildAabb(node.Walkmesh.RootAabb, allfaces, &file2);

                //Write to Wok
                std::stringstream file;
                ReportMdl << "Should write wok.\n";
                Wok->WriteWok(node, Data.MH.vLytPosition, &file);
                file << "\r\n\r\nAABB\r\n";
                file << file2.str();

                if(bDebug){
                    std::wstring sDir = sFullPath;
                    sDir.reserve(MAX_PATH);
                    PathRemoveFileSpecW(&sDir[0]);
                    sDir.resize(wcslen(sDir.c_str()));
                    sDir += L"\\debug_aabb.txt";
                    ReportMdl << "Will write aabb debug to: " << sDir.c_str() << "\n";
                    //std::ofstream filewrite(sDir.c_str());
                    HANDLE hFile = bead_CreateWriteFile(sDir);

                    //if(!filewrite.is_open()){
                    if(hFile == INVALID_HANDLE_VALUE){
                        ReportMdl << "'debug_aabb.txt' does not exist. No debug will be written.\n";
                    }
                    else{
                        //filewrite << file.str();
                        bead_WriteFile(hFile, file.str());
                        //filewrite.close();
                        CloseHandle(hFile);
                    }
                }
            }
        }
        ProgressPos(n);
    }
    ProgressPos(Data.MH.ArrayOfNodes.size());

    /// PART 4 ///
    /// Do the necessary mesh calculations
    /// Mesh: inverted counter
    /// Skin: T-bones, Q-bones
    /// Saber: inverted counter
    int nMeshCounter = 0;
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);

        if(node.Head.nType & NODE_SABER && node.Saber.nInvCount1 == -1 && node.Saber.nInvCount2 == -1){
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
        else if(node.Head.nType & NODE_MESH){
            //inverted counter
            if(node.Mesh.nMeshInvertedCounter == -1){
                nMeshCounter++;
                int Quo = nMeshCounter/100;
                int Mod = nMeshCounter%100;
                node.Mesh.nMeshInvertedCounter = pown(2, Quo)*100-nMeshCounter + (Mod ? Quo*100 : 0) + (Quo ? 0 : -1);
            }

            if(node.Head.nType & NODE_SKIN){
                //First, declare our empty location as a starting point
                Vector vPos;
                Quaternion qOrient;

                //Now we need to construct a path by adding all the locations from this node through all its parents to the root
                int nIndex = node.Head.nNodeNumber;
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

                Vector vBase = vPos; //Make copy
                Quaternion qBase = qOrient; //Make copy

                nIndex = node.Head.nNodeNumber;
                std::vector<int> Indices;
                //The price we have to pay for not going recursive
                while(nIndex != -1){
                    Indices.push_back(nIndex);
                    nIndex = GetNodeByNameIndex(nIndex).Head.nParentIndex;
                }
                /// Now we go from the parent and add all the positions.
                for(int a = Indices.size() - 1; a >= 0; a--){
                    Node & curnode2 = GetNodeByNameIndex(Indices.at(a));
                    Location locNode = curnode2.GetLocation();
                    vCurPosition = locNode.vPosition;
                    qCurOrientation = locNode.oOrientation.GetQuaternion();
                    vCurPosition.Rotate(qBase); //Note: rotating with the Base rotation!
                    vBase += vCurPosition;
                    qBase *= qCurOrientation;
                }

                //Oops! Forgot the last part in MDLOps, need to rotate the vector again.
                vBase *= -1.0;
                qBase = qBase.reverse();
                vBase.Rotate(qBase);

                //Loop through all the nodes, and do a similar operation to get the path for every node, then adding it to loc
                for(int n = 0; n < FH->MH.ArrayOfNodes.size(); n++){
                    Vector vRecord = vPos; //Make copy
                    Quaternion qRecord = qOrient; //Make copy
                    Node & curnode = FH->MH.ArrayOfNodes.at(n);

                    nIndex = curnode.Head.nNodeNumber;
                    Indices.clear();
                    //The price we have to pay for not going recursive
                    while(nIndex != -1){
                        Indices.push_back(nIndex);
                        nIndex = GetNodeByNameIndex(nIndex).Head.nParentIndex;
                    }
                    //ReportMdl << "Our Indices size is: " << Indices.size() << ".\n";
                    for(int a = Indices.size() - 1; a >= 0; a--){
                        Node & curnode2 = GetNodeByNameIndex(Indices.at(a));
                        Location locNode = curnode2.GetLocation();
                        vCurPosition = locNode.vPosition;
                        qCurOrientation = locNode.oOrientation.GetQuaternion();
                        vCurPosition.Rotate(qRecord); //Note: rotating with the Record rotation!
                        vRecord += vCurPosition;
                        qRecord *= qCurOrientation;
                    }

                    /// This code should fix the t-bone problems. Solution by ndix UR.
                    vRecord += vBase;

                    //Oops! Forgot the last part in MDLOps, need to rotate the vector again.
                    vRecord *= -1.0;
                    qRecord = qRecord.reverse();
                    vRecord.Rotate(qRecord);

                    //By now, lRecord holds the base loc + the path for this node. This should now be exactly what gets written in T and Q Bones!
                    node.Skin.Bones.at(n).TBone = vRecord;
                    node.Skin.Bones.at(n).QBone.SetQuaternion(qRecord);
                }
            }
        }
    }

    /// PART 5 ///
    /// Create patches through linked faces
    //This will take a while and needs to be optimized for speed, anything that can be taken out of it, should be
    CreatePatches();

    /// PART 6 ///
    /// Calculate vertex normals and vertex tangent space vectors
    Report("Calculating vertex normals and vertex tangent space vectors...");
    for(int pg = 0; pg < Data.MH.PatchArrayPointers.size(); pg++){
        int nPatchCount = Data.MH.PatchArrayPointers.at(pg).size();
        for(int p = 0; p < nPatchCount; p++){
            Patch & patch = Data.MH.PatchArrayPointers.at(pg).at(p);
            Vertex & vert = GetNodeByNameIndex(patch.nNodeNumber).Mesh.Vertices.at(patch.nVertex);

            for(int p2 = 0; p2 < nPatchCount; p2++){
                Patch & patch2 = Data.MH.PatchArrayPointers.at(pg).at(p2);
                if(&patch2 == &patch || patch2.nSmoothingGroups & patch.nSmoothingGroups){
                    for(int f = 0; f < patch2.FaceIndices.size(); f++){
                        Face & face = GetNodeByNameIndex(patch2.nNodeNumber).Mesh.Faces.at(patch2.FaceIndices.at(f));
                        Vertex & v1 = GetNodeByNameIndex(patch2.nNodeNumber).Mesh.Vertices.at(face.nIndexVertex[0]);
                        Vertex & v2 = GetNodeByNameIndex(patch2.nNodeNumber).Mesh.Vertices.at(face.nIndexVertex[1]);
                        Vertex & v3 = GetNodeByNameIndex(patch2.nNodeNumber).Mesh.Vertices.at(face.nIndexVertex[2]);
                        Vector Edge1 = v2 - v1;
                        Vector Edge2 = v3 - v1;
                        Vector Edge3 = v3 - v2;

                        //Vector vAdd = face.vNormal;
                        Vector vAdd = cross(Edge1, Edge2); //Cross product, unnormalized
                        vAdd.Normalize();

                        /// Crease Angle Test
                        /// if enabled, and the angle between the current normal and our candidate is greater than the crease angle, skip it
                        if(bCreaseAngle && Angle(vert.MDXData.vNormal, vAdd) > static_cast<double>(nCreaseAngle)) continue;

                        if(bSmoothAreaWeighting) vAdd *= face.fArea > 0.000001 ? face.fArea : 0.0;
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
    ReportMdl << "Done post-processing ascii...\n";
}
