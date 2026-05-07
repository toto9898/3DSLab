#pragma once

#include "Chunk.h"
#include "ContainerChunks.h"
#include "ObjectNode.h"
#include <cstdint>
#include <Eigen/Dense>
#include <vector>

namespace Debugger3DS::Parser::Chunks {

using Debugger3DS::Scene::AnimationTrack;
using Debugger3DS::Scene::AnimationKey;

/// @brief Object node tag chunk (0xB002) — top-level container for one animated mesh object node.
///
/// @details May appear any number of times as a child of KFDATA (0xB000).
/// Creates a new `ObjectNode` and makes it the current animation node in the
/// importer. Child chunks populate its track data, node ID, bounding box, etc.
///
/// @par Expected children
/// - NODE_HDR (0xB010) — required, links node to scene object.
/// - NODE_ID (0xB030) — optional (R3), overrides sequential node ordering.
/// - PIVOT (0xB013) — required, rotation/scale pivot point.
/// - BOUNDBOX (0xB014) — required, local AABB.
/// - INSTANCE_NAME (0xB011) — optional, per-instance name override.
/// - POS_TRACK_TAG (0xB020), ROT_TRACK_TAG (0xB021), SCL_TRACK_TAG (0xB022) — animation tracks.
/// - MORPH_TRACK_TAG (0xB026), HIDE_TRACK_TAG (0xB029) — optional animation tracks.
/// - MORPH_SMOOTH (0xB015) — optional smoothing angle for morph targets.
class ObjectNodeTagChunk : public Chunk {
public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    bool Process(Importer& importer)  override;
    std::string GetTypeName() const override { return "OBJECT_NODE_TAG"; }
};

/// @brief Node ID chunk (0xB030) — assigns an explicit ordering index to the current animation node.
///
/// @details Optional (Release 3); appears at most once per node tag chunk.
/// Overrides the implicit sequential node order used to resolve parent
/// references in NODE_HDR. AMBIENT_NODE_TAG nodes must always use ID 65535.
///
/// @par Binary layout
/// - `uint16_t nodeId` — explicit node index (0–65534; 65535 reserved for ambient nodes).
class NodeIdChunk : public Chunk {
private:
    uint16_t nodeId_ = 0; ///< Node ID assigned by the 3DS authoring tool.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "NODE_ID"; }
    std::string GetInfo()     const override;
};

/// @brief Instance name chunk (0xB011) — per-instance name for a duplicated (instanced) object.
///
/// @details Optional; appears at most once per OBJECT_NODE_TAG. Used to give a
/// unique name to one specific instance of a mesh object when the same mesh is
/// used multiple times. Instance names must be globally unique across the file.
///
/// @par Binary layout
/// - `string instanceName` — NUL-terminated, 1–10 characters + NUL terminator.
class InstanceNameChunk : public Chunk {
private:
    std::string instanceName_; ///< Override name for this instance.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "INSTANCE_NAME"; }
    std::string GetInfo()     const override;
};

/// @brief Bounding box chunk (0xB014) — local-space axis-aligned bounding box for the current node.
///
/// @details Required; appears exactly once per OBJECT_NODE_TAG. The bounding
/// box is expressed as min/max corners in object-local space (as offsets from
/// the local origin). Total chunk size is 30 bytes (6-byte header + 2×12 bytes).
///
/// @par Binary layout
/// - `float[3] min` — minimum corner (X, Y, Z).
/// - `float[3] max` — maximum corner (X, Y, Z).
class BoundBoxChunk : public Chunk {
private:
    Eigen::Vector3f min_; ///< Minimum corner.
    Eigen::Vector3f max_; ///< Maximum corner.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "BOUNDBOX"; }
    std::string GetInfo()     const override;
};

/// @brief Abstract base for all animation track chunks.
///
/// @details Each concrete track chunk stores a `TrackHeader` followed by
/// N key records, where N is given by the track header's key count field.
///
/// @par TrackHeader layout (14 bytes)
/// - `uint16_t flags` — loop and axis flags:
///   - Bit 0 (0x001) — loop/duplicate keys.
///   - Bit 1 (0x002) — smooth last key to first key.
///   - Bits 3–5 (0x038) — lock X/Y/Z axes.
///   - Bits 8–10 (0x700) — unlink X/Y/Z axes.
/// - `uint32_t reserved1`, `uint32_t reserved2` — not used; both zero.
/// - `uint32_t keyCount` — number of keyframes in this track.
///
/// @par KeyHeader layout (variable, 6–26 bytes)
/// Each key begins with:
/// - `int32_t  frame`   — frame number where the key occurs.
/// - `uint16_t flags`   — spline term presence bits:
///   - Bit 0 = tension, Bit 1 = continuity, Bit 2 = bias, Bit 3 = ease-to, Bit 4 = ease-from.
/// Followed by the present spline floats in bit order (tension, continuity, bias, ease-to, ease-from).
/// Each optional float is in the range [−1.0, 1.0] (ease-to/from: [0.0, 1.0]).
///
/// @tparam T  Value type stored at each keyframe (e.g. `Eigen::Vector3f`, `float`, `bool`).
template<typename T>
class TrackTagChunk : public Chunk {
protected:
    AnimationTrack<T> track_; ///< Accumulated keyframe data.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override final;
    std::string GetInfo() const override;

    /// @brief Read one keyframe's value into @p key after its `KeyHeader` has been read.
    virtual bool ReadKeyData(AnimationKey<T>& key) = 0;

    /// @brief Push the completed `track_` into the appropriate field of the importer.
    virtual void SetTrackInImporter(Importer& importer) = 0;

    /// @brief Return a read-only reference to the completed track.
    const AnimationTrack<T>& GetTrack() const { return track_; }
};

/// @brief Position track chunk (0xB020) — keyframed world-space position for the current node.
///
/// @details Required; appears exactly once per OBJECT_NODE_TAG. Each key value
/// is a world-space XYZ position.
///
/// @par Per-key value layout
/// - `float x, float y, float z` — position in global coordinates.
class PosTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "POS_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Rotation track chunk (0xB021) — keyframed axis-angle rotation for the current node.
///
/// @details Required; appears exactly once per OBJECT_NODE_TAG. Each key value
/// is an axis-angle representation: rotation angle followed by the rotation axis vector.
///
/// @par Per-key value layout
/// - `float angle` — rotation angle in degrees.
/// - `float ax, float ay, float az` — unit vector of the rotation axis.
class RotTrackTagChunk : public TrackTagChunk<Eigen::Vector4f> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "ROT_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<Eigen::Vector4f>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Scale track chunk (0xB022) — keyframed per-axis scale for the current node.
///
/// @details Required; appears exactly once per OBJECT_NODE_TAG.
///
/// @par Per-key value layout
/// - `float sx, float sy, float sz` — scale factors along X, Y, Z.
class SclTrackTagChunk : public TrackTagChunk<Eigen::Vector3f> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "SCL_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<Eigen::Vector3f>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Morph track chunk (0xB026) — keyframed target-mesh name for morph animation.
///
/// @details Optional; appears at most once per OBJECT_NODE_TAG. Each key value
/// is the name of the mesh to morph to at that frame. The target mesh must have
/// the same vertex count as the source mesh.
///
/// @par Per-key value layout
/// - `string targetName` — NUL-terminated name (1–10 characters + NUL) of the target mesh.
class MorphTrackTagChunk : public TrackTagChunk<std::string> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "MORPH_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<std::string>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Hide track chunk (0xB029) — keyframed visibility toggle for the current node.
///
/// @details Optional (Release 3); appears at most once per OBJECT_NODE_TAG.
/// Keys mark frames at which the object's visibility toggles. No additional
/// per-key value is stored beyond the KeyHeader.
class HideTrackTagChunk : public TrackTagChunk<bool> {
public:
    using TrackTagChunk::TrackTagChunk;
    std::string GetTypeName() const override { return "HIDE_TRACK_TAG"; }
    bool ReadKeyData(AnimationKey<bool>& key) override;
    void SetTrackInImporter(Importer& importer) override;
};

/// @brief Morph-smooth chunk (0xB015) — smoothing angle threshold for morph-target transitions.
///
/// @details Optional; appears at most once per OBJECT_NODE_TAG. Edges whose
/// dihedral angle exceeds this value are treated as hard creases during morphing.
///
/// @par Binary layout
/// - `float angle` — smoothing threshold angle in degrees (0.0–180.0).
class MorphSmoothChunk : public Chunk {
private:
    float morphSmooth_ = 0.0f; ///< Smoothing threshold (degrees).

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "MORPH_SMOOTH"; }
    std::string GetInfo()     const override;
};

/// @brief Pivot chunk (0xB013) — pivot point for rotation and scaling of the current node.
///
/// @details Required; appears exactly once per OBJECT_NODE_TAG. The pivot is
/// specified as an offset from the BOUNDBOX center and is used as the center
/// of rotation and scaling during animation playback.
///
/// @par Binary layout
/// - `float[3] offset` — X, Y, Z offset from the bounding box center.
class PivotChunk : public Chunk {
private:
    Eigen::Vector3f pivot_; ///< Pivot position.

public:
    using Chunk::Chunk;

    bool ReadData(Importer& importer) override;
    std::string GetTypeName() const override { return "PIVOT"; }
    std::string GetInfo()     const override;

    /// @brief Return the parsed pivot position.
    const Eigen::Vector3f& GetPivot() const { return pivot_; }
};

} // namespace Debugger3DS::Parser::Chunks