#include "Application.h"
#include "Importer.h"
#include "ObjectNode.h"
#include "Logger.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_bgfx.h>
#include <iostream>
#include <any>

namespace {

void SnapWindowToEdges(float snapDist = 20.0f) {
    ImVec2 pos  = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 disp = ImGui::GetIO().DisplaySize;
    bool snapped = false;
    float sx = pos.x, sy = pos.y;
    if      (pos.x < snapDist)                    { sx = 0; snapped = true; }
    else if (pos.x + size.x > disp.x - snapDist) { sx = disp.x - size.x; snapped = true; }
    if      (pos.y < snapDist)                    { sy = 0; snapped = true; }
    else if (pos.y + size.y > disp.y - snapDist)  { sy = disp.y - size.y; snapped = true; }
    if (snapped) ImGui::SetWindowPos(ImVec2(sx, sy));
}

} // anonymous namespace

namespace Debugger3DS {

bool Application::LoadScene(const std::string& filepath) {
    Importer importer;
    logging::Logger::enabled = false;
    if (!importer.Import3DS(filepath)) {
        errorLog_ += "[Import] Failed to load: " + filepath + "\n";
        logging::Logger::enabled = true;
        return false;
    }
    scene_ = importer.GetScene();
    // Extract directory from filepath for texture loading
    {
        auto pos = filepath.find_last_of("/\\");
        scene_.basePath = (pos != std::string::npos) ? filepath.substr(0, pos) : ".";
    }
    logging::Logger::enabled = true;
    return true;
}

void Application::OpenScene(const std::string& filepath) {
    errorLog_.clear();
    if (!LoadScene(filepath))
        return;

    // Clear previous GPU state
    renderer_.ClearAllMeshes();
    textureLoader_.Shutdown();
    selector_.Reset();
    nodeToDataId_.clear();

    // Upload new scene
    UploadScene();
    sceneLoaded_ = true;

    // Update window title
    auto lastSlash = filepath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;
    window_.SetTitle("3DSLab - " + filename);
}

void Application::UploadScene() {
    scenePanel_ = std::make_unique<UI::SceneTreePanel>(scene_);

    // Sync ImGui tree → 3D selection
    scenePanel_->SetNodeSelectionCallback([this](const std::vector<uint16_t>& nodeIds) {
        std::vector<int> dataIds;
        for (uint16_t nid : nodeIds) {
            auto it = nodeToDataId_.find(nid);
            if (it != nodeToDataId_.end())
                dataIds.push_back(it->second);
        }
        selector_.ClearSelection();
        if (!dataIds.empty())
            selector_.SelectMeshes(dataIds, false);
    });

    // Zoom callback
    scenePanel_->SetZoomCallback([this](const std::vector<uint16_t>& nodeIds) {
        camera_.ZoomToNodes(nodeIds, nodeToDataId_, renderer_);
    });

    // Upload meshes
    MeshUploader::UploadMeshes(renderer_, scene_, selector_, nodeToDataId_,
        [this](const std::any& userData) {
            if (!userData.has_value()) {
                scenePanel_->ClearSelection();
                std::cout << "\nNo object selected" << std::endl;
                return;
            }
            auto objectNode = std::any_cast<ObjectNodePtr>(userData);
            if (objectNode) {
                scenePanel_->SetSelectedNodeId(objectNode->nodeId);
                std::cout << objectNode->PrintInfo(0);
                if (objectNode->associatedMesh) {
                    std::cout << "\nMesh Matrix:" << std::endl;
                    std::cout << objectNode->associatedMesh->meshMatrix << std::endl;
                }
            }
        },
        textureLoader_);
}

void Application::SetupViewer() {
    // Initialize window
    window_.Init(1280, 720, "3DSLab");

    // Initialize bgfx renderer
    renderer_.Init(window_.GetNativeHandle(), window_.GetWidth(), window_.GetHeight());

    // Initialize camera
    camera_.Init(45.0f, 0.1f, 10000.0f);
    // Use fixed viewer defaults: turntable mode, default rotate speed
    camera_.SetRotateSpeed(1.0f);
    camera_.SetRotationMode(CameraController::RotationMode::Turntable);
    camera_.SetInvertRotation(false);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOther(window_.GetGLFWWindow(), true);
    ImGui_ImplBgfx_Init(255);

    // Handle window resize
    window_.onResize = [this](int w, int h) {
        renderer_.HandleResize(static_cast<uint16_t>(w), static_cast<uint16_t>(h));
    };

    // Handle file drop
    window_.onDrop = [this](const std::string& path) {
        // Accept only .3ds files (case-insensitive check)
        if (path.size() >= 4) {
            std::string ext = path.substr(path.size() - 4);
            for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (ext == ".3ds")
                OpenScene(path);
        }
    };

    // Initialize selector
    selector_.Init(renderer_, camera_);

    // Wire texture error messages into the error log
    textureLoader_.SetErrorCallback([this](const std::string& msg) {
        errorLog_ += "[Texture] " + msg + "\n";
    });

    // Create empty scene panel
    scenePanel_ = std::make_unique<UI::SceneTreePanel>(scene_);

    axisLines_ = MeshUploader::MakeCoordinateAxes(10.0);
}

void Application::Run() {
    while (!window_.ShouldClose()) {
        window_.ResetFrameInput();
        window_.PollEvents();

        uint16_t w = window_.GetWidth();
        uint16_t h = window_.GetHeight();
        if (w == 0 || h == 0) continue;

        ProcessInput();
        camera_.Update();

        // Determine pivot target: use selection center when available, otherwise origin
        bool hasSelection = selector_.HasSelection();
        Eigen::Vector3f pivotWorld = hasSelection ? selector_.GetSelectionCenter() : Eigen::Vector3f::Zero();

        // Set the orbit pivot so rotations orbit around the selected object (or origin).
        // This does NOT move the camera — it only affects future rotations.
        camera_.SetOrbitPivot(pivotWorld);

        // Inform renderer about pivot for optional debug marker
        renderer_.SetPivotPoint(pivotWorld);
        renderer_.SetShowPivotMarker(showPivotMarker_);

        // Set view/projection
        float aspect = static_cast<float>(w) / static_cast<float>(h);
        bool hmgDepth = renderer_.IsHomogeneousDepth();
        renderer_.SetViewTransform(camera_.GetViewMatrix(), camera_.GetProjectionMatrix(aspect, hmgDepth));

        renderer_.BeginFrame(w, h);

        // Headlight: light comes from the camera direction (slightly above for depth cues)
        Eigen::Vector3f eyePos = camera_.GetEyePosition();
        Eigen::Matrix4f view = camera_.GetViewMatrix();
        // Extract camera forward from view matrix row 2 (negated because view z is -forward)
        Eigen::Vector3f camForward(-view(2, 0), -view(2, 1), -view(2, 2));
        Eigen::Vector3f camUp(view(1, 0), view(1, 1), view(1, 2));
        Eigen::Vector3f lightDir = (-camForward + camUp * 0.3f).normalized();
        renderer_.SetLightUniforms(lightDir, eyePos);

        // Draw all meshes (opaque first, then transparent sorted back-to-front)
        renderer_.DrawAllMeshes(camera_.GetEyePosition());

        // Draw coordinate axes
        renderer_.DrawLines(axisLines_);

        // ImGui frame
        ImGui_ImplBgfx_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawImGui();

        ImGui::Render();
        ImGui_ImplBgfx_RenderDrawLists(ImGui::GetDrawData());

        renderer_.EndFrame();
    }

    // Cleanup
    ImGui_ImplBgfx_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    textureLoader_.Shutdown();
    renderer_.Shutdown();
    window_.Shutdown();
}

void Application::ProcessInput() {
    const auto& input = window_.GetInput();

    // Current mouse
    double curMouseX = input.mouseX;
    double curMouseY = input.mouseY;
    bool hadPrev = !firstMouse_;

    // Camera controls (only when ImGui doesn't want input)
    if (!ImGui::GetIO().WantCaptureMouse) {
        bool leftDrag  = input.mouseButtons[0];
        bool rightDrag = input.mouseButtons[1];
        if (hadPrev) {
            // Compute pivot screen coords (in pixels) for rotation center
            float pivotScreenX = static_cast<float>(window_.GetWidth()) * 0.5f;
            float pivotScreenY = static_cast<float>(window_.GetHeight()) * 0.5f;
            if (selector_.HasSelection()) {
                Eigen::Vector3f selCenter = selector_.GetSelectionCenter();
                // Project selCenter to screen using current camera view/proj
                float aspect = static_cast<float>(window_.GetWidth()) / static_cast<float>(window_.GetHeight());
                bool hmgDepth = renderer_.IsHomogeneousDepth();
                Eigen::Matrix4f view = camera_.GetViewMatrix();
                Eigen::Matrix4f proj = camera_.GetProjectionMatrix(aspect, hmgDepth);
                Eigen::Vector4f clip = proj * view * Eigen::Vector4f(selCenter.x(), selCenter.y(), selCenter.z(), 1.0f);
                if (std::abs(clip.w()) > 1e-6f) {
                    float ndcX = clip.x() / clip.w();
                    float ndcY = clip.y() / clip.w();
                    pivotScreenX = (ndcX * 0.5f + 0.5f) * static_cast<float>(window_.GetWidth());
                    pivotScreenY = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(window_.GetHeight());
                }
            }

            camera_.OnMouseDrag(static_cast<float>(prevMouseX_), static_cast<float>(prevMouseY_),
                                static_cast<float>(curMouseX), static_cast<float>(curMouseY),
                                leftDrag, rightDrag,
                                static_cast<float>(window_.GetWidth()),
                                static_cast<float>(window_.GetHeight()),
                                pivotScreenX, pivotScreenY);
        } else {
        }

        if (input.scrollDeltaY != 0)
            camera_.OnScroll(static_cast<float>(input.scrollDeltaY));
    }

    firstMouse_ = false;
    prevMouseX_ = curMouseX;
    prevMouseY_ = curMouseY;

    // Mesh selection
    selector_.ProcessInput(input, window_.GetWidth(), window_.GetHeight());
}

void Application::DrawImGui() {
    ImGui::SetNextWindowSize(ImVec2(320.0f, 600.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);

    bool visible = ImGui::Begin("Inspector");
    SnapWindowToEdges();

    if (!visible) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("InspectorTabs")) {
        if (ImGui::BeginTabItem("Scene")) {
            if (sceneLoaded_)
                scenePanel_->DrawContent();
            else
                ImGui::TextWrapped("Drop a .3ds file here to open it.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Interaction settings (simplified): suspend transparency and optional pivot marker
    if (ImGui::CollapsingHeader("Interaction", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("Show pivot marker", &showPivotMarker_)) {
            // toggle handled via renderer each frame
        }
    }

    if (ImGui::CollapsingHeader("Errors")) {
        if (errorLog_.empty()) {
            ImGui::TextDisabled("No errors.");
        } else {
            if (ImGui::SmallButton("Clear##errors"))
                errorLog_.clear();
            ImGui::BeginChild("##errorlog", ImVec2(0, 120), true);
            ImGui::TextUnformatted(errorLog_.c_str());
            ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
        }
    }

    ImGui::End();
}

} // namespace Debugger3DS
