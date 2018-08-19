#include "MDL.h"
#include "frame.h"
#include <algorithm>

/**
    Functions:
    Append()             // Helper
    AppendAabb()         // Helper
    GetNodeTreePrefix()  // Helper
    BuildGeometryNode()  // Helper
    BuildAnimationTree() // frame.h
    BuildGeometryTree()  // frame.h
    BuildTree()          // frame.h (MDL)
    BuildTree()          // frame.h (BWM)
/**/

std::vector<DataRegion> AllDataRegions;

HTREEITEM Append(BinaryFile & binf, const std::string & sString, LPARAM lParam = NULL, HTREEITEM hParentNew = NULL, HTREEITEM hAfterNew = NULL, UINT Flags = NULL){
    static HTREEITEM hPrev;
    if(sString.empty()) return hPrev;
    static HTREEITEM hParent;
    HTREEITEM hAfter;

    //Determine hParent
    if(hParentNew == NULL) {}
    else hParent = hParentNew;

    //Determine hAfter
    if(hAfterNew == NULL) hAfter = hPrev;
    else hAfter = hAfterNew;

    //Add item
    TVINSERTSTRUCT tvis;
    TVITEMEX * item = &tvis.itemex;
    item->mask = TVIF_TEXT | TVIF_PARAM;
    item->pszText = (char*) sString.c_str();
    item->cchTextMax = sString.size();
    item->lParam = lParam;
    if(Flags != NULL){
        item->mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
        item->state = Flags;
    }
    tvis.hParent = hParent;
    tvis.hInsertAfter = hAfter;
    hPrev = TreeView_InsertItem(hTree, &tvis);

    if(lParam){
        std::vector<DataRegion> * p_data = GetDataRegions({sString}, lParam);
        if(p_data != nullptr){
            for(DataRegion & reg : *p_data){
                reg.hItem = hPrev;
            }
            AllDataRegions.insert(AllDataRegions.end(), p_data->begin(), p_data->end());
        }
        /*
        /// Use this item to update the position structures so we may access the tree item by double clicking its data in the edit control.
        for(BinaryPosition & pos : binf.GetPositions()){
            if(pos.p_data == (void*) lParam) pos.hItem = hPrev;
        }
        */
    }

    return hPrev;
}

void AppendAabb(MDL & Mdl, Aabb * AABB, HTREEITEM TopLevel, int & nCount){
    HTREEITEM htiAabb = Append(Mdl, "aabb "+std::to_string(nCount), (LPARAM) AABB, TopLevel);
    nCount++;
    if(AABB->Child1.size() > 0) AppendAabb(Mdl, &(AABB->Child1.front()), htiAabb, nCount);
    if(AABB->Child2.size() > 0) AppendAabb(Mdl, &(AABB->Child2.front()), htiAabb, nCount);
}

std::string GetNodeTreePrefix(unsigned short nType){
    if(     nType == (NODE_HEADER | NODE_MESH | NODE_SABER)) return "(saber) ";
    else if(nType == (NODE_HEADER | NODE_MESH | NODE_AABB)) return "(aabb) ";
    else if(nType == (NODE_HEADER | NODE_MESH | NODE_DANGLY)) return "(dangly) ";
    else if(nType == (NODE_HEADER | NODE_MESH | NODE_SKIN)) return "(skin) ";
    else if(nType == (NODE_HEADER | NODE_MESH)) return "(mesh) ";
    else if(nType == (NODE_HEADER | NODE_REFERENCE)) return "(reference) ";
    else if(nType == (NODE_HEADER | NODE_EMITTER)) return "(emitter) ";
    else if(nType == (NODE_HEADER | NODE_LIGHT)) return "(light) ";
    else if(nType == NODE_HEADER) return "(basic) ";
    else return "(unknown) ";
}

void BuildAnimationNode(Node & node, HTREEITEM Prev, std::vector<unsigned> & offsets, MDL & Mdl, int nAnim){
    if(!Mdl.GetFileData()) return;
    FileHeader & Data = *Mdl.GetFileData();
    std::vector<Name> & Names = Data.MH.Names;
    Animation & anim = Data.MH.Animations.at(nAnim);

    /// If we're doing the hierarchy, we're just gonna add the children to this node and go recursive on this function and then return
    if(bModelHierarchy){
        offsets.push_back(node.nOffset);
        for(Node & curnode : anim.ArrayOfNodes){
            if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                MdlInteger<unsigned short> nNodeIndex = Mdl.GetNodeIndexByNameIndex(curnode.Head.nNameIndex);
                HTREEITEM Child = Append(Mdl, GetNodeTreePrefix(!nNodeIndex.Valid() ? static_cast<unsigned short>(0) : static_cast<unsigned short>(Data.MH.ArrayOfNodes.at(nNodeIndex).Head.nType)) + Mdl.GetNodeName(curnode), (LPARAM) &curnode, Prev);

                /// In case we are looping, stop the loop
                if(std::find(offsets.begin(), offsets.end(), curnode.nOffset) == offsets.end()){
                    //std::cout << Data.MH.Animations.at(nAnim).sName.c_str() << " > " << Mdl.GetNodeName(curnode) << " is OK!" << std::endl;
                    BuildAnimationNode(curnode, Child, offsets, Mdl, nAnim);
                }
                //else std::cout << Data.MH.Animations.at(nAnim).sName.c_str() << " > " << Mdl.GetNodeName(curnode) << " is not OK!" << std::endl;
            }
        }
        return;
    }

    if(node.Head.nNameIndex != 0){
        MdlInteger<unsigned short> nNodeIndex = Mdl.GetNodeIndexByNameIndex(node.Head.nNameIndex);
        HTREEITEM Controllers = Append(Mdl, "Controllers", (LPARAM) &node, Prev);
        for(Controller & ctrl : node.Head.Controllers){
            if(!nNodeIndex.Valid()) throw mdlexception("tree building error: we have controller data on an anim node that does not have a geo counterpart.");
            Append(Mdl, ReturnControllerName(ctrl.nControllerType, Data.MH.ArrayOfNodes.at(nNodeIndex).Head.nType) + (ctrl.nColumnCount > 15 ? "bezierkey" : "key"), (LPARAM) &ctrl, Controllers);
        }

        if(node.Head.nParentIndex.Valid()){
            MdlInteger<unsigned short> nNodeIndex2 = Mdl.GetNodeIndexByNameIndex(node.Head.nParentIndex, nAnim);
            if(!nNodeIndex2.Valid()) throw mdlexception("tree building error: dealing with a name index that does not have a node in animation.");
            Node & parent = Data.MH.Animations.at(nAnim).ArrayOfNodes.at(nNodeIndex2);
            Append(Mdl, "Parent: " + Mdl.GetNodeName(parent), (LPARAM) &parent, Prev);
        }
    }

    HTREEITEM Children = Append(Mdl, "Children", (LPARAM) &node, Prev);
    for(Node & child : anim.ArrayOfNodes){
        if(child.Head.nParentIndex == node.Head.nNameIndex){
            MdlInteger<unsigned short> nNodeIndex = Mdl.GetNodeIndexByNameIndex(child.Head.nNameIndex);
            HTREEITEM Child = Append(Mdl, GetNodeTreePrefix(!nNodeIndex.Valid() ? 0 : static_cast<unsigned short>(Data.MH.ArrayOfNodes.at(nNodeIndex).Head.nType)) + Mdl.GetNodeName(child), (LPARAM) &child, Children);
        }
    }
}

void BuildAnimationTree(HTREEITEM Animations, MDL & Mdl){
    if(!Mdl.GetFileData()) return;
    FileHeader & Data = *Mdl.GetFileData();

    /// Delete any leftover children, which means we can use this function to reconstruct this part of the tree
    TreeView_DeleteAllChildren(hTree, Animations);
  try{
    for(int an = 0; an < Data.MH.Animations.size(); an++){
        Animation & anim = Data.MH.Animations.at(an);
        HTREEITEM Anim = Append(Mdl, anim.sName, (LPARAM) &anim, Animations);
        std::vector<unsigned> offsets;
        offsets.reserve(anim.ArrayOfNodes.size());
        for(Node & node : anim.ArrayOfNodes){
            MdlInteger<unsigned short> nNodeIndex = Mdl.GetNodeIndexByNameIndex(node.Head.nNameIndex);
            HTREEITEM CurrentNode = Append(Mdl, GetNodeTreePrefix(!nNodeIndex.Valid() ? 0 : static_cast<unsigned short>(Data.MH.ArrayOfNodes.at(nNodeIndex).Head.nType)) + Mdl.GetNodeName(node), (LPARAM) &node, Anim);
            BuildAnimationNode(node, CurrentNode, offsets, Mdl, an);

            /// If we're doing the hierarchy, we only want to write out the first node, so stop after that
            if(bModelHierarchy) break;
        }

        /// Increment progress bar here
        ProgressStepIt();
    }
  }
  catch(std::exception & e){
    Error("An exception occurred in BuildAnimationTree(): " + std::string(e.what()));
  }
  catch(...){
    Error("An exception occurred in BuildAnimationTree()!");
  }
}

void BuildGeometryNode(Node & node, HTREEITEM Prev, std::vector<unsigned> & offsets, MDL & Mdl){
    if(!Mdl.GetFileData()) return;
    FileHeader & Data = *Mdl.GetFileData();
    std::vector<Name> & Names = Data.MH.Names;

    /// Increment progress bar here
    ProgressStepIt();

    /// If we're doing the hierarchy, we're just gonna add the children to this node and go recursive on this function and then return
    if(bModelHierarchy){
        offsets.push_back(node.nOffset);
        for(Node & curnode : Data.MH.ArrayOfNodes){
            if(curnode.Head.nParentIndex == node.Head.nNameIndex){
                HTREEITEM Child = Append(Mdl, GetNodeTreePrefix(curnode.Head.nType) + Mdl.GetNodeName(curnode), (LPARAM) &curnode, Prev);

                /// In case we are looping, stop the loop
                if(std::find(offsets.begin(), offsets.end(), curnode.nOffset) == offsets.end())
                    BuildGeometryNode(curnode, Child, offsets, Mdl);
            }
        }
        return;
    }

    if(node.Head.nType & NODE_LIGHT){
        HTREEITEM Light = Append(Mdl, "Light", (LPARAM) &node, Prev);
        //HTREEITEM LensFlares = Append(Mdl, "Lens Flares", (LPARAM) &node, Light);
        int nMaxSize = std::max(node.Light.FlareSizes.size(),
                       std::max(node.Light.FlarePositions.size(),
                       std::max(node.Light.FlareColorShifts.size(),
                                node.Light.FlareTextureNames.size())));
        for(int n = 0; n < nMaxSize; n++)
            Append(Mdl, "Lens Flare " + std::to_string(n), (LPARAM) &node, Light);
    }
    if(node.Head.nType & NODE_EMITTER){
        HTREEITEM Emitter = Append(Mdl, "Emitter", (LPARAM) &node, Prev);
    }
    if(node.Head.nType & NODE_REFERENCE){
        HTREEITEM Reference = Append(Mdl, "Reference", (LPARAM) &node, Prev);
    }
    if(node.Head.nType & NODE_MESH){
        HTREEITEM Mesh = Append(Mdl, "Mesh", (LPARAM) &node, Prev);
        HTREEITEM Vertices = Append(Mdl, "Vertices", (LPARAM) &node, Mesh);
        if(node.Mesh.Vertices.size() > 0){
            for(int n = 0; n < node.Mesh.Vertices.size(); n++)
                Append(Mdl, "Vertex " + std::to_string(n), (LPARAM) &(node.Mesh.Vertices[n]), Vertices);
        }
        HTREEITEM Faces = Append(Mdl, "Faces", (LPARAM) &node, Mesh);
        if(node.Mesh.Faces.size() > 0){
            for(int n = 0; n < node.Mesh.Faces.size(); n++){
                Append(Mdl, "Face " + std::to_string(n), (LPARAM) &(node.Mesh.Faces[n]), Faces);
            }
        }
    }
    if(node.Head.nType & NODE_SKIN){
        HTREEITEM Skin = Append(Mdl, "Skin", (LPARAM) &node, Prev);
        //HTREEITEM Bones = Append(Mdl, "Bones", (LPARAM) &node, Skin);
        if(node.Skin.Bones.size() > 0){
            for(int n = 0; n < node.Skin.Bones.size(); n++){
                std::string sBone = "Bone: " + Names.at(node.Skin.Bones.at(n).nNameIndex).sName;
                Append(Mdl, sBone, (LPARAM) &(node.Skin.Bones.at(n)), Skin);
            }
        }
    }
    if(node.Head.nType & NODE_DANGLY){
        HTREEITEM Danglymesh = Append(Mdl, "Danglymesh", (LPARAM) &node, Prev);
        for(int i = 0; i < node.Dangly.Constraints.size(); i++)
            Append(Mdl, "Dangly Vertex " + std::to_string(i), (LPARAM) &(node), Danglymesh);
    }
    if(node.Head.nType & NODE_AABB){
        HTREEITEM Walkmesh = Append(Mdl, "Aabb", (LPARAM) &node, Prev);
        int nCounter = 0;
        if(node.Walkmesh.nOffsetToAabb > 0) AppendAabb(Mdl, &(node.Walkmesh.RootAabb), Walkmesh, nCounter);
    }
    if(node.Head.nType & NODE_SABER){
        HTREEITEM Saber = Append(Mdl, "Lightsaber", (LPARAM) &node, Prev);
        if(node.Saber.SaberData.size() > 0){
            for(int i = 0; i < node.Saber.SaberData.size(); i++){
                Append(Mdl, "Lightsaber Vertex " + std::to_string(i), (LPARAM) &(node.Saber.SaberData[i]), Saber);
            }

        }
    }

    if(node.Head.nNameIndex != 0){
        HTREEITEM Controllers = Append(Mdl, "Controllers", (LPARAM) &node, Prev);
        for(Controller & ctrl : node.Head.Controllers){
            Append(Mdl, ReturnControllerName(ctrl.nControllerType, node.Head.nType), (LPARAM) &ctrl, Controllers);
        }

        if(node.Head.nParentIndex.Valid()){
            MdlInteger<unsigned short> nNodeIndex = Mdl.GetNodeIndexByNameIndex(node.Head.nParentIndex);
            if(!nNodeIndex.Valid()) throw mdlexception("tree building error: dealing with a name index that does not have a node in geometry.");
            Node & parent = Data.MH.ArrayOfNodes.at(nNodeIndex);
            Append(Mdl, "Parent: " + Mdl.GetNodeName(parent), (LPARAM) &parent, Prev);
        }
    }

    HTREEITEM Children = Append(Mdl, "Children", (LPARAM) &node, Prev);
    for(Node & curnode : Data.MH.ArrayOfNodes){
        if(curnode.Head.nParentIndex == node.Head.nNameIndex){
            HTREEITEM Child = Append(Mdl, GetNodeTreePrefix(curnode.Head.nType) + Mdl.GetNodeName(curnode), (LPARAM) &curnode, Children);
        }
    }
}

void BuildGeometryTree(HTREEITEM Nodes, MDL & Mdl){
    if(!Mdl.GetFileData()) return;
    FileHeader & Data = *Mdl.GetFileData();

    /// Delete any leftover children, which means we can use this function to reconstruct this part of the tree
    TreeView_DeleteAllChildren(hTree, Nodes);

  try{
    std::vector<unsigned> offsets;
    offsets.reserve(Data.MH.nNodeCount);
    for(Node & node : Data.MH.ArrayOfNodes){
        HTREEITEM CurrentNode = Append(Mdl, GetNodeTreePrefix(node.Head.nType) + Mdl.GetNodeName(node), (LPARAM) &node, Nodes);
        BuildGeometryNode(node, CurrentNode, offsets, Mdl);

        /// If we're doing the hierarchy, we only want to write out the first node, so stop after that
        if(bModelHierarchy) break;
    }
  }
  catch(std::exception & e){
    Error("An exception occurred in BuildGeometryTree(): " + std::string(e.what()));
  }
  catch(...){
    Error("An exception occurred in BuildGeometryTree()!");
  }
}

void BuildTree(MDL & Mdl){
    if(!Mdl.GetFileData()){
        std::cout << "No data. Do not build tree.\n";
        return;
    }
    FileHeader & Data = *Mdl.GetFileData();

    HTREEITEM Root = Append(Mdl, to_ansi(Mdl.GetFilename()), NULL, TVI_ROOT);
    HTREEITEM Header = Append(Mdl, "Header", (LPARAM) &(Data.MH), Root);

    HTREEITEM Animations = Append(Mdl, "Animations", NULL, Root);
    BuildAnimationTree(Animations, Mdl);

    HTREEITEM Nodes = Append(Mdl, "Geometry", NULL, Root);
    BuildGeometryTree(Nodes, Mdl);

    TreeView_Expand(hTree, Root, TVE_EXPAND);
    std::cout << "Model tree building done!\n";
}

void BuildTree(BWM & Bwm){
    if(!Bwm.GetData()) return;
    BWMHeader & Walkmesh = *Bwm.GetData();

    HTREEITEM Root = Append(Bwm, to_ansi(Bwm.GetFilename()), NULL, TVI_ROOT);
    Append(Bwm, "Header", (LPARAM) &Walkmesh, Root);
    HTREEITEM Verts = Append(Bwm, "Vertices", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.verts.size(); n++){
        Append(Bwm, "Vertex " + std::to_string(n), (LPARAM) &Walkmesh.verts[n], Verts);
    }
    ProgressStepIt();

    HTREEITEM Faces = Append(Bwm, "Faces", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.faces.size(); n++){
        Append(Bwm, "Face " + std::to_string(n), (LPARAM) &Walkmesh.faces[n], Faces);
    }
    ProgressStepIt();

    HTREEITEM Aabb = Append(Bwm, "Aabb", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.aabb.size(); n++){
        Append(Bwm, "aabb " + std::to_string(n), (LPARAM) &Walkmesh.aabb[n], Aabb);
    }
    ProgressStepIt();

    HTREEITEM Array2 = Append(Bwm, "Edges", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.edges.size(); n++){
        Append(Bwm, "Edge " + std::to_string(n), (LPARAM) &Walkmesh.edges[n], Array2);
    }
    ProgressStepIt();

    HTREEITEM Array3 = Append(Bwm, "Perimeters", (LPARAM) NULL, Root);
    for(int n = 0; n < Walkmesh.perimeters.size(); n++){
        Append(Bwm, "Perimeter " + std::to_string(n), (LPARAM) &Walkmesh.perimeters[n], Array3);
    }
    ProgressStepIt();

    InvalidateRect(hTree, nullptr, true);
    std::cout << "Walkmesh tree building done!\n";
}
