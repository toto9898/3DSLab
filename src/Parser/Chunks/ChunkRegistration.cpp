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

namespace Debugger3DS {
    
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
        factory.RegisterChunk<FaceArrayChunk>(ChunkType::FACE_ARRAY, stream);
        factory.RegisterChunk<TexVertsChunk>(ChunkType::TEX_VERTS, stream);
        factory.RegisterChunk<SmoothGroupChunk>(ChunkType::SMOOTH_GROUP, stream);
        factory.RegisterChunk<MeshMatrixChunk>(ChunkType::MESH_MATRIX, stream);
        factory.RegisterChunk<MshMatGroupChunk>(ChunkType::MSH_MAT_GROUP, stream);

        // Light chunks
        factory.RegisterChunk<NDirectLightChunk>(ChunkType::N_DIRECT_LIGHT, stream);
        factory.RegisterChunk<DLSpotlightChunk>(ChunkType::DL_SPOTLIGHT, stream);
        factory.RegisterChunk<DLOffChunk>(ChunkType::DL_OFF, stream);
        factory.RegisterChunk<DLInnerRangeChunk>(ChunkType::DL_INNER_RANGE, stream);
        factory.RegisterChunk<DLOuterRangeChunk>(ChunkType::DL_OUTER_RANGE, stream);
        factory.RegisterChunk<DLMultiplierChunk>(ChunkType::DL_MULTIPLIER, stream);

        // Camera chunks
        factory.RegisterChunk<NCameraChunk>(ChunkType::N_CAMERA, stream);
        factory.RegisterChunk<CamRangesChunk>(ChunkType::CAM_RANGES, stream);

        // Ambient light (container — color supplied by child COLOR_F/COLOR_24)
        factory.RegisterChunk<AmbientLightChunk>(ChunkType::AMBIENT_LIGHT, stream);

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

        // Object hidden flag
        factory.RegisterChunk<ObjHiddenChunk>(ChunkType::OBJ_HIDDEN, stream);
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
        
        // Color chunks
        factory.RegisterChunk<ColorFChunk>(ChunkType::COLOR_F, stream);
        factory.RegisterChunk<Color24Chunk>(ChunkType::COLOR_24, stream);
        factory.RegisterChunk<ColorFChunk>(ChunkType::LIN_COLOR_F, stream);
        factory.RegisterChunk<Color24Chunk>(ChunkType::LIN_COLOR_24, stream);
        
        // Percentage chunks (used as children of material property chunks)
        factory.RegisterChunk<IntPercentageChunk>(ChunkType::INT_PERCENTAGE, stream);
        factory.RegisterChunk<FloatPercentageChunk>(ChunkType::FLOAT_PERCENTAGE, stream);
    }
    
} // namespace Debugger3DS