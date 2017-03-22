#include <algorithm>
#include "MDL.h"

bool Ascii::Read(FileHeader * FH){
    std::string sID;

    std::cout<<"We made it into Ascii::Read.\n";

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
    bool bLightmapTverts = false;
    bool bWeights = false;
    bool bAabb = false;
    bool bConstraints = false;
    bool bKeys = false;
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
    while(nPosition < nBufferSize && !bStop){
        //std::cout<<"The name indexing while(), nPosition="<<nPosition<<".\n";
        ReadUntilText(sID);
        while(sID != "node" && nPosition < nBufferSize && !bStop){
            //std::cout<<"The node checking while(), nPosition="<<nPosition<<".\n";
            //std::cout<<"Not a node, skipping line, nPosition="<<nPosition<<" of "<<nBufferSize<<".\n";
            SkipLine();
            //std::cout<<"No throw in SkipLine(), nPosition="<<nPosition<<" of "<<nBufferSize<<".\n";
            ReadUntilText(sID);
            //std::cout<<"No throw in ReadUntilText(), nPosition="<<nPosition<<" of "<<nBufferSize<<".\n";
            if(sID == "endmodelgeom") bStop = true;
        }
        if(nPosition < nBufferSize && !bStop){
            //Once we get here, we have found a node. The name is only a keyword's strlen away!
            ReadUntilText(sID); //Get keyword
            ReadUntilText(sID, false); //Get name
            //std::cout<<"Found a node! ("<<sID<<") At nPosition="<<nPosition<<" of "<<nBufferSize<<".\n";
            //Got it! Now to save it the name array.
            Name name; //our new name
            name.cName = sID;
            FH->MH.Names.push_back(name);
        }
    }
    std::cout<<"Done indexing names ("<<FH->MH.Names.size()<<").\n";

    nPosition = 0;
    std::vector<int> nWeightIndexes;
    ///Loops for every row
    while(nPosition < nBufferSize && !bEnd && !bError){
        //First, check if we have a blank line, we'll just skip it here.
        if(EmptyRow()){
            SkipLine();
        }
        //Second, check if we are in some list of data. We should not look for a keyword then.
        else if(bKeys){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading keys data"<<""<<".\n";
            //This is keyed controller data
            //First, check if we're done. Save your position, cuz we'll need to revert if we're not done
            int nSavePos = nPosition;
            ReadUntilText(sID);
            if(sID == "endlist"){
                bKeys = false;
                SkipLine();
            }
            else{
                //Revert back to old position
                nPosition = nSavePos;

                //We've already read the timekeys, we're left with the values
                Animation & anim = FH->MH.Animations.back();
                Node & node = anim.ArrayOfNodes.back();
                Node & geonode = Model.GetNodeByNameIndex(node.Head.nNameIndex);
                Location loc = geonode.GetLocation();
                //Node & geonode = GetNodeByNameIndex(*FH, node.Head.nNameIndex);
                /*
                int nCtrl = 0;
                for(int c = 0; c < geonode.Head.Controllers.size(); ){
                    if(geonode.Head.Controllers.at(c).nControllerType == CONTROLLER_HEADER_POSITION){
                        nCtrl = c;
                        c = geonode.Head.Controllers.size();
                    }
                    else c++;
                }
                Controller & posctrl = geonode.Head.Controllers.at(nCtrl);
                */
                Controller & ctrl = node.Head.Controllers.back();

                if(!ReadFloat(fConvert)) bError = true; //First read the timekey, also check that we're valid

                if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                    Orientation NewOrientKey;
                    if(ReadFloat(fConvert)) NewOrientKey.fX = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) NewOrientKey.fY = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) NewOrientKey.fZ = fConvert;
                    else bError = true;
                    if(ReadFloat(fConvert)) NewOrientKey.fAngle = fConvert;
                    else bError = true;
                    NewOrientKey.ConvertToQuaternions();
                    node.Head.ControllerData.push_back(NewOrientKey.qX);
                    node.Head.ControllerData.push_back(NewOrientKey.qY);
                    node.Head.ControllerData.push_back(NewOrientKey.qZ);
                    node.Head.ControllerData.push_back(NewOrientKey.qW);
                }
                else if(ctrl.nControllerType == CONTROLLER_HEADER_POSITION){
                    if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fX);
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fY);
                    else bError = true;
                    if(ReadFloat(fConvert)) node.Head.ControllerData.push_back(fConvert - loc.vPosition.fZ);
                    else bError = true;
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

                SkipLine();
            }
        }
        else if(bEventlist){
            //First, check if we're done. Save your position, cuz we'll need to revert if we're not done
            int nSavePos = nPosition;
            ReadUntilText(sID);
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading sounds data("<<sID<<").\n";
            if(sID == "endlist"){
                bEventlist = false;
                SkipLine();
            }
            else{
                //Revert back to old position
                nPosition = nSavePos;
                Sound sound; //New sound
                if(ReadFloat(fConvert)) sound.fTime = fConvert;
                else bError = true;
                bFound = ReadUntilText(sID, false);
                if(!bFound){
                    std::cout<<"ReadUntilText() Sound name is missing!\n";
                    bError = true;
                }
                else if(sID.size() > 32){
                    Error("Sound name larger than 32 characters! Will truncate and continue.");
                    sID.resize(32);
                }
                else if(sID.size() > 16){
                    Warning("Sound name larger than 16 characters! This may cause problems in the game.");
                }
                sound.cName = sID;
                if(bFound){
                    Animation & anim = FH->MH.Animations.back();
                    anim.Sounds.push_back(sound);
                }
                SkipLine();
            }
        }
        else if(bAabb){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading aabb data"<<""<<".\n";
            //First, check if we're done. Save your position, cuz we'll need to revert if we're not done
            int nSavePos = nPosition;
            ReadUntilText(sID);
            if(sID == "endnode"){
                bAabb = false;
                nNode = 0; //aabb is special in that it goes on until the end of the node
                SkipLine();
            }
            else{
                //Revert back to old position
                nPosition = nSavePos;
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
                SkipLine();
            }
        }
        else if(bVerts){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading verts data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            Vertex vert; //New vert
            vert.MDXData.nNameIndex = node.Head.nNameIndex;
            SaberDataStruct saberdata; //New saberdata
            if(ReadFloat(fConvert)) vert.fX = fConvert;
            else bFound = false;
            if(ReadFloat(fConvert)) vert.fY = fConvert;
            else bFound = false;
            if(ReadFloat(fConvert)) vert.fZ = fConvert;
            else bFound = false;
            if(nNode & NODE_HAS_SABER){
                SaberDataStruct saberdata;
                saberdata.vVertex.fX = vert.fX;
                saberdata.vVertex.fY = vert.fY;
                saberdata.vVertex.fZ = vert.fZ;
            }
            else{
                vert.MDXData.vVertex.fX = vert.fX;
                vert.MDXData.vVertex.fY = vert.fY;
                vert.MDXData.vVertex.fZ = vert.fZ;
            }

            if(bFound){
                if(nNode & NODE_HAS_DANGLY){
                    node.Dangly.Data2.push_back(Vector(vert.fX, vert.fY, vert.fZ));
                }
                node.Mesh.Vertices.push_back(std::move(vert));
                if(nNode & NODE_HAS_SABER) node.Saber.SaberData.push_back(std::move(saberdata));
            }
            else bError = true;
            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bVerts = false;
                //node.Mesh.nNumberOfVerts = node.Mesh.Vertices.size();
            }
            SkipLine();
        }
        else if(bFaces){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading faces data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            Face face;
            face.nAdjacentFaces[0] = -1;
            face.nAdjacentFaces[1] = -1;
            face.nAdjacentFaces[2] = -1;
            VertIndicesStruct vertindex;
            if(ReadInt(nConvert)) face.nIndexVertex[0] = nConvert;
            else bFound = false;
            if(ReadInt(nConvert)) face.nIndexVertex[1] = nConvert;
            else bFound = false;
            if(ReadInt(nConvert)) face.nIndexVertex[2] = nConvert;
            else bFound = false;
            if(ReadInt(nConvert)) face.nSmoothingGroup = nConvert;
            else bFound = false;
            ReadFloat(fConvert); //Skipping
            ReadFloat(fConvert); //Skipping
            ReadFloat(fConvert); //Skipping
            if(ReadInt(nConvert)) face.nMaterialID = nConvert;
            else bFound = false;

            vertindex.nValues[0] = face.nIndexVertex[0];
            vertindex.nValues[1] = face.nIndexVertex[1];
            vertindex.nValues[2] = face.nIndexVertex[2];

            if(bFound){
                node.Mesh.VertIndices.push_back(vertindex);
                node.Mesh.Faces.push_back(face);
            }
            else bError = true;
            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bFaces = false;
            }
            SkipLine();
        }
        else if(bTverts){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            if(nNode & NODE_HAS_SABER){
                SaberDataStruct saberdata = node.Saber.SaberData.back();
                if(ReadFloat(fConvert)) saberdata.fUV[0] = fConvert;
                else bFound = false;
                if(ReadFloat(fConvert)) saberdata.fUV[1] = fConvert;
                else bFound = false;
            }
            else{
                Vertex & vert = node.Mesh.Vertices.back();
                if(ReadFloat(fConvert)) vert.MDXData.fUV1[0] = fConvert;
                else bFound = false;
                if(ReadFloat(fConvert)) vert.MDXData.fUV1[1] = fConvert;
                else bFound = false;
            }

            if(!bFound) bError = true;
            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bTverts = false;
            }
            SkipLine();
        }
        else if(bLightmapTverts){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading tverts data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            Vertex & vert = node.Mesh.Vertices.back();
            if(ReadFloat(fConvert)) vert.MDXData.fUV2[0] = fConvert;
            else bFound = false;
            if(ReadFloat(fConvert)) vert.MDXData.fUV2[1] = fConvert;
            else bFound = false;

            if(!bFound) bError = true;
            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bLightmapTverts = false;
            }
            SkipLine();
        }
        else if(bConstraints){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading constraints data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            if(ReadFloat(fConvert)) node.Dangly.Constraints.push_back(fConvert);
            else bError = true;

            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bConstraints = false;
            }
            SkipLine();
        }
        else if(bWeights){
            //if(DEBUG_LEVEL > 3) std::cout<<"Reading weights data"<<""<<".\n";
            Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
            bFound = true;
            bool bPresent = false;
            int z = 0;
            int nBoneIndex = 0;
            int nNameIndex = 0;
            while(bFound && z < 4){
                //Get first name
                bFound = ReadUntilText(sID, false, true);
                //if we found a name, loop through the name array to find our name index
                for(nNameIndex = 0; nNameIndex < FH->MH.Names.size() && bFound; nNameIndex++){
                    //check if there is a match
                    if(FH->MH.Names[nNameIndex].cName == sID){
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
                        node.Mesh.Vertices[nDataCounter].MDXData.fSkin2[z] = (double) nBoneIndex;

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
                    if(ReadFloat(fConvert)) node.Mesh.Vertices[nDataCounter].MDXData.fSkin1[z] = fConvert;
                }
                z++;
            }
            if(z==1){
                //This means we exited before writing a single piece of data
                std::cout<<"Didn't even find one name"<<""<<".\n";
                std::cout<<"DataCounter: "<<nDataCounter<<". DataMax: "<<nDataMax<<".\n";
                bError = true;
            }
            nDataCounter++;
            if(nDataCounter >= nDataMax){
                bWeights = false;
            }
            SkipLine();
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
                        Error("Model name larger than 32 characters! Will truncate and continue.");
                        sID.resize(32);
                    }
                    else if(sID.size() > 16){
                        Warning("Model name larger than 16 characters! This may cause problems in the game.");
                    }
                    if(bFound) FH->MH.GH.cName = sID;

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
                        Error("Supermodel name larger than 32 characters! Will truncate and continue.");
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
                    else if(sID == "saber") nType = NODE_HAS_HEADER | NODE_HAS_MESH | NODE_HAS_SABER;
                    else if(bFound) std::cout<<"ReadUntilText() has found some text (type?) that we cannot interpret: "<<sID<<"\n";
                    if(bFound) node.Head.nType = nType;

                    //Read name
                    bFound = ReadUntilText(sID, false); //Name
                    if(!bFound){
                        std::cout<<"ReadUntilText() ERROR: a node is without a name.\n";
                    }
                    else{
                        /*
                        Name name;
                        name.cName = sID;
                        FH->MH.Names.push_back(name);
                        */
                        node.Head.nNameIndex = GetNameIndex(sID, FH->MH.Names);
                        node.Head.nID1 = node.Head.nNameIndex;
                        /// need to find the name index here
                    }

                    //Get animation number (automatically -1 if geo)
                    node.nAnimation = nAnimation;

                    //Initialize node, so we don't leave stuff undefined if it's not present in the ascii
                    if(nType & NODE_HAS_HEADER){
                        node.Head.vPos.Set(0.0, 0.0, 0.0);
                        node.Head.oOrient.Quaternion(0.0, 0.0, 0.0, 1.0);
                        node.Head.oOrient.ConvertToAA(); //Just in case we use this info
                        //node.Head.nID1 = nNodeCounter;
                        //node.Head.nNameIndex = nNodeCounter;
                    }
                    if(nType & NODE_HAS_LIGHT){
                        node.Light.fFlareRadius = 0.0;
                        node.Light.nLightPriority = 0;
                        node.Light.nAffectDynamic = 0;
                        node.Light.nAmbientOnly = 0;
                        node.Light.nDynamicType = 0;
                        node.Light.nFadingLight = 0;
                        node.Light.nFlare = 0;
                        node.Light.nShadow = 0;
                    }
                    if(nType & NODE_HAS_EMITTER){
                        node.Emitter.fDeadSpace = 0.0;
                        node.Emitter.fBlastLength = 0.0;
                        node.Emitter.fBlastRadius = 0.0;
                        node.Emitter.nxGrid = 0;
                        node.Emitter.nyGrid = 0;
                        node.Emitter.nSpawnType = 0;
                        node.Emitter.nTwosidedTex = 0;
                        node.Emitter.nLoop = 0;
                        node.Emitter.nRenderOrder = 0;
                        node.Emitter.cUpdate = "";
                        node.Emitter.cRender = "";
                        node.Emitter.cBlend = "";
                        node.Emitter.cTexture = "";
                        node.Emitter.cChunkName = "";
                        node.Emitter.nUnknown6 = 0;
                        node.Emitter.nFlags = 0;
                    }
                    if(nType & NODE_HAS_MESH){
                        node.Mesh.cTexture1 = "";
                        node.Mesh.cTexture2 = "";
                        node.Mesh.cTexture3 = "";
                        node.Mesh.cTexture4 = "";
                        node.Mesh.vAverage.fX = 0.0;
                        node.Mesh.vAverage.fY = 0.0;
                        node.Mesh.vAverage.fZ = 0.0;
                        node.Mesh.fAmbient.fR = 0.0;
                        node.Mesh.fAmbient.fG = 0.0;
                        node.Mesh.fAmbient.fB = 0.0;
                        node.Mesh.fDiffuse.fR = 0.0;
                        node.Mesh.fDiffuse.fG = 0.0;
                        node.Mesh.fDiffuse.fB = 0.0;
                        node.Mesh.vBBmin.fX = 0.0;
                        node.Mesh.vBBmin.fY = 0.0;
                        node.Mesh.vBBmin.fZ = 0.0;
                        node.Mesh.vBBmax.fX = 0.0;
                        node.Mesh.vBBmax.fY = 0.0;
                        node.Mesh.vBBmax.fZ = 0.0;
                        node.Mesh.fRadius = 0.0;
                        node.Mesh.nUnknown3[0] = -1;
                        node.Mesh.nUnknown3[1] = -1;
                        node.Mesh.nUnknown3[2] = 0;
                        node.Mesh.fUVDirectionX = 0.0;
                        node.Mesh.fUVDirectionY = 0.0;
                        node.Mesh.fUVJitter = 0.0;
                        node.Mesh.fUVJitterSpeed = 0.0;
                        node.Mesh.nAnimateUV = 0;
                        node.Mesh.nBackgroundGeometry = 0;
                        node.Mesh.nBeaming = 0;
                        node.Mesh.nShadow = 0;
                        node.Mesh.nRotateTexture = 0;
                        node.Mesh.nHasLightmap = 0;
                        node.Mesh.nShininess = 0;
                        node.Mesh.nTextureNumber = 1;
                        node.Mesh.nRender = 0;
                        node.Mesh.nUnknown30 = 0;
                        node.Mesh.nUnknown31 = 0;
                        node.Mesh.nUnknown32 = 1;
                        node.Mesh.nUnknown33 = 1;
                        node.Mesh.fUnknown7 = 0.0;
                        node.Mesh.fTotalArea = 0.0;
                        node.Mesh.nSaberUnknown1 = 3;
                        node.Mesh.nSaberUnknown2 = 0;
                        node.Mesh.nSaberUnknown3 = 0;
                        node.Mesh.nSaberUnknown4 = 0;
                        node.Mesh.nSaberUnknown5 = 0;
                        node.Mesh.nSaberUnknown6 = 0;
                        node.Mesh.nSaberUnknown7 = 0;
                        node.Mesh.nSaberUnknown8 = 0;
                        node.Mesh.Faces.resize(0);
                        node.Mesh.Vertices.resize(0);
                        node.Mesh.nMeshInvertedCounter = 0; //For now
                        node.Mesh.nMdxDataBitmap = MDX_FLAG_VERTEX | MDX_FLAG_HAS_NORMAL;
                    }
                    if(nType & NODE_HAS_SKIN){
                        node.Skin.Bones.resize(0);
                    }
                    if(nType & NODE_HAS_DANGLY){
                        node.Dangly.fDisplacement = 0.0;
                        node.Dangly.fTightness = 0.0;
                        node.Dangly.fPeriod = 0.0;
                        node.Dangly.Constraints.resize(0);
                    }
                    if(nType & NODE_HAS_AABB){
                        node.Walkmesh.ArrayOfAabb.resize(0);
                    }
                    if(nType & NODE_HAS_SABER){
                        node.Saber.nInvCount1 = 0; //For now
                        node.Saber.nInvCount2 = 0; //For now
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
                                if(FH->MH.Names[nNameIndex].cName == sID){
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
                                if(FH->MH.Names[nNameIndex].cName == sID){
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
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "shadow" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nShadow = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "affectdynamic" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nAffectDynamic = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "ndynamictype" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nDynamicType = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "ambientonly" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nAmbientOnly = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "fadinglight" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nFadingLight = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if((sID == "lensflares" || sID == "flare") && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Light.nFlare = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "flareradius" && nNode & NODE_HAS_LIGHT){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Light.fFlareRadius = fConvert;
                    else bError = true;
                    SkipLine();
                }
                /// For EMITTER
                else if(sID == "deadspace" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fDeadSpace = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "blastlength" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fBlastLength = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "blastradius" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Emitter.fBlastRadius = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "xgrid" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nxGrid = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "ygrid" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nyGrid = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "spawntype" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nSpawnType = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "twosidedtex" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nTwosidedTex = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "loop" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nLoop = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "renderorder" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Emitter.nRenderOrder = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "update" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Emitter.cUpdate = sID;
                    else node.Emitter.cUpdate = "";
                    SkipLine();
                }
                else if(sID == "render" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Emitter.cRender = sID;
                    else node.Emitter.cRender = "";
                    SkipLine();
                }
                else if(sID == "blend" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Emitter.cBlend = sID;
                    else node.Emitter.cBlend = "";
                    SkipLine();
                }
                else if(sID == "texture" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Emitter.cTexture = sID;
                    else node.Emitter.cTexture = "";
                    SkipLine();
                }
                else if(sID == "chunkname" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Emitter.cChunkName = sID;
                    else node.Emitter.cChunkName = "";
                    SkipLine();
                }
                else if(sID == "p2p" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "p2p_sel" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_P2P_SEL;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "affectedbywind" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_AFFECTED_WIND;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "m_istinted" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_TINTED;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "bounce" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_BOUNCE;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "random" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_RANDOM;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "inherit" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "inheritvel" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_VEL;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "inherit_local" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_LOCAL;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "splat" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_SPLAT;
                    }
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "inherit_part" && nNode & NODE_HAS_EMITTER){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)){
                        if(nConvert) node.Emitter.nFlags = node.Emitter.nFlags | EMITTER_FLAG_INHERIT_PART;
                    }
                    else bError = true;
                    SkipLine();
                }
                /// For MESH
                else if(sID == "bitmap" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound) node.Mesh.cTexture1 = sID;
                    else node.Mesh.cTexture1 = "";
                    SkipLine();
                }
                else if(sID == "lightmap" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    bFound = ReadUntilText(sID, false);
                    if(bFound){
                        node.Mesh.cTexture2 = sID;
                        node.Mesh.nHasLightmap = 1;
                        node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV2;
                    }
                    else node.Mesh.cTexture2 = "";
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
                else if(sID == "rotatetexture" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nRotateTexture = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "beaming" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nBeaming = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "render" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nRender = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "shadow" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nShadow = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "shininess" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nShininess = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "animateuv" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadInt(nConvert)) node.Mesh.nAnimateUV = nConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "uvdirectionx" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVDirectionX = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "uvdirectiony" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVDirectionY = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "uvjitter" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVJitter = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "uvjitterspeed" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Mesh.fUVJitterSpeed = fConvert;
                    else bError = true;
                    SkipLine();
                }
                /// For DANGLY
                else if(sID == "displacement" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fDisplacement = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "tightness" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fTightness = fConvert;
                    else bError = true;
                    SkipLine();
                }
                else if(sID == "period" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    if(ReadFloat(fConvert)) node.Dangly.fPeriod = fConvert;
                    else bError = true;
                    SkipLine();
                }
                /// Next we have the data lists
                else if(sID == "verts" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bVerts = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "faces" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bFaces = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "tverts" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bTverts = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV1;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "lightmaptverts" && nNode & NODE_HAS_MESH){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bLightmapTverts = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Mesh.nMdxDataBitmap = node.Mesh.nMdxDataBitmap | MDX_FLAG_HAS_UV2;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "weights" && nNode & NODE_HAS_SKIN){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bWeights = true;
                    Node & node = FH->MH.ArrayOfNodes.at(nCurrentIndex);
                    node.Skin.Bones.resize(FH->MH.Names.size());
                    node.Skin.nNumberOfBonemap = FH->MH.Names.size();
                    nWeightIndexes.resize(0);
                    for(int i = 0; i < 18; i++ ) node.Skin.nBoneIndexes[i] = 0;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                else if(sID == "aabb" && nNode & NODE_HAS_AABB){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bAabb = true;
                    int nSavePos = nPosition; //Save position
                    if(ReadFloat(fConvert)) nPosition = nSavePos; //The data starts in this line, revert position
                    else SkipLine(); //if not, then data starts next line, so it is okay to skip
                }
                else if(sID == "constraints" && nNode & NODE_HAS_DANGLY){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    bConstraints = true;
                    if(ReadInt(nConvert)) nDataMax = nConvert;
                    else bError = true;
                    nDataCounter = 0;
                    SkipLine();
                }
                /// Next we have keyed controllers
                else if(sLastThree == "key" && ReturnController(sID.substr(0, sID.length()-3))){
                    if(DEBUG_LEVEL > 3) std::cout<<"Reading "<<sID<<".\n";
                    Animation & anim = FH->MH.Animations.back();
                    Node & node = anim.ArrayOfNodes.back();
                    Controller ctrl;
                    ctrl.nAnimation = nAnimation;
                    ctrl.nNameIndex = node.Head.nNameIndex;
                    ctrl.nControllerType = ReturnController(sID.substr(0, sID.length()-3));
                    ctrl.nTimekeyStart = node.Head.ControllerData.size();
                    ctrl.nUnknown2 = -1;
                    ctrl.nPadding[0] = 0;
                    ctrl.nPadding[1] = 0;
                    ctrl.nPadding[2] = 0;
                    //This is all we can tell right now.
                    int nSavePos = nPosition; //Save position

                    //To continue let's first get the column count
                    SkipLine();
                    nDataCounter = 0;
                    bFound = true;
                    while(bFound){
                        if(ReadFloat(fConvert)){
                            nDataCounter++;
                        }
                        else bFound = false;
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
                        nSavePos2 = nPosition;
                        ReadUntilText(sID);
                        if(sID=="endlist") bFound = false;
                        else{
                            nPosition = nSavePos2;
                            if(!EmptyRow()){
                                if(ReadFloat(fConvert)){
                                    node.Head.ControllerData.push_back(fConvert);
                                }
                                else bError = true;
                                nDataCounter++;
                            }
                            SkipLine();
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
                    nDataCounter = 0;
                    if(!bError){
                        node.Head.Controllers.push_back(ctrl);
                    }
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
                    ctrl.nPadding[0] = 0;
                    ctrl.nPadding[1] = 0;
                    ctrl.nPadding[2] = 0;

                    //First put in the 0.0 to fill the required timekey
                    node.Head.ControllerData.push_back(0.0);

                    //Now fill values
                    if(ctrl.nControllerType == CONTROLLER_HEADER_ORIENTATION){
                        if(ReadFloat(fConvert)) node.Head.oOrient.fX = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) node.Head.oOrient.fY = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) node.Head.oOrient.fZ = fConvert;
                        else bError = true;
                        if(ReadFloat(fConvert)) node.Head.oOrient.fAngle = fConvert;
                        else bError = true;
                        node.Head.oOrient.ConvertToQuaternions();
                        node.Head.ControllerData.push_back(node.Head.oOrient.qX);
                        node.Head.ControllerData.push_back(node.Head.oOrient.qY);
                        node.Head.ControllerData.push_back(node.Head.oOrient.qZ);
                        node.Head.ControllerData.push_back(node.Head.oOrient.qW);
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
                        Error("Animation name larger than 32 characters! Will truncate and continue.");
                        sID.resize(32);
                    }
                    else if(sID.size() > 16){
                        Warning("Animation name larger than 16 characters! This may cause problems in the game.");
                    }
                    Animation anim;
                    if(bFound) anim.cName = sID;

                    //Initialize animation in case something is left undefined
                    anim.cName2 = FH->MH.Names.front().cName;
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
                        Error("AnimRoot name larger than 32 characters! Will truncate and continue.");
                        sID.resize(32);
                    }
                    else if(sID.size() > 16){
                        Warning("AnimRoot name larger than 16 characters! This may cause problems in the game.");
                    }
                    Animation & anim = FH->MH.Animations.back();
                    if(bFound) anim.cName2 = sID;
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
    //All the data has now been read. Now what remains if to fill in all the data that derives from this data and rearrange some data.

    //Seems best to take care of aabb first
    for(int n = 0; n < FH->MH.ArrayOfNodes.size(); n++){
        Node & node = FH->MH.ArrayOfNodes[n];
        if(node.Head.nType & NODE_HAS_AABB){
            node.Walkmesh.RootAabb = node.Walkmesh.ArrayOfAabb.front();
            int nCounter = 1;
            BuildAabb(node.Walkmesh.RootAabb, node.Walkmesh.ArrayOfAabb, nCounter);
        }
    }

    /// TODO: face normals, vertex normals, bones

    std::cout<<"Done processing data, checking for errors...\n";
    if(bError){
        Error("Some kind of error has occured! Check the console! The program will now cleanup what it has read since the data is now broken.");
        return false;
    }
    return true;
}

int Ascii::GetNameIndex(std::string sName, std::vector<Name> Names){
    int n = 0;
    while(n < Names.size()){
        if(Names[n].cName == sName) return n;
        n++;
    }
    return -1;
}

void Ascii::BuildAabb(Aabb & AABB, std::vector<Aabb> & ArrayOfAabb, int & nCounter){
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

bool Ascii::ReadFloat(double & fNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' &&
       sBuffer[nPosition] == 0x0D &&
       sBuffer[nPosition] == 0x0A)
    {
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' &&
           sBuffer[nPosition] == 0x0D &&
           sBuffer[nPosition] == 0x0A)
        {
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Ascii::ReadFloat(): Reading: "<<sCheck<<". ";

    try{
        fNew = std::stof(sCheck, (size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout<<"Ascii::ReadFloat(): There was an error converting the string: "<<sCheck<<". Printing 0.0. \n";
        fNew = 0.0;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<fNew<<".\n";
    return true;
}

bool Ascii::ReadInt(int & nNew, bool bPrint){
    std::string sCheck;
    //First skip all spaces
    if(sBuffer[nPosition] == '#' &&
       sBuffer[nPosition] == 0x0D &&
       sBuffer[nPosition] == 0x0A)
    {
        return false;
    }
    while(sBuffer[nPosition] == 0x20){
        nPosition++;
        if(sBuffer[nPosition] == '#' &&
           sBuffer[nPosition] == 0x0D &&
           sBuffer[nPosition] == 0x0A)
        {
            return false;
        }
    }
    while(sBuffer[nPosition] != 0x20 &&
            sBuffer[nPosition] != '#' &&
            sBuffer[nPosition] != 0x0D &&
            sBuffer[nPosition] != 0x0A)
    {
        sCheck.push_back(sBuffer[nPosition]);
        nPosition++;
    }
    if(sCheck.length() == 0) return false;

    //Report
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Ascii::ReadInt(): Reading: "<<sCheck<<". ";

    try{
        nNew = stoi(sCheck,(size_t*) NULL);
    }
    catch(std::invalid_argument){
        std::cout<<"Ascii::ReadInt(): There was an error converting the string: "<<sCheck<<". Printing 0xFFFFFFFF. \n";
        nNew = 0xFFFFFFFF;
        return false;
    }
    if(bPrint || DEBUG_LEVEL > 5) std::cout<<"Converted: "<<nNew<<".\n";
    return true;
}

void Ascii::SkipLine(){
    if(sBuffer[nPosition] == 0x0A){
        nPosition+=1;
        return;
    }
    else if(sBuffer[nPosition] == 0x0D && sBuffer[nPosition+1] == 0x0A){
        nPosition+=2;
        return;
    }
    bool bStop = false;
    while(nPosition + 1 < nBufferSize && !bStop){
        if(sBuffer[nPosition] != 0x0D && sBuffer[nPosition+1] != 0x0A) nPosition++;
        else bStop = true;
    }
    nPosition+=2;
}

bool Ascii::EmptyRow(){
    int n = nPosition; //Do not use the iterator
    while(sBuffer[n] != 0x0D &&
        sBuffer[n+1] != 0x0A &&
                n+1 < nBufferSize)
    {
        if(sBuffer[n] != 0x20 || (sBuffer[n+1] != 0x20 && sBuffer[n+1] != 0x0D)) return false;
        n++;
    }
    return true;
}

bool Ascii::ReadUntilText(std::string & sHandle, bool bToLowercase, bool bStrictNoNewLine){
    sHandle = ""; //Make sure the handle is cleared
    while(nPosition < nBufferSize){
        //std::cout<<"Looping in ReadUntilText main while(), nPosition="<<nPosition<<".\n";
        if(sBuffer[nPosition] == 0x20){
            //Skip space
            nPosition++;
            if(nPosition >= nBufferSize) return false;
        }
        else if(sBuffer[nPosition] == 0x0A){
            if(bStrictNoNewLine) return false;
            nPosition++;
            if(nPosition >= nBufferSize) return false;
        }
        else if(sBuffer[nPosition] == '#'){
            //Return because end of line and nothing was found
            return false;
        }
        else{
            if(nPosition + 1 < nBufferSize){
                if(sBuffer[nPosition] == 0x0D &&
                 sBuffer[nPosition+1] == 0x0A)
                {
                    //Return because end of line and nothing was found
                    return false;
                }
            }
            //Now it gets interesting - we may actually have relevant text now
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
            do{
                //std::cout<<"Reading and saving non-null character. "<<sBuffer[nPosition]<<".\n";
                sHandle.push_back(sBuffer[nPosition]);
                nPosition++;
            }
            while(sBuffer[nPosition] != 0x20 &&
                  sBuffer[nPosition] != '#' &&
                  sBuffer[nPosition] != 0x0D &&
                  sBuffer[nPosition] != 0x0A &&
                  nPosition < nBufferSize);

            //Report
            //if(sHandle != "") std::cout<<"ReadUntilText() found the following string: "<<sHandle<<".\n";

            //convert to lowercase
            if(bToLowercase) std::transform(sHandle.begin(), sHandle.end(), sHandle.begin(), ::tolower);

            //Go back and tell them you've found something
            return true;
        }
    }
    //Go back and tell them you're done
    return false;
}

Node & Ascii::GetNodeByNameIndex(FileHeader & Data, int nIndex, int nAnimation){
    if(nAnimation == -1){/*
        aNode & Array = Data.MH.ArrayOfNodes;
        for(int n = 0; n < Array.size(); n++){
            if(Array[n].Head.nNameIndex == nIndex) return Array[n];
        }*/
    }
    else{
        std::vector<Node> & Array = Data.MH.Animations[nAnimation].ArrayOfNodes;
        for(int n = 0; n < Array.size(); n++){
            if(Array[n].Head.nNameIndex == nIndex) return Array[n];
        }
    }
}
