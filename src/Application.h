#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include "3DS/Importer.h"
#include "3DS/UI/SceneTreePanel.h"
#include "MeshSelector.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace Debugger3DS {

struct CameraAnimation {
    bool active = false;
    float startZoom = 1.0f;
    float targetZoom = 1.0f;
    Eigen::Vector3f startTranslation = Eigen::Vector3f::Zero();
    Eigen::Vector3f targetTranslation = Eigen::Vector3f::Zero();
    float duration = 0.4f; // seconds
    std::chrono::steady_clock::time_point startTime;
};

class Application {
public:
    Application() = default;
    
    // Load a 3DS scene from file
    bool LoadScene(const std::string& filepath);
    
    // Initialize viewer, UI, and mesh rendering
    void SetupViewer();
    
    // Run the viewer event loop (blocking)
    void Run();
    
private:
    Importer importer_;
    Scene scene_;
    
    igl::opengl::glfw::Viewer viewer_;
    igl::opengl::glfw::imgui::ImGuiPlugin imguiPlugin_;
    igl::opengl::glfw::imgui::ImGuiMenu imguiMenu_;
    std::unique_ptr<UI::SceneTreePanel> scenePanel_;
    std::unique_ptr<MeshSelector> selector_;
    std::unordered_map<uint16_t, int> nodeToDataId_;
    CameraAnimation cameraAnim_;
    
    struct MeshEntry {
        Eigen::MatrixXd V;
        Eigen::MatrixXi F;
        ObjectNodePtr node;       // null when falling back to raw meshes
        std::string meshName;
    };

    // Prepare mesh data from scene (with transforms applied non-destructively)
    std::vector<MeshEntry> GetMeshesToRender() const;
    
    // Upload meshes to the viewer and register them with the selector
    void UploadMeshes();
    
    // Draw coordinate axes overlay
    void DrawCoordinateAxes(double axisLength = 10.0);
    
    // Start a smooth camera animation to fit a node's mesh
    void ZoomToNode(uint16_t nodeId);
    // Zoom to fit multiple nodes
    void ZoomToNodes(const std::vector<uint16_t>& nodeIds);
};

} // namespace Debugger3DS
