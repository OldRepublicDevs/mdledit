#include "MDL.h"

bool bK2 = true;

Vector operator*(Vector v, const Matrix22 & m){
    v *= m;
    return v;
}

Vector operator*(Vector v, const double & f){
    v *= f;
    return v;
}

Vector operator/(Vector v, const double & f){
    v /= f;
    return v;
}

double operator*(const Vector & v, const Vector & v2){ //dot product
    return (v.fX * v2.fX + v.fY * v2.fY + v.fZ * v2.fZ);
}

double Angle(const Vector & v, const Vector & v2){
    return acos((v*v2)/(v.GetLength() * v2.GetLength()));
}

Vector operator/(Vector v, const Vector & v2){ //cross product
    v /= v2;
    return v;
}

Vector operator-(Vector v, const Vector & v2){
    v -= v2;
    return v;
}

Vector operator+(Vector v, const Vector & v2){
    v += v2;
    return v;
}

Orientation operator*(Orientation o1, const Orientation & o2){
    o1 *= o2;
    return o1;
}

bool VerticesEqual(const Vertex & v1, const Vertex & v2){
    if(fabs((v1 - v2).GetLength()) < 0.001) return true;
    else return false;
}

bool MDL::CheckNodes(std::vector<Node> & NodeArray, std::stringstream & ssReturn, int nAnimation){
    bool bMasterUpdate = false;
for(int b = 0; b < NodeArray.size(); b++){
    bool bUpdate = false;
    std::stringstream ssAdd;
    std::string sCont;
    if(nAnimation == -1) sCont = "geometry";
    else sCont = FH[0].MH.Animations.at(nAnimation).cName.c_str();
    ssAdd<<"\r\n - "<<FH[0].MH.Names[NodeArray[b].Head.nNameIndex].cName<<" ("<<NodeArray[b].Head.nType<<", "<<sCont<<")";
    if(NodeArray[b].Head.nType & NODE_HAS_HEADER){
        if(NodeArray[b].Head.nUnknownEmpty1 != 0){
            ssAdd<<"\r\n     - Header: Unknown short after NameIndex has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Head.ChildrenArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - ChildArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Head.ControllerArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - ControllerArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Head.ControllerDataArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - ControllerDataArray counts differ!";
            bUpdate = true;
        }
        for(int c = 0; c < NodeArray[b].Head.Controllers.size(); c++){
            Controller & ctrl = NodeArray.at(b).Head.Controllers.at(c);

            /***
                This if for checking controller "padding" values. These numbers are in no way random.
                The final number can be 0, 1 or 2, as observed up until now. Header and light controllers always have 0,
                while emitter and mesh controllers may have 1 or 2.
                In keyed controllers, light and emitter seem to group together against header and mesh.
                Selfillumcolor usually has the same padding values as scale, but they are different
                in for example: n_admrlsaulkar or 003ebof
            /**/
            if(ctrl.nControllerType==CONTROLLER_HEADER_POSITION && ctrl.nValueCount == 1 &&
                (ctrl.nPadding[0] == 12 &&
                 ctrl.nPadding[1] == 76 &&
                 ctrl.nPadding[2] == 0   )){}
            else if(ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 12 &&
                      ctrl.nPadding[1] == 76 &&
                      ctrl.nPadding[2] == 0   )){}
            else if(ctrl.nControllerType==CONTROLLER_HEADER_SCALING && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 12 &&
                      ctrl.nPadding[1] == 76 &&
                      ctrl.nPadding[2] == 0   )){}
            else if(ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -59 &&
                      ctrl.nPadding[1] == 73 &&
                      ctrl.nPadding[2] == 0   )){}
            else if(ctrl.nControllerType==CONTROLLER_HEADER_SCALING && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 49 &&
                      ctrl.nPadding[1] == 18 &&
                      ctrl.nPadding[2] == 0   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -111 &&
                      ctrl.nPadding[1] == 6 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 54 &&
                      ctrl.nPadding[1] == -66 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 57 &&
                      ctrl.nPadding[1] == -62 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 17 &&
                      ctrl.nPadding[1] == -83 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -35 &&
                      ctrl.nPadding[1] == -25 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -89 &&
                      ctrl.nPadding[1] == -63 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -51 &&
                      ctrl.nPadding[1] == -57 &&
                      ctrl.nPadding[2] == 9   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -54 &&
                      ctrl.nPadding[1] == -66 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -79 &&
                      ctrl.nPadding[1] == 20 &&
                      ctrl.nPadding[2] == 3   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 85 &&
                      ctrl.nPadding[1] == -43 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 64 &&
                      ctrl.nPadding[1] == -98 &&
                      ctrl.nPadding[2] == 3   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -49 &&
                      ctrl.nPadding[1] == -114 &&
                      ctrl.nPadding[2] == 3   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 41 &&
                      ctrl.nPadding[1] == -7 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -10 &&
                      ctrl.nPadding[1] == -64 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -50 &&
                      ctrl.nPadding[1] == -64 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -62 &&
                      ctrl.nPadding[1] == -125 &&
                      ctrl.nPadding[2] == 2   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 80 &&
                      ctrl.nPadding[1] == -112 &&
                      ctrl.nPadding[2] == 3   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -114 &&
                      ctrl.nPadding[1] == -121 &&
                      ctrl.nPadding[2] == 4   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 74 &&
                      ctrl.nPadding[1] == 47 &&
                      ctrl.nPadding[2] == 3   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -82 &&
                      ctrl.nPadding[1] == -68 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -112 &&
                      ctrl.nPadding[1] == -50 &&
                      ctrl.nPadding[2] == 1   )){}
            else if( (ctrl.nControllerType==CONTROLLER_LIGHT_COLOR || ctrl.nControllerType==CONTROLLER_LIGHT_MULTIPLIER || ctrl.nControllerType==CONTROLLER_LIGHT_RADIUS ||
                      ctrl.nControllerType==CONTROLLER_LIGHT_SHADOWRADIUS || ctrl.nControllerType==CONTROLLER_LIGHT_VERTICALDISPLACEMENT) && ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -5 &&
                      ctrl.nPadding[1] == 54 &&
                      ctrl.nPadding[2] == 0   )){}
            else if( (ctrl.nControllerType==CONTROLLER_HEADER_POSITION || ctrl.nControllerType==CONTROLLER_HEADER_ORIENTATION || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount >= 1 &&
                     (ctrl.nPadding[0] == 50 &&
                      ctrl.nPadding[1] == 18 &&
                      ctrl.nPadding[2] == 0   )){}
            else if(  ctrl.nValueCount >= 1 &&
                     (ctrl.nPadding[0] == 51 &&
                      ctrl.nPadding[1] == 18 &&
                      ctrl.nPadding[2] == 0   )){}
            // the following are all emitter controllers, but it's hard to mark this, leaving it open for now
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 7 &&
                      ctrl.nPadding[1] == -63 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 12 &&
                      ctrl.nPadding[1] == -63 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -73 &&
                      ctrl.nPadding[1] == -68 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -84 &&
                      ctrl.nPadding[1] == -70 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 95 &&
                      ctrl.nPadding[1] == -9 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 103 &&
                      ctrl.nPadding[1] == -9 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 13 &&
                      ctrl.nPadding[1] == 81 &&
                      ctrl.nPadding[2] == 2   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 125 &&
                      ctrl.nPadding[1] == 96 &&
                      ctrl.nPadding[2] == 2   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == 14 &&
                      ctrl.nPadding[1] == -33 &&
                      ctrl.nPadding[2] == 1   )){}
            else if(  ctrl.nValueCount == 1 &&
                     (ctrl.nPadding[0] == -118 &&
                      ctrl.nPadding[1] == -71 &&
                      ctrl.nPadding[2] == 1   )){}
            /// This is the fuck it check, there's too many, so just in case it's a mesh controller, let it pass
            else if( (ctrl.nControllerType==CONTROLLER_MESH_ALPHA || ctrl.nControllerType==CONTROLLER_MESH_SELFILLUMCOLOR) && ctrl.nValueCount == 1){}
            else{
                ssAdd<<"\r\n     - Previously unseen controller padding! ("<<(int)ctrl.nPadding[0]<<", "<<(int)ctrl.nPadding[1]<<", "<<(int)ctrl.nPadding[2]<<")";
                bUpdate = true;
            }
        }
    }
    if(NodeArray[b].Head.nType & NODE_HAS_LIGHT){
        if(NodeArray[b].Light.UnknownArray.nCount != 0){
            ssAdd<<"\r\n     - Light: Unknown array not empty!";
            bUpdate = true;
        }
        if(NodeArray[b].Light.FlareSizeArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - FlareSizeArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Light.FlarePositionArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - FlarePositionArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Light.FlareColorShiftArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - FlareColorShiftArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Light.FlareTextureNameArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - FlareTextureNameArray counts differ!";
            bUpdate = true;
        }
    }
    if(NodeArray[b].Head.nType & NODE_HAS_EMITTER){
        if(NodeArray[b].Emitter.nUnknown6 != 0){
            ssAdd<<"\r\n     - Emitter: Unknown short after RenderOrder has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Emitter.nZero1 != 0){
            ssAdd<<"\r\n     - Emitter: Zero1 has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Emitter.nZero2 != 0){
            ssAdd<<"\r\n     - Emitter: Zero2 has a value!";
            bUpdate = true;
        }
    }
    if(NodeArray[b].Head.nType & NODE_HAS_MESH){
        if(NodeArray[b].Mesh.nUnknown3[0] != -1 || NodeArray[b].Mesh.nUnknown3[1] != -1 || NodeArray[b].Mesh.nUnknown3[2] != 0){
            ssAdd<<"\r\n     - Mesh: The unknown -1 -1 0 array has a different value!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.nUnknown30 != 0){
            ssAdd<<"\r\n     - Mesh: Unknown30 byte after Render byte has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.nK2Unknown2 != 0){
            ssAdd<<"\r\n     - Mesh: K2Unknown2 has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.FaceArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - FaceArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.IndexCounterArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - IndexCounterArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.IndexLocationArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - IndexLocationArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Mesh.MeshInvertedCounterArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - MeshInvertedCounterArray counts differ!";
            bUpdate = true;
        }
    }
    if(NodeArray[b].Head.nType & NODE_HAS_SKIN){
        if(NodeArray[b].Skin.nUnknown1 != 0){
            ssAdd<<"\r\n     - Skin: Unknown int 1 has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Skin.nUnknown2 != 0){
            ssAdd<<"\r\n     - Skin: Unknown int 2 has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Skin.nUnknown3 != 0){
            ssAdd<<"\r\n     - Skin: Unknown int 3 has a value!";
            bUpdate = true;
        }
        if(NodeArray[b].Skin.QBoneArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - QBoneArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Skin.TBoneArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - TBoneArray counts differ!";
            bUpdate = true;
        }
        if(NodeArray[b].Skin.Array8Array.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - Array8Array counts differ!";
            bUpdate = true;
        }
    }
    if(NodeArray[b].Head.nType & NODE_HAS_DANGLY){
        if(NodeArray[b].Dangly.ConstraintArray.GetDoCountsDiffer()){
            ssAdd<<"\r\n     - ConstraintArray counts differ!";
            bUpdate = true;
        }
    }
    if(bUpdate){
        bMasterUpdate = true;
        ssReturn<<ssAdd.str();
    }
}
    return bMasterUpdate;
}

void MDL::CheckPeculiarities(){
    FileHeader * Data = &FH[0];
    std::stringstream ssReturn;
    bool bUpdate = false;
    ssReturn<<"Lucky you! Your model has some rare peculiarities:";
    for(int n = 0; n < 6; n++){
        if(Data->MH.GH.nUnknownEmpty[n] != 0){
            ssReturn<<"\r\n - Int "<<n<<" in the array of 6 zero int32s in the GH has a value!";
            bUpdate = true;
        }
    }
    if(Data->MH.GH.nRefCount != 0){
        ssReturn<<"\r\n - RefCount has a value!";
        bUpdate = true;
    }
    if(Data->MH.GH.nModelType != 2){
        ssReturn<<"\r\n - Header ModelType different than 2!";
        bUpdate = true;
    }
    if(Data->MH.nChildModelCount != 0){
        ssReturn<<"\r\n - ChildModelCount has a value!";
        bUpdate = true;
    }
    if(Data->MH.AnimationArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - AnimationArray counts differ!";
        bUpdate = true;
    }
    if(Data->MH.nUnknownEmpty3 != 0){
        ssReturn<<"\r\n - Unknown int32 after the Root Node Offset 2 in the MH has a value!";
        bUpdate = true;
    }
    if(Data->MH.nMdxLength2 != Data->nMdxLength){
        ssReturn<<"\r\n - MdxLength in FH and MdxLength2 in MH don't have the same value!";
        bUpdate = true;
    }
    if(Data->MH.nOffsetIntoMdx != 0){
        ssReturn<<"\r\n - OffsetIntoMdx after the MdxLength2 in the MH has a value!";
        bUpdate = true;
    }
    if(Data->MH.NameArray.GetDoCountsDiffer()){
        ssReturn<<"\r\n - NameArray counts differ!";
        bUpdate = true;
    }
    for(int a = 0; a < Data->MH.AnimationArray.nCount; a++){
        for(int n = 0; n < 6; n++){
            if(Data->MH.Animations[a].nUnknownEmpty3[n] != 0){
                ssReturn<<"\r\n - Int "<<n<<" in the array of 6 zero int32s in the Animation GH has a value!";
                bUpdate = true;
            }
        }
        if(Data->MH.Animations[a].nUnknownEmpty4 != 0){
            ssReturn<<"\r\n - Animation counterpart to RefCount has a value!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].SoundArray.GetDoCountsDiffer()){
            ssReturn<<"\r\n - SoundArray counts differ!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].nUnknownEmpty6 != 0){
            ssReturn<<"\r\n - Unknown int32 after SoundArrayHead has a value!";
            bUpdate = true;
        }
        if(Data->MH.Animations[a].nModelType != 5){
            ssReturn<<"\r\n - Animation ModelType is not 5!";
            bUpdate = true;
        }
        if(CheckNodes(Data->MH.Animations[a].ArrayOfNodes, ssReturn, a)) bUpdate = true;
    }
    if(CheckNodes(Data->MH.ArrayOfNodes, ssReturn, -1)) bUpdate = true;
    if(!bUpdate){
        std::cout<<"Checked for peculiarities, nothing to report.\n";
        return;
    }
    MessageBox(NULL, ssReturn.str().c_str(), "Notification", MB_OK);
}
