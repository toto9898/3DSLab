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

namespace Debugger3DS {

class MeshUploader {
public:
    struct MeshEntry {
        const Eigen::Vector3f* verts = nullptr;  // points into Mesh::vertices (non-owning)
        int numVerts = 0;
        const uint16_t* indices = nullptr;        // points into Mesh::faceIndices or inverted cache
        int numIndices = 0;
        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
        ObjectNodePtr node;
        std::string meshName;
        std::shared_ptr<Mesh> sourceMesh;  // original parsed mesh (for materials)
        bool invertedWinding = false;
    };

    // Prepare mesh data from scene (with transforms applied non-destructively)
    static std::vector<MeshEntry> GetMeshesToRender(const Scene& scene);

    // Upload meshes to the renderer, register them with the selector, and populate nodeToDataId.
    static void UploadMeshes(Renderer& renderer,
                             const Scene& scene,
                             MeshSelector& selector,
                             std::unordered_map<uint16_t, int>& nodeToDataId,
                             std::function<void(const std::any&)> selectionCallback,
                             TextureLoader& textureLoader);

    // Create line vertices for coordinate axes
    static std::vector<PosColorVertex> MakeCoordinateAxes(double axisLength = 10.0);
};

} // namespace Debugger3DS
