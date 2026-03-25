#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include "3DS/Importer.h"
#include "3DS/UI/SceneTreePanel.h"
#include "MeshSelector.h"
#include <string>
#include <memory>

namespace Debugger3DS {

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
    
    // Prepare mesh data from scene (with transforms applied non-destructively)
    std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>> GetMeshesToRender() const;
    
    // Upload meshes to the viewer and register them with the selector
    void UploadMeshes();
    
    // Draw coordinate axes overlay
    void DrawCoordinateAxes(double axisLength = 10.0);
};

} // namespace Debugger3DS
