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
	local ShapeArray = #()

    on create do (
        local node = selection[1]
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

	parameters paramblock rollout:mdlWalkmeshParams
	(
		ig_boxes type:#stringtab tabsizevariable:true
    	meshtype        type:#integer default:1 --ui:radio_meshtype
    	ig_recalc type:#integer default:1 --ui:chk_recalc
    	ig_multimode type:#integer default:1 ui:chk_multimode
    	room type:#integer default:-1 ui:spn_transition

    	-- exportwok is deemed redundant based on user input from DLA
    	--exportwok       type:#integer default:1 ui:box_exportwok

    	-- The next two properties will not be used at this point
    	-- I need to work out the code to support them
    	--autogrpfaces    type:#integer ui:box_autogrpfaces
    	--angle           type:#float ui:spn_angle
	)

	rollout mdlWalkmeshParams "Walkmesh Options" width:162 height:335
    (
        --radiobuttons radio_meshtype "" labels:#("Tile [aabb]", "Placeable [PWK]", "Door [DWK]") align:#left
        --checkbox box_exportwok "Export WOK File" align:#left
        --checkbox box_autogrpfaces "Auto-Group Faces" align:#left
        --spinner spn_angle "Angle:" align:#right

        group "Room Links"
        (
            listbox list_links height:5 selection:0
            button btn_removetrans "Remove Link" align:#center toolTip:"Removes selected link."
        )
        group "Add Link To Edge(s)"
        (
            spinner spn_transition "Room:" range:[-1,100,-1] offset:[0,3] align:#right type:#integer across:2
            button btn_addtrans "Apply" align:#center toolTip:"Adds room link to the currently selected edge(s)."
        )

        group "AABB Details"
        (
            listbox list_boxes "Boxes" height:5 selection:0
            label lbl_bl align:#left
            label lbl_tr align:#left
            label lbl_face align:#left
            radiobuttons rdo_display labels:#("No Display","Selected","All")
            button btn_listboxes "List aabb" width:144 align:#center toolTip:"List aabb data to Listener"
            button btn_makeaabb "Build WOK Data" width:144 align:#center toolTip:"Build the WOK aabb data"
            --checkbox chk_recalc "Recalc AABB on export"
            checkbox chk_multimode "Across multiple meshes"
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
            -- make sure there is a walk material for this node
            local node = selection[1]
            --local originalobject = modPanel.getCurrentObject()
            --modPanel.setCurrentObject node.baseObject
            --subObjectLevel = 2
            --modPanel.setCurrentObject originalobject
            --setSelectionLevel node {#edge}
            --subObjectLevel = 2
            if node.material == undefined then
            (
                node.material = nx_createNWNWalkMaterial()
            )

            if ig_boxes.count == 0 then
            (
                format "Rebuilding AABB list\r\n"
				local b = WriteAABBTree selection[1] verbose:false
				ig_boxes.count=0
				local p
				local tmp = #()
				for p in b do (
					ig_boxes[ig_boxes.count+1] = p
				)
			)

            GetLinks()

            -- populate list box
            /*
			local p
			local tmp = #()
			for p in ig_boxes do (
				join tmp #(p)
			)
			list_boxes.items = tmp
			*/
        )
        on mdlWalkmeshParams close do
        (
			-- delete the array of existing boxes
			for e in ShapeArray do
				delete e
			ShapeArray = #()
        )

        fn RedrawDisplayBoxes =
        (
			-- delete the array of existing boxes
			for e in ShapeArray do
				delete e
			ShapeArray = #()
			-- determine what to draw
			local tok
			local bot_left
			local top_right
			local boxpos
			if rdo_display.state == 2 then
			(
    			tok = filterString (list_boxes.selected) " "
    			bot_left = point3 (tok[1] as float) (tok[2] as float) (tok[3] as float)
    			top_right = point3 (tok[4] as float) (tok[5] as float) (tok[6] as float)
    			-- convert back to gmax cm units
    			bot_left *= 100
    			top_right *= 100
    			boxpos = (bot_left+top_right)/2
    			boxpos.z = bot_left.z
    			in coordsys parent ( boxpos += selection[1].pos )
        		s = box position:boxpos \
        		    length:(top_right.y-bot_left.y) \
        		    width:(top_right.x-bot_left.x) \
        		    height:(top_right.z-bot_left.z)
        		s.wirecolor = (color 255 0 0)
                append ShapeArray s
            ) else if rdo_display.state == 3 then
            (
                for b in ig_boxes do
                (
        			tok = filterString (b as string) " "
        			bot_left = point3 (tok[1] as float) (tok[2] as float) (tok[3] as float)
        			top_right = point3 (tok[4] as float) (tok[5] as float) (tok[6] as float)
        			-- convert back to gmax cm units
        			bot_left *= 100
        			top_right *= 100
        			boxpos = (bot_left+top_right)/2
        			boxpos.z = bot_left.z
        			in coordsys parent ( boxpos += selection[1].pos )
            		s = box position:boxpos \
            		    length:(top_right.y-bot_left.y) \
            		    width:(top_right.x-bot_left.x) \
            		    height:(top_right.z-bot_left.z)
                    append ShapeArray s
                )
            )
        )
        on list_boxes selected sel do
        (
            -- update the details labels
            local s = list_boxes.selected
            if s[1] != "#" then
            (
                local tok = filterString s " "
                lbl_bl.text = "BL: "+tok[1]+" "+tok[2]+" "+tok[3]
                lbl_tr.text = "TR: "+tok[4]+" "+tok[5]+" "+tok[6]
                lbl_face.text = "Face: "+tok[7]
            )
            -- Do drawing
            RedrawDisplayBoxes()
        )

        on btn_makeaabb pressed do
        (
            if meshtype != 1 then return 0  -- aabb not relevant
	        clearListener()

			local b = WriteAABBTree selection[1] verbose:false
			ig_boxes.count=0
			local p
			local tmp = #()
			for p in b do (
				ig_boxes[ig_boxes.count+1] = p
				join tmp #(p)
			)
			list_boxes.items = tmp
        )

        on btn_listboxes pressed do
        (
			local b
			clearListener()
			for b in ig_boxes do
			(
				format "%\r\n" b
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
