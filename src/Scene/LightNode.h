#pragma once

#include "ObjectNode.h"
#include <memory>
#include <string>
#include <cstdint>

namespace Debugger3DS::Scene {

struct Light;

/// @brief Animated position track for a spotlight's target point (L_TARGET_NODE_TAG).
struct LightTargetNode {
    std::string name;      ///< Matches the parent LightNode's `lightName` field.
    uint16_t nodeId = 0;   ///< NODE_ID assigned in the KFDATA section.
    AnimationTrack<Eigen::Vector3f> positionTrack; ///< World-space target position over time.
};
/// @brief Shared pointer to LightTargetNode.
using LightTargetNodePtr = std::shared_ptr<LightTargetNode>;

/// @brief Animated light node from the LIGHT_NODE_TAG block in the KFDATA section.
///
/// After Scene::BuildAnimationNodeAssociations() is called, #associatedLight and
/// #targetNode are resolved from the scene's light and target-node lists.
struct LightNode {
    uint16_t nodeId   = 0;        ///< NODE_ID assigned in the KFDATA section.
    uint16_t parentId = 0xFFFF;  ///< Parent node ID; 0xFFFF means no parent.
    std::string lightName;         ///< From NODE_HDR — matches Light::name.

    AnimationTrack<Eigen::Vector3f> positionTrack; ///< Animated world-space position.
    AnimationTrack<Eigen::Vector3f> colorTrack;    ///< Animated RGB light colour.
    AnimationTrack<float>           hotspotTrack;  ///< Animated hotspot cone half-angle (degrees).
    AnimationTrack<float>           falloffTrack;  ///< Animated falloff cone half-angle (degrees).

    std::shared_ptr<Light> associatedLight; ///< Resolved by Scene::BuildAnimationNodeAssociations().
    LightTargetNodePtr     targetNode;       ///< Matching L_TARGET_NODE_TAG (spotlights only).

    /// @return @c true if this node has a parent node.
    bool HasParent() const { return parentId != 0xFFFF; }
};
/// @brief Shared pointer to LightNode.
using LightNodePtr = std::shared_ptr<LightNode>;

} // namespace Debugger3DS::Scene
