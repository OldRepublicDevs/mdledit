/*-----------------------------------------------------------------------------\

    NWmax
    by Joco (jameswalker@clear.net.nz)

    Credits:
    Wayland Reid :-
        Extends Wayland's (wreid@spectrumanalytic.com) Import/Export script
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
    4. In the case of code used from the Bioware scripts their intellectual
       property rights still hold.
    5. No code has been knowingly included from other sources where that code
       is sold for commercial purposes.
     
       
       

   Updates:
   .8b61    Fix for global sanity check issues.
            Enhanced wok checker tool

   .8b22    Fix for fadinglight token not being parsed
            Fix for wok test not out putting to sanity check file
            Support for wok material file being fully driven from .ini file
            A number of misc bug fixes that I have lost track of.
\-----------------------------------------------------------------------------*/

-- Ensure stack and heap size is large enough
if ( heapSize < 7500000 ) then heapSize = 7500000
if ( stackLimit < 4000000 ) then stackLimit = 4000000

global gNX_maxver = (maxversion())[1]

ClearListener()
global gblVersion = "0.8 b61"
format "--------------------------------------------------------------\r\n"
format "\r\n"
format "                        NWmax v%\r\n" gblVersion
format "\r\n"
format "--------------------------------------------------------------\r\n"

--------------------------------------------------------------------------------
---
--- Global Variables and init setups
---
--------------------------------------------------------------------------------
global DEBUG = false	-- debug flag
global SPEEDT = false	-- speed timer flag
global strIndent1 = "  "
global strIndent2 = "    "
global strIndent3 = "      "
global strIndent4 = "        "
global strIndent5 = "          "
--
-- Load nwmax ini global vars
format "NWmax ini load\r\n"
global g_3dsmax = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "usemax"
global g_guiAnimListSize = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "animlistsize"
global g_verbose = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "verbose"
global gNX_dock = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "dock"
global gNX_visible = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "visible"
global gnx_sanity = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "sanity"
global g_basepointer = getINISetting (scriptsPath+"nwmax/nwmax.ini") "init" "basepointer"

-- check to see if over-ride in nxlocalprefs
if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "animlistsize") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "animlistsize") != ""
then (
	g_guiAnimListSize = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "animlistsize"
)

if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "verbose") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "verbose") != ""
then (
	g_verbose = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "verbose"
)

if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "dock") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "dock") != ""
then (
	gNX_dock = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "dock"
)

if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "visible") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "visible") != ""
then (
	gNX_visible = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "visible"
)

if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "sanity") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "sanity") != ""
then (
    gnx_sanity = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "sanity"
)

if (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "basepointer") != undefined and \
   (getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "basepointer") != ""
then (
	g_basepointer = getINISetting (scriptsPath+"nxlocalprefs.ini") "init" "basepointer"
)

-- get nxlocalprefs specific data and convert types
gnx_pos_nwmax = getINISetting (scriptsPath+"nxlocalprefs.ini") "position" "nwmax"
if (gnx_pos_nwmax == undefined) or (gnx_pos_nwmax == "") then
(
	gnx_pos_nwmax = [0,100]
) else
(
	gnx_pos_nwmax = execute(gnx_pos_nwmax)
)


--
-- Correct ini vars for undefined and set required defaults and type conversion
format "Correct ini vars\r\n"
if g_guiAnimListSize == undefined then (
   g_guiAnimListSize = 10
) else (
   g_guiAnimListSize = (g_guiAnimListSize as integer)
)
if g_verbose == undefined then (
   g_verbose = 0
) else (
   g_verbose = (g_verbose as integer)
)
if gNX_dock == undefined then (
	gNX_dock = 0
) else (
	gNX_dock = (gNX_dock as integer)
)
if gNX_visible == undefined then (
	gNX_visible = 0
) else (
	gNX_visible = (gNX_visible as integer)
)
if gnx_sanity == undefined or gnx_sanity == "" then (
    gnx_sanity = "11000111110111111111"
)
if g_basepointer == undefined then (
   g_basepointer = 1
) else (
   g_basepointer = (g_basepointer as integer)
)
--
-- set globals for 3dsmax v's gmax
format "Set globals for 3dsmax v's gmax\r\n"
global g_fextn = ""
global g_gmaxini = ""
global g_ismax = false
if g_3dsmax == "1" then (
   g_fextn = ".max"
   g_gmaxini = "3dsmax.ini"
   g_ismax = true
) else (
   g_fextn = ".gmax"
   g_gmaxini = "gmax.ini"
   g_ismax = false
)
--
-- load in all paths from gmax.ini or max file
format "Load in all paths from gmax.ini or max file\r\n"
global g_maps_path = #()
for i = 1 to 100 do
(
	if (getINISetting (scriptsPath+"..\\"+g_gmaxini) "BitmapDirs" ("Dir"+(i as string))) == "" then exit
	append g_maps_path (getINISetting (scriptsPath+"..\\"+g_gmaxini) "BitmapDirs" ("Dir"+(i as string)))
)
format "Maps search dirs:\r\n"
print g_maps_path
--
-- Exporting stream buffer
format "Setup exporting stream\r\n"
if g_ismax then
(
   global g_strBuffer = undefined	-- file stream buffer for exporting
   global g_delim = "\n"	-- Buffered export system delimter
) else
(
   global g_strBuffer = stringStream ""	-- StringStream buffer for compressed export system
   if g_verbose == 1 then (
      g_delim = "~\n"
   ) else (
      g_delim = "~"
   )
)
-- end 3dsmax v's gmax


global g_startpath = ""
if autostart == undefined then autostart = false
if autostart == true then g_startpath = "nwmax/"
--
-- Material surfaces used in WOK walkmeshes
global matsurfs = #("Dirt","Obscuring","Grass","Stone","Wood","Water", \
                      "Nonwalk","Transparent","Carpet","Metal","Puddles", \
                      "Swamp","Mud","Leaves","Lava","BottomlessPit", \
                      "DeepWater","Door", "Snow", "Sand")
global g_plugins = #()  -- list of plugins
global g_compiler = scriptsPath+"nwmax\\mdlcomp\\nwnmdlcomp.exe"
global g_exportPath = ""
global nwmax_handle		-- handle to the master floater window
--
-- Global stack to be used for files and generic stack routines
global g_FileHandles = #()
--
-- Specific import globals
global lastDanglyName = undefined
global AnimRootNodeName = undefined
global LastAnimNumber = undefined
global LastAnimEventNumber = undefined
--
-- Specific export globals
global animIndex = 0    -- used by animmesh routine for modelbase.animation index
--
-- used in plug in load % bar
global percent = 0
global com_load_total = 9
global com_load = 0
--
-- UI flags for active dialogs
global nx_animkeymaster_flag = false
global nx_armourcutter_flag = false
global nx_ramaker_flag = false
global nx_skindumper_flag = false
--
-- globale execute str
global nx_exec_str

--
-- These are for the Active Sanity Check rollout in aurorabase
-- per Bioware's export scripts code. Legacy system.
format "Set Sanity check default settings\r\n"
global runRotationCheck
global runAnimCheck
global runCollisionMeshCheck
global runTileNodeBoundaryCheck
global runBSACheck
global runAuraEmitterCheck
global runAuraRefCheck
global runNameCheck
global runScaleCheck
global runPartTextureCheck
global runWeldCheck
global runClassCheck
global runControllerCheck
global runAnimMeshCheck
global runDoganCheck
global runBoneCountCheck
global runBitmapNameCheck
global runFaceCheck
global runAnimrootCheck
global runWOKCheck


--
-- Include in all the global function prototypes
format "Load Function Prototypes\r\n"
filein (g_startpath+"nw_ie_inc/aurora_fn_proto.ms")

--------------------------------------------------------------------------------
---
--- Structures
---
--------------------------------------------------------------------------------
format "Define structs and utility fns\r\n"
struct nx_Tokenizer
(
    --
    -- used to parse out the imported mdl file
    --
    tokens = #(),

    fn SetString str =
    (
        str = trimright(trimleft(str))
        tokens = filterString str "\t ,"
    ),

    fn ReadToken =
    (
        if tokens.count > 0 then
        (
            local tok = tokens[1]
            deleteItem tokens 1
            if DEBUG then format "ReadToken: %\r\n" tok
            tok
        )
        else
        (
            undefined
        )
    ),

    fn PeekToken =
    (
        if tokens.count > 0 then tokens[1] else undefined
    ),

    fn ReadPoint3 =
    (
        local x = ReadToken() as float;
        local y = ReadToken() as float;
        local z = ReadToken() as float;
        if DEBUG then format "ReadPoint3: %\r\n" (point3 x y z)
        (point3 x y z)
    ),

    fn ReadFloat =
    (
        (ReadToken() as float)
    ),

    fn ReadInteger =
    (
        (ReadToken() as integer)
    ),

    fn ReadString =
    (
        (ReadToken())
    ),

    fn ReadColor =
    (
        local r = 255 * (ReadToken() as float);
        local g = 255 * (ReadToken() as float);
        local b = 255 * (ReadToken() as float);
        (color r g b)
    ),
    
    fn ReadBoolean =
    (
        local b = ReadToken()
        if (nx_lowercase b) == "true" then
            (true)
        else
            (false)
    ),

    fn ReadRemainingTokens =
    (
        local str = ""
        while (PeekToken() != undefined) do str += ReadToken() + " "
        substring str 1 (str.count-1)
    )
)

struct nx_MultiArray
(
    -- base array store
    ArrayData = #(),
    width = 1,
    
    fn append row =
    (
        if row.count != width then
            return false
        /*
        for e in row do (
            print e
            append ArrayData e
        )
        */
        
        join ArrayData row
        
        true
    ),
    
    fn delete row =
    (
        local realIndex, i
        realIndex = row * width
        if ArrayData.count < (realIndex+width-1) then
            return false
            
        for i = realIndex to (realIndex+width-1) do
        (
            deleteItem ArrayData i
        )
        true
    ),
    
    fn get row col =
    (
        local realIndex, i
        realIndex = ((row-1) * width) + col
        if ArrayData.count < realIndex then
            return false
        
        ArrayData[realIndex]
    ),
    
    fn size =
    (
        ArrayData.count / width
    )
)

struct nx_SkinWeights
(
    -- used to store the skin object and weights data
    Object,
    WeightsData = #()
)

fn nx_coderefresh =
(
	filein (g_startpath+"nw_ie_inc/aurora_fn_general.ms")
	filein (g_startpath+"nw_ie_inc/aurora_fn_sanity.ms")
	filein (g_startpath+"nw_ie_inc/aurora_fn_export.ms")
	filein (g_startpath+"nw_ie_inc/aurora_gui_speedbtns.ms")
	filein (g_startpath+"nw_ie_inc/nwn_gui_import.ms")
	filein (g_startpath+"nw_ie_inc/nwn_gui_about.ms")
	filein (g_startpath+"nw_ie_inc/nwn_gui_updaters.ms")
	max views redraw
)

--------------------------------------------------------------------------------
--
-- General functions
--
--------------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/aurora_fn_general.ms")

--
-- Start progress for loading
--
progressStart "PlugIns Loading ..."

--------------------------------------------------------------------------------
---
--- Export functions
---
--------------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/aurora_fn_sanity.ms")
filein (g_startpath+"nw_ie_inc/aurora_fn_export.ms")

--------------------------------------------------------------------------------
---
--- Scripted Plug-Ins
---
--------------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/aurora_mod_trimesh.ms")
filein (g_startpath+"nw_ie_inc/aurora_mod_flex.ms")
filein (g_startpath+"nw_ie_inc/aurora_mod_walkmesh.ms")
--filein (g_startpath+"nw_ie_inc/aurora_mod_placeable.ms")
--filein (g_startpath+"nw_ie_inc/aurora_mod_animesh.ms")
--filein (g_startpath+"nw_ie_inc/aurora_helper_reference.ms")
filein (g_startpath+"nw_ie_inc/aurora_helper_base.ms")
filein (g_startpath+"nw_ie_inc/aurora_helper_light.ms")
filein (g_startpath+"nw_ie_inc/aurora_helper_emitter.ms")

--filein (g_startpath+"nw_ie_inc/aurora_helper_tools.ms")
--------------------------------------------------------------------------------
---
---   Rollout definitions
---
--------------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/aurora_gui_speedbtns.ms")
filein (g_startpath+"nw_ie_inc/nwn_gui_import.ms")
filein (g_startpath+"nw_ie_inc/nwn_gui_about.ms")



--------------------------------------------------------------------------------
---
---   Setup the default sanity check values per ini string
---
--------------------------------------------------------------------------------
runRotationCheck = nx_itob(gnx_sanity[1] as integer)
runAnimCheck = nx_itob(gnx_sanity[2] as integer)
runCollisionMeshCheck = nx_itob(gnx_sanity[3] as integer)
runTileNodeBoundaryCheck = nx_itob(gnx_sanity[4] as integer)
runAuraEmitterCheck = nx_itob(gnx_sanity[5] as integer)
runAuraRefCheck = nx_itob(gnx_sanity[6] as integer)
runNameCheck = nx_itob(gnx_sanity[7] as integer)
runScaleCheck = nx_itob(gnx_sanity[8] as integer)
runPartTextureCheck = nx_itob(gnx_sanity[9] as integer)
runWeldCheck = nx_itob(gnx_sanity[10] as integer)
runClassCheck = nx_itob(gnx_sanity[11] as integer)
runControllerCheck = nx_itob(gnx_sanity[12] as integer)
runAnimMeshCheck = nx_itob(gnx_sanity[13] as integer)
runDoganCheck = nx_itob(gnx_sanity[14] as integer)
runBitmapNameCheck = nx_itob(gnx_sanity[15] as integer)
runFaceCheck = nx_itob(gnx_sanity[16] as integer)
runBSACheck = nx_itob(gnx_sanity[17] as integer)
runAnimrootCheck = nx_itob(gnx_sanity[18] as integer)
runBoneCountCheck = nx_itob(gnx_sanity[19] as integer)
runWOKCheck = nx_itob(gnx_sanity[20] as integer)



--------------------------------------------------------------------------------
---
---   Rollout Window creation
---
--------------------------------------------------------------------------------

fn nx_make_floater =
(
	nwmax_handle = newRolloutFloater "NWmax" 200 600 0 100 --192 600 0 100
	addRollout nx_util1			nwmax_handle rolledUp:true
	addRollout nx_util2			nwmax_handle rolledUp:true
	addRollout nx_util3			nwmax_handle rolledUp:true
	addRollout nx_util4			nwmax_handle rolledUp:true
	addRollout nx_util5			nwmax_handle rolledUp:true
	addRollout nx_util6			nwmax_handle rolledUp:true
	addRollout ImportRollout    nwmax_handle rolledUp:true
	--addRollout nx_prefs			nwmax_handle rolledUp:true
	addRollout AboutRollout     nwmax_handle rolledUp:false
	
	-- load in old position from local ini file
	nwmax_handle.pos = gnx_pos_nwmax
)

fn nx_destroy_floater =
(
	if nwmax_handle != undefined do
	(
	    -- make sure floater is not docked and unregister it
	    try (
	        cui.FloatDialogBar nwmax_handle
	        cui.UnRegisterDialogBar nwmax_handle
	    ) catch ()
	    -- close it.
	    closerolloutfloater nwmax_handle
	)
)


-- try to close out a floater if it already exists
nx_destroy_floater()

if gNX_visible == 1 then
(
	nx_make_floater()
	if gNX_dock == 1 then
	(
		-- Register floater as a dockable bar
		cui.RegisterDialogBar nwmax_handle \
		    maxSize:[-1,-1] \
		    style:#(#cui_dock_vert, #cui_floatable, #cui_handles)
		-- By default dock the bar
		cui.DockDialogBar nwmax_handle #cui_dock_left
	)
)

-- Set app title
cui.setAppTitle ("NWmax v" + gblVersion)
    
    
if IsSceneRedrawDisabled()==true do enablesceneredraw()

--------------------------------------------------------------------------------
---
---   GUI updaters
---
--------------------------------------------------------------------------------
filein (g_startpath+"nw_ie_inc/nwn_gui_updaters.ms")

------------------
--- selChanged ---
------------------
fn nx_selChanged =()

-- Call this function once to set all the UI elements properly
nx_selChanged()

-- Set up a global call back for selection changes
--callbacks.addscript #selectionSetChanged "selChanged()" id:#waysNWNtool

format "------------------------------Done----------------------------\r\n"
progressEnd()
max views redraw