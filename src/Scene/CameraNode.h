#pragma once

#include "ObjectNode.h"
#include <memory>
#include <string>
#include <cstdint>

namespace Debugger3DS::Scene {

struct Camera;

/// @brief Animated position track for a camera's look-at target (TARGET_NODE_TAG).
struct CameraTargetNode {
    std::string name;      ///< Matches the parent CameraNode's `cameraName` field.
    uint16_t nodeId = 0;   ///< NODE_ID assigned in the KFDATA section.
    AnimationTrack<Eigen::Vector3f> positionTrack; ///< World-space target position over time.
};
/// @brief Shared pointer to CameraTargetNode.
using CameraTargetNodePtr = std::shared_ptr<CameraTargetNode>;

/// @brief Animated camera node from the CAMERA_NODE_TAG block in the KFDATA section.
///
/// After Scene::BuildAnimationNodeAssociations() is called, #associatedCamera and
/// #targetNode are resolved from the scene's camera and target-node lists.
struct CameraNode {
    uint16_t nodeId    = 0;         ///< NODE_ID assigned in the KFDATA section.
    uint16_t parentId  = 0xFFFF;   ///< Parent node ID; 0xFFFF means no parent.
    std::string cameraName;         ///< From NODE_HDR — matches Camera::name.

    AnimationTrack<Eigen::Vector3f> positionTrack; ///< Animated world-space position.
    AnimationTrack<float>           rollTrack;     ///< Bank/roll angle in radians.
    AnimationTrack<float>           fovTrack;      ///< Horizontal field of view in degrees.

    std::shared_ptr<Camera> associatedCamera; ///< Resolved by Scene::BuildAnimationNodeAssociations().
    CameraTargetNodePtr     targetNode;        ///< Matching TARGET_NODE_TAG, resolved at build time.

    /// @return @c true if this node has a parent node.
    bool HasParent() const { return parentId != 0xFFFF; }
};
/// @brief Shared pointer to CameraNode.
using CameraNodePtr = std::shared_ptr<CameraNode>;

} // namespace Debugger3DS::Scene
