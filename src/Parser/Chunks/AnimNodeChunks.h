#pragma once

#include "ObjectNodeChunks.h"  // for TrackTagChunk<T>
#include <Eigen/Dense>

namespace Debugger3DS::Parser::Chunks {

// ---------------------------------------------------------------------------
/// @name Camera / target / light animation node containers (KFDATA section)
/// @{
// ---------------------------------------------------------------------------

/// @brief Camera node tag chunk (0xB003) — keyframe animation container for a camera object.
///
/// @details May appear any number of times as a child of KFDATA (0xB000),
/// once per animated camera. An OBJECT_NODE_TAG (0xB002) can be a hierarchical
/// parent of this chunk.
///
/// @par Expected children
/// - NODE_ID (0xB030) — optional (R3), ordering override.
/// - NODE_HDR (0xB010) — required, links node to camera object.
/// - POS_TRACK_TAG (0xB020) — camera position keys.
/// - FOV_TRACK_TAG (0xB023) — field-of-view angle keys.
/// - ROLL_TRACK_TAG (0xB024) — roll/bank angle keys.
class CameraNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
    bool Process(Importer& importer)  override;
    std::string GetTypeName() const override { return "CAMERA_NODE_TAG"; }
};

/// @brief Camera target node tag chunk (0xB004) — keyframe animation for a camera's look-at target.
///
/// @details May appear any number of times as a child of KFDATA (0xB000),
/// one per camera that has an animated target. The target shares the camera's
/// object name in NODE_HDR.
///
/// @par Expected children
/// - NODE_ID (0xB030) — optional (R3), ordering override.
/// - NODE_HDR (0xB010) — required, links to the camera's target.
/// - POS_TRACK_TAG (0xB020) — target position keys.
class TargetNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
    bool Process(Importer& importer)  override;
    std::string GetTypeName() const override { return "TARGET_NODE_TAG"; }
};

/// @brief Light node tag chunk (0xB005) — keyframe animation container for an omni-light object.
///
/// @details May appear any number of times as a child of KFDATA (0xB000).
///
/// @par Expected children
/// - NODE_ID (0xB030) — optional (R3), ordering override.
/// - NODE_HDR (0xB010) — required, links node to light object.
/// - POS_TRACK_TAG (0xB020) — light position keys.
/// - COL_TRACK_TAG (0xB025) — light color keys.
class LightNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
    bool Process(Importer& importer)  override;
    std::string GetTypeName() const override { return "LIGHT_NODE_TAG"; }
};

/// @brief Light target node tag chunk (0xB006) — keyframe animation for a spotlight's look-at target.
///
/// @details May appear any number of times as a child of KFDATA (0xB000),
/// one per spotlight that has an animated target.
///
/// @par Expected children
/// - NODE_ID (0xB030) — optional (R3), ordering override.
/// - NODE_HDR (0xB010) — required, links to the spotlight's target.
/// - POS_TRACK_TAG (0xB020) — target position keys.
class LTargetNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
    bool Process(Importer& importer)  override;
    std::string GetTypeName() const override { return "L_TARGET_NODE_TAG"; }
};

/// @}

// ---------------------------------------------------------------------------
/// @name Float animation track chunks (camera roll/fov, light hotspot/falloff)
/// @{
// ---------------------------------------------------------------------------

/// @brief FOV track chunk (0xB023) — keyframed camera field-of-view angle.
///
/// @details Required inside CAMERA_NODE_TAG. Each key stores the camera's
/// horizontal field-of-view angle in degrees.
///
/// @par Per-key value layout
/// - `float fov` — field-of-view angle in degrees (range: ~0.00025–160).
class FovTrackTagChunk : public TrackTagChunk<float> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "FOV_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<float>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Roll track chunk (0xB024) — keyframed camera or spotlight bank/roll angle.
///
/// @details Required inside CAMERA_NODE_TAG; optional inside SPOTLIGHT_NODE_TAG (R3).
///
/// @par Per-key value layout
/// - `float angle` — roll angle in degrees (−360 to +360).
class RollTrackTagChunk : public TrackTagChunk<float> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "ROLL_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<float>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Hotspot track chunk (0xB027) — keyframed spotlight inner (hotspot) cone angle.
///
/// @details Required inside SPOTLIGHT_NODE_TAG. The hotspot cone is the
/// bright, fully-illuminated inner region of the spotlight.
///
/// @par Per-key value layout
/// - `float angle` — hotspot cone half-angle in degrees (range: 1–160).
///   Must be ≤ the corresponding falloff angle.
class HotTrackTagChunk : public TrackTagChunk<float> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "HOT_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<float>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Falloff track chunk (0xB028) — keyframed spotlight outer (falloff) cone angle.
///
/// @details Required inside SPOTLIGHT_NODE_TAG. The falloff cone marks the
/// outer boundary of the spotlight beam; intensity attenuates from hotspot to falloff edge.
///
/// @par Per-key value layout
/// - `float angle` — falloff cone half-angle in degrees (range: 1–160).
///   Must be ≥ the corresponding hotspot angle.
class FallTrackTagChunk : public TrackTagChunk<float> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "FALL_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<float>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Color track chunk (0xB025) — keyframed RGB light color.
///
/// @details Required inside LIGHT_NODE_TAG and SPOTLIGHT_NODE_TAG. Optionally
/// also used inside AMBIENT_NODE_TAG for ambient light color animation.
///
/// @par Per-key value layout
/// - `float r, float g, float b` — RGB components, range 0.0–1.0.
class ColTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "COL_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @}

/// @brief Object-hidden flag chunk (0x4010) — marks the enclosing NAMED_OBJECT as hidden by default.
///
/// @details Optional; appears at most once inside NAMED_OBJECT (0x4000).
/// No binary data payload (header only, 6 bytes). Only applicable to
/// N_TRI_OBJECT mesh objects.
class ObjHiddenChunk : public Chunk {
public:
    using Chunk::Chunk;
    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "OBJ_HIDDEN"; }
};

/// @brief Spotlight node tag chunk (0xB007) — keyframe animation container for a spotlight object.
///
/// @details May appear any number of times as a child of KFDATA (0xB000).
///
/// @par Expected children
/// - NODE_ID (0xB030) — optional (R3), ordering override.
/// - NODE_HDR (0xB010) — required, links node to spotlight object.
/// - POS_TRACK_TAG (0xB020) — spotlight position keys.
/// - COL_TRACK_TAG (0xB025) — spotlight color keys.
/// - HOT_TRACK_TAG (0xB027) — hotspot cone angle keys.
/// - FALL_TRACK_TAG (0xB028) — falloff cone angle keys.
/// - ROLL_TRACK_TAG (0xB024) — spotlight roll/bank angle keys (R3).
class SpotlightNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "SPOTLIGHT_NODE_TAG"; }
};

/// @brief Ambient node tag chunk (0xB001) — keyframe animation of the scene ambient light color.
///
/// @details Optional (Release 3); may appear at most once per KFDATA (0xB000).
/// The node ID for AMBIENT_NODE_TAG must always be 65535, and the NODE_HDR
/// name must always be `"$AMBIENT$"` with parent index −1.
///
/// @par Expected children
/// - NODE_ID (0xB030) — required, must be 65535.
/// - NODE_HDR (0xB010) — required, name = "$AMBIENT$", parent = −1.
/// - COL_TRACK_TAG (0xB025) — ambient light color keys.
class AmbientNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "AMBIENT_NODE_TAG"; }
};

// ---------------------------------------------------------------------------
/// @name Named-object property flag chunks (MDATA section)
/// @{
// ---------------------------------------------------------------------------

/// @brief Object visible-in-lofter flag chunk (0x4011) — marks the object visible in the 3D Lofter.
///
/// @details Optional; appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
class ObjVisLofterChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_VIS_LOFTER"; }
};

/// @brief Object doesn't-cast-shadows flag chunk (0x4012) — disables shadow casting for this object.
///
/// @details Optional; appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
class ObjDoesntCastChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_DOESNT_CAST"; }
};

/// @brief Object background-matte flag chunk (0x4013) — marks the object as a background matte.
///
/// @details Optional; appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
class ObjMatteChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_MATTE"; }
};

/// @brief Object fast-display flag chunk (0x4014) — enables fast (box) display mode.
///
/// @details Optional (Release 2/3); appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
class ObjFastChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_FAST"; }
};

/// @brief Object procedural flag chunk (0x4015) — marks the object as a procedural stand-in.
///
/// @details Optional (Release 2/3); appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
/// When present, PROC_NAME (0x4181) and PROC_DATA (0x4182) must also be defined.
class ObjProceduralChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_PROCEDURAL"; }
};

/// @brief Object frozen flag chunk (0x4016) — marks the object as frozen (non-interactive).
///
/// @details Optional (Release 2/3); appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only).
class ObjFrozenChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_FROZEN"; }
};

/// @brief Object doesn't-receive-shadows flag chunk (0x4017) — disables shadow reception.
///
/// @details Optional (Release 3); appears at most once per NAMED_OBJECT (0x4000).
/// No binary data payload (6-byte header only). Only applies to N_TRI_OBJECT meshes.
class ObjDontRcvShadowChunk : public Chunk {
public:
    using Chunk::Chunk;
    std::string GetTypeName() const override { return "OBJ_DONT_RCVSHADOW"; }
};

/// @}

} // namespace Debugger3DS::Parser::Chunks
