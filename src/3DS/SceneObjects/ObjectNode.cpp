#include <iostream>
#include <cmath>
#include "ObjectNode.h"
#include "Mesh.h"
#include "../Logger.h"

namespace Debugger3DS {

    Eigen::Matrix4f ObjectNode::GetTransformAtFrame(uint32_t frame) const {
        // Get animated values for this frame
        Eigen::Vector3f position = positionTrack.HasKeys() ? 
            positionTrack.GetValueAtFrame(frame) : Eigen::Vector3f::Zero();
        
        Eigen::Vector4f rotation = rotationTrack.HasKeys() ? 
            rotationTrack.GetValueAtFrame(frame) : Eigen::Vector4f(0, 0, 0, 1);
        
        Eigen::Vector3f scale = scaleTrack.HasKeys() ? 
            scaleTrack.GetValueAtFrame(frame) : Eigen::Vector3f::Ones();
        
        if (associatedMeshName == "Arch27_012" || associatedMeshName == "Arch27_014" || associatedMeshName == "Arch27_015") {
            std::cout << "\n=== ObjectNode for " << associatedMeshName << " ===" << std::endl;
            std::cout << "Position: " << position.transpose() << std::endl;
            std::cout << "Rotation (axis, angle): " << rotation.transpose() << std::endl;
            std::cout << "Scale: " << scale.transpose() << std::endl;
            std::cout << "Pivot: " << pivot.transpose() << std::endl;
        }
        
        // Scale
        Eigen::Matrix4f scaleMatrix = Eigen::Matrix4f::Identity();
        scaleMatrix(0, 0) = scale.x();
        scaleMatrix(1, 1) = scale.y();
        scaleMatrix(2, 2) = scale.z();
        
        Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
        if (rotation.head<3>().norm() > 0.001f) {
            Eigen::AngleAxisf angleAxis(-rotation.w(), rotation.head<3>().normalized());
            rotationMatrix.block<3, 3>(0, 0) = angleAxis.toRotationMatrix();
        }
        
        Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
        translationMatrix.block<3, 1>(0, 3) = position;
        
        // Correct transformation order: T * R * S * (-Pivot)
        // This rotates and scales around the pivot point, then translates to final position
        if (!pivot.isZero()) {
            Eigen::Matrix4f negativePivotMatrix = Eigen::Matrix4f::Identity();
            negativePivotMatrix.block<3, 1>(0, 3) = -pivot;
            return translationMatrix * rotationMatrix * scaleMatrix * negativePivotMatrix;
        }
        
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    std::string ObjectNode::GetEffectiveName() const {
        return instanceName.empty() ? associatedMeshName : instanceName;
    }

    Eigen::Matrix4f ObjectNode::ApplyPivot(const Eigen::Matrix4f& transform) const {
        if (pivot.isZero()) {
            return transform;
        }
        
        Eigen::Matrix4f negativePivotMatrix = Eigen::Matrix4f::Identity();
        negativePivotMatrix.block<3, 1>(0, 3) = -pivot;
        
        return transform * negativePivotMatrix;
    }

    std::string ObjectNode::PrintInfo(uint32_t frame) const {
        std::ostringstream oss;
        oss << "  Object Node:" << std::endl;
        oss << "    Effective Name: " << GetEffectiveName() << std::endl;
        oss << "    Associated Mesh: " << associatedMeshName << std::endl;
        if (!instanceName.empty()) {
            oss << "    Instance Name: " << instanceName << std::endl;
        }
        oss << "    Node ID: " << nodeId << std::endl;
        oss << "    Parent Index: " << (HasParent() ? std::to_string(parentIndex) : "None") << std::endl;
        oss << "    Pivot: (" << pivot.x() << ", " << pivot.y() << ", " << pivot.z() << ")" << std::endl;
        oss << "    Frame: " << frame << std::endl;
        
        oss << "    Animation Tracks:" << std::endl;
        if (positionTrack.HasKeys()) {
            Eigen::Vector3f pos = positionTrack.GetValueAtFrame(frame);
            oss << "      Position: " << positionTrack.keys.size() << " keys, @frame " << frame << ": (" 
                << pos.x() << ", " << pos.y() << ", " << pos.z() << ")" << std::endl;
        }
        if (rotationTrack.HasKeys()) {
            Eigen::Vector4f rot = rotationTrack.GetValueAtFrame(frame);
            oss << "      Rotation: " << rotationTrack.keys.size() << " keys, @frame " << frame << ": angle=" 
                << rot.w() << ", axis=(" << rot.x() << ", " << rot.y() << ", " << rot.z() << ")" << std::endl;
        }
        if (scaleTrack.HasKeys()) {
            Eigen::Vector3f scl = scaleTrack.GetValueAtFrame(frame);
            oss << "      Scale: " << scaleTrack.keys.size() << " keys, @frame " << frame << ": (" 
                << scl.x() << ", " << scl.y() << ", " << scl.z() << ")" << std::endl;
        }
        if (morphTrack.HasKeys()) {
            std::string morph = morphTrack.GetValueAtFrame(frame);
            oss << "      Morph: " << morphTrack.keys.size() << " keys, @frame " << frame << ": " 
                << morph << std::endl;
        }
        if (hideTrack.HasKeys()) {
            bool hidden = hideTrack.GetValueAtFrame(frame);
            oss << "      Hide: " << hideTrack.keys.size() << " keys, @frame " << frame << ": " 
                << (hidden ? "true" : "false") << std::endl;
        }
        
        return oss.str();
    }

    // Template specializations for GetValueAtFrame
    template<>
    Eigen::Vector3f AnimationTrack<Eigen::Vector3f>::GetValueAtFrame(uint32_t frame) const {
        if (keys.empty()) {
            return Eigen::Vector3f::Zero();
        }
        
        // Single key
        if (keys.size() == 1 || frame <= keys[0].frame) {
            return keys[0].value;
        }
        
        // After last key
        if (frame >= keys.back().frame) {
            return keys.back().value;
        }
        
        // Find interpolation range
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (frame >= keys[i].frame && frame <= keys[i + 1].frame) {
                float t = (frame - keys[i].frame) / (keys[i + 1].frame - keys[i].frame);
                return keys[i].value + t * (keys[i + 1].value - keys[i].value);
            }
        }
        
        return keys[0].value;
    }

    template<>
    Eigen::Vector4f AnimationTrack<Eigen::Vector4f>::GetValueAtFrame(uint32_t frame) const {
        // Find exact frame match
        for (size_t i = 0; i < keys.size(); ++i) {
            if (frame == keys[i].frame) {
                return keys[i].value;
            }
        }
        
        // Fallback if frame not found
        return keys.empty() ? Eigen::Vector4f(0, 0, 0, 1) : keys[0].value;
    }

    template<>
    bool AnimationTrack<bool>::GetValueAtFrame(uint32_t frame) const {
        if (keys.empty()) {
            return false;
        }
        
        // Find the key at or before this frame
        for (int i = keys.size() - 1; i >= 0; --i) {
            if (frame >= keys[i].frame) {
                return keys[i].value;
            }
        }
        
        return keys[0].value;
    }

    template<>
    std::string AnimationTrack<std::string>::GetValueAtFrame(uint32_t frame) const {
        if (keys.empty()) {
            return "";
        }
        
        // Find the key at or before this frame
        for (int i = keys.size() - 1; i >= 0; --i) {
            if (frame >= keys[i].frame) {
                return keys[i].value;
            }
        }
        
        return keys[0].value;
    }

    template<>
    uint32_t AnimationTrack<Eigen::Vector3f>::GetDuration() const {
        return keys.empty() ? 0 : keys.back().frame - keys[0].frame;
    }

    template<>
    uint32_t AnimationTrack<Eigen::Vector4f>::GetDuration() const {
        return keys.empty() ? 0 : keys.back().frame - keys[0].frame;
    }

    template<>
    uint32_t AnimationTrack<bool>::GetDuration() const {
        return keys.empty() ? 0 : keys.back().frame - keys[0].frame;
    }

    template<>
    uint32_t AnimationTrack<std::string>::GetDuration() const {
        return keys.empty() ? 0 : keys.back().frame - keys[0].frame;
    }

} // namespace Debugger3DS