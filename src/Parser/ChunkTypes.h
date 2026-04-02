#pragma once

#include <cstdint>
#include <unordered_map>

namespace Debugger3DS {
    // 3DS file chunk type definitions from the 3DS format specification
    namespace ChunkType {

        // ── Single source of truth for all chunk types ──
        // Each entry: X( NAME, ID )
        #define CHUNK_TABLE(X)                                                    \
            /* Main file chunks */                                                \
            X( M3DMAGIC              , 0x4D4D )  /* Main chunk */                 \
            X( M3D_VERSION           , 0x0002 )  /* Version */                    \
            X( MDATA                 , 0x3D3D )  /* Mesh data */                  \
            X( KFDATA                , 0xB000 )  /* Keyframe data */              \
            X( MLIBMAGIC             , 0x3DAA )  /* Material library */           \
            X( CMAGIC               , 0xC23D )  /* C magic */                     \
            /* MDATA section chunks */                                            \
            X( MESH_VERSION          , 0x3D3E )  /* Mesh version */               \
            X( MASTER_SCALE          , 0x0100 )  /* Master scale */               \
            X( AMBIENT_LIGHT         , 0x2100 )  /* Ambient light */              \
            X( LO_SHADOW_BIAS        , 0x1400 )  /* Low shadow bias */            \
            X( HI_SHADOW_BIAS        , 0x1410 )  /* High shadow bias */           \
            X( SHADOW_MAP_SIZE       , 0x1420 )  /* Shadow map size */            \
            X( SHADOW_SAMPLES        , 0x1430 )  /* Shadow samples */             \
            X( SHADOW_RANGE          , 0x1440 )  /* Shadow range */               \
            X( SHADOW_FILTER         , 0x1450 )  /* Shadow filter */              \
            X( RAY_BIAS              , 0x1460 )  /* Ray bias */                   \
            X( O_CONSTS              , 0x1500 )  /* Object constants */            \
            X( BIT_MAP               , 0x1100 )  /* Bitmap background */           \
            X( SOLID_BGND            , 0x1200 )  /* Solid background */            \
            X( V_GRADIENT            , 0x1300 )  /* Vertical gradient */           \
            X( USE_BIT_MAP           , 0x1101 )  /* Use bitmap */                  \
            X( USE_SOLID_BGND        , 0x1201 )  /* Use solid background */        \
            X( USE_V_GRADIENT        , 0x1301 )  /* Use vertical gradient */       \
            X( FOG                   , 0x2200 )  /* Fog */                         \
            X( FOG_BGND              , 0x2210 )  /* Fog background */              \
            X( LAYER_FOG             , 0x2302 )  /* Layer fog */                   \
            X( DISTANCE_CUE          , 0x2300 )  /* Distance cue */                \
            X( DCUE_BGND             , 0x2310 )  /* Distance cue background */     \
            X( USE_FOG               , 0x2201 )  /* Use fog */                     \
            X( USE_LAYER_FOG         , 0x2303 )  /* Use layer fog */               \
            X( USE_DISTANCE_CUE      , 0x2301 )  /* Use distance cue */            \
            /* Viewport chunks */                                                  \
            X( VIEWPORT_LAYOUT       , 0x7001 )  /* Viewport layout */             \
            X( VIEWPORT_SIZE         , 0x7020 )  /* Viewport size */               \
            X( VIEWPORT_DATA_3       , 0x7011 )  /* Viewport data v3 */            \
            X( VIEWPORT_DATA         , 0x7012 )  /* Viewport data */               \
            /* Default view chunks */                                              \
            X( DEFAULT_VIEW          , 0x3000 )  /* Default view */                \
            X( VIEW_TOP              , 0x3010 )  /* View top */                    \
            X( VIEW_BOTTOM           , 0x3020 )  /* View bottom */                 \
            X( VIEW_LEFT             , 0x3030 )  /* View left */                   \
            X( VIEW_RIGHT            , 0x3040 )  /* View right */                  \
            X( VIEW_FRONT            , 0x3050 )  /* View front */                  \
            X( VIEW_BACK             , 0x3060 )  /* View back */                   \
            X( VIEW_USER             , 0x3070 )  /* View user */                   \
            X( VIEW_CAMERA           , 0x3080 )  /* View camera */                 \
            /* Object chunks */                                                    \
            X( NAMED_OBJECT          , 0x4000 )  /* Named object */                \
            X( N_TRI_OBJECT          , 0x4100 )  /* Triangle mesh object */        \
            X( POINT_ARRAY           , 0x4110 )  /* Vertex array */                \
            X( POINT_FLAG_ARRAY      , 0x4111 )  /* Vertex flags */                \
            X( FACE_ARRAY            , 0x4120 )  /* Face array */                  \
            X( MSH_MAT_GROUP         , 0x4130 )  /* Material groups */             \
            X( TEX_VERTS             , 0x4140 )  /* Texture vertices */            \
            X( SMOOTH_GROUP          , 0x4150 )  /* Smoothing groups */            \
            X( MESH_MATRIX           , 0x4160 )  /* Mesh matrix */                 \
            X( MESH_COLOR            , 0x4165 )  /* Mesh color */                  \
            X( MESH_TEXTURE_INFO     , 0x4170 )  /* Texture info */                \
            X( MSH_BOXMAP            , 0x4190 )  /* Box mapping */                 \
            X( PROC_NAME             , 0x4181 )  /* Procedural name */             \
            X( PROC_DATA             , 0x4182 )  /* Procedural data */             \
            /* Named object flags */                                               \
            X( OBJ_HIDDEN            , 0x4010 )  /* Object hidden */               \
            X( OBJ_VIS_LOFTER        , 0x4011 )  /* Visible in lofter */           \
            X( OBJ_DOESNT_CAST       , 0x4012 )  /* Doesn't cast shadow */         \
            X( OBJ_MATTE             , 0x4013 )  /* Matte object */                \
            X( OBJ_FAST              , 0x4014 )  /* Fast display */                \
            X( OBJ_PROCEDURAL        , 0x4015 )  /* Procedural object */           \
            X( OBJ_FROZEN            , 0x4016 )  /* Frozen object */               \
            X( OBJ_DONT_RCVSHADOW    , 0x4017 )  /* Don't receive shadow */        \
            /* Material chunks */                                                  \
            X( MAT_ENTRY             , 0xAFFF )  /* Material entry */              \
            X( MAT_NAME              , 0xA000 )  /* Material name */               \
            X( MAT_AMBIENT           , 0xA010 )  /* Ambient color */               \
            X( MAT_DIFFUSE           , 0xA020 )  /* Diffuse color */               \
            X( MAT_SPECULAR          , 0xA030 )  /* Specular color */              \
            X( MAT_SHININESS         , 0xA040 )  /* Shininess */                   \
            X( MAT_SHIN2PCT          , 0xA041 )  /* Shininess strength */          \
            X( MAT_TRANSPARENCY      , 0xA050 )  /* Transparency */                \
            X( MAT_XPFALL            , 0xA052 )  /* Transparency falloff */        \
            X( MAT_REFBLUR           , 0xA053 )  /* Reflection blur */             \
            X( MAT_TWO_SIDE          , 0xA081 )  /* Two-sided */                   \
            X( MAT_DECAL             , 0xA082 )  /* Decal */                       \
            X( MAT_ADDITIVE          , 0xA083 )  /* Additive blend */              \
            X( MAT_SELF_ILLUM        , 0xA084 )  /* Self illumination */           \
            X( MAT_WIRE              , 0xA085 )  /* Wire frame */                  \
            X( MAT_WIRE_SIZE         , 0xA087 )  /* Wire frame size */             \
            X( MAT_FACEMAP           , 0xA088 )  /* Face mapping */                \
            X( MAT_PHONGSOFT         , 0xA08C )  /* Phong soft */                  \
            X( MAT_WIREABS           , 0xA08E )  /* Wire absolute size */          \
            X( MAT_SHADING           , 0xA100 )  /* Shading type */                \
            X( MAT_TEXMAP            , 0xA200 )  /* Texture map */                 \
            X( MAT_SPECMAP           , 0xA204 )  /* Specular map */                \
            X( MAT_OPACMAP           , 0xA210 )  /* Opacity map */                 \
            X( MAT_REFLMAP           , 0xA220 )  /* Reflection map */              \
            X( MAT_BUMPMAP           , 0xA230 )  /* Bump map */                    \
            X( MAT_USE_XPFALL        , 0xA240 )  /* Use transparency falloff */    \
            X( MAT_USE_REFBLUR       , 0xA250 )  /* Use reflection blur */         \
            X( MAT_MAPNAME           , 0xA300 )  /* Map filename */                \
            X( MAT_ACUBIC            , 0xA310 )  /* Auto cubic reflection */       \
            X( MAT_SXP_TEXT_DATA     , 0xA320 )  /* SXP texture data */            \
            X( MAT_SXP_TEXT2_DATA    , 0xA321 )  /* SXP texture 2 data */          \
            X( MAT_SXP_OPAC_DATA     , 0xA322 )  /* SXP opacity data */            \
            X( MAT_SXP_TEXT_MASKDATA , 0xA324 )  /* SXP texture mask data */       \
            X( MAT_SXP_SPEC_DATA     , 0xA325 )  /* SXP specular data */           \
            X( MAT_SXP_TEXT2MASK_DATA, 0xA326 )  /* SXP texture 2 mask data */     \
            X( MAT_SXP_SELFI_DATA    , 0xA327 )  /* SXP self-illum data */         \
            X( MAT_SXP_OPACMASK_DATA , 0xA328 )  /* SXP opacity mask data */       \
            X( MAT_SXP_BUMPMASK_DATA , 0xA32A )  /* SXP bump mask data */          \
            X( MAT_SXP_SPECMASK_DATA , 0xA32C )  /* SXP specular mask data */      \
            X( MAT_SXP_SHINMASK_DATA , 0xA32E )  /* SXP shininess mask data */     \
            X( MAT_SXP_SELFIMASK_DATA, 0xA330 )  /* SXP self-illum mask data */    \
            X( MAT_SXP_REFL_MASKDATA , 0xA332 )  /* SXP reflection mask data */    \
            X( MAT_TEX2MAP           , 0xA33A )  /* Texture 2 map */               \
            X( MAT_SHINMAP           , 0xA33C )  /* Shininess map */               \
            X( MAT_SELFIMAP          , 0xA33D )  /* Self-illum map */              \
            X( MAT_TEXMASK           , 0xA33E )  /* Texture mask */                \
            X( MAT_TEX2MASK          , 0xA340 )  /* Texture 2 mask */              \
            X( MAT_OPACMASK          , 0xA342 )  /* Opacity mask */                \
            X( MAT_BUMPMASK          , 0xA344 )  /* Bump mask */                   \
            X( MAT_SHINMASK          , 0xA346 )  /* Shininess mask */              \
            X( MAT_SPECMASK          , 0xA348 )  /* Specular mask */               \
            X( MAT_REFLMASK          , 0xA34A )  /* Reflection mask */             \
            X( MAT_SELFIMASK         , 0xA34C )  /* Self-illum mask */             \
            X( MAT_MAP_TILING        , 0xA351 )  /* Map tiling */                  \
            X( MAT_MAP_TEXBLUR       , 0xA353 )  /* Map texture blur */            \
            X( MAT_MAP_USCALE        , 0xA354 )  /* Map U scale */                 \
            X( MAT_MAP_VSCALE        , 0xA356 )  /* Map V scale */                 \
            X( MAT_MAP_UOFFSET       , 0xA358 )  /* Map U offset */                \
            X( MAT_MAP_VOFFSET       , 0xA35A )  /* Map V offset */                \
            X( MAT_MAP_ANG           , 0xA35C )  /* Map angle */                   \
            X( MAT_MAP_COL1          , 0xA360 )  /* Map color 1 */                 \
            X( MAT_MAP_COL2          , 0xA362 )  /* Map color 2 */                 \
            X( MAT_MAP_RCOL          , 0xA364 )  /* Map red color */               \
            X( MAT_MAP_GCOL          , 0xA366 )  /* Map green color */             \
            X( MAT_MAP_BCOL          , 0xA368 )  /* Map blue color */              \
            /* Light chunks */                                                     \
            X( N_DIRECT_LIGHT        , 0x4600 )  /* Direct light */                \
            X( DL_SPOTLIGHT          , 0x4610 )  /* Spotlight */                   \
            X( DL_OFF                , 0x4620 )  /* Light off */                   \
            X( DL_ATTENUATE          , 0x4625 )  /* Attenuation */                 \
            X( DL_RAYSHAD            , 0x4627 )  /* Ray-traced shadows */          \
            X( DL_SHADOWED           , 0x4630 )  /* Shadowed */                    \
            X( DL_LOCAL_SHADOW2      , 0x4641 )  /* Local shadow 2 */              \
            X( DL_SEE_CONE           , 0x4650 )  /* See cone */                    \
            X( DL_SPOT_RECTANGULAR   , 0x4651 )  /* Spot rectangular */            \
            X( DL_SPOT_OVERSHOOT     , 0x4652 )  /* Spot overshoot */              \
            X( DL_SPOT_PROJECTOR     , 0x4653 )  /* Spot projector */              \
            X( DL_EXCLUDE            , 0x4654 )  /* Exclude */                     \
            X( DL_SPOT_ROLL          , 0x4656 )  /* Spot roll */                   \
            X( DL_SPOT_ASPECT        , 0x4657 )  /* Spot aspect */                 \
            X( DL_RAY_BIAS           , 0x4658 )  /* Ray bias */                    \
            X( DL_INNER_RANGE        , 0x4659 )  /* Inner range */                 \
            X( DL_OUTER_RANGE        , 0x465A )  /* Outer range */                 \
            X( DL_MULTIPLIER         , 0x465B )  /* Multiplier */                  \
            /* Camera chunks */                                                    \
            X( N_CAMERA              , 0x4700 )  /* Camera */                      \
            X( CAMERA_SEE_CONE       , 0x4710 )  /* Camera see cone */             \
            X( CAM_RANGES            , 0x4720 )  /* Camera ranges */               \
            /* Color chunks */                                                     \
            X( COLOR_F               , 0x0010 )  /* Float color */                 \
            X( COLOR_24              , 0x0011 )  /* 24-bit color */                \
            X( LIN_COLOR_24          , 0x0012 )  /* Linear 24-bit color */         \
            X( LIN_COLOR_F           , 0x0013 )  /* Linear float color */          \
            /* Percentage chunks */                                                \
            X( INT_PERCENTAGE        , 0x0030 )  /* Integer percentage */          \
            X( FLOAT_PERCENTAGE      , 0x0031 )  /* Float percentage */            \
            /* Keyframe chunks */                                                  \
            X( AMBIENT_NODE_TAG      , 0xB001 )  /* Ambient node */                \
            X( OBJECT_NODE_TAG       , 0xB002 )  /* Object node */                 \
            X( CAMERA_NODE_TAG       , 0xB003 )  /* Camera node */                 \
            X( TARGET_NODE_TAG       , 0xB004 )  /* Target node */                 \
            X( LIGHT_NODE_TAG        , 0xB005 )  /* Light node */                  \
            X( L_TARGET_NODE_TAG     , 0xB006 )  /* Light target node */           \
            X( SPOTLIGHT_NODE_TAG    , 0xB007 )  /* Spotlight node */              \
            X( KFSEG                 , 0xB008 )  /* Keyframe segment */            \
            X( KFCURTIME             , 0xB009 )  /* Current time */                \
            X( KFHDR                 , 0xB00A )  /* Keyframe header */             \
            /* Node chunks */                                                      \
            X( NODE_HDR              , 0xB010 )  /* Node header */                 \
            X( INSTANCE_NAME         , 0xB011 )  /* Instance name */               \
            X( PIVOT                 , 0xB013 )  /* Pivot point */                 \
            X( BOUNDBOX              , 0xB014 )  /* Bounding box */                \
            X( POS_TRACK_TAG         , 0xB020 )  /* Position track */              \
            X( ROT_TRACK_TAG         , 0xB021 )  /* Rotation track */              \
            X( SCL_TRACK_TAG         , 0xB022 )  /* Scale track */                 \
            X( FOV_TRACK_TAG         , 0xB023 )  /* FOV track */                   \
            X( ROLL_TRACK_TAG        , 0xB024 )  /* Roll track */                  \
            X( COL_TRACK_TAG         , 0xB025 )  /* Color track */                 \
            X( MORPH_TRACK_TAG       , 0xB026 )  /* Morph track */                 \
            X( HOT_TRACK_TAG         , 0xB027 )  /* Hotspot track */               \
            X( FALL_TRACK_TAG        , 0xB028 )  /* Falloff track */               \
            X( HIDE_TRACK_TAG        , 0xB029 )  /* Hide track */                  \
            X( NODE_ID               , 0xB030 )  /* Node ID */                     \
            X( MORPH_SMOOTH          , 0xB032 )  /* Morph smooth */

        // ── Expand: constexpr ID + string literal ──
        #define CHUNK_CONST(NAME, ID) \
            constexpr uint16_t NAME = ID; \
            constexpr auto NAME##_STR = #NAME;

        CHUNK_TABLE(CHUNK_CONST)
        #undef CHUNK_CONST

        // ── Expand: build name lookup map (one copy, C++17 inline) ──
        inline std::unordered_map<uint16_t, const char*> BuildChunkNameMap() {
            return {
                #define CHUNK_MAP_ENTRY(NAME, ID) { ID, #NAME },
                CHUNK_TABLE(CHUNK_MAP_ENTRY)
                #undef CHUNK_MAP_ENTRY
            };
        }

        inline auto sChunkName = BuildChunkNameMap();
    }
}