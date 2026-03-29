#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include "Importer.h"
#include "SceneTreePanel.h"
#include "MeshSelector.h"
#include "CameraController.h"
#include <string>
#include <memory>
#include <unordered_map>

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
    std::unique_ptr<CameraController> camera_;
    std::unordered_map<uint16_t, int> nodeToDataId_;
};

} // namespace Debugger3DS
