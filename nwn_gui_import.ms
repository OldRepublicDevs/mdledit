--------------------------------------------------------------------------------
--                                                                            --
-- NWN mdl Import/Export utility for GMax, v0.3b                              --
-- by Wayland (wreid@spectrumanalytic.com)                                    --
--                                                                            --
-- (c) Wayland Reid                                                           --
-- Based on Zaddix's original WOK file importer maxscript                     --
-- Based on code by (c) Copyright Zaddix, 2002                                --
--                                                                            --
-- Use this code in any way you see fit.                                      --
-- If you use any/all of the code in your own scripts,                        --
-- just credit me and Zaddix somewhere.                                       --
--                                                                            --
--------------------------------------------------------------------------------



----------------------------------------------------------------------------
----------------------------------------------------------------------------
---
--- ImportFile routines
---
----------------------------------------------------------------------------
----------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/aurora_fn_import.ms") -- this shouldn't be necessary because NWmax imports it globally on startup

-- import function for rollout and supporting utils
fn nx_importer =
(
	local bDoAnims = true
	local bDoGeom = true
	local targetAnimBase = undefined
    local mdlFileName = ImportRollout.import_filename_edit.text
    local animname	= undefined -- for loading single anims
    local insertpoint = 10

    case ImportRollout.rdo_options.state of
    (
	    1: ( bDoGeom=true; bDoAnims=true; )
	    2: ( bDoGeom=true; bDoAnims=false; )
	    3: (
				bDoGeom=false; bDoAnims=true;

                local base = undefined
                local basecount = 0
                for o in selection do
                (
                    if (iskindof o aurorabase) then
                    (
                        basecount += 1
                        base = o
                    )
                )
                if basecount == 0 then
                (
                    -- no model base was selected so see if only 1 in the scene.
                    -- If there is only 1 then select it
                    for o in $objects do
                    (
                        if (iskindof o aurorabase) then
                        (
                            basecount += 1
                            base = o
                        )
                    )
                )
                if basecount == 1 then (
                    targetAnimBase = base
                )

                if(targetAnimBase != undefined) then
                (
                    -- find the largets frame ref on model base
                    local largestframe = 0
                    for an in base.animations do
                    (
                        tempstr = filterString an " "
                        if (tempstr.count > 3) and ((tempstr[3] as integer) > largestframe) then
                        (
                            largestframe = tempstr[3] as integer
                        )
                    )
                    insertpoint = largestframe + 10
                )

				if ImportRollout.edt_animname.text.count > 0 then
				(
					animname = #()
	    	        append animname (ImportRollout.edt_animname.text)
    	        )

    	        if ImportRollout.spn_startframe.value > -1 then(
        	        insertpoint = ImportRollout.spn_startframe.value + 10
    	        )

	            -- check to make sure we have only a model base selected
	            if targetAnimBase == undefined then
	            (
    	            MessageBox "No target model aurorabase selected for Animation Import" title:"No Selection Found"
    	            return 0
	            )
	       )
    )

    if (not nx_existFile mdlFileName) then
    (
        MessageBox ("Model not imported.  " + mdlFileName + " does not exist.") title:"File does not exist"
    )
    else
    (
        clearListener()
        disableSceneRedraw()
        local pwk_file = (getFilenamePath mdlFileName)+(getFilenameFile  mdlFileName)+".pwk"
        local dwk_file = (getFilenamePath mdlFileName)+(getFilenameFile  mdlFileName)+".dwk"

        -- import MDL file
        ImportNWNmdl \
            mdlFileName \                            -- filename
            bDoAnims \                               -- import anims
            ImportRollout.warn_checkbox.checked \    -- show warnings
            loadGeom:bDoGeom \                       -- import geometry
            animLoadBase:targetAnimBase \            -- target aurorabase (if importing anims only)
            loadanimlist:animname \                  -- specific animation to import
            insertpoint:insertpoint \                                                       -- where to insert the new animation.
            showprogress:true \                                                             -- show progress
            location:[ImportRollout.spn_loc_x.value, ImportRollout.spn_loc_y.value, 0] \    -- import location
            uselytposition:ImportRollout.chk_uselyt.checked                                 -- use layout position

        -- import PWK file if available
        if (nx_existFile pwk_file) then
            ImportNWNwk pwk_file "pwk"

        -- import DWK file if available
        if (nx_existFile dwk_file) then
            ImportNWNwk dwk_file "dwk"

        enableSceneRedraw()

        if ImportRollout.chk_resetctrl.checked then
        (
					local objSum = $objects.count * 2.0
					local cnt = 0.0
					progressStart "Reset Controllers"
					format "Setting to tcb rotation\r\n"
					for o in $objects do
					(
			    		o.rotation.controller = tcb_rotation()
			    		cnt += 1
			    		if (progressUpdate (((cnt*100)/objSum) as integer)) == false then return 0
					)
					format "Setting back to linear rotation\r\n"
					for o in $objects do
					(
			    		o.rotation.controller = linear_rotation()
			    		cnt += 1
			    		if (progressUpdate (((cnt*100)/objSum) as integer)) == false then return 0
					)
					progressEnd()
        )

        -- Redraw gmax viewports
        clearSelection()
        max tool zoomextents all
        max views redraw

        percent = 0   -- does this do anything?
        gc() -- now wat iz dis?
    )
)

rollout ImportRollout "MDL Loading" width:162 height:250
(
	Group "Filename"
	(
		edittext import_filename_edit "" width:128 height:16
		button import_browse_button "Browse" width:70 height:24
	)
	Group "Options"
	(
		radioButtons rdo_options "" width:123 height:48 labels:#("Import Geom+Anims", "Import Geom Only", "Import Anims Only") default:1
		label lbl_animname "Anim name to load:" enabled:false
		edittext edt_animname enabled:false
		spinner spn_startframe "Insert Frame:" type:#integer range:[-1,100000,-1] enabled:false
		checkbox chk_resetctrl "Reset Orientation" checked:false
		checkbox warn_checkbox "Show warnings" width:128 height:16
		checkbox chk_uselyt "Use Layout Position" checked:true
		spinner spn_loc_x "Import Loc X:" range:[0,1000000,0] enabled:false
		spinner spn_loc_y "Import Loc Y:" range:[0,1000000,0] enabled:false
	)
	button import_button "Import" width:144 height:24

	on chk_uselyt changed val do
	(
        if(val) then val = false
        else val = true
		spn_loc_x.enabled = val
		spn_loc_y.enabled = val
	)

	on rdo_options changed val do
	(
		if val == 3 then (
			edt_animname.enabled = true
			lbl_animname.enabled = true
			spn_startframe.enabled = true
		)
		else (
			edt_animname.enabled = false
			lbl_animname.enabled = false
			spn_startframe.enabled = false
		)
	)

	on import_browse_button pressed do
	(
	    local filename = getOpenFileName caption:"Import mdl" types:"NWN mdl (*.mdl)|*.mdl|NWN ASCII Model (*.ascii)|*.ascii|All Files (*.*)|*.*|"
	    if (filename == undefined) then filename = ""
	    import_filename_edit.text = filename
	)

	on import_button pressed do
	(
		nx_importer()
	)
)

rollout nx_util6 "Area Tools" width:162 height:68
(
    -- ui
    button btn_minimap "MiniMap Maker" width:144 height:18 align:#center
    group "Layout File"
    (
        button btn_importlyt "Import" width:68 height:18 align:#left across:2 toolTip:"Import layout file. 'With Models' also imports relevant models from the same directory."
        button btn_exportlyt "Export" width:68 height:18 align:#right enabled:false toolTip:"Export layout file. 'With Models' also exports relevant models to the same directory."
        checkbox chk_withmodels "With Models" checked:false enabled:false
    )

    -- ensure only active if running max version of nwmax
    on nx_util6 open do
    (
	    if ( not g_ismax ) then
	    (
		    btn_minimap.enabled = false
	    )
    )

    -- actions
    on btn_minimap pressed do ( try (filein (scriptsPath+"NWmax/nw_ie_inc/nx_minimap_maker.ms")) catch() )

	on btn_importlyt pressed do
	(
	    local filename = getOpenFileName caption:"Import lyt" types:"KOTOR layout file (*.lyt)|*.lyt|"
	    if (filename != undefined) then ImportKOTORlyt filename showprogress:true withmodels:chk_withmodels.checked
	)
)
