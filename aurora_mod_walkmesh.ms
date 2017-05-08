/*-----------------------------------------------------------------------------\

    MDL Walkmesh Modifer

    This modifier stores the information needed by the Aurora engine for
    Walkmeshes. The actual application of the materials is done via the material
    navigator. This seems to be a very natural way to achieve this.

    BioWare noted in their original script that it would be better to create a
    NULL modifier in the SDK and them extend that.  Under GMAX we don't have
    that option unless you own the game developers SDK.  And I DON'T!   :-)

    Credits:
    Wayland Reid :-
        Extends Wayland's Import/Export script with rewrites as needed
    Zaddix :-
        Based on code from Zaddix's original WOK file importer
    BioWare :-
        This code is based on the information gleaned from BioWares
        3DS Max scripts. In many cases the Bioware code is used instead of
        "re-inventing the wheel". I have attempted to note all those places
        where I have used the Bioware code.


    Legal Stuff
    1. This is free software and is provided with no explicit or implied
       warranty. If you use this software then you do so at your own risk.
    2. If there is any law in your country that requires the author to provide
       any form of support or indemnity on the use of the this software you are
       not therefore authorised to use said software as no such indemnity or
       support will be provided (per 1) other than at the authors discretion.
    3. Credit has been given where code or methods have been integrated from
       other sources.
    4. No code has been knowingly included from other sources where that code
       is sold for commercial purposes.

\-------------------------------------------------------------------------------------------------*/
format "load: aurora_mod_walkmesh.ms\r\n"
nx_progressPlus()

plugin simpleMod aurorawalkmesh
name:"AuroraWalkmesh"
classID:#(0x882d65e4, 0xcd666735)
version:2
--replaceUI:true
(
    on create do (
        local node = selection[1]
        if(node != undefined and node.baseobject as string == "Editable_mesh") then (
            if(not (meshop.GetMapSupport node 0)) then (
                meshop.setMapSupport node 0 true
            )
            if((meshop.getNumMapVerts node 0) == 0) then (
                meshop.setNumMapVerts node 0 1
                meshop.setMapVert node 0 1 [1, 1, 1]
                meshop.buildMapFaces node 0
                for i = 1 to (meshop.getMapFace node 0) do (
                    meshop.setMapFace node 0 i [1, 1, 1]
                )
            )
        )
    )

	parameters paramblock rollout:mdlWalkmeshParams
	(
    	meshtype        type:#integer default:0 --ui:radio_meshtype
    	room type:#integer default:-1 ui:spn_transition
	)

	rollout mdlWalkmeshParams "Walkmesh Options" width:162 height:335
    (
        group "Walkmesh Type"
        (
            radiobuttons radio_meshtype "" default:0 labels:#("Area (aabb)", "Placeable (pwk)", "Door (dwk, closed)", "Door (dwk, open1)", "Door (dwk, open2)") align:#left
            --button btn_meshtypeconvert "Set Up" toolTip:"Creates necessary dummies, changes name, moves under correct node."
        )

        group "Room Links"
        (
            listbox list_links height:5 selection:0
            button btn_removetrans "Remove Link" align:#center toolTip:"Removes selected link."
        )
        group "Add Room Link To Edge(s)"
        (
            spinner spn_transition "Room:" range:[-1,100,-1] offset:[0,3] align:#right type:#integer across:2
            button btn_addtrans "Apply" align:#center toolTip:"Adds room link to the currently selected edge(s)."
        )

        fn GetLinks =
        (
            local temp = #()
            local node = selection[1]
            for f = 1 to (meshop.getNumFaces node) do
            (
                local ClrVertInds = getVCFace node f
                for i = 1 to 3 do
                (
                    local VertColor = getVertColor node ClrVertInds[i]
                    if(VertColor.b == 0 and VertColor.r == 0 and VertColor.g >= 100) then
                    (
                        append temp ("Edge " + ((f - 1)* 3 + i) as string + " to Room " + ((VertColor.g as Integer) - 100) as string)
                    )
                )
            )
			list_links.items = temp
        )
        fn GetSelectedLinkEdge =
        (
            if list_links.selection < 1 then return -1
            local s = list_links.selected
            local nIndex = -1
            if s[1] != "#" then
            (
                local tok = filterString s " "
                nIndex = tok[2] as integer - 1
            )
            return nIndex
        )
        on btn_addtrans pressed do
        (
            local node = selection[1]
            local selEdges = #()
            for EDGE in (getEdgeSelection node) do
            (
                append selEdges EDGE
            )
            local cColor = point3 0.0 ((100.0+room)/255.0) 0.0
            local nNewIndex = 0
            if(room == -1) then nNewIndex = 1
            else(
                for i = 1 to (meshop.getNumMapVerts node 0) do
                (
                    local ColorVert = meshop.getMapVert node 0 i
                    --local ColorVert = color (p3.x * 255.0) (p3.y * 255.0) (p3.z * 255.0)
                    format "Comparing colors: % and % \n" (ColorVert as color) cColor
                    if((ColorVert as color) == cColor) then
                    (
                        nNewIndex = i
                        exit
                    )
                )
                if(nNewIndex == 0) then
                (
                    nNewIndex = (meshop.getNumMapVerts node 0) + 1
                    meshop.setNumMapVerts node 0 nNewIndex keep:true
                    meshop.setMapVert node 0 nNewIndex (cColor)
                )
            )
            for i in selEdges do
            (
                local nFace = (i - 1) / 3
                local nEdge = mod (i - 1) 3
                local FaceInds = meshop.getMapFace node 0 (nFace+1)
                FaceInds[nEdge+1] = nNewIndex
                meshop.setMapFace node 0 (nFace+1) FaceInds
            )
            GetLinks()
            redrawViews()
        )

        on btn_removetrans pressed do
        (
            local node = selection[1]
            local nIndex = GetSelectedLinkEdge()
            if nIndex != -1 then
            (
                local nFace = nIndex / 3
                local nEdge = mod nIndex 3
                local FaceInds = meshop.getMapFace node 0 (nFace+1)
                FaceInds[nEdge+1] = 1
                meshop.setMapFace node 0 (nFace+1) FaceInds
                GetLinks()
                redrawViews()
            )
        )

        on mdlWalkmeshParams open do
        (
            local node = selection[1]
            if not classof node as string == "Editable_mesh" then return 0

            -- make sure there is a walk material for this node
            local matName = "NWN_WalkMaterial"
            if node.material == undefined or node.material.name != matName then
            (
                node.material = nx_createNWNWalkMaterial()
            )

            GetLinks()

            if(meshtype > 0) then radio_meshtype.state = meshtype
        )

        on mdlWalkmeshParams close do
        (
        )

        on radio_meshtype changed nmeshtype do
        (
            if(nmeshtype > 0) then
            (
                local oAuroraBase = undefined
                oParentBitchyOne = undefined
                local oMesh = selection[1]
                if oMesh != undefined then
                (
                    oParentBitchyOne = oMesh.parent
                    while(oAuroraBase == undefined and oParentBitchyOne != undefined) do
                    (
                        --format "Currently checking: % \n(class: %)\n" oParentBitchyOne (classof oParentBitchyOne)
                        if(classof oParentBitchyOne as string == "aurorabase") then
                        (
                            --format "Found aurorabase!\n"
                            oAuroraBase = oParentBitchyOne
                        )
                        else
                         oParentBitchyOne = oParentBitchyOne.parent
                    )
                )
                if (oAuroraBase == undefined) then messageBox "The mesh does not seem to be descended from an AuroraBase. Make sure that the mesh or one of its parents/ancestors is linked to an AuroraBase and try again." title:"Message" beep:false
                else
                (
                    meshtype = nmeshtype
                    local oWalkRoot = undefined
                    for oChild in oAuroraBase.children do
                    (
                        if(matchPattern oChild.name pattern:"*_pwk" or matchPattern oChild.name pattern:"*_dwk") then
                        (
                            oWalkRoot = oChild
                            exit
                        )
                    )

                    if nmeshtype == 1 then
                    (
                        for oChild in oAuroraBase.children do
                        (
                            if(oChild.modifiers["AuroraWalkmesh"] != undefined and oChild.modifiers["AuroraWalkmesh"].meshtype == 1 and oChild != oMesh) then
                            (
                                messageBox "This model already has an area walkmesh. It will be unlinked from the model." title:"Warning"
                                oChild.parent = undefined
                                oChild.modifiers["AuroraWalkmesh"].meshtype = 0
                                --exit
                            )
                        )

                        if oMesh.parent != oAuroraBase then
                        (
                            oMesh.parent = oAuroraBase
                        )

                        if oWalkRoot != undefined then
                        (
                            delete oWalkRoot
                        )

                        local oUse1 = undefined
                        local oUse2 = undefined
                        for oChild in oMesh.children do
                        (
                            if(matchPattern oChild.name pattern:"*pwk_*01" or matchPattern oChild.name pattern:"*dwk_*01") then
                            (
                                oUse1 = oChild
                            )
                            if(matchPattern oChild.name pattern:"*pwk_*02" or matchPattern oChild.name pattern:"*dwk_*02") then
                            (
                                oUse2 = oChild
                            )
                            if(oUse1 != undefined and oUse2 != undefined) then exit
                        )
                        if(oUse1 != undefined) then
                        (
                            delete oUse1
                        )
                        if(oUse2 != undefined) then
                        (
                            delete oUse2
                        )

                        if(matchPattern oMesh.name pattern:"*_wg*") then
                        (
                            oMesh.name = "WALKMESH"
                        )

                    )
                    else if nmeshtype == 2 then
                    (
                        for oChild in oAuroraBase.children do
                        (
                            if(oChild.modifiers["AuroraWalkmesh"] != undefined and oChild.modifiers["AuroraWalkmesh"].meshtype == 1 and oChild != oMesh) then
                            (
                                messageBox "This model already has an area walkmesh. It will be unlinked from the model." title:"Warning"
                                oChild.parent = undefined
                                oChild.modifiers["AuroraWalkmesh"].meshtype = 0
                                --exit
                            )
                        )

                        if(oWalkRoot == undefined) then
                        (
                            oWalkRoot = dummy pos:oAuroraBase.pos
                            oWalkRoot.boxsize = [10,10,10]
                            oWalkRoot.parent = oAuroraBase
                        )

                        if oMesh.parent != oWalkRoot then
                        (
                            oMesh.parent = oWalkRoot
                        )

                        for oChild in oWalkRoot.children do
                        (
                            if(oChild.modifiers["AuroraWalkmesh"] != undefined and oChild.modifiers["AuroraWalkmesh"].meshtype > 1 and oChild != oMesh) then
                            (
                                messageBox "This model already has a pwk/dwk walkmesh. It will be unlinked from the model." title:"Warning"
                                oChild.parent = undefined
                                oChild.modifiers["AuroraWalkmesh"].meshtype = 0
                                --exit
                            )
                        )

                        local oUse1 = undefined
                        local oUse2 = undefined
                        for oChild in oMesh.children do
                        (
                            if(matchPattern oChild.name pattern:"*pwk_*01" or matchPattern oChild.name pattern:"*dwk_*01") then
                            (
                                oUse1 = oChild
                            )
                            if(matchPattern oChild.name pattern:"*pwk_*02" or matchPattern oChild.name pattern:"*dwk_*02") then
                            (
                                oUse2 = oChild
                            )
                            if(oUse1 != undefined and oUse2 != undefined) then exit
                        )
                        if(oUse1 == undefined) then
                        (
                            oUse1 = dummy name:"pwk_use01" pos:[0,85,0]
                            oUse1.boxsize = [10,10,10]
                            oUse1.parent = oMesh
                        )
                        if(oUse2 == undefined) then
                        (
                            oUse2 = dummy name:"pwk_use02" pos:[0,-85,0]
                            oUse2.boxsize = [10,10,10]
                            oUse2.parent = oMesh
                        )

                        oUse1.name = "pwk_use01"
                        oUse2.name = "pwk_use02"

                        oWalkRoot.name = (oAuroraBase.name + "_pwk")

                        oMesh.name = (oAuroraBase.name + "_wg")
                    )
                    else if(nmeshtype == 3 or nmeshtype == 4 or nmeshtype == 5) then
                    (
                        for oChild in oAuroraBase.children do
                        (
                            if(oChild.modifiers["AuroraWalkmesh"] != undefined and oChild.modifiers["AuroraWalkmesh"].meshtype ==1 and oChild != oMesh) then
                            (
                                messageBox "This model already has an area walkmesh. It will be unlinked from the model." title:"Warning"
                                oChild.parent = undefined
                                oChild.modifiers["AuroraWalkmesh"].meshtype = 0
                                --exit
                            )
                        )

                        if(oWalkRoot == undefined) then
                        (
                            oWalkRoot = dummy pos:oAuroraBase.pos
                            oWalkRoot.boxsize = [10,10,10]
                            oWalkRoot.parent = oAuroraBase
                        )

                        if oMesh.parent != oWalkRoot then
                        (
                            oMesh.parent = oWalkRoot
                        )

                        for oChild in oWalkRoot.children do
                        (
                            if(oChild.modifiers["AuroraWalkmesh"] != undefined and oChild.modifiers["AuroraWalkmesh"].meshtype > 1 and oChild != oMesh) then
                            (
                                if(oChild.modifiers["AuroraWalkmesh"].meshtype < 3 or oChild.modifiers["AuroraWalkmesh"].meshtype == nmeshtype) then
                                (
                                    messageBox "This model already has a conflicting walkmesh. It will be unlinked from the model." title:"Warning"
                                    oChild.parent = undefined
                                    oChild.modifiers["AuroraWalkmesh"].meshtype = 0
                                    --exit
                                )
                            )
                        )

                        local oUse1 = undefined
                        local oUse2 = undefined
                        for oChild in oMesh.children do
                        (
                            if(matchPattern oChild.name pattern:"*pwk_*01" or matchPattern oChild.name pattern:"*dwk_*01") then
                            (
                                oUse1 = oChild
                            )
                            if(matchPattern oChild.name pattern:"*pwk_*02" or matchPattern oChild.name pattern:"*dwk_*02") then
                            (
                                oUse2 = oChild
                            )
                            if(oUse1 != undefined and oUse2 != undefined) then exit
                        )
                        if(oUse1 == undefined) then
                        (
                            oUse1 = dummy pos:[0,85,0]
                            oUse1.boxsize = [10,10,10]
                            oUse1.parent = oMesh
                        )
                        if(oUse2 == undefined) then
                        (
                            oUse2 = dummy pos:[0,-85,0]
                            oUse2.boxsize = [10,10,10]
                            oUse2.parent = oMesh
                        )

                        local sName
                        local sName1
                        local sName2
                        case nmeshtype of(
                            3:
                            (
                                sName1 = "DWK_dp_closed_01"
                                sName2 = "DWK_dp_closed_02"
                                sName = "DWK_wg_closed"
                            )
                            4:
                            (
                                sName1 = "DWK_dp_open1_01"
                                sName2 = "DWK_dp_open1_02"
                                sName = "DWK_wg_open1"
                            )
                            5:
                            (
                                sName1 = "DWK_dp_open2_01"
                                sName2 = "DWK_dp_open2_02"
                                sName = "DWK_wg_open2"
                            )
                        )

                        oUse1.name = sName1
                        oUse2.name = sName2

                        oWalkRoot.name = (oAuroraBase.name + "_DWK")

                        oMesh.name = sName
                    )
                    radio_meshtype.state = nmeshtype
                )
            )
        )
    )


	on map i p do
	(
		-- We do nothing here
		-- Per BioWare:
		-- This is a simplemod so it expects us to be adjusting
		-- the point positions, but we dont want to do that.
	)
)
