/*-----------------------------------------------------------------------------\

    Aurora Import Functions

    The importing of data is based on reading in the model a line at a time
    and storing the data in appropriate variables. This code is integrated
    from Wayland's v0.3b script with some additional enhancements in the
    areas of animations, pwk and dwk processing.


    GMAX MDL Tool for NWN
    by Joco (jameswalker@clear.net.nz)

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

\-----------------------------------------------------------------------------*/

struct model_def (
    newmodel,
    setsupermodel,
    classification,
    setanimationscale
)
/*
struct model_node_trimesh (
)

struct model_node_emitter (
)

struct model_node_light (
)

struct model_node_ref (
)
*/

-----------------------------------
-- Binary Model Importer Dialog  --
-----------------------------------
global fControl
rollout rl_binmdl "Decompile MDL"
(
    local tickCounter = 0
    label lbl_1 "Model decompile in progress"
    label lbl_ticker ""
    timer t_looking interval:1000

    on t_looking tick do
    (
        tickCounter += 1
        lbl_ticker.text = (tickCounter as string)
        if (deletefile fControl) or (tickCounter > 15) then
        (
            DestroyDialog rl_binmdl
        )
    )
)


-----------------------------------
-- Activity Bar update functions --
-----------------------------------
fn ImportActivityBar =
(
    percent +=0.1
    if percent > 100 then percent = 0
    ImportRollout.progress.value = percent
)

-------------------------------
--- Get Next Non-Blank line ---
-------------------------------
fn getNextNonBlankLine fileHandle=
(
    local line = ""
    while ((line == "") and (not eof fileHandle)) do
    (
        line = trimright (readDelimitedString fileHandle "\n")
    )
    return line
)


----------------------------------------------------------
-- Adds an MDL skin using the data in the weightsBuffer --
----------------------------------------------------------
fn addMDLSkin node buf =
(
    if DEBUG then format "%SKIN PROCESSING: addMDLSkin - start: node=%\r\n" strIndent3 node
    setCommandPanelTaskMode mode:#modify
    modPanel.setCurrentObject node.skin

    if not DEBUG then
        DisableSceneRedraw()

    local v = 0
    local i = 0
    local gmax_bug
    local bonelist = #()
    local weightlist = #()
    local i
    local b -- bone index
    local boneName
    local weight
    local tok

    if DEBUG then format "%SKIN PROCESSING: build up skin vert list\r\n" strIndent3
    local allVerts = #()
    for i = 1 to (skinops.getnumbervertices (node.skin)) do
    (
        append allVerts i
    )

    if DEBUG then format "%SKIN PROCESSING: Start loading weight data\r\n" strIndent3
    -- Work through the weight data
    for data in buf do
    (
        --ImportActivityBar()
        v += 1; -- increment vertex
        tok = nx_Tokenizer()
        tok.SetString(data)
        --format "v: % " v

        if DEBUG then format "%SKIN PROCESSING: build up the bone and weight lists for the vertex\r\n" strIndent4
        -- build up the bone and weight lists for the vertex
        while (tok.PeekToken() != undefined) do
        (
            b = 0   -- bone index
            boneName = tok.ReadString()
            weight = tok.ReadFloat()

            if DEBUG then format "%SKIN PROCESSING: Finding bone in skin\r\n" strIndent4
            for i = 1 to (skinops.GetNumberBones node.skin) do
            (
                if ((skinops.GetBoneName node.skin i 0) == boneName) then
                (
                    b = i
                    exit
                )
            )

            if (b == 0) then    -- bone not yet linked to skin
            (
                if DEBUG then format "%SKIN PROCESSING: Start linking bone to skin\r\n" strIndent4
                boneNode = getNodeByName boneName

                if boneNode == undefined then
                (
                    format "ERROR! Unable to find the bone '%' in the scene! Check bones are in mdl.\r\n" boneName
                )

                skinops.AddBone node.skin boneNode 0
                b = skinops.GetNumberBones node.skin

                -- also mark this node as a bone
                --format "Setting bone states for: %\r\n" boneNode.name
                boneNode.SetBoneEnable true 0
                boneNode.BoneAutoAlign = true
                boneNode.BoneFreezeLength = true
                boneNode.ShowLinks = true
                boneNode.ShowLinksOnly = false
                boneNode.BoneScaleType = #None

                -- exclude all the vertexes for this bone
                --format "Exclude all verts for bone: %\r\n" boneNode.name
                skinOps.SelectBone (node.skin) b
                skinOps.SelectVertices (node.skin) allVerts
                skinOps.buttonExclude (node.skin)
                skinOps.SetInnerRadius (node.skin) b 1 0.0
                skinOps.SetOuterRadius (node.skin) b 1 0.0
                skinOps.SetInnerRadius (node.skin) b 2 0.0
                skinOps.SetOuterRadius (node.skin) b 2 0.0

                -- set bone as absolute
                --skinops.selectBone (node.skin) b
                --skinops.setSelectedBonePropRelative (node.skin) 1
                if DEBUG then format "%SKIN PROCESSING: End linking bone to skin\r\n" strIndent4
            )


            gmax_bug = node.numVerts
            skinops.SelectVertices (node.skin) v
            skinops.buttonInclude (node.skin)
            --gmax_bug = node.numVerts
            --skinops.SetVertexWeights (node.skin) v b weight
            if DEBUG then (
                format "%addMDLSkin: vertex:% bone:%(%) weight=%\r\n" strIndent4 v boneName b weight
            )

            -- add to bone and weight lists
            append bonelist b
            append weightlist weight

        ) -- end While
        if DEBUG then format "%SKIN PROCESSING: done building up the bone and weight lists for the vertex\r\n" strIndent4

        -- add vertex weight data
        skinops.SetVertexWeights (node.skin) v bonelist weightlist
        bonelist = #()
        weightlist = #()
    )
    if DEBUG then format "%SKIN PROCESSING: Stop loading weight data\r\n" strIndent3

    if not DEBUG then
        EnableSceneRedraw()

    if DEBUG then format "%SKIN PROCESSING: end\r\n" strIndent3
)

--------------------------------------------------------------------------------
-- Import DWK/PWK file
--------------------------------------------------------------------------------
fn ImportNWNwk pFile type =
(
    local currentPath = getFilenamePath pFile

    -- Open up the file and check to see if its a binary file
    local binMdlStream = fopen pFile "rb"
    local isNotBinary = ReadByte binMdlStream
    fclose binMdlStream

    if (isNotBinary == 0) then
    (
        -- we need to decompile this model. We assume it is a real model and not junk
        local tstamp = (timestamp() as string)
        fControl = scriptsPath+"nwmax\\scratch\\"+tstamp+".txt"
        local pFilePath = getFilenamePath pFile
        local chkLoop
        local bDecompiled = false
        --clearListener()
        -- In theory we don't need this but will keep it here for sake
        -- of completeness
        /*
        format "<snoopstart file=%>\r\n" (scriptsPath+"nwmax\\scratch\\scratch.txt")
        format "Processing ...\r\n"
        format "<commandstart>\r\n"
        format "\"%\" -d \"%\" \"%\\\" > \"%\"\r\n" g_compiler pFile pFilePath (scriptsPath+"nwmax\\scratch\\decomp.txt")
        format "echo \"done\" > \"%\"\r\n" fControl
        format "</commandstart>\r\n"
        format "</snoopstart>\r\n"
        format "</snoopend>\r\n"
        */

        -- Start up model dialog to wait for this processing to complete
        createdialog rl_binmdl modal:true

        -- check for ascii model exists
        pFile = pFile + ".ascii"
        if (nx_existFile pFile) == false then
        (
            MessageBox("Binary PWK/DWK File not decompiled. Check file is valid or NWNmdlcomp is present.")
            return false
        )
    )

    -- Open up the file
    local mdlStream = openFile pFile mode:"r"
    local quitEarly = false
    lineNum = 0
    try (
        -- Go through the whole file
        while not (eof mdlStream) and not quitEarly do
        (
            local tok = nx_Tokenizer()
            local line = getNextNonBlankLine mdlStream
            tok.SetString (line)
            local idToken = tok.ReadToken()
            if (idToken[1] == "#") then idToken = "#"

            case idToken of
            (
                "#": () -- ignore comments
                "node": -- Node Struct
                (
                    -- Define initial node data
                    objType = tok.ReadToken()
                    objName = tok.ReadToken()
                    objWireColor = color 255 255 255
                    objAmbient = color 0 0 0
                    objDiffuse = color 0 0 255
                    objSpecular = color 0 0 0
                    objSelfIllum = color 0 0 0
                    objShine = 0
                    objAlpha = 1

                    parent = ""
                    textureName = ""

                    objPos = [0, 0, 0]
                    objRot = quat 0 0 0 0
                    objScale = [1, 1, 1]
                    objMaterial = undefined

                    objInheritcolor = 0
                    objRotatetexture = 0
                    objTransparencyhint = 0

                    numVerts = 0
                    verts = #()
                    numFaces = 0
                    faces = #()
                    numTVerts = 0
                    TVerts = #()
                    TVfaces = #()
                    smoothGroups = #()
                    surfacemats = #()
                )

                "parent": parent = tok.ReadToken()
                "position":
                (
                    objPos = tok.ReadPoint3()
                    objPos *= 100
                )
                "orientation":
                (
                    local axis = tok.ReadPoint3()
                    local angle = RadToDeg (tok.ReadFloat())
                    objRot = quat angle axis
                )
                "wirecolor": objWireColor = tok.ReadColor()
                "ambient": objAmbient = tok.ReadColor()
                "diffuse": objDiffuse = tok.ReadColor()
                "specular": objSpecular = tok.ReadColor()
                "shininess": objShine = tok.ReadInteger()
                "alpha": objAlpha = tok.ReadFloat()

                "selfillumcolor": objSelfIllum = tok.ReadColor()
                -- to cope with a bug in old versions of nwe
                "setfillumcolor": objSelfIllum = tok.ReadColor()
                -- bug coping ends

                "bitmap":
                (
                    textureName = tok.ReadToken()
                    if (textureName == "NULL") then textureName = ""
                )
                "verts": -- Verticies Struct
                (
                    numVerts = tok.ReadInteger()
                    for i = 1 to numVerts do
                    (
                        tok.SetString (getNextNonBlankLine mdlStream)
                        lineNum += 1
                        local p3 = tok.ReadPoint3()
                        p3 *= 100
                        append verts p3
                    )
                )
                "faces": -- Faces Struct
                (
                    local p1
                    local p3
                    numFaces = tok.ReadInteger()
                    for i = 1 to numFaces do
                    (
                        tok.SetString (getNextNonBlankLine mdlStream)
                        lineNum += 1

                        -- Get the current Face
                        -- mdl file is zero-based index, max expects
                        -- one-based index into vert array
                        p3 = (tok.ReadPoint3() + [1,1,1])
                        append faces p3

                        -- Get Smoothing Group
                        p1 = tok.ReadInteger()
                        append smoothGroups p1

                        -- Get the current TVface
                        p3 = (tok.ReadPoint3() + [1,1,1])
                        append TVfaces p3

                        -- Get Surface Material
                        p1 = tok.ReadInteger()
                        append surfacemats p1
                    )
                )
                "endnode": -- Done parsing mesh, so create the object
                (
                    if DEBUG then format "%ImportNWNwk: BUILD MESH OBJECT: % %\r\n" strIndent1 objType objName
                    local newObj
                    case objType of
                    (
                        "trimesh":
                        (
                            -- Make material, if necessary
                            -- Texture is allways assumed to be "" as there is no purpose
                            -- in having a texture/bitmap for a pwk mesh
                            local matName = objName
                            --objMaterial = sceneMaterials[matName]
                            -- if we are in a trimesh in a pwk or dwk file it
                            -- needs a walkmesh material against it
                            --objMaterial = nx_createNWNWalkMaterial()
                            --if (objMaterial == undefined) then
                            --(
                            --    objMaterial = standardmaterial name:matName
                            --    objMaterial.shaderType = 1 -- Blinn
                            --    objMaterial.ambient = objAmbient
                            --    objMaterial.diffuse = objDiffuse
                            --    objMaterial.specular = objSpecular
                            --    objMaterial.glossiness = objShine
                            --    objMaterial.selfillumcolor = objSelfIllum
                                -- alpha is no done on the trimesh modifier only
                                --objMaterial.opacity = objAlpha
                            --)

                            -- Make an editable mesh
                            local newObj = mesh name:objName \
                                            vertices:verts \
                                            faces:faces \
                                            wirecolor:objWireColor \
                                            material:undefined \
                                            materialIDS:surfacemats

                            -- special post processing to deal with some sort of odd Max vert position distortion
                            for verti = 1 to verts.count do
                            (
                                setVert newObj verti verts[verti]
                            )
                            update newObj

                            -- Assign smoothing groups
                            for i = 1 to smoothGroups.count do
                                setFaceSmoothGroup newObj i smoothGroups[i]

                            -- We need to process for the appropriate
                            -- modifiers to be applied to the mesh.
                            --aurora_mod = aurorawalkmesh()
                            --case type of
                            --(
                            --    "pwk": aurora_mod.meshtype = 2
                            --    "dwk": aurora_mod.meshtype = 3
                            --)
                            --addModifier newObj aurora_mod

                            -- Update the mesh
                            update newObj

                        ) --End trimesh

                        default:
                        (
                            newObj = dummy name:objName
                            newObj.boxsize = [5,5,5]
                        )
                    ) --End objType case

                    -- get the parent node, if we can
                    if parent != "" then
                        local itsParent = getNodeByName parent exact:true
                    if itsParent != undefined then
                    (
                        -- parent exists
                        newObj.parent = itsParent
                        newObj.scale.controller = tcb_scale()
                        newObj.position.controller = tcb_position()
                        newObj.rotation.controller = tcb_rotation()

                        -- remove anim on always static mesh
--                        in coordsys parent
--                        (
--                            with animate on at time 0 newObj.scale = objScale
--                            with animate on at time 0 newObj.rotation = objRot
--                            with animate on at time 0 newObj.position = objPos
--                        )
						newObj.rotation = objRot
						newObj.position = objPos
                    ) else
                    (
                        -- Build the parent node as it doesn't exist
                        local modelbase = getNodeByName (getFilenameFile pFile) exact:true
                        local newParent = dummy name:parent
                        aurora_mod = auroraplaceable()
                        case type of
                        (
                            "pwk": aurora_mod.meshtype = 1
                            "dwk": aurora_mod.meshtype = 2
                        )
                        max modify mode
                        modPanel.setCurrentObject newParent
                        modPanel.addModToSelection aurora_mod
                        -- link parent to the model base
                        newParent.parent = modelbase
                        -- configure the new object linked to parent
                        newObj.parent = newParent
                        newObj.scale.controller = tcb_scale()
                        newObj.position.controller = tcb_position()
                        newObj.rotation.controller = tcb_rotation()

                        -- remove anim on always static mesh
--                        in coordsys parent
--                        (
--                            with animate on at time 0 newObj.scale = objScale
--                            with animate on at time 0 newObj.rotation = objRot
--                            with animate on at time 0 newObj.position = objPos
--                        )
                    ) -- end if
                ) -- End endnode case
            ) -- end iToken case
        ) -- end while
    ) catch
    (
        --messageBox ("NWN importer Script Error reading mdl file at line:" + lineNum as string)
        close mdlStream
        throw
    )
    close mdlStream
)



--------------------------------------------------------------------------------
-- Import WOK file
--------------------------------------------------------------------------------
fn nx_LoadWok pFile =
(
    local currentPath = getFilenamePath pFile

    -- Open up the file and check to see if its a binary file
    local binMdlStream = fopen pFile "rb"
    local isNotBinary = ReadByte binMdlStream
    fclose binMdlStream

    if (isNotBinary == 0) then
    (
        -- we need to decompile this model. We assume it is a real model and not junk
        local tstamp = (timestamp() as string)
        fControl = scriptsPath+"nwmax\\scratch\\"+tstamp+".txt"
        local pFilePath = getFilenamePath pFile
        local chkLoop
        local bDecompiled = false
        --clearListener()
        -- In theory we don't need this but will keep it here for sake
        -- of completeness
        /*
        format "<snoopstart file=%>\r\n" (scriptsPath+"nwmax\\scratch\\scratch.txt")
        format "Processing ...\r\n"
        format "<commandstart>\r\n"
        format "\"%\" -d \"%\" \"%\\\" > \"%\"\r\n" g_compiler pFile pFilePath (scriptsPath+"nwmax\\scratch\\decomp.txt")
        format "echo \"done\" > \"%\"\r\n" fControl
        format "</commandstart>\r\n"
        format "</snoopstart>\r\n"
        format "</snoopend>\r\n"
        */

        -- Start up model dialog to wait for this processing to complete
        createdialog rl_binmdl modal:true

        -- check for ascii model exists
        pFile = pFile + ".ascii"
        if (nx_existFile pFile) == false then
        (
            MessageBox("Binary WOK File not decompiled. Check file is valid or NWNmdlcomp is present.")
            return false
        )
    )

    -- Open up the file
    local mdlStream = openFile pFile mode:"r"
    local quitEarly = false
    lineNum = 0
    try (
        -- Go through the whole file
        while not (eof mdlStream) and not quitEarly do
        (
            local tok = nx_Tokenizer()
            local line = getNextNonBlankLine mdlStream
            tok.SetString (line)
            local idToken = tok.ReadToken()
            if (idToken[1] == "#") then idToken = "#"

            case idToken of
            (
                "beginwalkmeshgeom":
                (
                    objWokName = tok.ReadToken()
                )
                "#": () -- ignore comments
                "node": -- Node Struct
                (
                    -- Define initial node data
                    objType = tok.ReadToken()
                    objName = tok.ReadToken()
                    objWireColor = color 255 255 255
                    objAmbient = color 0 0 0
                    objDiffuse = color 0 0 255
                    objSpecular = color 0 0 0
                    objSelfIllum = color 0 0 0
                    objShine = 0
                    objAlpha = 1

                    parent = ""
                    textureName = ""

                    objPos = [0, 0, 0]
                    objRot = quat 0 0 0 0
                    objScale = [1, 1, 1]
                    objMaterial = undefined

                    objInheritcolor = 0
                    objRotatetexture = 0
                    objTransparencyhint = 0

                    numVerts = 0
                    verts = #()
                    numFaces = 0
                    faces = #()
                    numTVerts = 0
                    TVerts = #()
                    TVfaces = #()
                    smoothGroups = #()
                    surfacemats = #()
                )

                "parent": parent = tok.ReadToken()
                "position":
                (
                    objPos = tok.ReadPoint3()
                    objPos *= 100
                )
                "orientation":
                (
                    local axis = tok.ReadPoint3()
                    local angle = RadToDeg (tok.ReadFloat())
                    objRot = quat angle axis
                )
                "wirecolor": objWireColor = tok.ReadColor()
                "ambient": objAmbient = tok.ReadColor()
                "diffuse": objDiffuse = tok.ReadColor()
                "specular": objSpecular = tok.ReadColor()
                "shininess": objShine = tok.ReadInteger()
                "alpha": objAlpha = tok.ReadFloat()

                "selfillumcolor": objSelfIllum = tok.ReadColor()
                -- to cope with a bug in old versions of nwe
                "setfillumcolor": objSelfIllum = tok.ReadColor()
                -- bug coping ends

                "bitmap":
                (
                    textureName = tok.ReadToken()
                    if (textureName == "NULL") then textureName = ""
                )
                "verts": -- Verticies Struct
                (
                    numVerts = tok.ReadInteger()
                    for i = 1 to numVerts do
                    (
                        tok.SetString (getNextNonBlankLine mdlStream)
                        lineNum += 1
                        local p3 = tok.ReadPoint3()
                        p3 *= 100
                        append verts p3
                    )
                )
                "faces": -- Faces Struct
                (
                    local p1
                    local p3
                    numFaces = tok.ReadInteger()
                    for i = 1 to numFaces do
                    (
                        tok.SetString (getNextNonBlankLine mdlStream)
                        lineNum += 1

                        -- Get the current Face
                        -- mdl file is zero-based index, max expects
                        -- one-based index into vert array
                        p3 = (tok.ReadPoint3() + [1,1,1])
                        append faces p3

                        -- Get Smoothing Group
                        p1 = tok.ReadInteger()
                        append smoothGroups p1

                        -- Get the current TVface
                        p3 = (tok.ReadPoint3() + [1,1,1])
                        append TVfaces p3

                        -- Get Surface Material
                        p1 = tok.ReadInteger()
                        append surfacemats p1
                    )
                )
                "endnode": -- Done parsing mesh, so create the object
                (
                    if DEBUG then format "%nx_LoadWok: BUILD MESH OBJECT: % %\r\n" strIndent1 objType objName
                    local newObj
                    case objType of
                    (
                        "aabb":
                        (
                            -- Make material, if necessary
                            -- Texture is allways assumed to be "" as there is no purpose
                            -- in having a texture/bitmap for a pwk mesh
                            local matName = objName
                            --objMaterial = sceneMaterials[matName]
                            -- if we are in a trimesh in a pwk or dwk file it
                            -- needs a walkmesh material against it
                            objMaterial = nx_createNWNWalkMaterial()
                            if (objMaterial == undefined) then
                            (
                                objMaterial = standardmaterial name:matName
                                objMaterial.shaderType = 1 -- Blinn
                                objMaterial.ambient = objAmbient
                                objMaterial.diffuse = objDiffuse
                                objMaterial.specular = objSpecular
                                objMaterial.glossiness = objShine
                                objMaterial.selfillumcolor = objSelfIllum
                                -- alpha is no done on the trimesh modifier only
                                --objMaterial.opacity = objAlpha
                            )

                            -- Make an editable mesh
                            local newObj = mesh name:objName \
                                            vertices:verts \
                                            faces:faces \
                                            wirecolor:objWireColor \
                                            material:objMaterial \
                                            materialIDS:surfacemats

                            -- special post processing to deal with some sort of odd Max vert position distortion
                            for verti = 1 to verts.count do
                            (
                                setVert newObj verti verts[verti]
                            )
                            update newObj

                            -- Assign smoothing groups
                            for i = 1 to smoothGroups.count do
                                setFaceSmoothGroup newObj i smoothGroups[i]

                            -- We need to process for the appropriate
                            -- modifiers to be applied to the mesh.
                            aurora_mod = aurorawalkmesh()
                            aurora_mod.meshtype = 1
                            addModifier newObj aurora_mod

                            -- Update the mesh
                            update newObj

                        ) --End trimesh

                        default:
                        (
                            newObj = dummy name:objName
                            newObj.boxsize = [5,5,5]
                        )
                    ) --End objType case

                    local itsParent
                    -- get the parent node, if we can
                    if objWokName != "" then
                    (
                        itsParent = getNodeByName objWokName exact:true
                    )
                    if itsParent != undefined then
                    (
                        -- parent exists
                        newObj.parent = itsParent
                        newObj.scale.controller = tcb_scale()
                        newObj.position.controller = tcb_position()
                        newObj.rotation.controller = tcb_rotation()

                        -- remove anim on always static mesh
--                        in coordsys parent
--                        (
--                            newObj.pos = objPos
--                            with animate on at time 0 newObj.scale = objScale
--                            with animate on at time 0 newObj.rotation = objRot
--                            with animate on at time 0 newObj.position = objPos
--                        )
                        if DEBUG then format "Set pos etc.\r\n"
                    )
                ) -- End endnode case
            ) -- end iToken case
        ) -- end while
    ) catch
    (
        --messageBox ("NWN importer Script Error reading mdl file at line:" + lineNum as string)
        if mdlStream != undefined then
            close mdlStream
        throw
    )
   if mdlStream != undefined then close mdlStream
)

-- bead-v modification: hopefully solving some problems with animation controllers
global gblDynBuildList_Node
-- end bead-v modification

--------------------------------------------------------------------------------
-- Import MDL file
--------------------------------------------------------------------------------
fn ImportNWNmdl pFile importAnims showWarnings \
                loadGeom:true \
                animLoadBase:undefined \
                insertpoint:0 \
                mappingtbl:undefined \
                loadanimlist:undefined \
                animloadtype:0 \
                showprogress:false \
                loadtiff:false \
                location:[0,0,0]
=
(
    -- Model heirarchy defs
    local MDL_DEF = 1
    local MDL_GEOM = 2
    local MDL_ANIM = 3
    local MDL_NOT_IN_MDL = 4

    -- temp modifier var
    local aurora_mod
    -- temp helper var
    local aurora_helper

    local currentPath = getFilenamePath pFile

    -- only progress further if there IS a file available to process
    if not (nx_existFile pFile) then return false

    -- Open up the file and check to see if its a binary file
    local binMdlStream = fopen pFile "rb"
    local isNotBinary = ReadByte binMdlStream
    fclose binMdlStream

    if (isNotBinary == 0) then
    (
        -- we need to decompile this model. We assume it is a real model and not junk
        local tstamp = (timestamp() as string)
        fControl = scriptsPath+"nwmax\\scratch\\"+tstamp+".txt"
        local pFilePath = getFilenamePath pFile
        local chkLoop
        local bDecompiled = false

        if g_ismax then
        (
            local cmdFile
            local strCmd
            cmdFile = createFile (scriptsPath+"nwmax\\scratch\\import.bat")
            format "\"%\" -d \"%\" \"%\\\"\r\n" g_compiler pFile pFilePath
            format "\"%\" -d \"%\" \"%\\\"\r\n" g_compiler pFile pFilePath to:cmdFile
            flush cmdFile
            close cmdFile
            strCmd = "\"" + scriptsPath + "nwmax\\scratch\\import.bat\""
            DOSCommand (strCmd)
        ) else
        (
            clearListener()
            nx_FlushBuffer reset:true
            -- Have to hard code the delim here into verbose mode
            format "<snoopstart file=%>%" (scriptsPath+"nwmax\\scratch\\scratch.txt") "~\n" to:g_strBuffer
            format "Processing ...%" g_delim to:g_strBuffer
            format "\"%\" -d \"%\" \"%\\\" > \"%\"%" g_compiler pFile pFilePath (scriptsPath+"nwmax\\scratch\\decomp.txt") "~\n" to:g_strBuffer
            format "echo \"done\" > \"%\"%" fControl "~\n" to:g_strBuffer

            format "<snoopstart file=%>%" (scriptsPath+"nwmax\\scratch\\import.bat") "~\n" to:g_strBuffer
            format "\"%\" -d \"%\" \"%\\\" > \"%\"%" g_compiler pFile pFilePath (scriptsPath+"nwmax\\scratch\\decomp.txt") "~\n" to:g_strBuffer
            format "echo \"done\" > \"%\"%" fControl "~\n" to:g_strBuffer
            format "</snoopstart>%" "~\n" to:g_strBuffer

            format "<commandstart>%" "~\n" to:g_strBuffer
            format "\"%import.bat\"%" (scriptsPath+"nwmax\\scratch\\") "~\n" to:g_strBuffer
            format "</commandstart>%" "~\n" to:g_strBuffer

            format "</snoopstart>%" "~\n" to:g_strBuffer
            format "</snoopend>%" "~" to:g_strBuffer
            if g_delim == "~\n" then format "\r\n" to:g_strBuffer
            nx_FlushBuffer force:true


            -- Start up model dialog to wait for this processing to complete
            createdialog rl_binmdl modal:true
         )
        -- check for ascii model exists
        pFile = pFile + ".ascii"
        if (nx_existFile pFile) == false then
        (
            MessageBox("Binary MDL File not decompiled. Check file is valid or NWNmdlcomp is present.")
            return false
        )
    )

    -- make sure we reset base panel status
    setCommandPanelTaskMode mode:#create

    -- Open up the file
    local mdlStream = openFile pFile mode:"r"

    --local numAnims = 0
    local animGap = 10
    local lastFrame = insertpoint - animGap
    local firstFrame = 0
    local lineNum = 0
    local mdl_pos = MDL_DEF
    local objProps = ""
    local quitEarly = false
    local in_aabb = false
    local in_emitter = false
    local bAnimIsThere = false
    local bHadAnimKey = false

    -- bounding anim key points for specified anim name on base.
    local lowerbound = 0
    local upperbound = 0


    -- Setup basis for model header
    local newModelClassification = 1
    local newModelAnimscale = 1.0
    local newModelSupermodel = "NULL"

    -- handle to this files model base
    local model_base = undefined

    -- Joco 24-Aug-03
    -- to fix having to have all the skinmeshes at the end of the mdl file
    -- we store the relevant data in this array using the structure SkinWeights.
    -- We then post process this data against the skin modifiers after the mdl
    -- file has been completely read, and therefore all the "bones" will have been
    -- processed.
    local weightsList = #()
    local newObj
    local tgaFilename
    local bmpFilename
    local matName
    local bmp
    local texMap
    local texAlphaMap

    local objFadingLight

    try (
        -- Go through the whole mdl file
        if showprogress then
        (
            progressStart "Importing MDL"
        )
        local fsize = nx_filesize mdlStream
        local fpos = 0.0
        while not (eof mdlStream) and not quitEarly do
        (
            if showprogress then
            (
                fpos = (filepos mdlStream) as float
                progressUpdate ((fpos/fsize) * 100)
            )
            local tok = nx_Tokenizer()
            local line = getNextNonBlankLine mdlStream
            tok.SetString (line)
            lineNum += 1

            --ImportActivityBar()

            -- Read the data ID tag
            local idToken = tok.ReadToken()

            -- if idToken is a string token the convert to lowercase
            if (isKindOf idToken string) then idToken = nx_lowercase idToken

            if (idToken[1] == "#") then idToken = "#"
            if (idToken == "donemodel") then
            (
                mdl_pos = MDL_NOT_IN_MDL
                quitEarly = true
            )

            -- typo clean ups
            case idToken of (
                "setfillumcolorkey": idToken = "selfillumcolorkey"
            )

            case mdl_pos of (
                ----------------------------------------------------------------
                -- Model definition
                ----------------------------------------------------------------
                MDL_DEF:
                (
                    if DEBUG then format "MDL_DEF:  Line:% :: %\r\n" lineNum idToken
                    case idToken of
                    (
                        "#": () -- ignore comments
                        "newmodel": newModelName = tok.ReadString()
                        "setsupermodel":
                        (
                            tok.ReadToken();
                            if tok.PeekToken() != undefined then
	                            newModelSupermodel = tok.ReadString()
                            else --1
	                            newModelSupermodel = "NULL"
                        )
                        "classification":
                        (
                            if DEBUG then format "classification processing START\r\n"
                            -- make the list all lower case to cope with differences in export scripts
                            --#("Character","Tile", "Door","Effect","WalkMesh","Other", "BodyPart", "Item", "GUI")
                            local class_types = #("character","tile", "door","effect","walkmesh","other","bodypart","item","gui")
                            local tempStr = nx_lowercase (tok.ReadString())
                            -- deal with case where effect comes through as effects
                            if tempStr == "effects" then tempStr = "effect"
                            newModelClassification = findItem class_types tempStr
                            if newModelClassification == 0 then newModelClassification = 1
                            if DEBUG then format "classification processing END\r\n"
                        )
                        "setanimationscale": newModelAnimscale = tok.ReadFloat()
                        "beginmodelgeom": mdl_pos = MDL_GEOM
                        -- trap anything not parsed
                        default: objProps += idToken + "=" + tok.ReadRemainingTokens() + "\r\n"
                    )
                ) -- End MDL_DEF case

                -- Model Geometry
                MDL_GEOM:
                (
                    if DEBUG then format "MDL_GEOM:  Line:% :: % :: %\r\n" lineNum idToken tok.tokens
                    if (idToken == "aabb") then
                    (
                        in_aabb = true
                        idToken = tok.ReadToken()

                    )
			-- bead-v modification: since this check only affects in_aabb, it should also check that it's set to true
                    if (not (isKindOf idToken float) and
                        not (isKindOf idToken integer) and in_aabb) then
		    (
                        -- bead-v modification: make it possible to start aabb data in a new line
                        tok.SetString (getNextNonBlankLine mdlStream)
                        lineNum += 1
                        idToken = tok.ReadToken()
                   	if (not (isKindOf idToken float) and
                            not (isKindOf idToken integer)) then
			(
				in_aabb = false
		    	)
                        -- bead-v end modification
		    )
                    if (in_aabb) then
                    (
                        local x = idToken as float
                        local y = tok.ReadFloat()
                        local z = tok.ReadFloat()
                        local bl = point3 x y z
                        local tr = tok.ReadPoint3()
                        local leaf = (tok.ReadInteger() != -1)
                        bl *= 100
                        tr *= 100
                        local box_pos = (bl+tr)/2
                        box_pos.z = bl.z
                        box_pos += objPos
                        if (leaf) then
                        (
                            -- local b = box pos:box_pos length:(tr.y-bl.y) width:(tr.x-bl.x) height:(tr.z-bl.z)
                        )
                    )
                    case idToken of
                    (
                        "#": () -- ignore comments
                        "node": -- Node Struct
                        (
                            -- Define initial node data
                            objType = tok.ReadToken()
                            objName = tok.ReadToken()
                            objWireColor = color 255 255 255
                            objAmbient = color 255 255 255
                            objDiffuse = color 0 0 255
                            objSpecular = color 0 0 0
                            objSelfIllum = color 0 0 0
                            objAlpha = 1
                            objShine = 0

                            objDanglymesh = 0
                            objPeriod = 0.0
                            objTightness = 0.0
                            objDisplacement = 0.0

                            objPos = [0, 0, 0]
                            objRot = quat 0 0 0 0
                            objScale = [1, 1, 1]
                            objScale_float = 1.0
                            objMaterial = undefined
                            textureName = ""


                            objTilefadeprop = 1
                            objRender = 1
                            objShadow = 1   -- it appears the default behaviour in the engine is for shadow unless explicitly turned off
                            objBeaming = undefined
                            objFadingLight = 0
                            objInheritcolor = 0
                            objRotatetexture = 0
                            --shininess       type:#integer
                            objTransparencyhint = 0


                            numVerts = 0
                            verts = #()
                            numFaces = 0
                            faces = #()
                            numTVerts = 0
                            TVerts = #()
                            TVfaces = #()
                            smoothGroups = #()
                            surfacemats = #()
                            numConstraints = 0
                            danglyConstraints = #()
                            numWeights = 0
                            weightBuffer = #()

                            -- lens flare var setup
                            a_texturenames = #() -- read multiple strings
                            a_flaresizes = #() -- read multiple floats
                            a_flarepositions = #() -- read multiple floats
                            a_flarecolorshifts = #() -- read multiple colors as point3

                            -- reset any emitter stuff
                            --objRender=undefined
                            --objRender_sel=undefined
                            --objSpawnType=undefined
                            --objIconSize=undefined
                            --objXsize=undefined
                            --objYsize=undefined
                            --objInherit=undefined
                            --objInherit_local=undefined
                            --objInheritvel=undefined
                            --objInherit_part=undefined
                            --objRenderorder=undefined
                            --objThreshold=undefined
                            --objCombinetime=undefined
                            --objDeadspace=undefined
                            --objColorStart=undefined
                            --objColorEnd=undefined
                            --objAlphaStart=undefined
                            --objAlphaEnd=undefined
                            --objSizeStart=undefined
                            --objSizeEnd=undefined
                            --objSizeStart_y=undefined
                            --objSizeEnd_y=undefined
                            --objBirthrate=undefined
                            --objLifeExp=undefined
                            --objMass=undefined
                            --objSpread=undefined
                            --objParticleRot=undefined
                            --objVelocity=undefined
                            --objRandvel=undefined
                            --objBounce_co=undefined
                            --objBlurlength=undefined
                            --objLoop=undefined
                            --objBounce=undefined
                            --objM_isTinted=undefined
                            --objSplat=undefined
                            --objAffectedbyWind=undefined
                            objTexture=undefined

                            --objTwosidedtext=undefined
                            --objXgrid=undefined
                            --objYgrid=undefined
                            --objFps=undefined
                            --objFrameStart=undefined
                            --objFrameEnd=undefined
                            --objRandom=undefined
                            objChunk=undefined
                            --objLightningDelay=undefined
                            --objLightningRadius=undefined
                            --objLightningSubDiv=undefined
                            --objLightningScale=undefined
                            --objBlastRadius=undefined
                            --objBlastLength=undefined
                            --objOpacity=undefined
                            --objP2p=undefined
                            -- needs special processing
                            --objP2p_type=undefined
                            --objP2p_sel=undefined
                            --objP2p_bezier2=undefined
                            --objP2p_bezier3=undefined
                            --objGrav=undefined
                            --objDrag=undefined
                        )

                        "parent": parent = tok.ReadToken()
                        "position":
                        (
                            objPos = tok.ReadPoint3()
                            objPos *= 100
                        )
                        "orientation":
                        (
                            local axis = tok.ReadPoint3()
                            local angle = RadToDeg (tok.ReadFloat())
                            objRot = quat angle axis
                        )
                        "scale":
                        (
                            objScale_float = tok.ReadFloat()
                            objScale = [1,1,1] --* objScale_float
                        )
                        "render":
                        (
                            if (nx_lowercase objType) == "emitter" then
                            (
                                objRender = tok.ReadString()
                                -- set render_sel
                                case nx_lowercase(objRender) of
                                (
                                    "normal": objRender_sel=1
                                    "linked": objRender_sel=2
                                    "billboard_to_local_z": objRender_sel=3
                                    "billboard_to_world_z": objRender_sel=4
                                    "aligned_to_world_z": objRender_sel=5
                                    "aligned_to_particle_dir": objRender_sel=6
                                    "motion_blur": objRender_sel=7
                                    default: objRender_sel=1
                                )
                            )
                            else -- is a normal mesh
                                objRender = tok.ReadInteger()
                        )
                        "wirecolor": objWireColor = tok.ReadColor()
                        "IconSize": objIconSize = tok.ReadFloat()
                        "ambient": objAmbient = tok.ReadColor()
                        "diffuse": objDiffuse = tok.ReadColor()
                        "specular": objSpecular = tok.ReadColor()

                        "shininess": objShine = tok.ReadInteger()

                        "transparencyhint": objTransparencyhint = tok.ReadInteger()

                        "tilefade":
                        (
                            objTilefadeprop = tok.ReadInteger()
                            case objTilefadeprop of
                            (
                                0: objTilefadeprop = 1
                                1: objTilefadeprop = 2
                                2: objTilefadeprop = 4
                                4: objTilefadeprop = 3
                                default: objTilefadeprop = 1
                            )
                        )

                        "selfillumcolor": objSelfIllum = tok.ReadColor()
                        "setfillumcolor": objSelfIllum = tok.ReadColor()

                        "danglymesh": objDanglymesh = tok.ReadInteger()
                        "period": objPeriod = tok.ReadFloat()
                        "tightness": objTightness = tok.ReadFloat()
                        "displacement": objDisplacement = tok.ReadFloat()

                        "alpha": objAlpha = tok.ReadFloat()
                -- bead-v modification: added kotor props
                        "animateuv": objAnimateuv = tok.ReadInteger()
                        "uvdirectionx": objUvdirectionx = tok.ReadFloat()
                        "uvdirectiony": objUvdirectiony = tok.ReadFloat()
                        "uvjitter": objUvjitter = tok.ReadFloat()
                        "uvjitterspeed": objUvjitterspeed = tok.ReadFloat()
                        "lightmapped": objLightmapped = tok.ReadInteger()
                        "m_blsbackgroundgeometry": objBackgroundgeometry = tok.ReadInteger()
                        "dirt_enabled": objDirt_enabled = tok.ReadInteger()
                        "dirt_texture": objDirt_texture = tok.ReadInteger()
                        "dirt_worldspace": objDirt_worldspace = tok.ReadInteger()
                        "hologram_donotdraw": objHologram_donotdraw = tok.ReadInteger()
                        "tangentspace": objTangentspace = tok.ReadInteger()
                -- end bead-v modification

                        "bitmap":
                        (
                            textureName = tok.ReadToken()
                            if (textureName == "NULL") then textureName = ""
                        )
                        "rotatetexture": objRotatetexture = tok.ReadInteger()
                        "verts": -- Verticies Struct
                        (
                            numVerts = tok.ReadInteger()
                            for i = 1 to numVerts do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1
                                local p3 = tok.ReadPoint3()
                                p3 *= 100
                                append verts p3
                            )
                        )
                        "constraints": -- Danlgy constraints
                        (
                            numConstraints = tok.ReadInteger()
                            for i = 1 to numConstraints do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1
                                local p1 = tok.ReadFloat()
                                append danglyConstraints p1
                            )
                        )
                        "weights": -- Weights for skin modifier
                        (
                            numWeights = tok.ReadInteger()
                            for i = 1 to numWeights do
                            (
                                append weightBuffer (getNextNonBlankLine mdlStream)
                            )
                            format "Loaded % weights\r\n" numWeights
                        )
                        "faces": -- Faces Struct
                        (
                            local p1
                            local p3
                            numFaces = tok.ReadInteger()
                            for i = 1 to numFaces do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                                -- Get the current Face
                                -- mdl file is zero-based index, max expects
                                -- one-based index into vert array
                                p3 = (tok.ReadPoint3() + [1,1,1])
                                append faces p3

                                -- Get Smoothing Group
                                p1 = tok.ReadInteger()
                                append smoothGroups p1

                                -- Get the current TVface
                                p3 = (tok.ReadPoint3() + [1,1,1])
                                append TVfaces p3

                                -- Get Surface Material
                                p1 = tok.ReadInteger()
                                append surfacemats p1
                            )
                        )

                        "tverts": -- Verticies Struct
                        (
                            numTVerts = tok.ReadInteger()
                            for i = 1 to numTVerts do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                    -- bead-v modification: only read two floats for the UVs
                                local bv1 = tok.ReadFloat()
                                local bv2 = tok.ReadFloat()
                                local p3 = [bv1, bv2, 0]

                            -- Old code:
                                --local p3 = tok.ReadPoint3()
                    -- bead-v end modification

                                append TVerts p3
                            )
                        )

                        --
                        -- Emitter key words
                        --
                        "update": objUpdate = tok.ReadString()
                        "update_sel": objUpdate_sel = tok.ReadInteger()
                        "blend": objBlend = tok.ReadString()
                        "blend_sel": objBlend_sel = tok.ReadInteger()
                        -- render is dealt with above.
                        "render_sel": objRender_sel = tok.ReadInteger()
                        "spawntype": objSpawnType = tok.ReadInteger()
                        "iconSize": objIconSize = tok.ReadFloat()
                        "xsize": objXsize = tok.ReadFloat()
                        "ysize": objYsize = tok.ReadFloat()
                        "inherit": objInherit = tok.ReadInteger()
                        "inherit_local": objInherit_local = tok.ReadInteger()
                        "inheritvel": objInheritvel = tok.ReadInteger()
                        "inherit_part": objInherit_part = tok.ReadInteger()
                        "renderorder": objRenderorder = tok.ReadInteger()
                        "threshold": objThreshold = tok.ReadFloat()
                        "combinetime": objCombinetime = tok.ReadFloat()
                        "deadspace": objDeadspace = tok.ReadFloat()
                        "colorstart": objColorStart = tok.ReadColor()
                        "colorend": objColorEnd = tok.ReadColor()
                        "alphastart": objAlphaStart = tok.ReadFloat()
                        "alphaend": objAlphaEnd = tok.ReadFloat()
                        "sizestart": objSizeStart = tok.ReadFloat()
                        "sizeend": objSizeEnd = tok.ReadFloat()
                        "sizestart_y": objSizeStart_y = tok.ReadFloat()
                        "sizeend_y": objSizeEnd_y = tok.ReadFloat()
                        "birthrate": objBirthrate = tok.ReadFloat()
                        "lifeexp": objLifeExp = tok.ReadFloat()
                        "mass": objMass = tok.ReadFloat()
                        "spread": objSpread = RadToDeg (tok.ReadFloat())
                        "particlerot": objParticleRot = tok.ReadFloat()
                        "velocity": objVelocity = tok.ReadFloat()
                        "randvel": objRandvel = tok.ReadFloat()
                        "bounce_co": objBounce_co = tok.ReadFloat()
                        "blurlength": objBlurlength = tok.ReadFloat()
                        "loop": objLoop = tok.ReadInteger()
                        "bounce": objBounce = tok.ReadInteger()
                        "m_istinted": objM_isTinted = tok.ReadInteger()
                        "splat": objSplat = tok.ReadInteger()
                        "affectedbywind": objAffectedbyWind = tok.ReadBoolean()
                        "texture": objTexture = tok.ReadString()
                        "twosidedtext": objTwosidedtext = tok.ReadInteger()
                        "twosidedtex": objTwosidedtext = tok.ReadInteger()
                        "xgrid": objXgrid = tok.ReadInteger()
                        "ygrid": objYgrid = tok.ReadInteger()
                        "fps": objFps = tok.ReadInteger()
                        "framestart": objFrameStart = tok.ReadInteger()
                        "frameend": objFrameEnd = tok.ReadInteger()
                        "random": objRandom = tok.ReadInteger()
                        "chunkname": objChunk = tok.ReadString()
                        "lightningdelay": objLightningDelay = tok.ReadFloat()
                        "lightningradius": objLightningRadius= tok.ReadFloat()
                        "lightningsubdiv": objLightningSubDiv= tok.ReadFloat()
                        "lightningscale": objLightningScale= tok.ReadFloat()
                        "blastradius": objBlastRadius= tok.ReadFloat()
                        "blastlength": objBlastLength= tok.ReadFloat()
                        "opacity": objOpacity = tok.ReadFloat()
                        "p2p": objP2p = tok.ReadInteger()
                        "p2p_type": objP2p_type = tok.ReadString()
                        "p2p_sel": objP2p_sel = tok.ReadInteger()
                        "p2p_bezier2": objP2p_bezier2 = tok.ReadFloat()
                        "p2p_bezier3": objP2p_bezier3 = tok.ReadFloat()
                        "grav": objGrav = tok.ReadFloat()
                        "drag": objDrag = tok.ReadFloat()
             -- bead-v modification: add kotor props
                        "lightningzigzag": objLightningzigzag = tok.ReadFloat()
                        "colorMid": objColorMid = tok.ReadFloat()
                        "alphaMid": objAlphaMid = tok.ReadFloat()
                        "sizeMid": objSizeMid = tok.ReadFloat()
                        "sizeMid_y": objSizeMid_y = tok.ReadFloat()
                        "percentStart": objPercentStart = tok.ReadFloat()
                        "percentMid": objPercentMid = tok.ReadFloat()
                        "percentEnd": objPercentEnd = tok.ReadFloat()
                        "m_frandombirthrate": objRandomBirthRate = tok.ReadFloat()
                        "targetsize": objTargetsize = tok.ReadFloat()
                        "numcontrolpts": objNumcontrolpts = tok.ReadFloat()
                        "controlptradius": objControlptradius = tok.ReadFloat()
                        "controlptdelay": objControlptdelay = tok.ReadFloat()
                        "tangentspread": objTangentspread = tok.ReadFloat()
                        "tangentlength": objTangentlength = tok.ReadFloat()
                        "detonate": objDetonate = tok.ReadFloat()
                        "numbranches": objNumBranches = tok.ReadInteger()
                        "controlptsmoothing": objControlptsmoothing = tok.ReadFloat()
                        "m_bframeblending": objFrameBlending = tok.ReadInteger()
                        "depth_texture": objDepth_texture = tok.ReadInteger()
                        "m_sdepthtexturename": objDepthTextureName = tok.ReadString()
             -- end bead-v modification
                        -- Emitter key words end

                        -- Light key words
                        "radius": objRadius = tok.ReadFloat()
                        "multiplier": objMultiplier = tok.ReadFloat()
             -- bead-v modification: add kotor props
                        "shadowradius": objShadowRadius = tok.ReadFloat()
                        "verticaldisplacement": objVerticalDisplacement = tok.ReadFloat()
             -- end bead-v modification

                        "lightpriority": objLightpriority = tok.ReadInteger()
                        "ndynamictype": objNdynamictype = tok.ReadInteger()
                        "color": objLight_color = tok.ReadPoint3()      -- was "light_color"
                        "ambientonly": objAmbientOnly = tok.ReadInteger()
                        "affectdynamic": objAffectDynamic = tok.ReadInteger()
                        "shadow": objShadow = tok.ReadInteger() -- is also a trimesh keyword
                        "beaming": objBeaming = tok.ReadInteger()
                        "fadinglight": objFadingLight = tok.ReadInteger()
                        -- Light key words end

                        -- Lens Flare key words
                        "lensflares": objLensflares = tok.ReadInteger()
		    -- bead-v modification: adding the "flare" keyword
                        "flare": objLensflares = tok.ReadInteger()
		    -- end bead-v modification
                        "flareradius": objFlareradius = tok.ReadInteger()
                        "texturenames": -- read multiple strings
                        (
                            numEntries = tok.ReadInteger()
                            for i = 1 to numEntries do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                                -- Get Lens texture
                                append a_texturenames (tok.ReadString())
                            )
                        )
                        "flaresizes": -- read multiple ints
                        (
                            numEntries = tok.ReadInteger()
                            for i = 1 to numEntries do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                                -- Get Lens texture
                                append a_flaresizes (tok.ReadFloat())
                            )
                        )
                        "flarepositions": -- read multiple floats
                        (
                            numEntries = tok.ReadInteger()
                            for i = 1 to numEntries do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                                -- Get Lens texture
                                append a_flarepositions (tok.ReadFloat())
                            )
                        )
                        "flarecolorshifts": -- read multiple colors as point3
                        (
                            numEntries = tok.ReadInteger()
                            for i = 1 to numEntries do
                            (
                                tok.SetString (getNextNonBlankLine mdlStream)
                                lineNum += 1

                                -- Get Lens texture
                                append a_flarecolorshifts (tok.ReadPoint3())
                            )
                        )
                        -- Lens Flare keywords end

                        -- Reference key words
                        "refmodel": objRefModel = tok.ReadString()
                        "reattachable": objReattachable = tok.ReadInteger()
                        -- Reference key words end

                        "endnode": -- Done parsing mesh, so create the object
                        (
                            if loadGeom then
                            (
                                if DEBUG then format "%BUILD MESH OBJECT: % %\r\n" strIndent1 objType objName
                                in_aabb = false
                                newObj = undefined
                                case of
                                (
                                    ((objType == "trimesh") or
                                     (objType == "animmesh") or
                                     (objType == "danglymesh") or
                                     (objType == "skin") or
                                     (objType == "aabb")):
                                    (
                                        -- Make material, if necessary
                                        matName = objName
                                        if ((textureName != "") and
                                            (textureName != "NULL") and
                                            (textureName != undefined)) then matName = textureName
                                        objMaterial = sceneMaterials[matName]
                                        if (objType == "aabb") then
                                        (
                                            if DEBUG then format "%BUILD MESH OBJECT: make aabb material\r\n" strIndent1
                                            objMaterial = nx_createNWNWalkMaterial()
                                            objProps = ""
                                        )
                                        if (objMaterial == undefined) and (textureName != "") then
                                        (
                                            if DEBUG then format "%BUILD MESH OBJECT: make material and if possible find tga file\r\n" strIndent1
                                            objMaterial = standardmaterial name:matName
                                            objMaterial.shaderType = 1 -- Blinn
                                            objMaterial.ambient = objAmbient
                                            objMaterial.diffuse = objDiffuse
                                            objMaterial.specular = objSpecular
                                            objMaterial.glossiness = objShine
                                            objMaterial.selfillumcolor = objSelfIllum
                                            -- alpha done on trimesh modifier only now
                                            --objMaterial.opacity = objAlpha
                                            if (textureName != "") then
                                            (
                                                -- We want to see where the texture file is.
                                                -- Look in local dir first then check in g_maps_path
                                                if DEBUG then format "Look in local dir for texture\r\n"
                                                tgaFilename = currentPath + textureName + ".tga"
                                                bmpFilename = currentPath + textureName + ".bmp"
                                                if not ((nx_existFile tgaFilename) or (nx_existFile bmpFilename)) then
                                                (
                                                    -- search through the global bitmap paths taken from gmax
                                                    if DEBUG then format "Searching gmax global paths: %\r\n" g_maps_path
                                                    for i = 1 to (g_maps_path.count) do
                                                    (
                                                        tgaFilename = g_maps_path[i] + "\\" + textureName + ".tga"
                                                        bmpFilename = g_maps_path[i] + "\\" + textureName + ".bmp"
                                                        if ((nx_existFile tgaFilename) or (nx_existFile bmpFilename)) then exit
                                                    )
                                                )
                                                if ((nx_existFile tgaFilename) or (nx_existFile bmpFilename)) then
                                                (
                                                    if (nx_existFile tgaFilename) then
                                                        bmp = openBitMap(tgaFilename)
                                                    else --2
                                                        bmp = openBitMap(bmpFilename)

                                                    if loadtiff then
                                                    (
                                                        texMap = BitmapTexture bitmap:(nx_tiff_from_tex bmp) name:textureName
                                                    ) else
                                                    (
                                                        texMap = BitmapTexture bitmap:bmp name:textureName
                                                    )
                                                    objMaterial.mapAmounts[2] = 100
                                                    objMaterial.maps[2] = texMap
                                                    objMaterial.mapEnables[2] = true;
                                                    showTextureMap objMaterial texMap true
                                                )
                                                else --3
                                                (
                                                    format "#Missing texture file: %\r\n" textureName
                                                    if (showWarnings) then messagebox ("Missing texture file: " + textureName)
                                                )
                                            )
                                        )

                                        -- Make an editable mesh
                                        if (textureName != "") then
                                        (
                                            if DEBUG then format "%BUILD MESH OBJECT: make the actual mesh with material\r\n" strIndent1
                                            newObj = mesh name:objName \
                                                            vertices:verts \
                                                            faces:faces \
                                                            wirecolor:objWireColor \
                                                            material:objMaterial \
                                                            materialIDS:surfacemats
                                        ) else (
                                            if DEBUG then format "%BUILD MESH OBJECT: make the actual mesh with NO material\r\n" strIndent1
                                            newObj = mesh name:objName \
                                                            vertices:verts \
                                                            faces:faces \
                                                            wirecolor:objWireColor \
                                                            materialIDS:surfacemats
                                        )

                                        -- special post processing to deal with some sort of odd Max vert position distortion
                                        for verti = 1 to verts.count do
                                        (
                                            setVert newObj verti verts[verti]
                                        )
                                        update newObj

                                        if DEBUG then format "%BUILD MESH OBJECT: Created type: %\r\n" strIndent1 (classof newObj)

                                        if DEBUG then format "%BUILD MESH OBJECT: set up smoothing groups\r\n" strIndent1
                                        -- Assign smoothing groups
                                        for i = 1 to smoothGroups.count do
                                        (
                                            setFaceSmoothGroup newObj i smoothGroups[i]
                                        )

                                        if DEBUG then format "%BUILD MESH OBJECT: add texture\r\n" strIndent1
                                        -- Add texture, if we have them
                                        if numTVerts > 0 do
                                        (
                                            setNumTVerts newObj numTVerts
                                            for i = 1 to numTVerts do setTVert newObj i TVerts[i]
                                            buildTVFaces newObj
                                            for i = 1 to numFaces do setTVFace newObj i TVfaces[i]
                                        )

                                        if DEBUG then format "%BUILD MESH OBJECT: apply constraints if have them\r\n" strIndent1
                                        -- we have a danglymesh so apply constraints
                                        if numConstraints > 0 then
                                        (
                                            for i = 1 to numConstraints do
                                            (
                                                local c = (color danglyConstraints[i] danglyConstraints[i] danglyConstraints[i])
                                                if DEBUG then format "%apply constraint: % %\r\n" strIndent2 i c
                                                meshop.SetVertColor newObj 0 i c
                                            )
                                        )

                                        --
                                        -- We need to process for the appropriate
                                        -- modifiers to be applied to the mesh.
                                        --
                                        if DEBUG then format "%BUILD MESH OBJECT: process for modifiers\r\n" strIndent1
                                        case objType of
                                        (
                                            "danglymesh":
                                            (
                                                if DEBUG then format "%--> dangly mesh\r\n" strIndent1
                                                -- create and apply the flex modifier
                                                aurora_mod = auroraflex()
                                                aurora_mod.period = objPeriod
                                                aurora_mod.tightness = objTightness
                                                aurora_mod.displacement = objDisplacement
                                                addModifier newObj aurora_mod

                                                -- assume is always a danglymesh
                                                aurora_mod = auroratrimesh()
                                                aurora_mod.tilefadeprop = objTilefadeprop
                                                aurora_mod.scale = objScale_float
                                                aurora_mod.render = objRender
                                                aurora_mod.shadow = objShadow
                                                if objBeaming!=undefined then aurora_mod.beaming = objBeaming
                                                aurora_mod.inheritcolor = objInheritcolor
                                                aurora_mod.selfillumcolor = objSelfillum
                                                aurora_mod.rotatetexture = objRotatetexture
                                                aurora_mod.alpha = objAlpha
                                                aurora_mod.transparencyhint = objTransparencyhint
                                                aurora_mod.ambient = objAmbient
                                                aurora_mod.diffuse = objDiffuse
                                                aurora_mod.specular = objSpecular
                                                aurora_mod.shininess = objShine
                                        -- bead-v modification: added kotor props
                                                aurora_mod.animateuv = objAnimateuv
                                                aurora_mod.uvdirectionx = objUvdirectionx
                                                aurora_mod.uvdirectiony = objUvdirectiony
                                                aurora_mod.uvjitter = objUvjitter
                                                aurora_mod.uvjitterspeed = objUvjitterspeed
                                                aurora_mod.lightmapped = objLightmapped
                                                aurora_mod.m_blsBackgroundGeometry = objBackgroundgeometry
                                                aurora_mod.dirt_enabled = objDirt_enabled
                                                aurora_mod.dirt_texture = objDirt_texture
                                                aurora_mod.dirt_worldspace = objDirt_worldspace
                                                aurora_mod.hologram_donotdraw = objHologram_donotdraw
                                                aurora_mod.tangentspace = objTangentspace
                                        -- end bead-v modification
                                                -- we now force extra data to always be true
                                                --if newObj.material == undefined then
                                                --(
                                                    aurora_mod.extra_mat_data = 1
                                                --)
                                                addModifier newObj aurora_mod
                                            )
                                            "trimesh":
                                            (
                                                if DEBUG then format "%--> trimesh\r\n" strIndent1
                                                -- not sure if should do this
                                                -- for all trimesh nodes or not.
                                                aurora_mod = auroratrimesh()
                                                aurora_mod.tilefadeprop = objTilefadeprop
                                                aurora_mod.scale = objScale_float
                                                aurora_mod.render = objRender
                                                aurora_mod.shadow = objShadow
                                                if objBeaming!=undefined then aurora_mod.beaming = objBeaming
                                                aurora_mod.inheritcolor = objInheritcolor
                                                aurora_mod.selfillumcolor = objSelfillum
                                                aurora_mod.rotatetexture = objRotatetexture
                                                aurora_mod.alpha = objAlpha
                                                aurora_mod.transparencyhint = objTransparencyhint
                                                aurora_mod.ambient = objAmbient
                                                aurora_mod.diffuse = objDiffuse
                                                aurora_mod.specular = objSpecular
                                                aurora_mod.shininess = objShine
                                        -- bead-v modification: added kotor props
                                                aurora_mod.animateuv = objAnimateuv
                                                aurora_mod.uvdirectionx = objUvdirectionx
                                                aurora_mod.uvdirectiony = objUvdirectiony
                                                aurora_mod.uvjitter = objUvjitter
                                                aurora_mod.uvjitterspeed = objUvjitterspeed
                                                aurora_mod.lightmapped = objLightmapped
                                                aurora_mod.m_blsBackgroundGeometry = objBackgroundgeometry
                                                aurora_mod.dirt_enabled = objDirt_enabled
                                                aurora_mod.dirt_texture = objDirt_texture
                                                aurora_mod.dirt_worldspace = objDirt_worldspace
                                                aurora_mod.hologram_donotdraw = objHologram_donotdraw
                                                aurora_mod.tangentspace = objTangentspace
                                        -- end bead-v modification
                                                -- we now force extra data to always be true
                                                --if newObj.material == undefined then
                                                --(
                                                    aurora_mod.extra_mat_data = 1
                                                --)
                                                addModifier newObj aurora_mod
                                            )

                                            "aabb":
                                            (
                                                if DEBUG then format "%--> aabb\r\n" strIndent1
                                                -- As the only aabb we have in an mdl file is a wok
                                                -- mesh there is only one possible meshtype.
                                                -- Remember that pwk/dwk are imported seperately from
                                                -- their own files.
                                                aurora_mod = aurorawalkmesh()
                                                aurora_mod.meshtype = 1
                                                addModifier newObj aurora_mod
                                            )
                                        )

                                        -- Update the mesh
                                        update newObj


                                    ) --End trimesh, danglymesh, skin, aabb CASE

                                    -- Joco 23-Aug-03: Create emitter and make sure
                                    -- only previously defined attributes are applied
                                    (objType == "emitter"):
                                    (
                                        if DEBUG then format "%BUILD MESH OBJECT: Is an emitter!\r\n" strIndent1
                                        newObj = auroraemitter name:objName
                                        objRot += quat 180 (point3 1 0 0) -- NWN Emitters starts in +z, GMax cone manipulator starts in -z
                                        newObj.delegate.Angle = 15;      -- make the cone narrow
                                        if objIconSize == undefined then objIconSize = 100
                                        newObj.delegate.Distance = objIconSize;  -- Should look to use Icon Size for this value, if available.
                                        newObj.wirecolor = objWireColor
                                        -- Aurora emitters are actually square to make the "cone" a square one.
                                        newObj.delegate.UseSquare = true
                                        -- assign data values
                                        if objUpdate!=undefined then (
                                            newObj.update = objUpdate
                                            case nx_lowercase(objUpdate) of
                                            (
                                                "fountain":     objUpdate_sel=1
                                                "single":       objUpdate_sel=2
                                                "explosion":    objUpdate_sel=3
                                                "lightning":    objUpdate_sel=4
                                            )
                                        )
                                        if objUpdate_sel!=undefined then newObj.update_sel = objUpdate_sel
                                        if objBlend!=undefined then (
                                            newObj.blend = objBlend
                                            case nx_lowercase(objBlend) of
                                            (
                                                "normal": objBlend_sel = 1
                                                "punch-through":  objBlend_sel = 2
                                                "lighten": objBlend_sel = 3
                                            )
                                        )
                                        if objBlend_sel!=undefined then (
                                            --if objBlend_sel == 3 then objBlend_sel = 2
                                            newObj.blend_sel = objBlend_sel
                                        )
                                        if objRender!=undefined then newObj.render = objRender
                                        if objRender_sel!=undefined then newObj.render_sel = objRender_sel
                                        if objSpawnType!=undefined then newObj.spawnType = objSpawnType
                                        if objIconSize!=undefined then newObj.IconSize = objIconSize
                                        if objXsize!=undefined then newObj.xsize = objXsize
                                        if objYsize!=undefined then newObj.ysize = objYsize
                                        if objInherit!=undefined then newObj.inherit = objInherit
                                        if objInherit_local!=undefined then newObj.inherit_local = objInherit_local
                                        if objInheritvel!=undefined then newObj.inheritvel = objInheritvel
                                        if objInherit_part!=undefined then newObj.inherit_part = objInherit_part
                                        if objRenderorder!=undefined then newObj.renderorder = objRenderorder
                                        if objThreshold!=undefined then newObj.threshold = objThreshold
                                        if objCombinetime!=undefined then newObj.combinetime = objCombinetime
                                        if objDeadspace!=undefined then newObj.deadspace = objDeadspace
                                        if objColorStart!=undefined then newObj.colorStart = objColorStart
                                        if objColorEnd!=undefined then newObj.colorEnd = objColorEnd
                                        if objAlphaStart!=undefined then newObj.alphaStart = objAlphaStart
                                        if objAlphaEnd!=undefined then newObj.alphaEnd = objAlphaEnd
                                        if objSizeStart!=undefined then newObj.sizeStart = objSizeStart
                                        if objSizeEnd!=undefined then newObj.sizeEnd = objSizeEnd
                                        if objSizeStart_y!=undefined then newObj.sizeStart_y = objSizeStart_y
                                        if objSizeEnd_y!=undefined then newObj.sizeEnd_y = objSizeEnd_y
                                        if objBirthrate!=undefined then newObj.birthrate = objBirthrate
                                        if objLifeExp!=undefined then newObj.lifeExp = objLifeExp
                                        if objMass!=undefined then newObj.mass = objMass
                                        if objSpread!=undefined then newObj.spread = objSpread
                                        if objParticleRot!=undefined then newObj.particleRot = objParticleRot
                                        if objVelocity!=undefined then newObj.velocity = objVelocity
                                        if objRandvel!=undefined then newObj.randvel = objRandvel
                                        if objBounce_co!=undefined then newObj.bounce_co = objBounce_co
                                        if objBlurlength!=undefined then newObj.blurlength = objBlurlength
                                        if objLoop!=undefined then newObj.loop = objLoop
                                        if objBounce!=undefined then newObj.bounce = objBounce
                                        if objM_isTinted!=undefined then newObj.m_isTinted = objM_isTinted
                                        if objSplat!=undefined then newObj.Splat = objSplat
                                        if objAffectedbyWind!=undefined then newObj.affectedbyWind = objAffectedbyWind
                                        if objTexture!=undefined then newObj.texture = objTexture
                                        -- if objTwosidedtext!=undefined then newObj.twosidedtext = objTwosidedtext
                                        if objTwosidedtext!=undefined then newObj.twosidedtex = objTwosidedtext
                                        if objXgrid!=undefined then newObj.xgrid = objXgrid
                                        if objYgrid!=undefined then newObj.ygrid = objYgrid
                                        if objFps!=undefined then newObj.fps = objFps
                                        if objFrameStart!=undefined then newObj.frameStart = objFrameStart
                                        if objFrameEnd!=undefined then newObj.frameEnd = objFrameEnd
                                        if objRandom!=undefined then newObj.random = objRandom
                                        if objChunk!=undefined then newObj.chunkName = objChunk
                                        if objChunk!=undefined then newObj.chunky = 1
                                        if objLightningDelay!=undefined then newObj.lightningDelay = objLightningDelay
                                        if objLightningRadius!=undefined then newObj.lightningRadius = objLightningRadius
                                        if objLightningSubDiv!=undefined then newObj.lightningSubDiv = objLightningSubDiv
                                        if objLightningScale!=undefined then newObj.lightningScale = objLightningScale
                             -- bead-v modification: add kotor props
                                        if objLightningzigzag!=undefined then newObj.lightningzigzag = objLightningzigzag
                                        if objColorMid!=undefined then newObj.colorMid = objColorMid
                                        if objAlphaMid!=undefined then newObj.alphaMid = objAlphaMid
                                        if objSizeMid!=undefined then newObj.sizeMid = objSizeMid
                                        if objSizeMid_y!=undefined then newObj.sizeMid_y = objSizeMid_y
                                        if objPercentStart!=undefined then newObj.percentStart = objPercentStart
                                        if objPercentMid!=undefined then newObj.percentMid = objPercentMid
                                        if objPercentEnd!=undefined then newObj.percentEnd = objPercentEnd
                                        if objRandomBirthRate!=undefined then newObj.m_fRandomBirthRate = objRandomBirthRate
                                        if objTargetsize!=undefined then newObj.targetsize = objTargetsize
                                        if objNumcontrolpts!=undefined then newObj.numcontrolpts = objNumcontrolpts
                                        if objControlptradius!=undefined then newObj.controlptradius = objControlptradius
                                        if objControlptdelay!=undefined then newObj.controlptdelay = objControlptdelay
                                        if objTangentspread!=undefined then newObj.tangentspread = objTangentspread
                                        if objTangentlength!=undefined then newObj.tangentlength = objTangentlength
                                        if objDetonate!=undefined then newObj.detonate = objDetonate
                                        if objNumBranches!=undefined then newObj.numBranches = objNumBranches
                                        if objControlptsmoothing!=undefined then newObj.controlptsmoothing = objControlptsmoothing
                                        if objFrameBlending!=undefined then newObj.m_bFrameBlending = objFrameBlending
                                        if objDepth_texture!=undefined then newObj.depth_texture = objDepth_texture
                                        if objDepthTextureName!=undefined then newObj.m_sDepthTextureName = objDepthTextureName
                             -- end bead-v modification
                                        if objBlastRadius!=undefined then newObj.blastRadius = objBlastRadius
                                        if objBlastLength!=undefined then newObj.blastLength = objBlastLength
                                        if objOpacity!=undefined then newObj.opacity = objOpacity
                                        if objP2p!=undefined then newObj.p2p = objP2p
                                        -- needs special processing
                                        if objP2p_type!=undefined then
                                        (
                                            newObj.p2p_type = objP2p_type
                                            case nx_lowercase(objP2p_type) of
                                            (
                                                "bezier": objP2p_sel = 1
                                                "gravity": objP2p_sel = 2
                                            )
                                        )
                                        if objP2p_sel!=undefined then newObj.p2p_sel = objP2p_sel
                                        if objP2p_bezier2!=undefined then newObj.p2p_bezier2 = objP2p_bezier2
                                        if objP2p_bezier3!=undefined then newObj.p2p_bezier3 = objP2p_bezier3
                                        if objGrav!=undefined then newObj.grav = objGrav
                                        if objDrag!=undefined then newObj.drag = objDrag
                                    )

                                    -- Joco 23-Aug-03: Create light and make sure
                                    -- only previously defined attributes are applied.
                                    (objType == "light"):
                                    (
                                        newObj = auroraDlight radius:1000 name:objName
                                        newObj.delegate.boxsize = [10,10,10]
                                        newObj.wirecolor = objWireColor
                                        if objRadius!=undefined then newObj.radius = objRadius * 100
                            -- bead-v modification: add kotor props
                                        if objShadowRadius!=undefined then newObj.shadowradius = objShadowRadius
                                        if objVerticalDisplacement!=undefined then newObj.verticaldisplacement = objVerticalDisplacement
                            -- end bead-v modification
                                        if objMultiplier!=undefined then newObj.multiplier = objMultiplier -- bead-v: fixed multipler to multiplier
                                        if objLightpriority!=undefined then newObj.lightpriority = objLightpriority
                                        if objNdynamictype!=undefined then newObj.nDynamicType = objNdynamictype
                                        if objLight_color!=undefined then
                                        (
                                            newObj.rgb = (color (objLight_color.x*255) (objLight_color.y*255) (objLight_color.z*255))
                                            -- if objLight_color is < [0,0,0] then is a negtaive light
                                            if (objLight_color.x<0) or (objLight_color.y<0) or (objLight_color.z<0) then newObj.negativeLight = 1
                                        )
                                        if objAmbientOnly!=undefined then newObj.ambientOnly = objAmbientOnly
                                        if objAffectDynamic!=undefined then newObj.affectDynamic = objAffectDynamic
                                        if objShadow!=undefined then newObj.Shadow = objShadow
                                        format "#beaming=%\r\n" objBeaming
                                        if objBeaming!=undefined then newObj.beaming = objBeaming
                                        if objLensflares!=undefined then newObj.lensflares = objLensflares
                                        if objFadingLight!=undefined then newObj.fadingLight = objFadingLight

                                        -- load the flare info if there is some
                                        if a_texturenames.count > 0 then
                                        (
                                            newObj.lens_flares = #()    -- just making sure is a blank array
                                            newObj.flareRadius = objFlareradius
                                            local flareIndex
                                            local flareAdd
                                            for flareIndex = 1 to a_texturenames.count do
                                            (
                                                flareAdd = a_texturenames[flareIndex]+" "+ \
                                                        (a_flaresizes[flareIndex] as string)+" "+ \
                                                        (a_flarepositions[flareIndex] as string)+" "+ \
                                                        (a_flarecolorshifts[flareIndex].x as string)+" "+ \
                                                        (a_flarecolorshifts[flareIndex].y as string)+" "+ \
                                                        (a_flarecolorshifts[flareIndex].z as string)
                                                append (newObj.lens_flares) flareAdd
                                            )
                                        )
                                        a_texturenames = #()
                                        a_flaresizes = #()
                                        a_flarepositions = #()
                                        a_flarecolorshifts = #()
                                    )

                                    (objType == "reference"):
                                    (
                                        newObj = aurorareference name:objName
                                        newObj.delegate.boxsize = [5,5,5]
                                        if objRefModel!=undefined then newObj.refModel = objRefModel
                                        if objReattachable!=undefined then newObj.reattachable = objReattachable
                                    )

                                    default:
                                    (
                                        if (newModelName == objName) then
                                        (
                                            if DEBUG then format "%BUILD MESH OBJECT: Is Model Base\r\n" strIndent1
                                            -- this is the base
                                            newObj = aurorabase name:objName
                                            newObj.delegate.boxsize = [100,100,5]
                                            newObj.pos = location
                                            newObj.classification_sel = newModelClassification
                                            newObj.setanimationscale = newModelAnimscale
                                            newObj.setsupermodel = newModelSupermodel
                                            -- set the export path to be the import path
                                            newObj.export_path = currentPath

                                            -- trap handle to model base for later use
                                            model_base = newObj

                                            --objProps += "mdl_root=true\r\n"
                                            -- make the pointer to the front
                                            if g_basepointer == 1 then
                                            (
                                                local tri = ngon()
                                                tri.radius = 25
                                                tri.nsides = 3
                                                tri.rotation *= (quat 90 [0,0,1])
                                                tri.name = "ignore_"+tri.name
                                                tri.parent = newObj
                                                in coordsys parent tri.pos = [0,50,0]
                                                tri.wireColor = (color 92 179 240)
                                            )
                                        ) else
                                        (
                                            if DEBUG then format "%BUILD MESH OBJECT: Is a Dummy!\r\n" strIndent1
                                            newObj = dummy name:objName
                                            newObj.boxsize = [5,5,5]
                                        )
                                    )
                                ) --End objType case

                                local itsParent = for p in objects where p.name == parent collect p
                                if itsParent != undefined and itsParent.count > 0 then
                                (
    -- check the controller types are correct!
                                    newObj.parent = itsParent[1]
                            -- bead-v modification: we're now making all controllers bezier from the get go, except orientation is linear
                                    --newObj.scale.controller = tcb_scale()
                                    --newObj.position.controller = tcb_position()
                                    --newObj.rotation.controller = tcb_rotation()
                                    newObj.scale.controller = bezier_scale()
                                    newObj.position.controller = bezier_position()
                                    newObj.rotation.controller = linear_rotation()
                            -- end bead-v modification
                                    in coordsys parent
                                    (
                                        --if newModelClassification == 1 then
                                        --(
                                        --    format "Setting time 0 anim keys.\r\n"
                                        --    with animate on at time 0 newObj.scale = objScale
                                        --    with animate on at time 0 newObj.rotation = objRot
                                        --    with animate on at time 0 newObj.position = objPos
                                        --) else
                                        --(
                                            at time 0 newObj.scale = objScale
                                            at time 0 newObj.rotation = objRot
                                            at time 0 newObj.position = objPos
                                        --)
                                    )
                                )
                                -- old style node data store - redundant
                                -- keeping it at the moment for debug purposes
                                setUserPropBuffer newObj objProps
                                setUserProp newObj "node_type" objType
                                objProps = ""   -- null object properties string

                                -- Skin modifier must be created after proper positioning.
                                --
                                -- PROBLEM!!! This assumes that the bones have been created
                                -- i.e. that the bones are defined in the mdl before the
                                -- the skin is.

                                -- Possible solutions:
                                -- 1. cache the weights info to an array and apply after mdl file is parsed
                                -- 2. do multiple passes across the mdl file
                                --      a. pass 1 does all normal stuff EXCEPT skins and Animations(?)
                                --      b. pass 2 does the skin and animation processing
                                --
                                -- Option 1 taken for time being.

                                if ((objType == "skin") and (numWeights == numVerts)) then
                                (
                                    if DEBUG then format "%SKIN PROCESSING\r\n" strIndent2
                                    -- Modifier must be applied here, rather than inside
                                    -- the addMDLSkin finction where it should be.
                                    -- I think its a gmax maxscript bug.
                                    addmodifier newObj (skin()) -- Add skin modifier to created obj
                                    --addMDLSkin newObj weightBuffer  -- Apply weight info to the skin modifer

                                    -- Joco 24-Aug-03
                                    -- Add data to the list of Weights structures
                                    -- for post processing once all the bones
                                    -- are in place
                                    append weightsList (nx_SkinWeights newObj weightBuffer)
                                )
                            ) -- End If for loadGeom

                        ) -- End endnode case and create mesh block

                        "endmodelgeom": -- End of model geometry
                        (
                            mdl_pos = MDL_ANIM
                            if (not importAnims) then quitEarly = true;

                            --
                            -- Joco 24-Aug-03
                            -- Now that all the model geometry is done (i.e. all the bones
                            -- should be built) we can post process the skin weights to their
                            -- skin mesh modifiers
                            if loadGeom then (
                                for i = 1 to weightsList.count do
                                (
                                    -- Apply weight info to the skin modifer
                                    addMDLSkin (weightsList[i]).Object (weightsList[i]).WeightsData
                                )
                            )
                            -- ideally let the garbage collector kill off this local var
                            weightsList = undefined
                        )

                        default:
                        (
                            objProps += idToken + "=" + tok.ReadRemainingTokens() + "\r\n"
                        )
                    ) -- end iktoken case statement
                ) -- End MDL_GEOM case


                ----------------------------------------------------------------
                -- Load model animations
                ----------------------------------------------------------------
                MDL_ANIM:
                (
                    if DEBUG then format "MDL_ANIM:  Line:% :: %\r\n" lineNum idToken

                    case idToken of
                    (
                        "#": () -- ignore comments

                        "newanim": -- New animation
                        (
                            animName = tok.ReadToken()
                            animRoot = getNodeByName(tok.ReadToken())
                            -- deal with the loading of anims only as the model base will
                            -- not likely be the same as the base refered to src anim mdl
                            if animRoot == undefined then
                            (
                                -- as inmroot is undefined we make it use the one we supplied
                                animRoot = animLoadBase
                            )

                            if DEBUG then format "Anim name = %\r\n" animName

                            animLength = 0
                            --numAnims += 1
                            numEvents = 0
                            --anim_property = "anim_name_" + (numAnims as string)
                            --setUserProp animRoot anim_property animName
                            --anim_property = "anim_firstframe_" + (numAnims as string)
                            --setUserProp animRoot anim_property firstFrame

                            bAnimIsThere = false


                            if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                            (
                                firstFrame = lastFrame + animGap

                                -- need to modify to cope with layering animation.
                                -- we don't want add the anim name in if it alread exists
                                -- only add in if name not already there
                                for an in animRoot.animations do
                                (
                                    stran = filterString an " "
                                    if stran[1] == animName then (
                                        bAnimIsThere = true
                                        -- given name is there we want put the keys at that point
                                        -- unless insert mode 2 or 3
                                        if animloadtype == 1 then (
                                            firstFrame = stran[2] as integer
                                        )
                                    )
                                )
                                -- Add animation data to animation root
                                if not bAnimIsThere then (
                                    animRoot.animations[animRoot.animations.count+1] = \
                                        animName + " " + (firstFrame as string)
                                )
                            )
                        )
                        "length": -- animtion length
                        (
                            animLength = nx_round((tok.ReadToken() as float) * frameRate) as integer

                            if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                            (
                                lastFrame = firstFrame + animLength
                                if not bAnimIsThere then
                                (
                                    -- Add animation data to animation root
                                    animRoot.animations[animRoot.animations.count] = \
                                        animRoot.animations[animRoot.animations.count]+ \
                                        " " + (lastFrame as string)
                                )
                            )
                        )
                        "transtime": -- animtion length
                        (
                            animTrans = (tok.ReadToken() as float)
                            --anim_property = "anim_transtime_" + (numAnims as string)
                            --setUserProp animRoot anim_property animTrans

                            if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                            (
                                if not bAnimIsThere then
                                (
                                    -- Add animation data to animation root
                                    animRoot.animations[animRoot.animations.count] = \
                                        animRoot.animations[animRoot.animations.count]+ \
                                        " " + (animTrans as string) + " 1"
                                )
                            )
                        )
                        "animroot": -- animation play root
                        (
                           animRootPlay = tok.ReadToken()

                            if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                            (
                                if not bAnimIsThere then
                                (
                                    -- Add animation data to animation root
                                    animRoot.animations[animRoot.animations.count] = \
                                        animRoot.animations[animRoot.animations.count]+ \
                                        " " + animRootPlay
                                )
                            )
                        )
                        "event": -- animtion event
                        (
                            --numEvents += 1 -- bead-v: moved further down

                            if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                            (
                                local event_frame = nx_round((tok.ReadFloat()*frameRate) + firstFrame)
                                local event_name = tok.ReadString()
                                local bDupEvent = false
                                -- test to see if this event is already at this frame
                                for ev in animRoot.events do
                                (
                                    strev = filterString ev " "
                                    if strev == (event_name + " " + (event_frame as string)) then (
                                        bDupEvent = true
                                    )
                                )
                                -- Add event data to animation root
                                if not bDupEvent then
                                (
                                    numEvents += 1 -- bead-v: move this here, as it makes more sense
                                    animRoot.events[animRoot.events.count+1] = \
                                        event_name + " " + (event_frame as string)
                                )
                            )
                        )
                        "eventlist": -- bead-v modification
                        (
                            local keyCount = undefined;
                            if (tok.PeekToken() != undefined) then keyCount = tok.ReadInteger()

                            tok.SetString (getNextNonBlankLine mdlStream)

                            while ((tok.PeekToken() != "endlist") and (keyCount != 0)) do (
                                if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                                (
                                    local event_frame = nx_round((tok.ReadFloat()*frameRate) + firstFrame)
                                    local event_name = tok.ReadString()
                                    local bDupEvent = false
                                    -- test to see if this event is already at this frame
                                    for ev in animRoot.events do
                                    (
                                        strev = filterString ev " "
                                        if strev == (event_name + " " + (event_frame as string)) then (
                                            bDupEvent = true
                                        )
                                    )
                                    -- Add event data to animation root
                                    if not bDupEvent then
                                    (
                                        numEvents += 1
                                        animRoot.events[animRoot.events.count+1] = \
                                            event_name + " " + (event_frame as string)
                                    )
                                )

                                -- read in next line from import data stream
                                if (keyCount != undefined) then(
                                    keyCount -= 1
                                    if keyCount > 0 then tok.SetString (getNextNonBlankLine mdlStream)
                                )
                                else
                                    tok.SetString (getNextNonBlankLine mdlStream)
                            )
                        )
                        "node":
                        (
                            objType = tok.ReadString()
                            bHadAnimKey = false -- start re looking for anim key processing
                            -- deal with part mapping at this point
                            -- see if the object name needs to be over-ridden. If there is no mapping table then we do nothing,
                            -- else we look for a part name to map to. no part name found means a blank string or undefined is
                            -- assigned. This should result in all the key building code being skipped until the next part is read in.
                            objName = tok.ReadString()
                            bMapped = false
                            -- process mapping table if it is provided
                            if mappingtbl != undefined then
                            (
                                for mapindex = 1 to mappingtbl.count by 2 do
                                (
                                    if nx_lowercase(mappingtbl[mapindex]) == nx_lowercase(objName) then (
                                        --format "mapping: % --> %\r\n" mappingtbl[mapindex] objName
                                        objName = mappingtbl[mapindex + 1]
                                        bMapped = true
                                    )
                                )
                                if not bMapped then (
                                    objName = ""
                                )
                            )
                            animObj = getNodeByName(objName)
                            if (animObj != undefined) then
                            (
                                if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then
                                (
                                    -- as we do NOT want to be animating WOKs we need to supress this step in that instance
                                    if (animObj.modifiers["AuroraWalkmesh"] == undefined) then
                                    (
                                        -- We have an animation node obj
                                        -- record lower and upper bounds
                                        lowerbound = (firstFrame - 1)
                                        upperbound = (lastFrame + 1)
                                        in coordsys parent
                                        (
                                            -- collect some data on animObj in timeslider time zero
                                            -- then proceed to generate some starting animation info.
                                            zeroPos = at time 0 animObj.pos
                                            zeroRot = at time 0 animObj.rotation
                                            if DEBUG then format "zeroPos=% zeroRot=%\r\n" zeroPos zeroRot
                                            --keytime = 0
                                            keytime = (firstFrame - 1)
                                            -- using 'animate on' context generate some key frames
                                            if DEBUG then format "zeroPos = %, zeroRot = %\r\n" zeroPos zeroRot
                                            local newkey = addNewKey animObj.pos.controller keytime
                                            newkey.value = zeroPos
                                            if(classof animObj.pos.controller == bezier_position) then(
                                                newkey.inTangentType = #linear
                                                newkey.outTangentType = #linear
                                            )
                                            --with animate on at time keytime animObj.pos = zeroPos
                                            with animate on at time keytime animObj.rotation = zeroRot
                                            -- stop key frame generate
                                            if DEBUG then format "Obj=% keytime=% Pos=% Rot=%\r\n" animObj keytime (at time keytime animObj.pos) (at time keytime animObj.rotation)

                                            /*
                                            -- move keytime to last frame plus 1.
                                            keytime = (lastFrame + 1)
                                            -- using 'animate on' context generate some key frames
                                            -- Sets a key frame at the last frame + 1
                                -- bead-v modification: do not create last frame here
                                            --with animate on at time keytime animObj.pos = zeroPos
                                            --with animate on at time keytime animObj.rotation = zeroRot
                                -- end bead-v modification
                                            -- stop key frame generate
                                            if DEBUG then format "Obj=% keytime=% Pos=% Rot=%\r\n" animObj keytime (at time keytime animObj.pos) (at time keytime animObj.rotation)
                                            */
                                        )
                                    )
                                )
                            ) -- end if

                        ) -- end 'Node' case
                        "endnode":
                        (
                            if DEBUG then format "endnode processing START\r\n"
                            if DEBUG then format "objName = % : animObj = %\r\n" objName animObj
                            animObj = getNodeByName(objName)
                            if (bHadAnimKey == false) and (animObj != undefined) then
                            (
	                            -- want to remove the redundant rot and pos keys created at the start of node processing.
	                            -- bead-v: currently only position and orientation are removed. What about scale?

	                            -- position 1st
	                            if DEBUG then format "get pos keys count\r\n"
	                            ksize = animObj.pos.controller.keys.count
	                            got_lower = false
	                            got_upper = false
	                            i = 1
                    -- bead-v modification: while !got_lower OR !got_upper, otherwise it quits as soon as it finds one of them
	                            --while (i <= ksize) and ((got_lower == false) or (got_upper == false)) do
                    -- end bead-v modification
	                            while (i <= ksize) and ((got_lower == false) and (got_upper == false)) do
	                            (
                                    if(numKeys animObj.pos.controller > 0) then
                                    (
                                        if (animObj.pos.controller.keys[i].time == lowerbound) then
                                        (
                                            if DEBUG then format "delete lower\r\n"
                                            got_lower = true
                                            if DEBUG then format "deleting lower\r\n"
                                            deleteKey animObj.pos.controller.keys i
                                            if DEBUG then format "deleted lower\r\n"
                                        )
                                        else --4
                                        (
                                            if (animObj.pos.controller.keys[i].time == upperbound) then
                                            (
                                                if DEBUG then format "delete upper\r\n"
                                                got_upper = true
                                                deleteKey animObj.pos.controller.keys i
                                            )
                                        )
                                    )
		                            i += 1
	                            )

	                            -- rotation 2nd
	                            if DEBUG then format "get rot keys count\r\n"
	                            ksize = animObj.rotation.controller.keys.count
	                            got_lower = false
	                            got_upper = false
	                            i = 1
                    -- bead-v modification: while !got_lower OR !got_upper, otherwise it quits as soon as it finds one of them
	                            --while (i <= ksize) and ((got_lower == false) or (got_upper == false)) do
                    -- end bead-v modification
	                            while (i <= ksize) and ((got_lower == false) and (got_upper == false)) do
	                            (
                                    if(numKeys animObj.rotation.controller > 0) then
                                    (
                                        if (animObj.rotation.controller.keys[i].time == lowerbound) then
                                        (
                                            if DEBUG then format "delete lower (rot)\r\n"
                                            got_lower = true
                                            deleteKey animObj.rotation.controller.keys i
                                        )
                                        else --5
                                        (
                                            if (animObj.rotation.controller.keys[i].time == upperbound) then
                                            (
                                                if DEBUG then format "delete upper\r\n"
                                                got_upper = true
                                                deleteKey animObj.rotation.controller.keys i
                                            )
                                        )
                                    )
		                            i += 1
	                            )
                            )
                            if DEBUG then format "endnode processing END\r\n"
                        )   -- end "endnode"
                        "doneanim": ()
                        "parent": ()
                        default:
                        (
                            local str_token = idToken as string
                            local str_length = str_token.count
                            --format "%, length=%\n" str_token str_length
                            if (str_length > 3) then
                            (
                                local objAnim = getNodeByName(objName)
                                --format "% == key, objAnim = %\n" (substring str_token (str_length - 2) 3) objAnim
                                if((substring str_token (str_length - 2) 3) == "key" and (objAnim != undefined)) then
                                (
                                    -- if we get here, then we're definitely parsing a controller with a valid objAnim
                                    str_token = substring str_token 1 (str_length-3)
                                    str_length = str_token.count
                                    local bBezier = false

                                    -- Determine whether we have a bezier controller on our hands
                                    if(str_length > 6) then
                                    (
                                        if((substring str_token (str_length - 5) 6) == "bezier") then
                                        (
                                            str_token = substring str_token 1 (str_length-6)
                                            str_length = str_token.count
                                            bBezier = true
                                        )
                                    )

                                    -- Get correct objAnim
                                    if(str_token == "scale" or
                                       str_token == "alpha" or
                                       str_token == "selfillumcolor") then(
                                        if (classof animObj) == Editable_mesh then (
                                            -- we have a trimesh and not a emitter
                                            animObj = animObj.modifiers["AuroraTrimesh"]
                                        )
                                    )

                                    local ctrl_data = "float"
                                    local objController
                                    case str_token of
                                    (
                                        -- pos & orient
                                        --- header
                                        "position": (
                                            ctrl_data = "position"
                                            if(objAnim.position.controller == undefined) then objAnim.position.controller = bezier_position()
                                            objController = objAnim.position.controller
                                        )
                                        "orientation": (
                                            ctrl_data = "rotation"
                                            if(objAnim.rotation.controller == undefined) then objAnim.rotation.controller = linear_rotation()
                                            objController = objAnim.rotation.controller
                                        )

                                        -- colors
                                        --- mesh
                                        "selfillumcolor": (
                                            ctrl_data = "color"
                                            if(objAnim.selfillumcolor.controller == undefined) then objAnim.selfillumcolor.controller = bezier_color()
                                            objController = objAnim.selfillumcolor.controller
                                        )
                                        --- light
                                        "color": (
                                            ctrl_data = "color"
                                            if(objAnim.rgb.controller == undefined) then objAnim.rgb.controller = bezier_color()
                                            objController = objAnim.rgb.controller
                                        )
                                        --- emitter
                                        "colorstart": (
                                            ctrl_data = "color"
                                            if(objAnim.colorStart.controller == undefined) then objAnim.colorStart.controller = bezier_color()
                                            objController = objAnim.colorStart.controller
                                        )
                                        "colormid": (
                                            ctrl_data = "color"
                                            if(objAnim.colorMid.controller == undefined) then objAnim.colorMid.controller = bezier_color()
                                            objController = objAnim.colorMid.controller
                                        )
                                        "colorend": (
                                            ctrl_data = "color"
                                            if(objAnim.colorEnd.controller == undefined) then objAnim.colorEnd.controller = bezier_color()
                                            objController = objAnim.colorEnd.controller
                                        )

                                        -- floats
                                        --- header
                                        "scale": (
                                            if(objAnim.scale.controller == undefined) then objAnim.scale.controller = bezier_float()
                                            objController = objAnim.scale.controller
                                        )
                                        --- mesh
                                        "alpha": (
                                            if(objAnim.alpha.controller == undefined) then objAnim.alpha.controller = bezier_float()
                                            objController = objAnim.alpha.controller
                                        )
                                        --- light
                                        "radius": (
                                            if(objAnim.radius.controller == undefined) then objAnim.radius.controller = bezier_float()
                                            objController = objAnim.radius.controller
                                        )
                                        "shadowradius": (
                                            if(objAnim.shadowradius.controller == undefined) then objAnim.shadowradius.controller = bezier_float()
                                            objController = objAnim.shadowradius.controller
                                        )
                                        "verticaldisplacement": (
                                            if(objAnim.verticaldisplacement.controller == undefined) then objAnim.verticaldisplacement.controller = bezier_float()
                                            objController = objAnim.verticaldisplacement.controller
                                        )
                                        "multiplier": (
                                            if(objAnim.multiplier.controller == undefined) then objAnim.multiplier.controller = bezier_float()
                                            objController = objAnim.multiplier.controller
                                        )
                                        --- emitter
                                        "alphastart": (
                                            if(objAnim.alphaStart.controller == undefined) then objAnim.alphaStart.controller = bezier_float()
                                            objController = objAnim.alphaStart.controller
                                        )
                                        "alphamid": (
                                            if(objAnim.alphaMid.controller == undefined) then objAnim.alphaMid.controller = bezier_float()
                                            objController = objAnim.alphaMid.controller
                                        )
                                        "alphaend": (
                                            if(objAnim.alphaEnd.controller == undefined) then objAnim.alphaEnd.controller = bezier_float()
                                            objController = objAnim.alphaEnd.controller
                                        )
                                        "percentstart": (
                                            if(objAnim.percentStart.controller == undefined) then objAnim.percentStart.controller = bezier_float()
                                            objController = objAnim.percentStart.controller
                                        )
                                        "percentmid": (
                                            if(objAnim.percentMid.controller == undefined) then objAnim.percentMid.controller = bezier_float()
                                            objController = objAnim.percentMid.controller
                                        )
                                        "percentend": (
                                            if(objAnim.percentEnd.controller == undefined) then objAnim.percentEnd.controller = bezier_float()
                                            objController = objAnim.percentEnd.controller
                                        )
                                        "sizestart": (
                                            if(objAnim.sizeStart.controller == undefined) then objAnim.sizeStart.controller = bezier_float()
                                            objController = objAnim.sizeStart.controller
                                        )
                                        "sizemid": (
                                            if(objAnim.sizeMid.controller == undefined) then objAnim.sizeMid.controller = bezier_float()
                                            objController = objAnim.sizeMid.controller
                                        )
                                        "sizeend": (
                                            if(objAnim.sizeEnd.controller == undefined) then objAnim.sizeEnd.controller = bezier_float()
                                            objController = objAnim.sizeEnd.controller
                                        )
                                        "sizestart_y": (
                                            if(objAnim.sizeStart_y.controller == undefined) then objAnim.sizeStart_y.controller = bezier_float()
                                            objController = objAnim.sizeStart_y.controller
                                        )
                                        "sizemid_y": (
                                            if(objAnim.sizeMid_y.controller == undefined) then objAnim.sizeMid_y.controller = bezier_float()
                                            objController = objAnim.sizeMid_y.controller
                                        )
                                        "sizeend_y": (
                                            if(objAnim.sizeEnd_y.controller == undefined) then objAnim.sizeEnd_y.controller = bezier_float()
                                            objController = objAnim.sizeEnd_y.controller
                                        )
                                        "framestart": (
                                            if(objAnim.frameStart.controller == undefined) then objAnim.frameStart.controller = bezier_float()
                                            objController = objAnim.frameStart.controller
                                        )
                                        "frameend": (
                                            if(objAnim.frameEnd.controller == undefined) then objAnim.frameEnd.controller = bezier_float()
                                            objController = objAnim.frameEnd.controller
                                        )
                                        "fps": (
                                            if(objAnim.fps.controller == undefined) then objAnim.fps.controller = bezier_float()
                                            objController = objAnim.fps.controller
                                        )
                                        "lightningdelay": (
                                            if(objAnim.lightningDelay.controller == undefined) then objAnim.lightningDelay.controller = bezier_float()
                                            objController = objAnim.lightningDelay.controller
                                        )
                                        "lightningradius": (
                                            if(objAnim.lightningRadius.controller == undefined) then objAnim.lightningRadius.controller = bezier_float()
                                            objController = objAnim.lightningRadius.controller
                                        )
                                        "lightningscale": (
                                            if(objAnim.lightningScale.controller == undefined) then objAnim.lightningScale.controller = bezier_float()
                                            objController = objAnim.lightningScale.controller
                                        )
                                        "lightningsubdiv": (
                                            if(objAnim.lightningSubDiv.controller == undefined) then objAnim.lightningSubDiv.controller = bezier_float()
                                            objController = objAnim.lightningSubDiv.controller
                                        )
                                        "lightningzigzag": (
                                            if(objAnim.lightningzigzag.controller == undefined) then objAnim.lightningzigzag.controller = bezier_float()
                                            objController = objAnim.lightningzigzag.controller
                                        )
                                        "birthrate": (
                                            if(objAnim.birthrate.controller == undefined) then objAnim.birthrate.controller = bezier_float()
                                            objController = objAnim.birthrate.controller
                                        )
                                        "m_frandombirthrate": (
                                            if(objAnim.m_fRandomBirthRate.controller == undefined) then objAnim.m_fRandomBirthRate.controller = bezier_float()
                                            objController = objAnim.m_fRandomBirthRate.controller
                                        )
                                        "lifeexp": (
                                            if(objAnim.lifeExp.controller == undefined) then objAnim.lifeExp.controller = bezier_float()
                                            objController = objAnim.lifeExp.controller
                                        )
                                        "mass": (
                                            if(objAnim.mass.controller == undefined) then objAnim.mass.controller = bezier_float()
                                            objController = objAnim.mass.controller
                                        )
                                        "spread": (
                                            if(objAnim.spread.controller == undefined) then objAnim.spread.controller = bezier_float()
                                            objController = objAnim.spread.controller
                                        )
                                        "particlerot": (
                                            if(objAnim.particleRot.controller == undefined) then objAnim.particleRot.controller = bezier_float()
                                            objController = objAnim.particleRot.controller
                                        )
                                        "velocity": (
                                            if(objAnim.velocity.controller == undefined) then objAnim.velocity.controller = bezier_float()
                                            objController = objAnim.velocity.controller
                                        )
                                        "randvel": (
                                            if(objAnim.randvel.controller == undefined) then objAnim.randvel.controller = bezier_float()
                                            objController = objAnim.randvel.controller
                                        )
                                        "bounce_co": (
                                            if(objAnim.bounce_co.controller == undefined) then objAnim.bounce_co.controller = bezier_float()
                                            objController = objAnim.bounce_co.controller
                                        )
                                        "blurlength": (
                                            if(objAnim.blurlength.controller == undefined) then objAnim.blurlength.controller = bezier_float()
                                            objController = objAnim.blurlength.controller
                                        )
                                        "targetsize": (
                                            if(objAnim.targetsize.controller == undefined) then objAnim.targetsize.controller = bezier_float()
                                            objController = objAnim.targetsize.controller
                                        )
                                        "numcontrolpts": (
                                            if(objAnim.numcontrolpts.controller == undefined) then objAnim.numcontrolpts.controller = bezier_float()
                                            objController = objAnim.numcontrolpts.controller
                                        )
                                        "controlptradius": (
                                            if(objAnim.controlptradius.controller == undefined) then objAnim.controlptradius.controller = bezier_float()
                                            objController = objAnim.controlptradius.controller
                                        )
                                        "controlptdelay": (
                                            if(objAnim.controlptdelay.controller == undefined) then objAnim.controlptdelay.controller = bezier_float()
                                            objController = objAnim.controlptdelay.controller
                                        )
                                        "tangentspread": (
                                            if(objAnim.tangentspread.controller == undefined) then objAnim.tangentspread.controller = bezier_float()
                                            objController = objAnim.tangentspread.controller
                                        )
                                        "tangentlength": (
                                            if(objAnim.tangentlength.controller == undefined) then objAnim.tangentlength.controller = bezier_float()
                                            objController = objAnim.tangentlength.controller
                                        )
                                        "detonate": (
                                            if(objAnim.detonate.controller == undefined) then objAnim.detonate.controller = bezier_float()
                                            objController = objAnim.detonate.controller
                                        )
                                        "p2p_bezier2": (
                                            if(objAnim.p2p_bezier2.controller == undefined) then objAnim.p2p_bezier2.controller = bezier_float()
                                            objController = objAnim.p2p_bezier2.controller
                                        )
                                        "p2p_bezier3": (
                                            if(objAnim.p2p_bezier3.controller == undefined) then objAnim.p2p_bezier3.controller = bezier_float()
                                            objController = objAnim.p2p_bezier3.controller
                                        )
                                        "grav": (
                                            if(objAnim.grav.controller == undefined) then objAnim.grav.controller = bezier_float()
                                            objController = objAnim.grav.controller
                                        )
                                        "drag": (
                                            if(objAnim.drag.controller == undefined) then objAnim.drag.controller = bezier_float()
                                            objController = objAnim.drag.controller
                                        )
                                        "xsize": (
                                            if(objAnim.xsize.controller == undefined) then objAnim.xsize.controller = bezier_float()
                                            objController = objAnim.xsize.controller
                                        )
                                        "ysize": (
                                            if(objAnim.ysize.controller == undefined) then objAnim.ysize.controller = bezier_float()
                                            objController = objAnim.ysize.controller
                                        )
                                        "threshold": (
                                            if(objAnim.threshold.controller == undefined) then objAnim.threshold.controller = bezier_float()
                                            objController = objAnim.threshold.controller
                                        )
                                        "combinetime": (
                                            if(objAnim.combinetime.controller == undefined) then objAnim.combinetime.controller = bezier_float()
                                            objController = objAnim.combinetime.controller
                                        )
                                    )

                                    if(objController != undefined) then(
                                        --format "About to read %\n" ctrl_data
                                        case ctrl_data of
                                        (
                                            "position":(
                                                bHadAnimKey = true

                                                local keyCount = undefined;
                                                if (tok.PeekToken() != undefined) then keyCount = tok.ReadInteger()

                                                in coordsys parent(
                                                    tok.SetString (getNextNonBlankLine mdlStream)
                                                    lineNum += 1

                                                    local newkey  -- bead-v: defined here so that the last key is saved after the while is done
                                                    while ((tok.PeekToken() != "endlist") and (keyCount != 0)) do (

                                                        local keytime = nx_round(firstFrame + (frameRate*tok.ReadFloat()))

                                                        local p3 = tok.ReadPoint3()
                                                        p3 *= 100
                                                        local p3in
                                                        local p3out
                                                        if(bBezier) then (
                                                            local p3in = tok.ReadPoint3()
                                                            local p3out = tok.ReadPoint3()
                                                            if p3in == undefined then p3in = [0, 0, 0]
                                                            if p3out == undefined then p3out = [0, 0, 0]
                                                        )
                                                        if DEBUG then format "line:% keytime:%   p3:% in:% out:%\r\n" lineNum keytime p3 p3in p3out

                                                        if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then (
                                                            newkey = addnewkey animObj.pos.controller keytime
                                                            newkey.value = p3
                                                            if(bBezier) then (
                                                                newkey.inTangentType = #custom
                                                                newkey.outTangentType = #custom
                                                                newkey.x_locked = false
                                                                newkey.y_locked = false
                                                                newkey.z_locked = false
                                                                newkey.inTangent = p3in
                                                                newkey.outTangent = p3out
                                                            )
                                                            else(
                                                                newkey.inTangentType = #linear
                                                                newkey.outTangentType = #linear
                                                            )
                                                        )

                                                        -- read in next line from import data stream
                                                        if (keyCount != undefined) then (
                                                            keyCount -= 1
                                                            if keyCount > 0 then
                                                            (
                                                                tok.SetString (getNextNonBlankLine mdlStream)
                                                             )
                                                        ) else
                                                        (
                                                            tok.SetString (getNextNonBlankLine mdlStream)
                                                        )
                                                        lineNum += 1
                                                    ) -- end while

                                                    -- Write last two extra keys
                                                    local lastkey = addNewKey animObj.pos.controller (lastFrame + 1)
                                                    lastkey.value = newkey.value
                                                    lastkey.inTangentType = newkey.inTangentType
                                                    lastkey.outTangentType = newkey.outTangentType
                                                    lastkey.x_locked = newkey.x_locked
                                                    lastkey.y_locked = newkey.y_locked
                                                    lastkey.z_locked = newkey.z_locked
                                                    lastkey.inTangent = newkey.inTangent
                                                    lastkey.outTangent = newkey.outTangent

                                                    lastkey = addNewKey animObj.pos.controller (lastFrame + 2)
                                                    lastkey.value = zeroPos
                                                    lastkey.inTangentType = #linear
                                                    lastkey.outTangentType = #linear
                                                )
                                            )
                                            "rotation":(
                                                local keytime
                                                local axis
                                                local angle
                                                local objRot
                                                local originalObjRot
                                                bHadAnimKey = true

                                                local keyCount = undefined
                                                if (tok.PeekToken() != undefined) then keyCount = tok.ReadInteger()

                                                in coordsys parent(

                                                    tok.SetString (getNextNonBlankLine mdlStream)

                                                    while ((tok.PeekToken() != "endlist") and (keyCount != 0)) do (
                                                        lineNum += 1
                                                        keytime = nx_round(firstFrame + (frameRate*tok.ReadFloat()))

                                                        axis = tok.ReadPoint3()
                                                        angle = RadToDeg (tok.ReadFloat())
                                                        objRot = quat angle axis

                                                        if DEBUG then format "line:% keytime:%  axis:%  angle:% quaternion:%\r\n" lineNum keytime axis angle objRot

                                                        local existingPosKey = getKeyIndex animObj.pos.controller keytime
                                                        local existingPos
                                                        if (existingPosKey > 0) then existingPos = animObj.pos.keys[existingPosKey].value

                                                        if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then (
                                                            --with animate on at time keytime animObj.rotation = objRot
                                                            local newkey = addnewkey animObj.rotation.controller keytime
                                                            newkey.value = objRot
                                                        )

                                                        if (existingPosKey == 0) then(
                                                            local newPosKey = getKeyIndex animObj.pos.controller keytime
                                                            if (newPosKey != 0) then
                                                            (
                                                                deleteKey animObj.pos.controller newPosKey
                                                            )
                                                        )
                                                        else
                                                            animObj.pos.keys[existingPosKey].value = existingPos


                                                        if (keyCount != undefined) then (
                                                            keyCount -= 1
                                                            if keyCount > 0 then
                                                            (
                                                                tok.SetString (getNextNonBlankLine mdlStream)
                                                             )
                                                        )
                                                        else
                                                            tok.SetString (getNextNonBlankLine mdlStream)
                                                    )

                                                    local lastkey = addNewKey animObj.rotation.controller (lastFrame + 1)
                                                    lastkey.value = newkey.value
                                                    lastkey = addNewKey animObj.rotation.controller (lastFrame + 2)
                                                    lastkey.value = zeroRot
                                                )
                                            )
                                            "color":(
                                                -- Get key count
                                                local keyCount = undefined;
                                                if (tok.PeekToken() != undefined) then keyCount = tok.ReadInteger()

                                                -- Go to first row
                                                tok.SetString (getNextNonBlankLine mdlStream)

                                                while ((tok.PeekToken() != "endlist") and (keyCount != 0)) do (
                                                    lineNum += 1

                                                    local keytime = nx_round(firstFrame + (frameRate*tok.ReadFloat()))

                                                    local this_value = tok.ReadColor()
                                                    local in_value
                                                    local out_value
                                                    if(bBezier) then(
                                                        in_value = tok.ReadPoint3()
                                                        out_value = tok.ReadPoint3()
                                                    )

                                                    if DEBUG then format "line:% keytime:%   color:%\r\n" lineNum keytime this_value

                                                    if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then (
                                                        newkey = addNewKey objController keytime
                                                        newkey.value = this_value
                                                        if(bBezier) then(
                                                            newkey.inTangentType = #custom
                                                            newkey.outTangentType = #custom
                                                            newkey.x_locked = false
                                                            newkey.inTangent = in_value
                                                            newkey.outTangent = out_value

                                                        )
                                                        else(
                                                            newkey.inTangentType = #linear
                                                            newkey.outTangentType = #linear
                                                        )
                                                    )

                                                    if (keyCount != undefined) then (
                                                        keyCount -= 1
                                                        if (keyCount > 0) then tok.SetString (getNextNonBlankLine mdlStream)
                                                    )
                                                    else
                                                        tok.SetString (getNextNonBlankLine mdlStream)
                                                )
                                            )
                                            "float":(
                                                -- set bHadAnimKey if scale (presumably for the main animation track)
                                                if(str_token == "scale") then bHadAnimKey = true

                                                -- Get key count
                                                local keyCount = undefined;
                                                if (tok.PeekToken() != undefined) then keyCount = tok.ReadInteger()

                                                -- Go to first row
                                                tok.SetString (getNextNonBlankLine mdlStream)

                                                local newkey
                                                while ((tok.PeekToken() != "endlist") and (keyCount != 0)) do (
                                                    lineNum += 1
                                                    local keytime = nx_round(firstFrame + (frameRate*tok.ReadFloat()))

                                                    local this_value = tok.ReadFloat()
                                                    if str_token == "radius" then this_value *= 100
                                                    local in_value
                                                    local out_value
                                                    if(bBezier) then(
                                                        in_value = tok.ReadFloat()
                                                        out_value = tok.ReadFloat()
                                                    )
                                                    --format "Adding float % (bezier % %) at time %\n" this_value in_value out_value keytime

                                                    if (loadanimlist == undefined) or ((findItem loadanimlist animname) > 0) then (
                                                        newkey = addnewkey objController keytime
                                                        newkey.value = this_value
                                                        if(bBezier) then(
                                                            newkey.inTangentType = #custom
                                                            newkey.outTangentType = #custom
                                                            newkey.x_locked = false
                                                            newkey.inTangent = in_value
                                                            newkey.outTangent = out_value

                                                        )
                                                        else
                                                        (
                                                            newkey.inTangentType = #linear
                                                            newkey.outTangentType = #linear
                                                        )
                                                    )

                                                    -- read in next line from import data stream
                                                    if (keyCount != undefined) then(
                                                        keyCount -= 1
                                                        if keyCount > 0 then tok.SetString (getNextNonBlankLine mdlStream)
                                                    )
                                                    else
                                                        tok.SetString (getNextNonBlankLine mdlStream)
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    ) -- end idtoken case
                ) -- End MDL_ANIM case

                MDL_NOT_IN_MDL:
                (
	                --
	                -- MDL is finished being loaded. Do any mandatory post processing that is needed
	                --
			       if (importAnims) then
			        (
			            if lastFrame > 0 then (
			                animationRange = (interval 0 lastFrame)
			            ) else (
			                animationRange = (interval 0 60)
			            )
			        ) else (
				        lastFrame = animationRange.end
			        )
                )

            ) -- end mdl_pos case statement
        ) -- end while not eof
        --lastFrame += animGap
     )
    catch
    (
        --messageBox ("NWN importer Script Error reading mdl file at line:" + lineNum as string)
        close mdlStream
        throw
    )
    close mdlStream
    nx_FlushBuffer()
    if showprogress then
    (
        progressEnd()
    )
)


-- listener command line command
-- Usage: massload <file pattern> <target> {<anims:true>}
fn massload pattern:undefined target:undefined anims:true =
(
    if (pattern == undefined) or (target==undefined) then
    (
        format "Usage:  massload pattern:<filepath\pattern> target:<target dir> {<anims:true|false>}\r\n"
        return 0
    )

    if not (nx_existDir target) then
    (
        messageBox "Target directory does not exist"
        return 0
    )

    resetMaxFile #noPrompt

    -- get all the files to load from the pattern provided
    local filesToLoad = getFiles pattern
    if target[target.count] != "\\" then target += "\\"
    for file in filesToLoad do
    (
        -- import the file
        ImportNWNmdl file anims false

         -- import pwk file if available
        local pwk_file = (getFilenamePath file)+(getFilenameFile  file)+".pwk"
        if ( nx_existFile pwk_file ) then ImportNWNwk pwk_file "pwk"
        -- import dwk file if available
        local dwk_file = (getFilenamePath file)+(getFilenameFile  file)+".dwk"
        if ( nx_existFile dwk_file ) then ImportNWNwk dwk_file "dwk"

		local modelBase
        for h in $helpers do
        (
           if (iskindof h aurorabase) then
           (
               modelBase = h
               exit
           )
        )
        -- save the file using the files model name
        saveMaxFile (target + modelBase.name)
        -- reset scene
      resetMaxFile #noPrompt
    )
)
