#pragma once

#include "NamedObject.h"
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;

    // Mesh object — geometry stored in GPU-friendly flat arrays
    class Mesh : public NamedObject {
    public:
        // GPU-ready geometry (flat, contiguous)
        std::vector<Eigen::Vector3f> vertices;
        std::vector<uint16_t>        faceIndices;    // flat triplets: [a0,b0,c0, a1,b1,c1, ...]
        std::vector<Eigen::Vector2f> texCoords;

        // Per-face metadata (parallel arrays, indexed by face number = i/3 of faceIndices)
        std::vector<uint16_t>                   faceFlags;      // edge visibility flags
        std::vector<std::shared_ptr<Material>>  faceMaterials;  // one per face

        std::vector<uint32_t> smoothingGroups;
        Eigen::Matrix4f meshMatrix = Eigen::Matrix4f::Identity();

        // Material assignments with shared pointers (face-index lists per material)
        std::map<std::shared_ptr<Material>, std::vector<uint16_t>> materialGroups;

        Mesh() : NamedObject("Unnamed Mesh") {}
        Mesh(const std::string& meshName) : NamedObject(meshName) {}
        
        // Get bounding box
        std::pair<Eigen::Vector3f, Eigen::Vector3f> GetBoundingBox() const;
        
        // Get face indices with inverted winding (b and c swapped).
        // Lazily computed on first call and cached.
        const std::vector<uint16_t>& GetInvertedWindingIndices() const;

        // Smoothing group methods
        void SetSmoothingGroups(const std::vector<uint32_t>& groups) { smoothingGroups = groups; }
        const std::vector<uint32_t>& GetSmoothingGroups() const { return smoothingGroups; }
        size_t GetFaceCount() const { return faceIndices.size() / 3; }

    private:
        mutable std::vector<uint16_t> invertedWindingIndices_;
    };
    
} // namespace Debugger3DS