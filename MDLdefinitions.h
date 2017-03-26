#ifndef MDLDEFINITIONS_H_INCLUDED
#define MDLDEFINITIONS_H_INCLUDED

#define MDL_OFFSET 12
#define ANIM_OFFSET 136

#define CONVERT_CONTROLLER_SINGLE        1
#define CONVERT_CONTROLLER_KEYED         2
#define CONVERT_HEADER                   3
#define CONVERT_LIGHT                    4
#define CONVERT_EMITTER                  5
#define CONVERT_MESH                     6
#define CONVERT_SKIN                     7
#define CONVERT_DANGLY                   8
#define CONVERT_AABB                     9
#define CONVERT_SABER                    10
#define CONVERT_ENDNODE                  11
#define CONVERT_ANIMATION                12
#define CONVERT_ANIMATION_NODE           13
#define CONVERT_MODEL_GEO                14
#define CONVERT_MODEL                    15

#define K1_FUNCTION_POINTER_0 4273776
#define K1_FUNCTION_POINTER_1 4216096
#define K2_FUNCTION_POINTER_0 4285200
#define K2_FUNCTION_POINTER_1 4216320

#define CLASS_OTHER           0x00
#define CLASS_EFFECT          0x01
#define CLASS_TILE            0x02
#define CLASS_CHARACTER       0x04
#define CLASS_DOOR            0x08
#define CLASS_SABER           0x10 //Probably more general, weapon or item
#define CLASS_PLACEABLE       0x20
//#define CLASS_40            0x40
//#define CLASS_80            0x80

#define NODE_HAS_HEADER       0x0001
#define NODE_HAS_LIGHT        0x0002
#define NODE_HAS_EMITTER      0x0004
//#define NODE_HAS_CAMERA     0x0008
//#define NODE_HAS_REFERENCE  0x0010
#define NODE_HAS_MESH         0x0020
#define NODE_HAS_SKIN         0x0040
//#define NODE_HAS_ANIM       0x0080
#define NODE_HAS_DANGLY       0x0100
#define NODE_HAS_AABB         0x0200
//#define NODE_HAS_400        0x0400
#define NODE_HAS_SABER        0x0800
//#define NODE_HAS_GOB        0x1000
//#define NODE_HAS_COLLISION  0x2000
//#define NODE_HAS_SPHERE     0x4000
//#define NODE_HAS_CAPSULE    0x8000

#define NODE_SIZE_HEADER      80
#define NODE_SIZE_LIGHT       92
#define NODE_SIZE_EMITTER     224
//#define NODE_SIZE_CAMERA    0
//#define NODE_SIZE_REFERENCE 0
#define NODE_SIZE_MESH        340
#define NODE_SIZE_SKIN        100
//#define NODE_SIZE_ANIM      0
#define NODE_SIZE_DANGLY      28
#define NODE_SIZE_AABB        1
//#define NODE_SIZE_400       0
#define NODE_SIZE_SABER       20

#define EMITTER_FLAG_P2P                0x0001
#define EMITTER_FLAG_P2P_SEL            0x0002
#define EMITTER_FLAG_AFFECTED_WIND      0x0004
#define EMITTER_FLAG_TINTED             0x0008
#define EMITTER_FLAG_BOUNCE             0x0010
#define EMITTER_FLAG_RANDOM             0x0020
#define EMITTER_FLAG_INHERIT            0x0040
#define EMITTER_FLAG_INHERIT_VEL        0x0080
#define EMITTER_FLAG_INHERIT_LOCAL      0x0100
#define EMITTER_FLAG_SPLAT              0x0200
#define EMITTER_FLAG_INHERIT_PART       0x0400
//#define EMITTER_FLAG_0800             0x0800
//#define EMITTER_FLAG_1000             0x1000
//#define EMITTER_FLAG_2000             0x2000
//#define EMITTER_FLAG_3000             0x4000
//#define EMITTER_FLAG_4000             0x8000

#define MDX_FLAG_VERTEX                0x0001
#define MDX_FLAG_HAS_UV1               0x0002
#define MDX_FLAG_HAS_UV2               0x0004
#define MDX_FLAG_HAS_UV3               0x0008
#define MDX_FLAG_HAS_UV4               0x0010
#define MDX_FLAG_HAS_NORMAL            0x0020
#define MDX_FLAG_0040         /*??*/   0x0040
#define MDX_FLAG_HAS_TANGENT1          0x0080
#define MDX_FLAG_HAS_TANGENT2 /*??*/   0x0100
#define MDX_FLAG_HAS_TANGENT3 /*??*/   0x0200
#define MDX_FLAG_HAS_TANGENT4 /*??*/   0x0400
//#define MDX_FLAG_0800                0x0800
//#define MDX_FLAG_1000                0x1000
//#define MDX_FLAG_2000                0x2000
//#define MDX_FLAG_3000                0x4000
//#define MDX_FLAG_4000                0x8000

#define AABB_NO_CHILDREN    0x00
#define AABB_POSITIVE_X     0x01
#define AABB_POSITIVE_Y     0x02
#define AABB_POSITIVE_Z     0x04
#define AABB_NEGATIVE_X     0x08
#define AABB_NEGATIVE_Y     0x10
#define AABB_NEGATIVE_Z     0x20
//#define AABB_40           0x40
//#define AABB_80           0x80

#define MATERIAL_NONE           0
#define MATERIAL_DIRT           1
#define MATERIAL_OBSCURING      2
#define MATERIAL_GRASS          3
#define MATERIAL_STONE          4
#define MATERIAL_WOOD           5
#define MATERIAL_WATER          6
#define MATERIAL_NONWALK        7
#define MATERIAL_TRANSPARENT    8
#define MATERIAL_CARPET         9
#define MATERIAL_METAL          10
#define MATERIAL_PUDDLES        11
#define MATERIAL_SWAMP          12
#define MATERIAL_MUD            13
#define MATERIAL_LEAVES         14
#define MATERIAL_LAVA           15
#define MATERIAL_BOTTOMLESSPIT  16
#define MATERIAL_DEEPWATER      17
#define MATERIAL_DOOR           18
#define MATERIAL_SNOW           19
#define MATERIAL_SAND           20

///All controller numbers apparently must be divisible by 4
#define CONTROLLER_HEADER_POSITION              8
#define CONTROLLER_HEADER_ORIENTATION           20
#define CONTROLLER_HEADER_SCALING               36
///------------------------------------------------
#define CONTROLLER_ANIMATION_RANDOMBIRTHRATE    240 //ndix UR
//FairStrides and ndix UR talk about a 'scalekeys' controller in lightsabers
/*
So yeah, as far as I know, this controller is the only implementation of "scale" or
"scaling" of any kind. It can, however, be used just like a lot of other controllers in any of
3 different ways: 'single value', 'keyed', or 'bezier keyed'. The single value usage is the same
as how alpha and selfillumcolor work. It's sort of the same as how position and orientation work,
but those actually are copied to the mesh header for whatever reason (maybe because they are
required, whereas scale 1 can be default). Basically when you connect "scale" in the ascii model
to a single-value controller entry, you get working per-mesh scale (per-node actually, because it
is available on all node types).

Incidentally, fx_part_04 seems to be the only non-lightsaber TSL model that actually *uses* this
feature (single-value scale controller for 'initial' scale). It has scale 2.5.  The sabers also
use it to start their scale at 0.0 (and yeah, then they have animations using the scale controller
in keyed mode to scale up and down like you describe). fx_part_04 also animates the scale. The
following other TSL models animate scale but start at scale 1.0: fx_koltobub, plc_starmap, v_grnadhs_imp.
*/
///------------------------------------------------
#define CONTROLLER_LIGHT_COLOR                  76
#define CONTROLLER_LIGHT_RADIUS                 88
#define CONTROLLER_LIGHT_SHADOWRADIUS           96
#define CONTROLLER_LIGHT_VERTICALDISPLACEMENT   100
#define CONTROLLER_LIGHT_MULTIPLIER             140
///------------------------------------------------
#define CONTROLLER_EMITTER_ALPHAEND             80
#define CONTROLLER_EMITTER_ALPHASTART           84
#define CONTROLLER_EMITTER_BRITHRATE            88
#define CONTROLLER_EMITTER_BOUNCE_CO            92
#define CONTROLLER_EMITTER_COMBINETIME          96
#define CONTROLLER_EMITTER_DRAG                 100
#define CONTROLLER_EMITTER_FPS                  104
#define CONTROLLER_EMITTER_FRAMEEND             108
#define CONTROLLER_EMITTER_FRAMESTART           112
#define CONTROLLER_EMITTER_GRAV                 116
#define CONTROLLER_EMITTER_LIFEEXP              120
#define CONTROLLER_EMITTER_MASS                 124
#define CONTROLLER_EMITTER_P2P_BEZIER2          128
#define CONTROLLER_EMITTER_P2P_BEZIER3          132
#define CONTROLLER_EMITTER_PARTICLEROT          136
#define CONTROLLER_EMITTER_RANDVEL              140
#define CONTROLLER_EMITTER_SIZESTART            144
#define CONTROLLER_EMITTER_SIZEEND              148
#define CONTROLLER_EMITTER_SIZESTART_Y          152
#define CONTROLLER_EMITTER_SIZEEND_Y            156
#define CONTROLLER_EMITTER_SPREAD               160
#define CONTROLLER_EMITTER_THRESHOLD            164
#define CONTROLLER_EMITTER_VELOCITY             168
#define CONTROLLER_EMITTER_XSIZE                172
#define CONTROLLER_EMITTER_YSIZE                176
#define CONTROLLER_EMITTER_BLURLENGTH           180
#define CONTROLLER_EMITTER_LIGHTNINGDELAY       184
#define CONTROLLER_EMITTER_LIGHTNINGRADIUS      188
#define CONTROLLER_EMITTER_LIGHTNINGSCALE       192
#define CONTROLLER_EMITTER_LIGHTNINGSUBDIV      196
#define CONTROLLER_EMITTER_LIGHTNINGZIGZAG      200
//#define CONTROLLER_EMITTER_204                204
//#define CONTROLLER_EMITTER_208                208
//#define CONTROLLER_EMITTER_212                212
#define CONTROLLER_EMITTER_ALPHAMID             216
#define CONTROLLER_EMITTER_PERCENTSTART         220
#define CONTROLLER_EMITTER_PERCENTMID           224
#define CONTROLLER_EMITTER_PERCENTEND           228
#define CONTROLLER_EMITTER_SIZEMID              232
#define CONTROLLER_EMITTER_SIZEMID_Y            236
#define CONTROLLER_EMITTER_RANDOMBIRTHRATE      240
#define CONTROLLER_EMITTER_TARGETSIZE           252
#define CONTROLLER_EMITTER_NUMCONTROLPTS        256
#define CONTROLLER_EMITTER_CONTROLPTRADIUS      260
#define CONTROLLER_EMITTER_CONTROLPTDELAY       264
#define CONTROLLER_EMITTER_TANGENTSPREAD        268
#define CONTROLLER_EMITTER_TANGENTLENGTH        272
//#define CONTROLLER_EMITTER_276                276
//#define CONTROLLER_EMITTER_280                280
#define CONTROLLER_EMITTER_COLORMID             284
/// ... many missing controller numbers ... ///
#define CONTROLLER_EMITTER_COLOREND             380
//#define CONTROLLER_EMITTER_384                384
//#define CONTROLLER_EMITTER_388                388
#define CONTROLLER_EMITTER_COLORSTART           392
/// ... many missing controller numbers ... ///
#define CONTROLLER_EMITTER_DETONATE             502
///------------------------------------------------
#define CONTROLLER_MESH_SELFILLUMCOLOR          100
#define CONTROLLER_MESH_ALPHA                   132

#endif // MDLDEFINITIONS_H_INCLUDED
