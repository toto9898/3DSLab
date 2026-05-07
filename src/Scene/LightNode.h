#pragma once

#include "ObjectNode.h"
#include <memory>
#include <string>
#include <cstdint>

namespace Debugger3DS {

    struct Light;

    // Position track for an animated spotlight target (from L_TARGET_NODE_TAG)
    struct LightTargetNode {
        std::string name;      // matches the parent light's name
        uint16_t nodeId = 0;
        AnimationTrack<Eigen::Vector3f> positionTrack;
    };
    using LightTargetNodePtr = std::shared_ptr<LightTargetNode>;

    // Animated light node from LIGHT_NODE_TAG in the KFDATA section
    struct LightNode {
        uint16_t nodeId   = 0;
        uint16_t parentId = 0xFFFF;  // 0xFFFF = no parent
        std::string lightName;        // from NODE_HDR — matches Light::name

        AnimationTrack<Eigen::Vector3f> positionTrack;
        AnimationTrack<Eigen::Vector3f> colorTrack;    // animated RGB color
        AnimationTrack<float>           hotspotTrack;  // animated hotspot cone angle (degrees)
        AnimationTrack<float>           falloffTrack;  // animated falloff cone angle (degrees)

        // Resolved by Scene::BuildAnimationNodeAssociations()
        std::shared_ptr<Light> associatedLight;
        LightTargetNodePtr     targetNode;  // matching L_TARGET_NODE_TAG (spotlight only)

        bool HasParent() const { return parentId != 0xFFFF; }
    };
    using LightNodePtr = std::shared_ptr<LightNode>;

} // namespace Debugger3DS
