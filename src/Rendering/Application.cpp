#include "Application.h"
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
    logging::Logger::enabled = false;
    if (!importer_.Import3DS(filepath)) {
        logging::log << "Failed to load 3DS file" << std::endl;
        return false;
    }
    scene_ = importer_.GetScene();
    logging::Logger::enabled = true;
    return true;
}

void Application::SetupViewer() {
    // Initialize window
    window_.Init(1280, 720, "3DSLab");

    // Initialize bgfx renderer
    renderer_.Init(window_.GetNativeHandle(), window_.GetWidth(), window_.GetHeight());

    // Initialize camera
    camera_.Init(45.0f, 0.1f, 10000.0f);

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

    // Initialize selector
    selector_.Init(renderer_, camera_);

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
        });

    axisLines_ = MeshUploader::MakeCoordinateAxes(10.0);

    std::cout << "\nControls:\n";
    std::cout << "  N - Next mesh\n";
    std::cout << "  P - Previous mesh\n";
    std::cout << "  C - Clear selection\n";
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

        // Set view/projection
        float aspect = static_cast<float>(w) / static_cast<float>(h);
        bool hmgDepth = renderer_.IsHomogeneousDepth();
        renderer_.SetViewTransform(camera_.GetViewMatrix(), camera_.GetProjectionMatrix(aspect, hmgDepth));

        renderer_.BeginFrame(w, h);

        // Set light direction (from upper-right-front) and eye position for Phong shading
        Eigen::Vector3f lightDir = Eigen::Vector3f(0.3f, 1.0f, 0.5f).normalized();
        renderer_.SetLightUniforms(lightDir, camera_.GetEyePosition());

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
    renderer_.Shutdown();
    window_.Shutdown();
}

void Application::ProcessInput() {
    const auto& input = window_.GetInput();

    // Compute mouse delta
    float deltaX = 0, deltaY = 0;
    if (!firstMouse_) {
        deltaX = static_cast<float>(input.mouseX - prevMouseX_);
        deltaY = static_cast<float>(input.mouseY - prevMouseY_);
    }
    firstMouse_ = false;
    prevMouseX_ = input.mouseX;
    prevMouseY_ = input.mouseY;

    // Camera controls (only when ImGui doesn't want input)
    if (!ImGui::GetIO().WantCaptureMouse) {
        bool leftDrag  = input.mouseButtons[0];
        bool rightDrag = input.mouseButtons[1];
        camera_.OnMouseDrag(deltaX, deltaY, leftDrag, rightDrag,
                            static_cast<float>(window_.GetWidth()),
                            static_cast<float>(window_.GetHeight()));
        if (input.scrollDeltaY != 0)
            camera_.OnScroll(static_cast<float>(input.scrollDeltaY));
    }

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
            scenePanel_->DrawContent();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace Debugger3DS
