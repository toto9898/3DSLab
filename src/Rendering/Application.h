#pragma once

#include "SceneTreePanel.h"
#include "Window.h"
#include "Renderer.h"
#include "CameraController.h"
#include "MeshSelector.h"
#include "MeshUploader.h"
#include "TextureLoader.h"
#include "CerrRedirect.h"
#include <sstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <Eigen/Dense>

namespace Debugger3DS {

class Application {
public:
    Application() = default;

    // Initialize window, renderer, UI (starts with empty scene)
    void SetupViewer();

    // Run the main loop (blocking)
    void Run();

    // Open a 3DS file: clears previous scene and loads the new one
    void OpenScene(const std::string& filepath);

private:
    Scene scene_;

    Window window_;
    Renderer renderer_;
    CameraController camera_;
    MeshSelector selector_;
    TextureLoader textureLoader_;

    std::unique_ptr<UI::SceneTreePanel> scenePanel_;
    std::unordered_map<uint16_t, int> nodeToDataId_;

    std::vector<PosColorVertex> axisLines_;

    // Previous mouse position for computing deltas
    double prevMouseX_ = 0;
    double prevMouseY_ = 0;
    bool firstMouse_ = true;

    // Interaction settings
    float arcballRotateSpeed_ = 1.0f; // kept for compatibility but not exposed in UI
    // Pivot behavior: by default use selection center when available
    // Debug: show pivot marker in the scene
    bool showPivotMarker_ = false;

    bool LoadScene(const std::string& filepath);
    void UploadScene();
    void ProcessInput();
    void DrawImGui();
    bool sceneLoaded_ = false;

    // Accumulated error messages routed via std::cerr redirect
    std::string errorLog_;
    std::unique_ptr<CerrRedirect> cerrRedirect_;

    // Accumulated log output routed via std::cout redirect
    std::string logBuffer_;
    std::unique_ptr<CoutRedirect> coutRedirect_;
    std::ostringstream loggerStream_;
};

} // namespace Debugger3DS
