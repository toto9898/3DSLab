#pragma once

#include "Scene.h"
#include <functional>
#include <unordered_set>

namespace Debugger3DS::UI {

using Debugger3DS::Scene::Scene;
using Debugger3DS::Scene::Mesh;
using Debugger3DS::Scene::Material;
using Debugger3DS::Scene::Light;
using Debugger3DS::Scene::Camera;
using Debugger3DS::Scene::ObjectNode;
using Debugger3DS::Scene::ObjectNodePtr;

/// @brief ImGui panel that displays the full 3DS scene hierarchy.
///
/// Shows scene metadata, meshes, materials, lights, cameras, and the animated
/// object node tree.  Supports selection highlighting and programmatic zoom.
class SceneTreePanel {
public:
    /// @param scene Scene to display (held by const reference — the panel does not own the scene).
    explicit SceneTreePanel(const Scene& scene);

    /// @brief Draw the panel inside its own ImGui window.
    void Draw();

    /// @brief Draw only the panel contents, for embedding inside a parent ImGui window.
    void DrawContent();

    /// @brief Highlight @p nodeId and auto-expand all ancestor nodes in the tree.
    void SetSelectedNodeId(uint16_t nodeId);

    /// @brief Remove the current selection highlight.
    void ClearSelection();

    /// @brief Register a callback fired when the user clicks a node.
    /// @param callback Receives the clicked node's ID and all descendant IDs.
    void SetNodeSelectionCallback(std::function<void(const std::vector<uint16_t>&)> callback);

    /// @brief Register a callback fired when the user clicks the zoom button on a node.
    /// @param callback Receives the node's ID and all descendant IDs.
    void SetZoomCallback(std::function<void(const std::vector<uint16_t>&)> callback);

private:
    const Scene& scene_;

    uint16_t selectedNodeId_ = 0xFFFF;
    bool forceOpenHierarchy_ = false;
    bool scrollToSelected_ = false;
    std::unordered_set<uint16_t> nodesToOpen_;
    std::function<void(const std::vector<uint16_t>&)> nodeSelectionCallback_;
    std::function<void(const std::vector<uint16_t>&)> zoomCallback_;

    void DrawSceneInfo();
    void DrawMeshesSection();
    void DrawMaterialsSection();
    void DrawLightsSection();
    void DrawCamerasSection();
    void DrawObjectNodeHierarchy();

    void DrawMeshNode(int index, const std::shared_ptr<Mesh>& mesh);
    void DrawMeshDetails(const std::shared_ptr<Mesh>& mesh, const char* uid);
    void DrawMaterialNode(int index, const std::shared_ptr<Material>& mat);
    void DrawLightNode(int index, const std::shared_ptr<Light>& light);
    void DrawCameraNode(int index, const std::shared_ptr<Camera>& cam);
    void DrawObjectNode(const ObjectNodePtr& node, const std::vector<ObjectNodePtr>& allNodes);
};

} // namespace Debugger3DS::UI
