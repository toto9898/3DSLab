#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <Eigen/Dense>
#include "../Animation/AnimationHeaders.h"

namespace Debugger3DS {

    // Forward declaration
    class Mesh;

    // Bounding box structure
    struct BoundBox {
        Eigen::Vector3f min;
        Eigen::Vector3f max;
        
        BoundBox() : min(Eigen::Vector3f::Zero()), max(Eigen::Vector3f::Zero()) {}
        BoundBox(const Eigen::Vector3f& minVal, const Eigen::Vector3f& maxVal) : min(minVal), max(maxVal) {}
    };

    // Animation keyframe for different track types
    template<typename T>
    struct AnimationKey : public KeyHeader {
        T value;               // The actual animated value
        
        AnimationKey() = default;
        AnimationKey(uint32_t frame, const T& val) : value(val) {
            this->frame = frame;
        }
    };

    // Animation track for a specific property
    template<typename T>
    struct AnimationTrack {
        TrackHeader header;                    // Track header with flags and key count
        std::vector<AnimationKey<T>> keys;     // Keyframes
        
        AnimationTrack() = default;
        
        // Convenience accessors to header data
        uint16_t GetFlags() const { return header.flags; }
        uint32_t GetKeyCount() const { return header.keyCount; }
        
        // Get interpolated value at specific frame
        T GetValueAtFrame(uint32_t frame) const;
        
        // Check if track has any keys
        bool HasKeys() const { return !keys.empty(); }
        
        // Get track duration
        uint32_t GetDuration() const;
    };

    // Object node representing one animated object instance
    struct ObjectNode {
        // Node identification
        uint16_t nodeId = 0;              // NODE_ID
        std::string associatedMeshName;   // From NODE_HDR - name of the NAMED_OBJECT mesh
        uint16_t parentIndex = 0xFFFF;    // From NODE_HDR - parent node index (0xFFFF = no parent)
        uint16_t nodeFlags = 0;           // From NODE_HDR
        
        // Instance information
        std::string instanceName;         // INSTANCE_NAME (if different from associatedMeshName)
        
        // Geometric data
        Eigen::Vector3f pivot;            // PIVOT point
        BoundBox boundingBox;             // BOUNDBOX
        
        // Animation tracks
        AnimationTrack<Eigen::Vector3f> positionTrack;    // POS_TRACK_TAG
        AnimationTrack<Eigen::Vector4f> rotationTrack;    // ROT_TRACK_TAG (angle + axis)
        AnimationTrack<Eigen::Vector3f> scaleTrack;       // SCL_TRACK_TAG
        AnimationTrack<std::string> morphTrack;           // MORPH_TRACK_TAG
        AnimationTrack<bool> hideTrack;                   // HIDE_TRACK_TAG
        
        // Morph smoothing
        float morphSmooth = 0.0f;         // MORPH_SMOOTH
        
        // Associated mesh reference (resolved at runtime)
        std::shared_ptr<Mesh> associatedMesh = nullptr;   // Link to the NAMED_OBJECT mesh
        
        // Constructor
        ObjectNode() : pivot(Eigen::Vector3f::Zero()) {}
        
        // Get transformation matrix at specific frame
        Eigen::Matrix4f GetTransformAtFrame(uint32_t frame) const;
        
        // Get the effective name (instanceName if set, otherwise associatedMeshName)
        std::string GetEffectiveName() const;
        
        // Apply pivot transformation
        Eigen::Matrix4f ApplyPivot(const Eigen::Matrix4f& transform) const;
        
        // Check if this node has a parent
        bool HasParent() const { return parentIndex != 0xFFFF; }
        
        // Print node information for debugging
        std::string PrintInfo(uint32_t frame) const;
    };

    // Template specializations for common animation track operations
    template<>
    Eigen::Vector3f AnimationTrack<Eigen::Vector3f>::GetValueAtFrame(uint32_t frame) const;
    
    template<>
    Eigen::Vector4f AnimationTrack<Eigen::Vector4f>::GetValueAtFrame(uint32_t frame) const;
    
    template<>
    bool AnimationTrack<bool>::GetValueAtFrame(uint32_t frame) const;
    
    template<>
    std::string AnimationTrack<std::string>::GetValueAtFrame(uint32_t frame) const;

} // namespace Debugger3DS