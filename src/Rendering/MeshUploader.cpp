#include "MeshUploader.h"
#include <iostream>
#include <ranges>
#include <any>

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

void MeshUploader::UploadMeshes(igl::opengl::glfw::Viewer& viewer,
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

        int data_id = (uploadedCount == 0) ? viewer.data().id : viewer.append_mesh();
        viewer.data(data_id).set_mesh(entry.V, entry.F);
        viewer.data(data_id).set_face_based(true);

        if (uploadedCount > 0) {
            Eigen::RowVector3d color;
            color << (uploadedCount * 0.3) - floor(uploadedCount * 0.3),
                     (uploadedCount * 0.7) - floor(uploadedCount * 0.7),
                     (uploadedCount * 0.5) - floor(uploadedCount * 0.5);
            viewer.data(data_id).set_colors(color);
        }

        if (entry.node) {
            auto nodeTransform = scene.GetNodeGlobalTransform(entry.node);
            selector.AddMeshWithTransform(data_id, std::any(entry.node), entry.meshName,
                                          entry.node->boundingBox.min, entry.node->boundingBox.max,
                                          nodeTransform);
            nodeToDataId[entry.node->nodeId] = data_id;
        } else {
            selector.AddMesh(data_id, std::any{}, entry.meshName);
        }
        ++uploadedCount;
    }

    selector.EnableSelection();
}

void MeshUploader::DrawCoordinateAxes(igl::opengl::glfw::Viewer& viewer, double axisLength) {
    Eigen::MatrixXd start(1, 3), end(1, 3);
    start << 0, 0, 0;

    end << axisLength, 0, 0;
    viewer.data().add_edges(start, end, Eigen::RowVector3d(1, 0, 0));

    end << 0, axisLength, 0;
    viewer.data().add_edges(start, end, Eigen::RowVector3d(0, 1, 0));

    end << 0, 0, axisLength;
    viewer.data().add_edges(start, end, Eigen::RowVector3d(0, 0, 1));
}

} // namespace Debugger3DS
