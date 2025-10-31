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
#include "ColorFChunk.h"
#include "Color24Chunk.h"
#include "NodeHdrChunk.h"
#include "PercentageChunk.h"
#include "SmoothGroupChunk.h"
#include "KeyframeChunks.h"
#include "ObjectNodeChunks.h"
#include "ChunkFactory.h"

namespace Debugger3DS {
    
    // Register all chunk types with the factory
    void RegisterAllChunks(std::istream& stream) {
        auto& factory = ChunkFactory::GetInstance();
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
        
        // Material chunks
        factory.RegisterChunk<MatEntryChunk>(ChunkType::MAT_ENTRY, stream);
        factory.RegisterChunk<MatNameChunk>(ChunkType::MAT_NAME, stream);
        factory.RegisterChunk<MatShininessChunk>(ChunkType::MAT_SHININESS, stream);
        factory.RegisterChunk<MatShin2PctChunk>(ChunkType::MAT_SHIN2PCT, stream);
        factory.RegisterChunk<MatTransparencyChunk>(ChunkType::MAT_TRANSPARENCY, stream);
        factory.RegisterChunk<MatXpfallChunk>(ChunkType::MAT_XPFALL, stream);
        factory.RegisterChunk<MatRefblurChunk>(ChunkType::MAT_REFBLUR, stream);
        factory.RegisterChunk<MatSelfIllumChunk>(ChunkType::MAT_SELF_ILLUM, stream);
        factory.RegisterChunk<MatWireSizeChunk>(ChunkType::MAT_WIRE_SIZE, stream);
        factory.RegisterChunk<MatShadingChunk>(ChunkType::MAT_SHADING, stream);
        
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