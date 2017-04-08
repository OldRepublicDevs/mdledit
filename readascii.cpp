#include "MDL.h"

/**
    Functions:
    ASCII::Read()
    ASCII::BuildAabb()
    MDL::GatherChildren()
    MDL::AsciiPostProcess()
/**/

bool ASCII::Read(MDL & Mdl){
    std::cout<<"We made it into ASCII::Read.\n";

    std::unique_ptr<FileHeader> & FH = Mdl.GetFileData();
    std::string sID;

    //Set stuff to zero
    nPosition = 0;
    FH->MH.Animations.resize(0);
    FH->MH.Names.resize(0);

    int nConvert;
    double fConvert;
    bool bError = false;
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
    bool bTverts1 = false;
    bool bTverts2 = false;
    bool bTverts3 = false;
    bool bWeights = false;
    bool bAabb = false;
    bool bConstraints = false;
    bool bKeys = false;
    bool bTextureNames = false;
    bool bFlarePositions = false;
    bool bFlareSizes = false;
    bool bFlareColorShifts = false;
    unsigned int nNodeCounter = 0;
    Node * PreviousNode;
    unsigned int nDataMax;
    unsigned int nDataCounter;
    int nCurrentIndex = -1;

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
    std::cout<<"Done indexing names ("<<FH->MH.Names.size()<<").\n";

    nPosition = 0;
    ///Loops for every row
    bool * lpbList = nullptr;
    while(nPosition < sBuffer.size() && !bEnd && !bError){
        //First set our current list
        if(bKeys) lpbList = &bKeys;
        else if(bWeights) lpbList = &bWeights;
        else if(bEventlist) lpbList = &bEventlist;
        else if(bAabb) lpbList = &bAabb;
        else if(bVerts) lpbList = &bVerts;
        else if(bFaces) lpbList = &bFaces;
        else if(bTverts) lpbList = &bTverts;
        else if(bTverts1) lpbList = &bTverts1;
        else if(bTverts2) lpbList = &bTverts2;
        else if(bTverts3) lpbList = &bTverts3;
        else if(bConstraints) lpbList = &bConstraints;
        else if(bFlarePositions) lpbList = &bFlarePositions;
        else if(bFlareSizes) lpbList = &bFlareSizes;
        else if(bFlareColorShifts) lpbList = &bFlareColorShifts;
        else if(bTextureNames) lpbList = &bTextureNames;
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
                if(bKeys){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading key data "<<nDataCounter<<".\n";
                    //We've already read the timekeys, we're left with the values
                    Animation & anim = FH->MH.Animations.back();
                    Node & node = anim.ArrayOfNodes.back();
                    Node & geonode = Mdl.GetNodeByNameIndex(node.Head.nNameIndex);
                    Location loc = geonode.GetLocation();
                    Controller & ctrl = node.Head.Controllers.back();

                    if(!ReadFloat(fConvert)) bError = true; //First read the timekey, also check that we're valid

                    if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION && ctrl.nColumnCount < 16){
                        double fX, fY, fZ, fA;
                        if(ReadFloat(fConvert)) fX = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fY = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fZ = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fA = fConvert;
                        else bError = true;
                        Orientation NewOrientKey;
                        NewOrientKey.AA(fX, fY, fZ, fA);
                        NewOrientKey.ConvertToQuaternions();
                        node.Head.ControllerData.push_back(NewOrientKey.Get(QU_X));
                        node.Head.ControllerData.push_back(NewOrientKey.Get(QU_Y));
                        node.Head.ControllerData.push_back(NewOrientKey.Get(QU_Z));
                        node.Head.ControllerData.push_back(NewOrientKey.Get(QU_W));
                    }
                    else if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                        if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fX);
                        else bError = true;
                        if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fY);
                        else bError = true;
                        if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fZ);
                        else bError = true;
                        if(ctrl.nColumnCount > 16){
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                            if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert);
                            else bError = true;
                        }
                    }
                    else{
                        bFound = true;
                        while(bFound){
                            if(ReadFloat(fConvert)){
                                //std::cout<<"Reading non-position keys...\n";
                                node.Head.ControllerData.push_back(fConvert);
                            }
                            else bFound = false;
                        }
                    }
                }
                else if(bEventlist){
                    Sound sound; //New sound
                    if(ReadFloat(fConvert)) sound.fTime = fConvert;
                    else bError = true;
                    bFound = ReadUntilText(sID, false);
                    if(!bFound){
                        std::cout<<"ReadUntilText() Sound name is missing!\n";
                        bError = true;
                    }
                    else if(sID.size() > 32){
                        Error("Sound name larger than the limit, 32 characters! Will truncate and continue.");
                        sID.resize(32);
                    }
                    else if(sID.size() > 16){
                        Warning("Sound name larger than 16 characters! This may cause problems in the game.");
                    }
                    sound.sName = sID;
                    if(bFound){
                        Animation & anim = FH->MH.Animations.back();
                        anim.Sounds.push_back(sound);
                    }
                    else bError = true;
                }
                else if(bAabb){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading aabb data "<<nDataCounter<<".\n";
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
                    else bError = true;
                }
                else if(bVerts){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading verts data "<<nDataCounter<<".\n";
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
                    else bError = true;
                }
                else if(bFaces){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading faces data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    Face face;

                    //Currently we read the regular NWMax version with only a single set of tvert indices
                    if(ReadInt(nConvert)) face.nIndexVertex[0] = nConvert;
                    else bFound = false;
                    if(ReadInt(nConvert)) face.nIndexVertex[1] = nConvert;
                    else bFound = false;
                    if(ReadInt(nConvert)) face.nIndexVertex[2] = nConvert;
                    else bFound = false;
                    if(ReadInt(nConvert)) face.nSmoothingGroup = nConvert;
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

                    if(bFound){
                        node.Mesh.Faces.push_back(face);
                    }
                    else bError = true;
                }
                else if(bTverts){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    Vector vUV;
                    if(ReadFloat(fConvert)) vUV.fX = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vUV.fY = fConvert;
                    else bFound = false;

                    if(bFound) node.Mesh.TempTverts.push_back(vUV);
                    else bError = true;
                }
                else if(bTverts1){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    Vector vUV;
                    if(ReadFloat(fConvert)) vUV.fX = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vUV.fY = fConvert;
                    else bFound = false;

                    if(bFound) node.Mesh.TempTverts1.push_back(vUV);
                    else bError = true;
                }
                else if(bTverts2){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    Vector vUV;
                    if(ReadFloat(fConvert)) vUV.fX = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vUV.fY = fConvert;
                    else bFound = false;

                    if(bFound) node.Mesh.TempTverts2.push_back(vUV);
                    else bError = true;
                }
                else if(bTverts3){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    Vector vUV;
                    if(ReadFloat(fConvert)) vUV.fX = fConvert;
                    else bFound = false;
                    if(ReadFloat(fConvert)) vUV.fY = fConvert;
                    else bFound = false;

                    if(bFound) node.Mesh.TempTverts3.push_back(vUV);
                    else bError = true;
                }
                else if(bConstraints){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading constraints data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.TempConstraints.push_back(fConvert);
                    else bError = true;
                }
                else if(bWeights){
                    //if(DEBUG_LEVEL > 3) std::cout<<"Reading weights data"<<""<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = true;
                    bool bPresent = false;
                    int z = 0;
                    int nBoneIndex = 0;
                    int nNameIndex = 0;
                    Weight weight;
                    std::vector<int> & nWeightIndexes = node.Skin.BoneNameIndexes;
                    while(bFound && z < 4){
                        //Get first name
                        bFound = ReadUntilText(sID, false, true);
                        //if we found a name, loop through the name array to find our name index
                        for(nNameIndex = 0; nNameIndex < FH->MH.Names.size() && bFound; nNameIndex++){
                            //check if there is a match
                            if(FH->MH.Names[nNameIndex].sName == sID){
                                //We have found the name index for the current name, now we need to make sure this name has been indexed in the skin
                                //Check if we already have this name indexed in the skin
                                bPresent = false;
                                for(nBoneIndex = 0; nBoneIndex < nWeightIndexes.size() && !bPresent; ){
                                    if(nWeightIndexes[nBoneIndex] == nNameIndex){
                                        bPresent = true;
                                    }
                                    else nBoneIndex++;
                                }
                                if(!bPresent){
                                    //This is a new name index, so we need to add it to the skin's index list
                                    nWeightIndexes.push_back(nNameIndex);
                                    nBoneIndex = nWeightIndexes.size() - 1; //Update nBoneIndex so it always points to the correct bone

                                    //We also add it to the bonemap.
                                    node.Skin.Bones[nNameIndex].fBonemap = (double) nBoneIndex;

                                    //We can just add it into the mdl's bone index list as well, what the heck
                                    if(nBoneIndex < 18){
                                        node.Skin.nBoneIndexes[nBoneIndex] = nNameIndex;
                                    }
                                    else Warning("Warning! A skin has more than 18 bones, which is the number of available slots in one of the lists. I do not know how this affects the game.");
                                }
                                //By here, we have gotten our nNameIndex and nBoneIndex, and everything is indexed properly
                                weight.fWeightIndex[z] = (double) nBoneIndex;

                                //Since we found the name, we don't need to keep looping anymore
                                nNameIndex = FH->MH.Names.size();
                            }
                        }
                        if(nNameIndex == FH->MH.Names.size() && bFound){
                            //we failed to find the name in the name array. This data is broken.
                            std::cout<<"Reading weights data: failed to find name in name array! Name: "<<sID<<".\n";
                            bFound = false;
                            bError = true;
                        }
                        else if(bFound){
                            //We found the name in the name array. We are therefore ready to read the value as well.
                            if(ReadFloat(fConvert)) weight.fWeightValue[z] = fConvert;
                        }
                        z++;
                    }
                    if(z==1){
                        //This means we exited before writing a single piece of data
                        std::cout<<"Didn't even find one name"<<""<<".\n";
                        std::cout<<"DataCounter: "<<nDataCounter<<". DataMax: "<<nDataMax<<".\n";
                        bError = true;
                    }
                    else if(!bError){
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
                    else bError = true;
                }
                else if(bFlareSizes){
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Light.FlareSizes.push_back(fConvert);
                    else bError = true;
                }
                else if(bFlarePositions){
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Light.FlarePositions.push_back(fConvert);
                    else bError = true;
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
                    else bError = true;
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
            //std::cout<<"No data to read. Read keyword instead"<<""<<".\n";
            bFound = ReadUntilText(sID);
            std::string sLastThree;
            if(sID.length()> 3) sLastThree = sID.substr(sID.length()-3);
            else sLastThree = sID;
            if(!bFound) SkipLine(); //This will have already been done above, no need to look for it again
            else{
                /// Main header stuff
                if(sID == "newmodel"){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    //Read the model name
                    bFound = ReadUntilText(sID, false);
                    if(!bFound){
                        std::cout<<"ReadUntilText() Error! Model name is missing!\n";
                        bError = true;
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
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    ReadUntilText(sID, false); //This should get us the model name first. I won't verify at this point.
                    bFound = ReadUntilText(sID, false);
                    if(!bFound){
                        std::cout<<"ReadUntilText() Supermodel name is missing!\n";
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
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFound = ReadUntilText(sID);
                    if(!bFound){
                        std::cout<<"ReadUntilText() Classification is missing!\n";
                    }
                    if(sID == "other") FH->MH.nClassification = CLASS_OTHER;
                    else if(sID == "effect") FH->MH.nClassification = CLASS_EFFECT;
                    else if(sID == "tile") FH->MH.nClassification = CLASS_TILE;
                    else if(sID == "character") FH->MH.nClassification = CLASS_CHARACTER;
                    else if(sID == "door") FH->MH.nClassification = CLASS_DOOR;
                    else if(sID == "placeable") FH->MH.nClassification = CLASS_PLACEABLE;
                    else if(sID == "saber") FH->MH.nClassification = CLASS_SABER;
                    else if(bFound) std::cout<<"ReadUntilText() has found some text that we cannot interpret: "<<sID<<"\n";
                    SkipLine();
                }
                else if(sID == "setanimationscale"){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    if(ReadFloat(fConvert)) FH->MH.fScale = fConvert;
                    SkipLine();
                }
                else if(sID == "beginmodelgeom"){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bGeometry = true;
                    SkipLine();
                }
                else if(sID == "bmin" && bGeometry && nNode == 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    if(ReadFloat(fConvert)) FH->MH.vBBmin.fX = fConvert;
                    if(ReadFloat(fConvert)) FH->MH.vBBmin.fY = fConvert;
                    if(ReadFloat(fConvert)) FH->MH.vBBmin.fZ = fConvert;
                    SkipLine();
                }
                else if(sID == "bmax" && bGeometry && nNode == 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    if(ReadFloat(fConvert)) FH->MH.vBBmax.fX = fConvert;
                    if(ReadFloat(fConvert)) FH->MH.vBBmax.fY = fConvert;
                    if(ReadFloat(fConvert)) FH->MH.vBBmax.fZ = fConvert;
                    SkipLine();
                }
                else if(sID == "radius" && bGeometry && nNode == 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    if(ReadFloat(fConvert)) FH->MH.fRadius = fConvert;
                    SkipLine();
                }
                /// Common case for nodes, also for animation nodes
                else if(sID == "node" && (bGeometry || bAnimation) && nNode == 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node node; //our new node

                    //Read type
                    int nType;
                    bFound = ReadUntilText(sID); //Get type
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: a node is without any other specification.\n";
                    }
                    if(sID == "dummy") nType = NODE_HAS_HEADER;
                    else if(sID == "light") nType = NODE_HAS_HEADER | NODE_HAS_LIGHT;
                    else if(sID == "emitter") nType = NODE_HAS_HEADER | NODE_HAS_EMITTER;
                    else if(sID == "trimesh") nType = NODE_HAS_HEADER | NODE_HAS_MESH;
                    else if(sID == "skin") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_SKIN;
                    else if(sID == "danglymesh") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_DANGLY;
                    else if(sID == "aabb") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_AABB;
                    else if(sID == "lightsaber") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_SABER;
                    else if(bFound){
                        std::cout<<"ReadUntilText() has found some text (type?) that we cannot interpret: "<<sID<<"\n";
                        bError = true;
                    }
                    if(bFound) node.Head.nType = nType;

                    //Read name
                    bFound = ReadUntilText(sID, false); //Name
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: a node is without a name.\n";
                    }
                    else{
                        bool bErase = false;
                        if(sID.substr(0, 6) == "2081__"){
                            node.Head.nType = node.Head.nType | NODE_HAS_SABER;
                            bErase = true;
                        }
                        node.Head.nNameIndex = Mdl.GetNameIndex(sID);
                        node.Head.nID1 = node.Head.nNameIndex;
                        if(bErase) FH->MH.Names.at(node.Head.nNameIndex).sName = FH->MH.Names.at(node.Head.nNameIndex).sName.substr(6);
                    }

                    //Get animation number (automatically -1 if geo)
                    node.nAnimation = nAnimation;

                    //Initialize node <-- This is now mostly taken care of in the struct definitions, so no need for anything but the default values here.
                    if(nType & NODE_HAS_HEADER){
                        node.Head.vPos.Set(0.0, 0.0, 0.0);
                        node.Head.oOrient.Quaternion(0.0, 0.0, 0.0, 1.0);
                    }
                    if(nType & NODE_HAS_EMITTER){
                        node.Emitter.cDepthTextureName = "NULL";
                    }
                    if(nType & NODE_HAS_MESH){
                        node.Mesh.nTextureNumber = 1;
                        node.Mesh.nSaberUnknown1 = 3;
                        node.Mesh.nMdxDataBitmap = MDX_FLAG_VERTEX | MDX_FLAG_HAS_NORMAL;
                    }

                    //Finish up
                    nNode = nType;
                    if(bFound){
                        if(bGeometry){
                            nCurrentIndex = node.Head.nNameIndex;
                            FH->MH.ArrayOfNodes.at(node.Head.nNameIndex) = std::move(node);
                        }
                        else if(bAnimation){
                            Animation & anim = FH->MH.Animations.back();
                            anim.ArrayOfNodes.push_back(std::move(node));
                        }
                        nNodeCounter++;
                    }
                    else bError;
                    SkipLine();
                }
                else if(sID == "parent" && nNode & NODE_HAS_HEADER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFound = ReadUntilText(sID, false);
                    if(bGeometry){
                        Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                        if(sID == "NULL") node.Head.nParentIndex = -1;
                        else if(bFound){
                            int nNameIndex = 0;
                            //if we found a name, loop through the name array to find our name index
                            bFound = false;
                            while(nNameIndex < FH->MH.Names.size() && !bFound){
                                //check if there is a match
                                if(FH->MH.Names[nNameIndex].sName == sID){
                                    //We have found the name index for the current name
                                    bFound = true;
                                }
                                else nNameIndex++;
                            }
                            if(nNameIndex == FH->MH.Names.size()) std::cout<<"Failed to find parent.\n";
                            else node.Head.nParentIndex = nNameIndex;
                        }
                        else bError = true;
                    }
                    else if(bAnimation){
                        Animation & anim = FH->MH.Animations.back();
                        Node & animnode = anim.ArrayOfNodes.back();
                        if(sID == "NULL") animnode.Head.nParentIndex = -1;
                        else if(bFound){
                            int nNameIndex = 0;
                            //if we found a name, loop through the name array to find our name index
                            bFound = false;
                            while(nNameIndex < FH->MH.Names.size() && !bFound){
                                //check if there is a match
                                if(FH->MH.Names[nNameIndex].sName == sID){
                                    //We have found the name index for the current name
                                    bFound = true;
                                }
                                else nNameIndex++;
                            }
                            if(nNameIndex == FH->MH.Names.size()) std::cout<<"Failed to find parent.\n";
                            else animnode.Head.nParentIndex = nNameIndex;
                        }
                        else bError = true;
                    }
                    SkipLine();
                }
                /// Now come the various node fields
                /// For LIGHT
                else if(sID == "lightpriority" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nLightPriority = nConvert;
                    SkipLine();
                }
                else if(sID == "shadow" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nShadow = nConvert;
                    SkipLine();
                }
                else if(sID == "affectdynamic" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nAffectDynamic = nConvert;
                    SkipLine();
                }
                else if(sID == "ndynamictype" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nDynamicType = nConvert;
                    SkipLine();
                }
                else if(sID == "ambientonly" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nAmbientOnly = nConvert;
                    SkipLine();
                }
                else if(sID == "fadinglight" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nFadingLight = nConvert;
                    SkipLine();
                }
                else if((sID == "lensflares" || sID == "flare") && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nFlare = nConvert;
                    SkipLine();
                }
                else if(sID == "flareradius" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Light.fFlareRadius = fConvert;
                    SkipLine();
                }
                /// For EMITTER
                else if(sID == "deadspace" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fDeadSpace = fConvert;
                    SkipLine();
                }
                else if(sID == "blastlength" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fBlastLength = fConvert;
                    SkipLine();
                }
                else if(sID == "blastradius" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fBlastRadius = fConvert;
                    SkipLine();
                }
                else if(sID == "numbranches" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nBranchCount = nConvert;
                    SkipLine();
                }
                else if(sID == "controlptsmoothing" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fControlPointSmoothing = fConvert;
                    SkipLine();
                }
                else if(sID == "xgrid" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nxGrid = nConvert;
                    SkipLine();
                }
                else if(sID == "ygrid" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nyGrid = nConvert;
                    SkipLine();
                }
                else if(sID == "spawntype" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nSpawnType = nConvert;
                    SkipLine();
                }
                else if(sID == "twosidedtex" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nTwosidedTex = nConvert;
                    SkipLine();
                }
                else if(sID == "loop" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nLoop = nConvert;
                    SkipLine();
                }
                else if(sID == "m_bframeblending" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nFrameBlending = nConvert;
                    SkipLine();
                }
                else if(sID == "m_sdepthtexturename" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
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
                else if(sID == "update" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
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
                else if(sID == "render" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
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
                else if(sID == "blend" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
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
                else if(sID == "texture" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
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
                else if(sID == "chunkname" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
                        if(sID.size() > 16){
                            Error("ChunkName larger than the limit, 16 characters! Will truncate and continue.");
                            sID.resize(16);
                        }
                        node.Emitter.cChunkName = sID;
                    }
                    SkipLine();
                }
                else if(sID == "p2p" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P;
                    }
                    SkipLine();
                }
                else if(sID == "p2p_sel" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P_SEL;
                    }
                    SkipLine();
                }
                else if(sID == "affectedbywind" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_AFFECTED_WIND;
                    }
                    SkipLine();
                }
                else if(sID == "m_istinted" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_TINTED;
                    }
                    SkipLine();
                }
                else if(sID == "bounce" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_BOUNCE;
                    }
                    SkipLine();
                }
                else if(sID == "random" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_RANDOM;
                    }
                    SkipLine();
                }
                else if(sID == "inherit" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT;
                    }
                    SkipLine();
                }
                else if(sID == "inheritvel" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_VEL;
                    }
                    SkipLine();
                }
                else if(sID == "inherit_local" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_LOCAL;
                    }
                    SkipLine();
                }
                else if(sID == "splat" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_SPLAT;
                    }
                    SkipLine();
                }
                else if(sID == "inherit_part" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_PART;
                    }
                    SkipLine();
                }
                else if(sID == "depth_texture" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_DEPTH_TEXTURE;
                    }
                    SkipLine();
                }
                else if(sID == "renderorder" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_RENDER_ORDER;
                    }
                    SkipLine();
                }
                /// For MESH
                else if(sID == "bitmap" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
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
                else if(sID == "bitmap2" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
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
                else if(sID == "texture0" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
                        if(sID.size() > 16){
                            Error("Texture0 name larger than the limit, 16 characters! Will truncate and continue.");
                            sID.resize(16);
                        }
                        node.Mesh.cTexture3 = sID;
                    }
                    SkipLine();
                }
                else if(sID == "texture1" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
                        if(sID.size() > 16){
                            Error("Texture1 name larger than the limit, 16 characters! Will truncate and continue.");
                            sID.resize(16);
                        }
                        node.Mesh.cTexture4 = sID;
                    }
                    SkipLine();
                }
                else if(sID == "lightmap" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
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
                else if(sID == "diffuse" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fR = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fG = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Mesh.fDiffuse.fB = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "ambient" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fAmbient.fR = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Mesh.fAmbient.fG = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Mesh.fAmbient.fB = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "tangentspace" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert != 0) node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_TANGENT1;
                    }
                    SkipLine();
                }
                else if(sID == "lightmapped" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nHasLightmap = nConvert;
                    SkipLine();
                }
                else if(sID == "rotatetexture" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nRotateTexture = nConvert;
                    SkipLine();
                }
                else if(sID == "m_blsbackgroundgeometry" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nBackgroundGeometry = nConvert;
                    SkipLine();
                }
                else if(sID == "dirt_enabled" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nDirtEnabled = nConvert;
                    SkipLine();
                }
                else if(sID == "dirt_texture" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nDirtTexture = nConvert;
                    SkipLine();
                }
                else if(sID == "dirt_worldspace" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nDirtCoordSpace = nConvert;
                    SkipLine();
                }
                else if(sID == "hologram_donotdraw" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nHideInHolograms = nConvert;
                    SkipLine();
                }
                else if(sID == "beaming" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nBeaming = nConvert;
                    SkipLine();
                }
                else if(sID == "render" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nRender = nConvert;
                    SkipLine();
                }
                else if(sID == "shadow" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nShadow = nConvert;
                    SkipLine();
                }
                else if(sID == "transparencyhint" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nTransparencyHint = nConvert;
                    SkipLine();
                }
                else if(sID == "animateuv" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nAnimateUV = nConvert;
                    SkipLine();
                }
                else if(sID == "uvdirectionx" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVDirectionX = fConvert;
                    SkipLine();
                }
                else if(sID == "uvdirectiony" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVDirectionY = fConvert;
                    SkipLine();
                }
                else if(sID == "uvjitter" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVJitter = fConvert;
                    SkipLine();
                }
                else if(sID == "uvjitterspeed" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVJitterSpeed = fConvert;
                    SkipLine();
                }
                /// For DANGLY
                else if(sID == "displacement" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fDisplacement = fConvert;
                    SkipLine();
                }
                else if(sID == "tightness" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fTightness = fConvert;
                    SkipLine();
                }
                else if(sID == "period" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fPeriod = fConvert;
                    SkipLine();
                }
                /// Next we have the DATA LISTS
                else if(sID == "verts" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bVerts = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "faces" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFaces = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "tverts" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTverts = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV1;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "lightmaptverts" && nNode & NODE_HAS_MESH || sID == "tverts1" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTverts1 = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV2;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "tverts2" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTverts2 = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV3;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "tverts3" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTverts3 = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV4;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "weights" && nNode & NODE_HAS_SKIN){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bWeights = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Skin.Bones.resize(FH->MH.Names.size());
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "aabb" && nNode & NODE_HAS_AABB){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bAabb = true;
                    int nSavePos = nPosition; //Save position
                    nDataMax = -1;
                    nDataCounter = 0;
                    if(ReadFloat(fConvert)) nPosition = nSavePos; //The data starts in this line, revert position
                    else SkipLine(); //if not, then data starts next line, so it is okay to skip
                }
                else if(sID == "constraints" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bConstraints = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "texturenames" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTextureNames = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "flaresizes" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFlareSizes = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "flarepositions" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFlarePositions = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "flarecolorshifts" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFlareColorShifts = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    SkipLine();
                }
                /// Next we have bezier controllers
                else if(safesubstr(sID, sID.length()-9) == "bezierkey" && ReturnController(safesubstr(sID, 0, sID.length()-9))){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Animation & anim = FH->MH.Animations.back();
                    Node & node = anim.ArrayOfNodes.back();
                    Controller ctrl;
                    ctrl.nAnimation = nAnimation;
                    ctrl.nNameIndex = node.Head.nNameIndex;
                    ctrl.nControllerType = ReturnController(safesubstr(sID, 0, sID.length()-9));
                    ctrl.nTimekeyStart = node.Head.ControllerData.size();
                    ctrl.nUnknown2 = -1;
                    if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                        ctrl.nUnknown2 = ctrl.nControllerType + 8;
                    }
                    if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_SCALING ||
                       Mdl.GetNodeByNameIndex(ctrl.nNameIndex).Head.nType % NODE_HAS_MESH){
                        //For non-emitter and non-light controllers
                        ctrl.nPadding[0] = 50;
                        ctrl.nPadding[1] = 18;
                        ctrl.nPadding[2] = 0;
                    }
                    else{
                        //all emitter and light controllers
                        ctrl.nPadding[0] = 51;
                        ctrl.nPadding[1] = 18;
                        ctrl.nPadding[2] = 0;
                    }
                    //This is all we can tell right now.
                    int nSavePos = nPosition; //Save position

                    //To continue let's first get the column count
                    SkipLine();
                    nDataCounter = 0;
                    bFound = true;
                    while(bFound){
                        if(!EmptyRow() || nDataCounter > 0){
                            if(ReadFloat(fConvert, sID)){
                                nDataCounter++;
                            }
                            else{
                                std::cout<<"This is not a float: "<<sID<<".\n";
                                bError = true;
                            }
                            if(EmptyRow()) bFound = false;
                        }
                        else SkipLine();
                    }
                    if(nDataCounter == 0){
                        std::cout<<"keyed controller error: no data at all in the first line after the token line.\n";
                        bError = true;
                    }

                    ctrl.nColumnCount = 16 + (nDataCounter - 1) / 3;
                    //std::cout<<"Column count for "<<sID<<" is "<<nDataCounter - 1<<"\n";

                    //Now let's get the row count. It is actually best to also read the timekeys in this step
                    nPosition = nSavePos;
                    SkipLine();
                    nDataCounter = 0;
                    bFound = true;
                    int nSavePos2;
                    while(bFound){
                        //std::cout<<"Looking.. Position="<<nPosition<<".\n";
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
                                        std::cout<<"This is not a float: "<<sID<<".\n";
                                        bError = true;
                                    }
                                    nDataCounter++;
                                }
                                SkipLine();
                            }
                        }
                    }
                    if(nDataCounter == 0){
                        std::cout<<"keyed controller error: no data at all in the first line after the token line.\n";
                        bError = true;
                    }
                    ctrl.nValueCount = nDataCounter;
                    ctrl.nDataStart = node.Head.ControllerData.size();

                    //We now have all the data, append the controller, reset the position and prepare for actually reading the keys.
                    nPosition = nSavePos;
                    if(!bError){
                        node.Head.Controllers.push_back(ctrl);
                    }
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    bKeys = true;
                    SkipLine();
                }
                /// Next we have keyed controllers
                else if(safesubstr(sID, sID.length()-3) == "key" && ReturnController(safesubstr(sID, 0, sID.length()-3))){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Animation & anim = FH->MH.Animations.back();
                    Node & node = anim.ArrayOfNodes.back();
                    Controller ctrl;
                    ctrl.nAnimation = nAnimation;
                    ctrl.nNameIndex = node.Head.nNameIndex;
                    ctrl.nControllerType = ReturnController(safesubstr(sID, 0, sID.length()-3));
                    ctrl.nTimekeyStart = node.Head.ControllerData.size();
                    ctrl.nUnknown2 = -1;
                    if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                        ctrl.nUnknown2 = ctrl.nControllerType + 8;
                    }
                    if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION ||
                       ctrl.nControllerType == CONTROLLER_HEADER_SCALING ||
                       Mdl.GetNodeByNameIndex(ctrl.nNameIndex).Head.nType % NODE_HAS_MESH){
                        //For non-emitter and non-light controllers
                        ctrl.nPadding[0] = 50;
                        ctrl.nPadding[1] = 18;
                        ctrl.nPadding[2] = 0;
                    }
                    else{
                        //all emitter and light controllers
                        ctrl.nPadding[0] = 51;
                        ctrl.nPadding[1] = 18;
                        ctrl.nPadding[2] = 0;
                    }
                    //This is all we can tell right now.
                    int nSavePos = nPosition; //Save position

                    //To continue let's first get the column count
                    SkipLine();
                    nDataCounter = 0;
                    bFound = true;
                    while(bFound){
                        if(!EmptyRow() || nDataCounter > 0){
                            if(ReadFloat(fConvert, sID)){
                                nDataCounter++;
                            }
                            else{
                                std::cout<<"This is not a float: "<<sID<<".\n";
                                bError = true;
                            }
                            if(EmptyRow()) bFound = false;
                        }
                        else SkipLine();
                    }
                    if(nDataCounter == 0){
                        std::cout<<"keyed controller error: no data at all in the first line after the token line.\n";
                        bError = true;
                    }

                    ctrl.nColumnCount = nDataCounter - 1;
                    //std::cout<<"Column count for "<<sID<<" is "<<nDataCounter - 1<<"\n";

                    //Now let's get the row count. It is actually best to also read the timekeys in this step
                    nPosition = nSavePos;
                    SkipLine();
                    nDataCounter = 0;
                    bFound = true;
                    int nSavePos2;
                    while(bFound){
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
                                        std::cout<<"This is not a float: "<<sID<<".\n";
                                        bError = true;
                                    }
                                    nDataCounter++;
                                }
                                SkipLine();
                            }
                        }
                    }
                    if(nDataCounter == 0){
                        std::cout<<"keyed controller error: no data at all in the first line after the token line.\n";
                        bError = true;
                    }
                    ctrl.nValueCount = nDataCounter;
                    ctrl.nDataStart = node.Head.ControllerData.size();

                    //We now have all the data, append the controller, reset the position and prepare for actually reading the keys.
                    nPosition = nSavePos;
                    if(!bError){
                        node.Head.Controllers.push_back(ctrl);
                    }
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else nDataMax = -1;
                    nDataCounter = 0;
                    bKeys = true;
                    SkipLine();
                }
                /// Next we have single controllers
                else if(ReturnController(sID)){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    Controller ctrl;
                    ctrl.nControllerType = ReturnController(sID);
                    ctrl.nUnknown2 = -1;
                    ctrl.nTimekeyStart = node.Head.ControllerData.size();
                    ctrl.nDataStart = node.Head.ControllerData.size() + 1;
                    ctrl.nValueCount = 1;
                    ctrl.nNameIndex = node.Head.nNameIndex;
                    ctrl.nAnimation = -1;
                    if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                        //Sometimes orientation and scaling have these values as well
                        ctrl.nPadding[0] = 12;
                        ctrl.nPadding[1] = 76;
                        ctrl.nPadding[2] = 0;
                    }
                    else if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                        ctrl.nPadding[0] = -59;
                        ctrl.nPadding[1] = 73;
                        ctrl.nPadding[2] = 0;
                    }
                    else if(ctrl.nControllerType == CONTROLLER_HEADER_SCALING){
                        ctrl.nPadding[0] = 49;
                        ctrl.nPadding[1] = 18;
                        ctrl.nPadding[2] = 0;
                    }
                    else if(Mdl.GetNodeByNameIndex(ctrl.nNameIndex).Head.nType & NODE_HAS_LIGHT){
                        ctrl.nPadding[0] = -5;
                        ctrl.nPadding[1] = 54;
                        ctrl.nPadding[2] = 0;
                    }
                    else{
                        //This is largely gonna be for mesh and emitter controllers, they can have a range of unknown values.
                        ctrl.nPadding[0] = 0;
                        ctrl.nPadding[1] = 0;
                        ctrl.nPadding[2] = 1; //This one seems to always be non-zero
                    }

                    //First put in the 0.0 to fill the required timekey
                    node.Head.ControllerData.push_back(0.0);

                    //Now fill values
                    if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                        double fX, fY, fZ, fAngle;
                        if(ReadFloat(fConvert)) fX = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fY = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fZ = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) fAngle = fConvert;
                        else bError = true;
                        node.Head.oOrient.AA(fX, fY, fZ, fAngle);
                        node.Head.oOrient.ConvertToQuaternions();
                        node.Head.ControllerData.push_back(node.Head.oOrient.Get(QU_X));
                        node.Head.ControllerData.push_back(node.Head.oOrient.Get(QU_Y));
                        node.Head.ControllerData.push_back(node.Head.oOrient.Get(QU_Z));
                        node.Head.ControllerData.push_back(node.Head.oOrient.Get(QU_W));
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
                            std::cout<<"Single controller error: no data at all.\n";
                            bError = true;
                        }
                        ctrl.nColumnCount = nDataCounter;

                        if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                            node.Head.vPos.fX = node.Head.ControllerData.at(ctrl.nDataStart + 0);
                            node.Head.vPos.fY = node.Head.ControllerData.at(ctrl.nDataStart + 1);
                            node.Head.vPos.fZ = node.Head.ControllerData.at(ctrl.nDataStart + 2);
                        }
                    }

                    if(!bError){
                        node.Head.Controllers.push_back(ctrl);
                    }
                    SkipLine();
                }
                /// General ending tokens
                else if(sID == "endnode" && nNode > 0){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    nNode = 0;
                    nCurrentIndex = -1;
                    SkipLine();
                }
                else if(sID == "endmodelgeom"){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bGeometry = false;
                    FH->MH.nNodeCount = nNodeCounter;
                    nNodeCounter = 0;
                    SkipLine();

                }
                /// Animation tokens
                else if(sID == "newanim" && !bAnimation){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bAnimation = true;
                    nAnimation++;

                    //Get Animation Name
                    bFound = ReadUntilText(sID, false); //Name
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: an animation name is missing!\n";
                        bError = true;
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
                    anim.SoundArray.nCount = 0;
                    anim.Sounds.resize(0);
                    anim.nPadding[0] = 0;
                    anim.nPadding[1] = 0;
                    anim.nPadding[2] = 0;
                    anim.ArrayOfNodes.reserve(FH->MH.Names.size());

                    //Finish up
                    if(bFound) FH->MH.Animations.push_back(anim);
                    SkipLine();
                }
                else if(sID == "length" && bAnimation){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Animation & anim = FH->MH.Animations.back();
                    if(ReadFloat(fConvert)) anim.fLength = fConvert;
                    SkipLine();
                }
                else if(sID == "transtime" && bAnimation){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Animation & anim = FH->MH.Animations.back();
                    if(ReadFloat(fConvert)) anim.fTransition = fConvert;
                    SkipLine();
                }
                else if(sID == "animroot" && bAnimation){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFound = ReadUntilText(sID, false);
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: an animation's AnimRoot name is missing.\n";
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
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bEventlist = true;
                    SkipLine();
                }
                else if(sID == "doneanim" && bAnimation){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bAnimation = false;
                    nNodeCounter = 0;
                    SkipLine();
                }
                else if(sID == "donemodel"){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bEnd = true;
                }
                //These are the names we should ignore, because we expect them to appear, but we have no use for them
                else if(sID == "filedependancy") SkipLine();
                else if(sID == "specular") SkipLine();
                else if(sID == "wirecolor") SkipLine();
                else if(sID == "shininess") SkipLine();
                else if(sID == "name") SkipLine();
                else{
                    std::cout<<"ReadUntilText() has found some text that we cannot interpret: "<<sID<<"\n";
                    SkipLine();
                }
            }
        }
    }
    std::cout<<"Done reading ascii, checking for errors...\n";
    if(bError){
        Error("Some kind of error has occured! Check the console! The program will now cleanup what it has read since the data is now broken.");
        return false;
    }

    //Seems best to take care of aabb immediately
    for(int n = 0; n < FH->MH.ArrayOfNodes.size(); n++){
        Node & node = FH->MH.ArrayOfNodes[n];
        if(node.Head.nType & NODE_HAS_AABB){
            node.Walkmesh.RootAabb = node.Walkmesh.ArrayOfAabb.front();
            int nCounter = 1;
            BuildAabb(node.Walkmesh.RootAabb, node.Walkmesh.ArrayOfAabb, nCounter);
        }
    }

    return true;
}

void ASCII::BuildAabb(Aabb & AABB, std::vector<Aabb> & ArrayOfAabb, int & nCounter){
    if(AABB.nID == -1){
        if(nCounter >= ArrayOfAabb.size()) return;
        AABB.nChild1 = 1;
        AABB.nChild2 = 1;
        AABB.Child1.push_back(ArrayOfAabb[nCounter]);
        nCounter++;
        BuildAabb(AABB.Child1.front(), ArrayOfAabb, nCounter);
        AABB.Child2.push_back(ArrayOfAabb[nCounter]);
        nCounter++;
        BuildAabb(AABB.Child2.front(), ArrayOfAabb, nCounter);
    }
    else{
        AABB.nChild1 = 0;
        AABB.nChild2 = 0;
    }
}

void MDL::GatherChildren(Node & node, std::vector<Node> & ArrayOfNodes, Vector vFromRoot){
    node.Head.ChildIndices.resize(0); //Reset child array
    node.Head.ChildIndices.reserve(ArrayOfNodes.size());

    if(node.Head.nType & NODE_HAS_MESH){
        /// Let's do the transformations/translations here. First orientation, then translation.
        Location loc = node.GetLocation();
        vFromRoot.Rotate(loc.oOrientation);
        vFromRoot+= loc.vPosition;

        node.Head.vFromRoot = vFromRoot;
    }

    //Update the animation node IDs
    if(node.nAnimation != -1){
        node.Head.nID1 = GetNodeByNameIndex(node.Head.nNameIndex).Head.nID1;
    }

    for(int n = 0; n < ArrayOfNodes.size(); n++){
        if(ArrayOfNodes[n].Head.nParentIndex == node.Head.nNameIndex){
            //The nodes with this index is a child, adopt it
            //node.Head.Children.push_back(ArrayOfNodes[n]);
            node.Head.ChildIndices.push_back(ArrayOfNodes[n].Head.nNameIndex);
            GatherChildren(ArrayOfNodes[n], ArrayOfNodes, vFromRoot);
        }
    }
    node.Head.ChildIndices.shrink_to_fit();
}

void MDL::AsciiPostProcess(){
    std::cout<<"Ascii post-processing...\n";
    FileHeader & Data = *FH;

    /// PART 0 ///
    /// Get rid of the duplication marks
    for(int n = 0; n < Data.MH.Names.size(); n++){
        std::string & sNode = Data.MH.Names.at(n).sName;
        for(int i = 0; i < 4 && sNode.length() > 5 + i; i++){
            if(safesubstr(sNode, sNode.length() - 5 - i, 5) == "__dpl"){
                    sNode = safesubstr(sNode, 0, sNode.length() - 5 - i);
                    i = 6;
            }
        }
    }

    /// PART 1 ///
    /// Do supernodes
    /// This loads up all the supermodels and calculates the supernode numbers
    Data.MH.GH.nTotalNumberOfNodes = Data.MH.nNodeCount;
    if(Data.MH.cSupermodelName != "NULL"){
        std::vector<MDL> Supermodels;
        bool bFoundAll = LoadSupermodel(*this, Supermodels);
        //First, update the TotalNodeCount
        if(Supermodels.size() != 0 && bFoundAll){
            int nTotalSupermodelNodes = Supermodels.front().GetFileData()->MH.GH.nTotalNumberOfNodes;
            std::cout<<"Total Supermodel Nodes: "<<nTotalSupermodelNodes<<"\n";
            if(nTotalSupermodelNodes > 0)
                Data.MH.GH.nTotalNumberOfNodes += 1 + nTotalSupermodelNodes;

            //Next we need the largest supernode number. The largest is definitely in the first supermodel, right? right?!
            short nMaxSupernode = 0;
            for(int n = 0; n < Supermodels.front().GetFileData()->MH.ArrayOfNodes.size(); n++){
                nMaxSupernode = std::max(nMaxSupernode, Supermodels.front().GetFileData()->MH.ArrayOfNodes.at(n).Head.nID1);
            }
            short nCurrentSupernode = nMaxSupernode + 1;

            //Update the supernode number for every node
            for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
                Node & node = Data.MH.ArrayOfNodes.at(n);
                //If Name Index is 0, then supernode is zero. But this is the default situation, so we just need to exclude that case.
                if(node.Head.nNameIndex != 0){
                    std::string & sNodeName = Data.MH.Names.at(node.Head.nNameIndex).sName;
                    std::string & sParentName = Data.MH.Names.at(node.Head.nParentIndex).sName;
                    bool bFound = false;
                    for(int m = 0; m < Supermodels.size() && !bFound; m++){
                        MDL & supermdl = Supermodels.at(m);
                        FileHeader & Data2 = *Supermodels.at(m).GetFileData();
                        for(int s = 0; s < Data2.MH.Names.size() && !bFound; s++){
                            if(Data2.MH.Names.at(s).sName == sNodeName){
                                if(Data2.MH.Names.at(supermdl.GetNodeByNameIndex(s).Head.nParentIndex).sName == sParentName || supermdl.GetNodeByNameIndex(s).Head.nParentIndex == 0){
                                    //Both the name and the parent name are equal. We really are dealing with the matching node. Get its supernode.
                                    node.Head.nID1 = supermdl.GetNodeByNameIndex(s).Head.nID1;
                                    bFound = true;
                                }
                            }
                        }
                    }
                    if(!bFound){
                        //I'm reusing the variable, but we're looking for the parent now
                        bFound = false;
                        if(Data.MH.ArrayOfNodes.at(node.Head.nParentIndex).Head.nNameIndex != 0){
                            std::string & sNodeName2 = Data.MH.Names.at(Data.MH.ArrayOfNodes.at(node.Head.nParentIndex).Head.nNameIndex).sName;
                            std::string & sParentName2 = Data.MH.Names.at(Data.MH.ArrayOfNodes.at(node.Head.nParentIndex).Head.nParentIndex).sName;
                            for(int m = 0; m < Supermodels.size() && !bFound; m++){
                                MDL & supermdl = Supermodels.at(m);
                                FileHeader & Data2 = *Supermodels.at(m).GetFileData();
                                for(int s = 0; s < Data2.MH.Names.size() && !bFound; s++){
                                    if(Data2.MH.Names.at(s).sName == sNodeName2){
                                        if(Data2.MH.Names.at(supermdl.GetNodeByNameIndex(s).Head.nParentIndex).sName == sParentName2 || supermdl.GetNodeByNameIndex(s).Head.nParentIndex == 0){
                                            //Both the name and the parent name are equal. We really are dealing with the matching node.
                                            bFound = true;
                                        }
                                    }
                                }
                            }
                        }
                        else bFound = true;
                        if(bFound){
                            //This means that the parent is also present in the supermodel. In this case, we just give it the next supernode number.
                            node.Head.nID1 = nCurrentSupernode;
                            nCurrentSupernode++;
                        }
                        else{
                            //If we couldn't find the parent, it must be new. So, we have a different algorithm for the ID.
                            node.Head.nID1 += nTotalSupermodelNodes + 1;
                        }
                    }
                }
            }
        }
        Supermodels.clear();
        Supermodels.shrink_to_fit();
    }

    /// PART 2 ///
    /// Gather all the children (the indices!!!)
    /// This part means going from every node only specifying its parent to everyone node also specifying its children
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

    /// PART 3 ///
    /// Interpret ascii data
    /// This constructs the Mesh.Vertices, Mesh.VertIndices, Dangly.Data2, Dangly.Constraints and Saber.SaberData structures.
    /// And not to forget the weights.
    for(int n = 0; n < Data.MH.ArrayOfNodes.size(); n++){
        Node & node = Data.MH.ArrayOfNodes.at(n);
        if(node.Head.nType & NODE_HAS_MESH && !(node.Head.nType & NODE_HAS_SABER)){
            for(int f = 0; f < node.Mesh.Faces.size(); f++){
                Face & face = node.Mesh.Faces.at(f);
                for(int i = 0; i < 3; i++){
                    if(!face.bProcessed[i]){
                        bool bIgnoreVert = true, bIgnoreTvert = true, bIgnoreTvert1 = true, bIgnoreTvert2 = true, bIgnoreTvert3 = true;
                        Vertex vert;
                        SaberDataStruct saberdata;
                        vert.MDXData.nNameIndex = node.Head.nNameIndex;

                        if(node.Mesh.TempVerts.size() > 0){
                            bIgnoreVert = false;
                            vert.assign(node.Mesh.TempVerts.at(face.nIndexVertex[i]));

                            vert.vFromRoot = node.Mesh.TempVerts.at(face.nIndexVertex[i]);
                            vert.vFromRoot.Rotate(node.GetLocation().oOrientation);
                            vert.vFromRoot+=node.Head.vFromRoot;

                            vert.MDXData.vVertex = node.Mesh.TempVerts.at(face.nIndexVertex[i]);

                            if(node.Head.nType & NODE_HAS_DANGLY){
                                node.Dangly.Data2.push_back(node.Mesh.TempVerts.at(face.nIndexVertex[i]));
                                node.Dangly.Constraints.push_back(node.Dangly.TempConstraints.at(face.nIndexVertex[i]));
                            }

                            if(node.Head.nType & NODE_HAS_SKIN){
                                vert.MDXData.Weights = node.Skin.TempWeights.at(face.nIndexVertex[i]);
                            }
                        }
                        if(node.Mesh.TempTverts.size() > 0 && face.nTextureCount >= 1){
                            bIgnoreTvert = false;
                            vert.MDXData.vUV1 = node.Mesh.TempTverts.at(face.nIndexTvert[i]);
                        }
                        //We simply ignore the other three UVs if this is a saber
                        if(node.Mesh.TempTverts1.size() > 0 && face.nTextureCount >= 2){
                            bIgnoreTvert1 = false;
                            vert.MDXData.vUV2 = node.Mesh.TempTverts1.at(face.nIndexTvert1[i]);
                        }
                        if(node.Mesh.TempTverts2.size() > 0 && face.nTextureCount >= 3){
                            bIgnoreTvert2 = false;
                            vert.MDXData.vUV3 = node.Mesh.TempTverts2.at(face.nIndexTvert2[i]);
                        }
                        if(node.Mesh.TempTverts3.size() > 0 && face.nTextureCount >= 4){
                            bIgnoreTvert3 = false;
                            vert.MDXData.vUV4 = node.Mesh.TempTverts3.at(face.nIndexTvert3[i]);
                        }

                        //Find identical verts
                        for(int f2 = f; f2 < node.Mesh.Faces.size(); f2++){
                            Face & face2 = node.Mesh.Faces.at(f2);
                            for(int i2 = 0; i2 < 3; i2++){
                                //Make sure that we're only changing what's past our current position if we are in the same face.
                                if(f2 != f || i2 > i){
                                    if( (face2.nIndexVertex[i2] == face.nIndexVertex[i] || bIgnoreVert) &&
                                        (face2.nIndexTvert[i2] == face.nIndexTvert[i] || bIgnoreTvert) &&
                                        (face2.nIndexTvert1[i2] == face.nIndexTvert1[i] || bIgnoreTvert1) &&
                                        (face2.nIndexTvert2[i2] == face.nIndexTvert2[i] || bIgnoreTvert2) &&
                                        (face2.nIndexTvert3[i2] == face.nIndexTvert3[i] || bIgnoreTvert3) &&
                                        !face2.bProcessed[i2] )
                                    {
                                        //If we find a reference to the exact same vert, we have to link to it
                                        //Actually we only need to link vert indexes, the correct UV are now already included in the Vertex struct
                                        face2.nIndexVertex[i2] = node.Mesh.Vertices.size();
                                        face2.bProcessed[i2] = true;
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
                VertIndicesStruct vertindex;
                vertindex.nValues[0] = face.nIndexVertex[0];
                vertindex.nValues[1] = face.nIndexVertex[1];
                vertindex.nValues[2] = face.nIndexVertex[2];
                node.Mesh.VertIndices.push_back(std::move(vertindex));
            }
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
        else if(node.Head.nType & NODE_HAS_SABER){

            ///Saber interpretation goes here.

        }
    }

    /// DONE ///
    std::cout<<"Done post-processing ascii...\n";
}
