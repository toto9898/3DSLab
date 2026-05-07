#pragma once

#include "ObjectNode.h"
#include <memory>
#include <string>
#include <cstdint>

namespace Debugger3DS {

    struct Camera;

    // Position track for an animated camera target (from TARGET_NODE_TAG)
    struct CameraTargetNode {
        std::string name;      // matches the parent camera's name
        uint16_t nodeId = 0;
        AnimationTrack<Eigen::Vector3f> positionTrack;
    };
    using CameraTargetNodePtr = std::shared_ptr<CameraTargetNode>;

    // Animated camera node from CAMERA_NODE_TAG in the KFDATA section
    struct CameraNode {
        uint16_t nodeId    = 0;
        uint16_t parentId  = 0xFFFF;  // 0xFFFF = no parent
        std::string cameraName;        // from NODE_HDR — matches Camera::name

        AnimationTrack<Eigen::Vector3f> positionTrack;
        AnimationTrack<float>           rollTrack;  // bank/roll angle (radians)
        AnimationTrack<float>           fovTrack;   // horizontal FOV (degrees)

        // Resolved by Scene::BuildAnimationNodeAssociations()
        std::shared_ptr<Camera> associatedCamera;
        CameraTargetNodePtr     targetNode;         // matching TARGET_NODE_TAG

        bool HasParent() const { return parentId != 0xFFFF; }
    };
    using CameraNodePtr = std::shared_ptr<CameraNode>;

} // namespace Debugger3DS
