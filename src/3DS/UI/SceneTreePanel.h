#pragma once

#include "../Scene.h"
#include <functional>
#include <unordered_set>

namespace Debugger3DS::UI {

    // Renders a collapsible ImGui window showing the full 3DS scene hierarchy:
    // scene info, meshes, materials, lights, cameras, and the object node tree.
    class SceneTreePanel {
    public:
        explicit SceneTreePanel(const Scene& scene);
        void Draw();
        // Draw just the content (without Begin/End), for embedding in a parent window
        void DrawContent();

        // Highlight a node and auto-expand its ancestors in the tree
        void SetSelectedNodeId(uint16_t nodeId);
        // Clear the current selection highlight
        void ClearSelection();
        // Callback fired when user clicks a node in the tree (passes nodeId)
        void SetNodeSelectionCallback(std::function<void(uint16_t)> callback);
        // Callback fired when user clicks the zoom button on a node
        void SetZoomCallback(std::function<void(uint16_t)> callback);

    private:
        const Scene& scene_;

        uint16_t selectedNodeId_ = 0xFFFF;
        bool forceOpenHierarchy_ = false;
        bool scrollToSelected_ = false;
        std::unordered_set<uint16_t> nodesToOpen_;
        std::function<void(uint16_t)> nodeSelectionCallback_;
        std::function<void(uint16_t)> zoomCallback_;

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
