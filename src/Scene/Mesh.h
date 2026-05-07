#pragma once

#include "NamedObject.h"
#include <vector>
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

        // Compact per-face material: index into materialPalette (0xFF = no material)
        std::vector<uint8_t>                    faceMaterialIndices;
        std::vector<std::shared_ptr<Material>>   materialPalette;

        Eigen::Matrix4f meshMatrix = Eigen::Matrix4f::Identity();

        // Static visibility flag (from OBJ_HIDDEN chunk on the NAMED_OBJECT)
        bool isHidden = false;

        Mesh() : NamedObject("Unnamed Mesh") {}
        Mesh(const std::string& meshName) : NamedObject(meshName) {}
        
        // Get bounding box
        std::pair<Eigen::Vector3f, Eigen::Vector3f> GetBoundingBox() const;
        
        // Get face indices with inverted winding (b and c swapped).
        // Lazily computed on first call and cached.
        const std::vector<uint16_t>& GetInvertedWindingIndices() const;

        size_t GetFaceCount() const { return faceIndices.size() / 3; }

        // Material palette helpers
        std::shared_ptr<Material> GetFaceMaterial(size_t faceIdx) const {
            if (faceIdx < faceMaterialIndices.size() && faceMaterialIndices[faceIdx] != 0xFF) {
                return materialPalette[faceMaterialIndices[faceIdx]];
            }
            return nullptr;
        }

        uint8_t GetOrAddMaterialIndex(const std::shared_ptr<Material>& mat) {
            for (uint8_t i = 0; i < static_cast<uint8_t>(materialPalette.size()); ++i) {
                if (materialPalette[i] == mat) return i;
            }
            materialPalette.push_back(mat);
            return static_cast<uint8_t>(materialPalette.size() - 1);
        }

    private:
        mutable std::vector<uint16_t> invertedWindingIndices_;
    };
    
} // namespace Debugger3DS