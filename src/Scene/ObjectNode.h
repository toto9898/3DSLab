#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <Eigen/Dense>
#include "AnimationHeaders.h"

namespace Debugger3DS::Scene {

// Forward declaration
class Mesh;

/// @brief Axis-aligned bounding box (single precision) for a node instance.
struct BoundBox {
    Eigen::Vector3f min; ///< Minimum corner.
    Eigen::Vector3f max; ///< Maximum corner.

    BoundBox() : min(Eigen::Vector3f::Zero()), max(Eigen::Vector3f::Zero()) {}
    /// @param minVal Minimum corner.
    /// @param maxVal Maximum corner.
    BoundBox(const Eigen::Vector3f& minVal, const Eigen::Vector3f& maxVal) : min(minVal), max(maxVal) {}
};

/// @brief A single keyframe value for an animation track of type @p T.
template<typename T>
struct AnimationKey : public KeyHeader {
    T value; ///< The animated value at this keyframe.

    AnimationKey() = default;
    /// @param frame Frame number.
    /// @param val   Keyframe value.
    AnimationKey(uint32_t frame, const T& val) : value(val) {
        this->frame = frame;
    }
};

/// @brief Typed animation track holding a sequence of keyframes.
///
/// Specialisations of GetValueAtFrame() provide interpolation logic for
/// @c Eigen::Vector3f (linear), @c Eigen::Vector4f (axis-angle rotation),
/// @c float (linear), @c bool (step), and @c std::string (step).
template<typename T>
struct AnimationTrack {
    TrackHeader header;                ///< Track metadata (flags, key count).
    std::vector<AnimationKey<T>> keys; ///< Ordered keyframes.

    AnimationTrack() = default;

    uint16_t GetFlags()    const { return header.flags; }    ///< Raw track flags.
    uint32_t GetKeyCount() const { return header.keyCount; } ///< Number of keyframes.

    /// @brief Interpolate or step to the value at the given frame.
    /// @param frame Frame number to evaluate.
    T GetValueAtFrame(uint32_t frame) const;

    /// @return @c true if the track contains at least one keyframe.
    bool HasKeys() const { return !keys.empty(); }

    /// @brief Total duration of the track in frames (last key frame number).
    uint32_t GetDuration() const;
};

// Forward declaration for smart pointer type
struct ObjectNode;
/// @brief Shared pointer alias for ObjectNode.
using ObjectNodePtr = std::shared_ptr<ObjectNode>;

/// @brief One animated object instance from the KFDATA section (OBJ_NODE_TAG).
///
/// After Scene::BuildObjectNodeHierarchy() is called, #parentNode and
/// #associatedMesh are resolved from the scene's node and mesh lists.
struct ObjectNode {
    uint16_t nodeId = 0;              ///< NODE_ID chunk value.
    std::string associatedMeshName;   ///< Mesh name from NODE_HDR (maps to a NAMED_OBJECT).
    uint16_t parentId = 0xFFFF;       ///< Parent node ID from NODE_HDR; 0xFFFF = root.
    ObjectNodePtr parentNode = nullptr; ///< Resolved parent pointer.
    uint16_t nodeFlags = 0;           ///< Raw NODE_HDR flags.

    std::string instanceName;         ///< INSTANCE_NAME chunk value (may differ from #associatedMeshName).

    Eigen::Vector3f pivot;            ///< PIVOT offset in object space.
    BoundBox boundingBox;             ///< BOUNDBOX chunk data.

    /// @name Animation tracks
    /// @{
    AnimationTrack<Eigen::Vector3f> positionTrack; ///< POS_TRACK_TAG — world-space position.
    AnimationTrack<Eigen::Vector4f> rotationTrack; ///< ROT_TRACK_TAG — angle (w) + axis (xyz).
    AnimationTrack<Eigen::Vector3f> scaleTrack;    ///< SCL_TRACK_TAG — non-uniform scale.
    AnimationTrack<std::string>     morphTrack;    ///< MORPH_TRACK_TAG — target mesh name.
    AnimationTrack<bool>            hideTrack;     ///< HIDE_TRACK_TAG — visibility on/off.
    /// @}

    float morphSmooth = 0.0f; ///< MORPH_SMOOTH value.

    std::shared_ptr<Mesh> associatedMesh = nullptr; ///< Resolved mesh pointer.
    Eigen::Matrix4f cachedMeshMatrixInverse = Eigen::Matrix4f::Identity(); ///< Inverse of Mesh::meshMatrix.

    ObjectNode() : pivot(Eigen::Vector3f::Zero()) {}

    /// @brief Compute the local-to-world transform at the given frame.
    Eigen::Matrix4f GetTransformAtFrame(uint32_t frame) const;

    /// @return #instanceName if non-empty, otherwise #associatedMeshName.
    std::string GetEffectiveName() const;

    /// @brief Concatenate the pivot offset into @p transform.
    Eigen::Matrix4f ApplyPivot(const Eigen::Matrix4f& transform) const;

    /// @return @c true if this node has a parent.
    bool HasParent() const { return parentId != 0xFFFF; }

    /// @brief Return a formatted string summarising this node at the given frame.
    std::string PrintInfo(uint32_t frame) const;
};

/// @cond SPECIALIZATIONS
template<>
Eigen::Vector3f AnimationTrack<Eigen::Vector3f>::GetValueAtFrame(uint32_t frame) const;

template<>
Eigen::Vector4f AnimationTrack<Eigen::Vector4f>::GetValueAtFrame(uint32_t frame) const;

template<>
bool AnimationTrack<bool>::GetValueAtFrame(uint32_t frame) const;

template<>
std::string AnimationTrack<std::string>::GetValueAtFrame(uint32_t frame) const;

template<>
float AnimationTrack<float>::GetValueAtFrame(uint32_t frame) const;
/// @endcond

} // namespace Debugger3DS::Scene