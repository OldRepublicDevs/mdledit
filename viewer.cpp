#include "MDL.h"
#include "dialog.h"
#include "geoview.h"

std::vector<GeoView> GeoViews;

void MDL::OpenGeoViewer(std::vector<std::string>cItem, LPARAM lParam){
    //Probably only nodes will ever be able to fire this function.
    //Still I should make sure I'm only running stuff on appropriate nodes
    if((cItem[1] == "Geometry") || ((cItem[3] == "Geometry") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node* node = (Node*) lParam;
        if(node->Head.nType & NODE_HAS_MESH){
            //Now we're ready to create the GeoView
            GeoViews.push_back(GeoView());
            GeoViews.back().SetData(*node);
            if(!(GeoViews.back().Run())){
                std::cout<<string_format("GeoView window creation failed!\n");
            }
        }
    }
}

void MDL::OpenViewer(std::vector<std::string>cItem, LPARAM lParam){
    std::stringstream sName;
    std::stringstream sPrint;

    if((cItem[0] == "")) return;

    /// Animation ///
    else if((cItem[1] == "Animations")){
        Animation * anim = (Animation*) lParam;

        sName<<"animation "<<anim->cName;
        ConvertToAscii(CONVERT_ANIMATION, sPrint, (void*) lParam);

        DialogWindow ctrldata;
        if(!ctrldata.Run()){
            std::cout<<string_format("DialogWindow creation failed!\n");
        }
        SetWindowText(ctrldata.hMe, sName.str().c_str());
        SetWindowText(GetDlgItem(ctrldata.hMe, IDDB_EDIT), sPrint.str().c_str());
    }
    /// Anim Node ///
    else if((cItem[1] == "Animated Nodes") || ((cItem[3] == "Animated Nodes") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node * node = (Node*) lParam;

        sName<<"animated node "<<FH[0].MH.Names[node->Head.nNameIndex].cName;
        ConvertToAscii(CONVERT_ANIMATION_NODE, sPrint, (void*) lParam);
        ConvertToAscii(CONVERT_ENDNODE, sPrint, (void*) lParam);

        DialogWindow ctrldata;
        if(!ctrldata.Run()){
            std::cout<<string_format("DialogWindow creation failed!\n");
        }
        SetWindowText(ctrldata.hMe, sName.str().c_str());
        SetWindowText(GetDlgItem(ctrldata.hMe, IDDB_EDIT), sPrint.str().c_str());
    }
    /// Geo Node ///
    else if((cItem[1] == "Geometry") || ((cItem[3] == "Geometry") && ((cItem[1] == "Children") || (cItem[3] == "Parent")))){
        Node * node = (Node*) lParam;

        sName<<"node "<<FH[0].MH.Names[node->Head.nNameIndex].cName;
        sPrint<<"";
        //sPrint = "";
        ConvertToAscii(CONVERT_HEADER, sPrint, (void*) lParam);
        if(node->Head.nType & NODE_HAS_AABB){
            ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            ConvertToAscii(CONVERT_AABB, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_SABER){
            ConvertToAscii(CONVERT_SABER, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_DANGLY){
            ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            ConvertToAscii(CONVERT_DANGLY, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_SKIN){
            ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
            ConvertToAscii(CONVERT_SKIN, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_MESH){
            ConvertToAscii(CONVERT_MESH, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_EMITTER){
            ConvertToAscii(CONVERT_EMITTER, sPrint, (void*) lParam);
        }
        else if(node->Head.nType & NODE_HAS_LIGHT){
            ConvertToAscii(CONVERT_LIGHT, sPrint, (void*) lParam);
        }
        ConvertToAscii(CONVERT_ENDNODE, sPrint, (void*) lParam);

        DialogWindow ctrldata;
        if(!ctrldata.Run()){
            std::cout<<string_format("DialogWindow creation failed!\n");
        }
        else{
            SetWindowText(ctrldata.hMe, sName.str().c_str());
            SetWindowText(GetDlgItem(ctrldata.hMe, IDDB_EDIT), sPrint.str().c_str());
        }
    }
    /// Controller ///
    else if((cItem[1] == "Controllers")){
        Controller * ctrl = (Controller*) lParam;

        std::string sLocation;
        if(ctrl->nAnimation == -1){
            sLocation = "geometry";
        }
        else{
            sLocation = "animation '" + std::string(FH[0].MH.Animations[ctrl->nAnimation].cName.c_str()) + "'";
        }
        std::string sController = ReturnControllerName(ctrl->nControllerType, GetNodeByNameIndex(ctrl->nNameIndex).Head.nType);
        if(ctrl->nColumnCount & 16) sController+="bezierkey";
        else if(ctrl->nAnimation != -1) sController+="key";
        sName<<sLocation<<" > node '"<<FH[0].MH.Names[ctrl->nNameIndex].cName<<"' > controller '"<<sController<<"'";
        //sName<<"controller '"<<sController<<"' in node '"<<FH[0].MH.Names[ctrl->nNameIndex].cName<<"' in "<<sLocation;
        if(!(cItem[3] == "Geometry")){
            ConvertToAscii(CONVERT_CONTROLLER_KEYED, sPrint, (void*) lParam);
        }
        else{
            ConvertToAscii(CONVERT_CONTROLLER_SINGLE, sPrint, (void*) lParam);
        }

        DialogWindow ctrldata;
        if(!ctrldata.Run()){
            std::cout<<string_format("DialogWindow creation failed!\n");
        }
        SetWindowText(ctrldata.hMe, sName.str().c_str());
        SetWindowText(GetDlgItem(ctrldata.hMe, IDDB_EDIT), sPrint.str().c_str());
    }
    else return;
}
