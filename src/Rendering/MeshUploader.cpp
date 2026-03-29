#include "MeshUploader.h"
#include <iostream>
#include <ranges>
#include <any>
#include <cmath>

namespace Debugger3DS {

std::vector<MeshUploader::MeshEntry> MeshUploader::GetMeshesToRender(const Scene& scene) {
    std::vector<MeshEntry> meshData;

    bool hasValidAssociations = std::ranges::any_of(scene.objectNodes,
        [](const auto& node) { return node->associatedMesh != nullptr; });

    if (scene.objectNodes.empty() || !hasValidAssociations) {
        for (const auto& mesh : scene.meshes) {
            MeshEntry entry;
            mesh->ToEigenMatrices(entry.V, entry.F);
            entry.meshName = mesh->name;
            meshData.push_back(std::move(entry));
        }
    } else {
        for (const auto& node : scene.objectNodes) {
            if (node->associatedMesh) {
                MeshEntry entry;
                Eigen::Matrix4f nodeTransform = scene.GetNodeGlobalTransform(node);
                node->associatedMesh->ToEigenMatrices(entry.V, entry.F, nodeTransform);
                if (node->isReflected)
                    entry.F.col(1).swap(entry.F.col(2));
                entry.node = node;
                entry.meshName = node->associatedMeshName;
                meshData.push_back(std::move(entry));
            }
        }
    }

    return meshData;
}

void MeshUploader::UploadMeshes(Renderer& renderer,
                                 const Scene& scene,
                                 MeshSelector& selector,
                                 std::unordered_map<uint16_t, int>& nodeToDataId,
                                 std::function<void(const std::any&)> selectionCallback) {
    auto meshData = GetMeshesToRender(scene);

    selector.SetSelectionCallback(std::move(selectionCallback));

    size_t uploadedCount = 0;
    for (auto& entry : meshData) {
        if (entry.meshName == "$$$DUMMY")
            continue;

        // Generate a distinct color per mesh
        float r, g, b;
        if (uploadedCount == 0) {
            r = 0.8f; g = 0.8f; b = 0.8f; // first mesh: light gray
        } else {
            r = static_cast<float>(uploadedCount * 0.3 - std::floor(uploadedCount * 0.3));
            g = static_cast<float>(uploadedCount * 0.7 - std::floor(uploadedCount * 0.7));
            b = static_cast<float>(uploadedCount * 0.5 - std::floor(uploadedCount * 0.5));
        }

        uint32_t color = Renderer::PackColor(r, g, b);
        int meshId = renderer.UploadMesh(entry.V, entry.F, color);

        if (entry.node) {
            auto nodeTransform = scene.GetNodeGlobalTransform(entry.node);
            selector.AddMeshWithTransform(meshId, std::any(entry.node), color, entry.meshName,
                                          entry.node->boundingBox.min, entry.node->boundingBox.max,
                                          nodeTransform);
            nodeToDataId[entry.node->nodeId] = meshId;
        } else {
            selector.AddMesh(meshId, std::any{}, color, entry.meshName);
        }

        // Store the original color in the selector
        // (The selector already stored a default — overwrite via its internal vector)
        // This is handled by AddMesh storing the color.

        ++uploadedCount;
    }
}

std::vector<PosColorVertex> MeshUploader::MakeCoordinateAxes(double axisLength) {
    float len = static_cast<float>(axisLength);
    uint32_t red   = Renderer::PackColor(1.0f, 0.0f, 0.0f);
    uint32_t green = Renderer::PackColor(0.0f, 1.0f, 0.0f);
    uint32_t blue  = Renderer::PackColor(0.0f, 0.0f, 1.0f);

    return {
        {0, 0, 0, red},   {len, 0, 0, red},
        {0, 0, 0, green}, {0, len, 0, green},
        {0, 0, 0, blue},  {0, 0, len, blue},
    };
}

} // namespace Debugger3DS
