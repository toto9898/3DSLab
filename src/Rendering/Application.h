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

namespace Debugger3DS::Rendering {

using Debugger3DS::Scene::Scene;
using Debugger3DS::UI::SceneTreePanel;

/// @brief Top-level application object that owns the window, renderer, camera, and UI.
///
/// Manages the main loop and scene loading lifecycle.
class Application {
public:
    Application() = default;

    /// @brief Initialize the window, renderer, and UI (starts with an empty scene).
    void SetupViewer();

    /// @brief Run the main loop (blocking until the window is closed).
    void Run();

    /// @brief Open a 3DS file: clears the previous scene and loads the new one.
    /// @param filepath Path to the .3ds file.
    void OpenScene(const std::string& filepath);

private:
    Scene scene_;

    Window window_;
    Renderer renderer_;
    CameraController camera_;
    MeshSelector selector_;
    TextureLoader textureLoader_;

    std::unique_ptr<SceneTreePanel> scenePanel_;
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
    bool scrollErrorsToBottom_ = false;

    // Accumulated log output routed via std::cout redirect
    std::string logBuffer_;
    std::unique_ptr<CoutRedirect> coutRedirect_;
    bool scrollOutputToBottom_ = false;

    std::ostringstream loggerStream_;
    bool scrollLoggerToBottom_ = false;
};

} // namespace Debugger3DS::Rendering
