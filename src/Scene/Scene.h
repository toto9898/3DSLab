#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "ObjectNode.h"
#include "CameraNode.h"
#include "LightNode.h"

namespace Debugger3DS::Scene {

/// @brief Complete in-memory representation of a loaded 3DS scene.
///
/// All data is populated by Importer::Import3DS().  After loading, call
/// BuildObjectNodeHierarchy() and BuildAnimationNodeAssociations() (both are
/// called automatically by the importer) to resolve parent/child and
/// name-based cross-references.
class Scene {
public:
    uint32_t version     = 0;          ///< 3DS file format version.
    uint32_t meshVersion = 0;          ///< Mesh sub-version.
    float masterScale    = 1.0f;       ///< Global scene scale factor.
    std::string basePath;              ///< Directory containing the loaded .3ds file (for texture lookup).
    Eigen::Vector3f ambientLight = Eigen::Vector3f(0.1f, 0.1f, 0.1f); ///< Scene ambient light colour.

    /// @name Keyframe animation
    /// @{
    uint16_t kfRevision      = 0;      ///< KFDATA revision number.
    std::string kfFilename;            ///< Original filename stored in the KFDATA header.
    uint32_t animationLength = 100;    ///< Total animation length in frames.
    uint32_t segmentStart    = 0;      ///< Active segment start frame.
    uint32_t segmentEnd      = 100;    ///< Active segment end frame.
    uint32_t currentFrame    = 0;      ///< Currently displayed frame.
    /// @}

    std::vector<std::shared_ptr<Material>> materials;   ///< All materials in the scene.
    std::vector<std::shared_ptr<Mesh>>     meshes;      ///< All geometry objects.
    std::vector<std::shared_ptr<Light>>    lights;      ///< All light sources.
    std::vector<std::shared_ptr<Camera>>   cameras;     ///< All cameras.
    std::vector<ObjectNodePtr>             objectNodes; ///< Mesh animation nodes from the KFDATA section.

    /// @name Animated camera/light nodes (KFDATA section)
    /// @{
    std::vector<CameraNodePtr>       cameraNodes;       ///< CAMERA_NODE_TAG entries.
    std::vector<LightNodePtr>        lightNodes;        ///< LIGHT_NODE_TAG entries.
    std::vector<CameraTargetNodePtr> cameraTargetNodes; ///< TARGET_NODE_TAG entries.
    std::vector<LightTargetNodePtr>  lightTargetNodes;  ///< L_TARGET_NODE_TAG entries.
    /// @}

    /// @brief Link each ObjectNode to its parent and to its associated Mesh by name.
    void BuildObjectNodeHierarchy();

    /// @brief Link CameraNode/LightNode entries to their Camera/Light objects and target nodes.
    void BuildAnimationNodeAssociations();

    /// @brief Compute the world-space transform of a node at a given frame.
    /// @param node  The node to evaluate.
    /// @param frame Frame number; pass @c 0xFFFFFFFF to use Scene::currentFrame.
    Eigen::Matrix4f GetNodeGlobalTransform(const ObjectNodePtr& node, uint32_t frame = 0xFFFFFFFF) const;

    /// @brief Collect @p nodeId and all descendant node IDs (BFS).
    std::vector<uint16_t> GetDescendantNodeIds(uint16_t nodeId) const;

    /// @name Lookup helpers
    /// @{
    std::shared_ptr<Mesh>       FindMesh(const std::string& name);              ///< Find mesh by name; returns @c nullptr if not found.
    std::shared_ptr<Mesh>       FindMesh(const std::string& name) const;        ///< Find mesh by name (const overload).
    std::shared_ptr<Light>      FindLight(const std::string& name) const;       ///< Find light by name; returns @c nullptr if not found.
    std::shared_ptr<Camera>     FindCamera(const std::string& name) const;      ///< Find camera by name; returns @c nullptr if not found.
    /// @}

    /// @brief Print a human-readable scene summary to @c logging::log.
    void PrintInfo() const;
};

} // namespace Debugger3DS::Scene
