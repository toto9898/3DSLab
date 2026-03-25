#pragma once

#include "../Scene.h"

namespace Debugger3DS::UI {

    // Renders a collapsible ImGui window showing the full 3DS scene hierarchy:
    // scene info, meshes, materials, lights, cameras, and the object node tree.
    class SceneTreePanel {
    public:
        explicit SceneTreePanel(const Scene& scene);
        void Draw();

    private:
        const Scene& scene_;

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
