#pragma once

#include "NamedObject.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <Eigen/Dense>

namespace Debugger3DS::Scene {

// Forward declarations
struct Material;

/// @brief Triangle mesh geometry stored in GPU-friendly flat arrays.
///
/// Vertices, face indices, texture coordinates and per-face material assignments
/// are populated by the parser.  The renderer uploads these directly to bgfx buffers.
class Mesh : public NamedObject {
public:
    std::vector<Eigen::Vector3f> vertices;     ///< Vertex positions.
    std::vector<uint16_t>        faceIndices;  ///< Flat triplets: [a0,b0,c0, a1,b1,c1, …]
    std::vector<Eigen::Vector2f> texCoords;    ///< UV coordinates, one per vertex.

    /// @brief Per-face material index into #materialPalette. 0xFF means no material.
    std::vector<uint8_t>                   faceMaterialIndices;
    std::vector<std::shared_ptr<Material>> materialPalette; ///< Materials used by this mesh.

    Eigen::Matrix4f meshMatrix = Eigen::Matrix4f::Identity(); ///< Object-space transform baked into the geometry.

    /// @brief Static visibility flag set by an OBJ_HIDDEN chunk on the NAMED_OBJECT.
    bool isHidden = false;

    Mesh() : NamedObject("Unnamed Mesh") {}
    /// @param meshName Mesh name.
    Mesh(const std::string& meshName) : NamedObject(meshName) {}

    /// @brief Compute the axis-aligned bounding box of the vertex positions.
    /// @return Pair of (min, max) corners.
    std::pair<Eigen::Vector3f, Eigen::Vector3f> GetBoundingBox() const;

    /// @brief Return face indices with winding order reversed (b and c swapped).
    /// Result is lazily computed on first call and cached.
    const std::vector<uint16_t>& GetInvertedWindingIndices() const;

    /// @brief Number of triangles.
    size_t GetFaceCount() const { return faceIndices.size() / 3; }

    /// @brief Look up the material assigned to a face.
    /// @param faceIdx Zero-based face index.
    /// @return Shared pointer to the material, or @c nullptr if unassigned.
    std::shared_ptr<Material> GetFaceMaterial(size_t faceIdx) const {
        if (faceIdx < faceMaterialIndices.size() && faceMaterialIndices[faceIdx] != 0xFF) {
            return materialPalette[faceMaterialIndices[faceIdx]];
        }
        return nullptr;
    }

    /// @brief Return the palette index for @p mat, inserting it if not already present.
    /// @param mat Material to look up or add.
    /// @return Index in #materialPalette (fits in a @c uint8_t).
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

} // namespace Debugger3DS::Scene