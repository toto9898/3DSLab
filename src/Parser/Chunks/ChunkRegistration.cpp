#include "ContainerChunks.h"
#include "M3dVersionChunk.h"
#include "MeshVersionChunk.h"
#include "MasterScaleChunk.h"
#include "NamedObjectChunk.h"
#include "NTriObjectChunk.h"
#include "PointArrayChunk.h"
#include "FaceArrayChunk.h"
#include "TexVertsChunk.h"
#include "MeshMatrixChunk.h"
#include "MshMatGroupChunk.h"
#include "MaterialChunks.h"
#include "ColorChunks.h"
#include "NodeHdrChunk.h"
#include "PercentageChunk.h"
#include "SmoothGroupChunk.h"
#include "KeyframeChunks.h"
#include "ObjectNodeChunks.h"
#include "AnimNodeChunks.h"
#include "LightChunks.h"
#include "CameraChunks.h"
#include "ChunkFactory.h"

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;
    
    // Register all chunk types with the factory
    void RegisterAllChunks(ChunkFactory& factory, std::istream& stream) {
        factory.RegisterChunk<M3dMagicChunk>(ChunkType::M3DMAGIC, stream);

        // Main file chunks
        factory.RegisterChunk<M3dVersionChunk>(ChunkType::M3D_VERSION, stream);
        factory.RegisterChunk<MDataChunk>(ChunkType::MDATA, stream);
        factory.RegisterChunk<KfDataChunk>(ChunkType::KFDATA, stream);
        
        // MDATA section chunks
        factory.RegisterChunk<MeshVersionChunk>(ChunkType::MESH_VERSION, stream);
        factory.RegisterChunk<MasterScaleChunk>(ChunkType::MASTER_SCALE, stream);

        // Keyframe chunks
        factory.RegisterChunk<NodeHdrChunk>(ChunkType::NODE_HDR, stream);
        factory.RegisterChunk<KfHdrChunk>(ChunkType::KFHDR, stream);
        factory.RegisterChunk<KfSegChunk>(ChunkType::KFSEG, stream);
        factory.RegisterChunk<KfCurTimeChunk>(ChunkType::KFCURTIME, stream);
        
        // Object node chunks
        factory.RegisterChunk<ObjectNodeTagChunk>(ChunkType::OBJECT_NODE_TAG, stream);
        factory.RegisterChunk<NodeIdChunk>(ChunkType::NODE_ID, stream);
        factory.RegisterChunk<InstanceNameChunk>(ChunkType::INSTANCE_NAME, stream);
        factory.RegisterChunk<BoundBoxChunk>(ChunkType::BOUNDBOX, stream);
        factory.RegisterChunk<PosTrackTagChunk>(ChunkType::POS_TRACK_TAG, stream);
        factory.RegisterChunk<RotTrackTagChunk>(ChunkType::ROT_TRACK_TAG, stream);
        factory.RegisterChunk<SclTrackTagChunk>(ChunkType::SCL_TRACK_TAG, stream);
        factory.RegisterChunk<MorphTrackTagChunk>(ChunkType::MORPH_TRACK_TAG, stream);
        factory.RegisterChunk<HideTrackTagChunk>(ChunkType::HIDE_TRACK_TAG, stream);
        factory.RegisterChunk<MorphSmoothChunk>(ChunkType::MORPH_SMOOTH, stream);
        factory.RegisterChunk<PivotChunk>(ChunkType::PIVOT, stream);
        
        // Object chunks
        factory.RegisterChunk<NamedObjectChunk>(ChunkType::NAMED_OBJECT, stream);
        factory.RegisterChunk<NTriObjectChunk>(ChunkType::N_TRI_OBJECT, stream);
        factory.RegisterChunk<PointArrayChunk>(ChunkType::POINT_ARRAY, stream);
        // POINT_FLAG_ARRAY (0x4111) — has binary payload, left unregistered (auto-skipped)
        factory.RegisterChunk<FaceArrayChunk>(ChunkType::FACE_ARRAY, stream);
        factory.RegisterChunk<TexVertsChunk>(ChunkType::TEX_VERTS, stream);
        factory.RegisterChunk<SmoothGroupChunk>(ChunkType::SMOOTH_GROUP, stream);
        factory.RegisterChunk<MeshMatrixChunk>(ChunkType::MESH_MATRIX, stream);
        factory.RegisterChunk<MshMatGroupChunk>(ChunkType::MSH_MAT_GROUP, stream);
        // MESH_COLOR (0x4165), MESH_TEXTURE_INFO (0x4170), MSH_BOXMAP (0x4190),
        // PROC_NAME (0x4181), PROC_DATA (0x4182) — have binary payloads, left unregistered (auto-skipped)

        // Light chunks
        factory.RegisterChunk<NDirectLightChunk>(ChunkType::N_DIRECT_LIGHT, stream);
        factory.RegisterChunk<DLSpotlightChunk>(ChunkType::DL_SPOTLIGHT, stream);
        factory.RegisterChunk<DLOffChunk>(ChunkType::DL_OFF, stream);
        factory.RegisterChunk<DLInnerRangeChunk>(ChunkType::DL_INNER_RANGE, stream);
        factory.RegisterChunk<DLOuterRangeChunk>(ChunkType::DL_OUTER_RANGE, stream);
        factory.RegisterChunk<DLMultiplierChunk>(ChunkType::DL_MULTIPLIER, stream);
        factory.RegisterChunk<DLAttenuateChunk>(ChunkType::DL_ATTENUATE, stream);
        factory.RegisterChunk<DLRayshadChunk>(ChunkType::DL_RAYSHAD, stream);
        factory.RegisterChunk<DLShadowedChunk>(ChunkType::DL_SHADOWED, stream);
        // DL_LOCAL_SHADOW2 (0x4641) — has binary payload (float, float, int16), left unregistered
        factory.RegisterChunk<DLSeeConeChunk>(ChunkType::DL_SEE_CONE, stream);
        factory.RegisterChunk<DLSpotRectangularChunk>(ChunkType::DL_SPOT_RECTANGULAR, stream);
        factory.RegisterChunk<DLSpotOvershootChunk>(ChunkType::DL_SPOT_OVERSHOOT, stream);
        // DL_SPOT_PROJECTOR (0x4653), DL_EXCLUDE (0x4654), DL_SPOT_ROLL (0x4656),
        // DL_SPOT_ASPECT (0x4657), DL_RAY_BIAS (0x4658) — have binary payloads, left unregistered

        // Camera chunks
        factory.RegisterChunk<NCameraChunk>(ChunkType::N_CAMERA, stream);
        factory.RegisterChunk<CamRangesChunk>(ChunkType::CAM_RANGES, stream);
        factory.RegisterChunk<CamSeeConeChunk>(ChunkType::CAMERA_SEE_CONE, stream);

        // Ambient light (container — color supplied by child COLOR_F/COLOR_24)
        factory.RegisterChunk<AmbientLightChunk>(ChunkType::AMBIENT_LIGHT, stream);

        // File-type root chunks (pure containers — children parsed via base class)
        factory.RegisterChunk<MlibMagicChunk>(ChunkType::MLIBMAGIC, stream);
        factory.RegisterChunk<CMagicChunk>(ChunkType::CMAGIC, stream);

        // Shadow/global setting chunks with binary payloads — left unregistered (auto-skipped):
        //   LO_SHADOW_BIAS (0x1400), HI_SHADOW_BIAS (0x1410), SHADOW_MAP_SIZE (0x1420),
        //   SHADOW_SAMPLES (0x1430), SHADOW_RANGE (0x1440), SHADOW_FILTER (0x1450),
        //   RAY_BIAS (0x1460), O_CONSTS (0x1500)

        // Background chunks: register flag-only and pure containers; skip data-bearing ones
        // BIT_MAP (0x1100) has a string payload — left unregistered
        factory.RegisterChunk<UseBitMapChunk>(ChunkType::USE_BIT_MAP, stream);    // flag-only
        factory.RegisterChunk<SolidBgndChunk>(ChunkType::SOLID_BGND, stream);     // pure container
        factory.RegisterChunk<UseSolidBgndChunk>(ChunkType::USE_SOLID_BGND, stream); // flag-only
        // V_GRADIENT (0x1300) has a float prefix before children — left unregistered
        factory.RegisterChunk<UseVGradientChunk>(ChunkType::USE_V_GRADIENT, stream); // flag-only

        // Atmosphere chunks: register flag-only ones; skip mixed data+children ones
        // FOG (0x2200), LAYER_FOG (0x2302), DISTANCE_CUE (0x2300) — mixed, left unregistered
        factory.RegisterChunk<FogBgndChunk>(ChunkType::FOG_BGND, stream);            // flag-only
        factory.RegisterChunk<UseFogChunk>(ChunkType::USE_FOG, stream);              // flag-only
        factory.RegisterChunk<UseLayerFogChunk>(ChunkType::USE_LAYER_FOG, stream);   // flag-only
        factory.RegisterChunk<DcueBgndChunk>(ChunkType::DCUE_BGND, stream);          // flag-only
        factory.RegisterChunk<UseDistanceCueChunk>(ChunkType::USE_DISTANCE_CUE, stream); // flag-only

        // Viewport and default-view chunks all have binary payloads — left unregistered (auto-skipped):
        //   VIEWPORT_LAYOUT (0x7001), VIEWPORT_SIZE (0x7020), VIEWPORT_DATA_3 (0x7011),
        //   VIEWPORT_DATA (0x7012), DEFAULT_VIEW (0x3000) children (VIEW_TOP … VIEW_CAMERA)
        factory.RegisterChunk<DefaultViewChunk>(ChunkType::DEFAULT_VIEW, stream); // pure container

        // Camera/light animation node container tags
        factory.RegisterChunk<CameraNodeTagChunk>(ChunkType::CAMERA_NODE_TAG, stream);
        factory.RegisterChunk<TargetNodeTagChunk>(ChunkType::TARGET_NODE_TAG, stream);
        factory.RegisterChunk<LightNodeTagChunk>(ChunkType::LIGHT_NODE_TAG, stream);
        factory.RegisterChunk<LTargetNodeTagChunk>(ChunkType::L_TARGET_NODE_TAG, stream);

        // Animated float tracks (camera fov/roll, spotlight hotspot/falloff, light color)
        factory.RegisterChunk<FovTrackTagChunk>(ChunkType::FOV_TRACK_TAG, stream);
        factory.RegisterChunk<RollTrackTagChunk>(ChunkType::ROLL_TRACK_TAG, stream);
        factory.RegisterChunk<HotTrackTagChunk>(ChunkType::HOT_TRACK_TAG, stream);
        factory.RegisterChunk<FallTrackTagChunk>(ChunkType::FALL_TRACK_TAG, stream);
        factory.RegisterChunk<ColTrackTagChunk>(ChunkType::COL_TRACK_TAG, stream);

        // Object hidden + other named-object property flags
        factory.RegisterChunk<ObjHiddenChunk>(ChunkType::OBJ_HIDDEN, stream);
        factory.RegisterChunk<ObjVisLofterChunk>(ChunkType::OBJ_VIS_LOFTER, stream);
        factory.RegisterChunk<ObjDoesntCastChunk>(ChunkType::OBJ_DOESNT_CAST, stream);
        factory.RegisterChunk<ObjMatteChunk>(ChunkType::OBJ_MATTE, stream);
        factory.RegisterChunk<ObjFastChunk>(ChunkType::OBJ_FAST, stream);
        factory.RegisterChunk<ObjProceduralChunk>(ChunkType::OBJ_PROCEDURAL, stream);
        factory.RegisterChunk<ObjFrozenChunk>(ChunkType::OBJ_FROZEN, stream);
        factory.RegisterChunk<ObjDontRcvShadowChunk>(ChunkType::OBJ_DONT_RCVSHADOW, stream);

        // Spotlight / ambient animation node containers (stubs — children parsed via base class)
        factory.RegisterChunk<SpotlightNodeTagChunk>(ChunkType::SPOTLIGHT_NODE_TAG, stream);
        factory.RegisterChunk<AmbientNodeTagChunk>(ChunkType::AMBIENT_NODE_TAG, stream);

        // Material chunks
        factory.RegisterChunk<MatEntryChunk>(ChunkType::MAT_ENTRY, stream);
        factory.RegisterChunk<MatNameChunk>(ChunkType::MAT_NAME, stream);
        factory.RegisterChunk<MatAmbientChunk>(ChunkType::MAT_AMBIENT, stream);
        factory.RegisterChunk<MatDiffuseChunk>(ChunkType::MAT_DIFFUSE, stream);
        factory.RegisterChunk<MatSpecularChunk>(ChunkType::MAT_SPECULAR, stream);
        factory.RegisterChunk<MatShininessChunk>(ChunkType::MAT_SHININESS, stream);
        factory.RegisterChunk<MatShin2PctChunk>(ChunkType::MAT_SHIN2PCT, stream);
        factory.RegisterChunk<MatTransparencyChunk>(ChunkType::MAT_TRANSPARENCY, stream);
        factory.RegisterChunk<MatXpfallChunk>(ChunkType::MAT_XPFALL, stream);
        factory.RegisterChunk<MatRefblurChunk>(ChunkType::MAT_REFBLUR, stream);
        factory.RegisterChunk<MatSelfIllumChunk>(ChunkType::MAT_SELF_ILLUM, stream);
        factory.RegisterChunk<MatWireSizeChunk>(ChunkType::MAT_WIRE_SIZE, stream);
        factory.RegisterChunk<MatShadingChunk>(ChunkType::MAT_SHADING, stream);
        factory.RegisterChunk<MatTexmapChunk>(ChunkType::MAT_TEXMAP, stream);
        factory.RegisterChunk<MatMapNameChunk>(ChunkType::MAT_MAPNAME, stream);

        // Material property flag chunks
        factory.RegisterChunk<MatTwoSideChunk>(ChunkType::MAT_TWO_SIDE, stream);
        factory.RegisterChunk<MatDecalChunk>(ChunkType::MAT_DECAL, stream);
        factory.RegisterChunk<MatAdditiveChunk>(ChunkType::MAT_ADDITIVE, stream);
        factory.RegisterChunk<MatWireChunk>(ChunkType::MAT_WIRE, stream);
        factory.RegisterChunk<MatFacemapChunk>(ChunkType::MAT_FACEMAP, stream);
        factory.RegisterChunk<MatPhongsoftChunk>(ChunkType::MAT_PHONGSOFT, stream);
        factory.RegisterChunk<MatWireAbsChunk>(ChunkType::MAT_WIREABS, stream);
        factory.RegisterChunk<MatUseXpfallChunk>(ChunkType::MAT_USE_XPFALL, stream);
        factory.RegisterChunk<MatUseRefblurChunk>(ChunkType::MAT_USE_REFBLUR, stream);

        // Additional texture map containers
        factory.RegisterChunk<MatSpecmapChunk>(ChunkType::MAT_SPECMAP, stream);
        factory.RegisterChunk<MatOpacmapChunk>(ChunkType::MAT_OPACMAP, stream);
        factory.RegisterChunk<MatReflmapChunk>(ChunkType::MAT_REFLMAP, stream);
        factory.RegisterChunk<MatBumpmapChunk>(ChunkType::MAT_BUMPMAP, stream);
        factory.RegisterChunk<MatTex2mapChunk>(ChunkType::MAT_TEX2MAP, stream);
        factory.RegisterChunk<MatShinmapChunk>(ChunkType::MAT_SHINMAP, stream);
        factory.RegisterChunk<MatSelfimapChunk>(ChunkType::MAT_SELFIMAP, stream);

        // Texture mask containers
        factory.RegisterChunk<MatTexmaskChunk>(ChunkType::MAT_TEXMASK, stream);
        factory.RegisterChunk<MatTex2maskChunk>(ChunkType::MAT_TEX2MASK, stream);
        factory.RegisterChunk<MatOpacmaskChunk>(ChunkType::MAT_OPACMASK, stream);
        factory.RegisterChunk<MatBumpmaskChunk>(ChunkType::MAT_BUMPMASK, stream);
        factory.RegisterChunk<MatShinmaskChunk>(ChunkType::MAT_SHINMASK, stream);
        factory.RegisterChunk<MatSpecmaskChunk>(ChunkType::MAT_SPECMASK, stream);
        factory.RegisterChunk<MatReflmaskChunk>(ChunkType::MAT_REFLMASK, stream);
        factory.RegisterChunk<MatSelfimaskChunk>(ChunkType::MAT_SELFIMASK, stream);

        // MAT_ACUBIC (0xA310) — has binary payload (1+1+2+4+4 bytes), left unregistered

        // Map property chunks all have numeric/string payloads — left unregistered (auto-skipped):
        //   MAT_MAP_TILING (0xA351), MAT_MAP_TEXBLUR (0xA353), MAT_MAP_USCALE (0xA354),
        //   MAT_MAP_VSCALE (0xA356), MAT_MAP_UOFFSET (0xA358), MAT_MAP_VOFFSET (0xA35A),
        //   MAT_MAP_ANG (0xA35C), MAT_MAP_COL1 (0xA360), MAT_MAP_COL2 (0xA362),
        //   MAT_MAP_RCOL (0xA364), MAT_MAP_GCOL (0xA366), MAT_MAP_BCOL (0xA368)

        // SXP procedural data chunks have variable binary payloads — left unregistered (auto-skipped)

        
        // Color chunks
        factory.RegisterChunk<ColorFChunk>(ChunkType::COLOR_F, stream);
        factory.RegisterChunk<Color24Chunk>(ChunkType::COLOR_24, stream);
        factory.RegisterChunk<ColorFChunk>(ChunkType::LIN_COLOR_F, stream);
        factory.RegisterChunk<Color24Chunk>(ChunkType::LIN_COLOR_24, stream);
        
        // Percentage chunks (used as children of material property chunks)
        factory.RegisterChunk<IntPercentageChunk>(ChunkType::INT_PERCENTAGE, stream);
        factory.RegisterChunk<FloatPercentageChunk>(ChunkType::FLOAT_PERCENTAGE, stream);
    }
    
} // namespace Debugger3DS::Parser::Chunks