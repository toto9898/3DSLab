#pragma once

#include "Importer.h"
#include "SceneTreePanel.h"
#include "Window.h"
#include "Renderer.h"
#include "CameraController.h"
#include "MeshSelector.h"
#include "MeshUploader.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Debugger3DS {

class Application {
public:
    Application() = default;

    // Load a 3DS scene from file
    bool LoadScene(const std::string& filepath);

    // Initialize window, renderer, UI, and mesh rendering
    void SetupViewer();

    // Run the main loop (blocking)
    void Run();

private:
    Importer importer_;
    Scene scene_;

    Window window_;
    Renderer renderer_;
    CameraController camera_;
    MeshSelector selector_;

    std::unique_ptr<UI::SceneTreePanel> scenePanel_;
    std::unordered_map<uint16_t, int> nodeToDataId_;

    std::vector<PosColorVertex> axisLines_;

    // Previous mouse position for computing deltas
    double prevMouseX_ = 0;
    double prevMouseY_ = 0;
    bool firstMouse_ = true;

    void ProcessInput();
    void DrawImGui();
};

} // namespace Debugger3DS
