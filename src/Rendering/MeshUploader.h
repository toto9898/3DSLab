#pragma once

#include "Renderer.h"
#include "Scene.h"
#include "Mesh.h"
#include "MeshSelector.h"
#include "ObjectNode.h"
#include "TextureLoader.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <any>

namespace Debugger3DS::Rendering {

using Debugger3DS::Scene::Scene;
using Debugger3DS::Scene::Mesh;
using Debugger3DS::Scene::ObjectNode;
using Debugger3DS::Scene::ObjectNodePtr;

/// @brief Utility class that converts parsed scene data into GPU-ready mesh entries.
class MeshUploader {
public:
    /// @brief Per-mesh data ready for upload to the GPU.
    struct MeshEntry {
        const uint16_t* indices = nullptr;        ///< Index pointer into Mesh::faceIndices or inverted cache.
        int numIndices = 0;                       ///< Number of indices.
        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity(); ///< Model-to-world transform.
        ObjectNodePtr node;                       ///< Source animation node (may be null).
        std::string meshName;                     ///< Name of the source mesh.
        std::shared_ptr<Mesh> sourceMesh;         ///< Original parsed mesh (for material data).
        bool invertedWinding = false;             ///< @c true if face winding was flipped.
    };

    /// @brief Collect transformed mesh entries from a scene.
    static std::vector<MeshEntry> GetMeshesToRender(const Scene& scene);

    /// @brief Upload meshes to the renderer, register them with the selector, and populate @p nodeToDataId.
    /// @param renderer          Target renderer.
    /// @param scene             Source scene.
    /// @param selector          Mesh selector to register uploaded meshes with.
    /// @param nodeToDataId      Output map from node ID to renderer data ID.
    /// @param selectionCallback Callback invoked when a mesh is selected.
    /// @param textureLoader     Texture loader for material textures.
    static void UploadMeshes(Renderer& renderer,
                             const Scene& scene,
                             MeshSelector& selector,
                             std::unordered_map<uint16_t, int>& nodeToDataId,
                             std::function<void(const std::any&)> selectionCallback,
                             TextureLoader& textureLoader);

    /// @brief Build line vertices for the world-space coordinate axes.
    /// @param axisLength Length of each axis in world units.
    static std::vector<PosColorVertex> MakeCoordinateAxes(double axisLength = 10.0);
};

} // namespace Debugger3DS::Rendering
