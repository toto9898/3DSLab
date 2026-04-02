#include "MeshUploader.h"
#include "Material.h"
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
            entry.verts = mesh->vertices.data();
            entry.numVerts = static_cast<int>(mesh->vertices.size());
            entry.indices = mesh->faceIndices.data();
            entry.numIndices = static_cast<int>(mesh->faceIndices.size());
            entry.meshName = mesh->name;
            entry.sourceMesh = mesh;
            meshData.push_back(std::move(entry));
        }
    } else {
        for (const auto& node : scene.objectNodes) {
            if (node->associatedMesh) {
                MeshEntry entry;
                Eigen::Matrix4f nodeTransform = scene.GetNodeGlobalTransform(node);
                bool nodeReflected = nodeTransform.block<3, 3>(0, 0).determinant() < 0.0f;
                bool meshReflected = node->associatedMesh->meshMatrix.block<3, 3>(0, 0).determinant() < 0.0f;
                bool reflected = nodeReflected != meshReflected;

                entry.verts = node->associatedMesh->vertices.data();
                entry.numVerts = static_cast<int>(node->associatedMesh->vertices.size());
                entry.indices = reflected
                    ? node->associatedMesh->GetInvertedWindingIndices().data()
                    : node->associatedMesh->faceIndices.data();
                entry.numIndices = static_cast<int>(node->associatedMesh->faceIndices.size());
                entry.modelMatrix = nodeTransform;
                entry.node = node;
                entry.meshName = node->associatedMeshName;
                entry.sourceMesh = node->associatedMesh;
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
                                 std::function<void(const std::any&)> selectionCallback,
                                 TextureLoader& textureLoader) {
    auto meshData = GetMeshesToRender(scene);

    selector.SetSelectionCallback(std::move(selectionCallback));

    size_t uploadedCount = 0;
    for (auto& entry : meshData) {
        if (entry.meshName == "$$$DUMMY")
            continue;

        // Extract per-face material properties
        int nFaces = entry.numIndices / 3;
        std::vector<FaceMaterial> faceMats(static_cast<size_t>(nFaces));
        bool hasMaterialColors = false;

        if (entry.sourceMesh && static_cast<int>(entry.sourceMesh->faceMaterials.size()) == nFaces) {
            for (int f = 0; f < nFaces; ++f) {
                const auto& mat = entry.sourceMesh->faceMaterials[static_cast<size_t>(f)];
                if (mat) {
                    hasMaterialColors = true;
                    auto& fm = faceMats[static_cast<size_t>(f)];
                    fm.ambient = mat->ambient;
                    fm.diffuse = mat->diffuse;
                    fm.specular = mat->specular;
                    fm.shininess = mat->shininess;
                    fm.transparency = mat->transparency;
                    fm.selfIllumination = mat->selfIllumination;
                    fm.specularStrength = mat->shininessPercent;
                }
            }
        }

        uint32_t color;
        int meshId;
        if (hasMaterialColors) {
            color = Renderer::PackColor(
                faceMats[0].diffuse.x(), faceMats[0].diffuse.y(), faceMats[0].diffuse.z());

            // Find first material with a texture map
            bgfx::TextureHandle texHandle = BGFX_INVALID_HANDLE;
            if (entry.sourceMesh) {
                for (const auto& mat : entry.sourceMesh->faceMaterials) {
                    if (mat && !mat->textureMap.empty()) {
                        texHandle = textureLoader.LoadTexture(scene.basePath, mat->textureMap);
                        break;
                    }
                }
            }

            if (bgfx::isValid(texHandle) && !entry.sourceMesh->texCoords.empty()) {
                // Build per-vertex UVs: use real UVs for faces whose material has a texture
                // or no material at all (inherit mesh texture). Set sentinel (-1,-1) only for
                // faces explicitly assigned to an untextured material.
                std::vector<Eigen::Vector2f> maskedUVs = entry.sourceMesh->texCoords;
                std::vector<bool> vertexUntextured(maskedUVs.size(), false);
                for (int f = 0; f < nFaces; ++f) {
                    const auto& mat = entry.sourceMesh->faceMaterials[static_cast<size_t>(f)];
                    bool faceIsUntextured = (mat && mat->textureMap.empty());
                    if (faceIsUntextured) {
                        for (int c = 0; c < 3; ++c) {
                            int vi = entry.indices[3 * f + c];
                            vertexUntextured[static_cast<size_t>(vi)] = true;
                        }
                    }
                }
                // A vertex shared between textured and untextured faces keeps its UV
                for (int f = 0; f < nFaces; ++f) {
                    const auto& mat = entry.sourceMesh->faceMaterials[static_cast<size_t>(f)];
                    bool faceIsTextured = (!mat || !mat->textureMap.empty());
                    if (faceIsTextured) {
                        for (int c = 0; c < 3; ++c)
                            vertexUntextured[static_cast<size_t>(entry.indices[3 * f + c])] = false;
                    }
                }
                for (size_t i = 0; i < maskedUVs.size(); ++i) {
                    if (vertexUntextured[i])
                        maskedUVs[i] = Eigen::Vector2f(-1e6f, -1e6f);
                }
                meshId = renderer.UploadMesh(
                    entry.sourceMesh->vertices, entry.indices, entry.numIndices,
                    faceMats, maskedUVs, texHandle, entry.modelMatrix);
            } else {
                meshId = renderer.UploadMesh(
                    entry.sourceMesh->vertices, entry.indices, entry.numIndices,
                    faceMats, entry.modelMatrix);
            }
        } else {
            // Fallback: generate a distinct color per mesh
            float r, g, b;
            if (uploadedCount == 0) {
                r = 0.8f; g = 0.8f; b = 0.8f;
            } else {
                r = static_cast<float>(uploadedCount * 0.3 - std::floor(uploadedCount * 0.3));
                g = static_cast<float>(uploadedCount * 0.7 - std::floor(uploadedCount * 0.7));
                b = static_cast<float>(uploadedCount * 0.5 - std::floor(uploadedCount * 0.5));
            }
            color = Renderer::PackColor(r, g, b);
            meshId = renderer.UploadMesh(
                entry.sourceMesh->vertices, entry.indices, entry.numIndices,
                color, entry.modelMatrix);
        }

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
