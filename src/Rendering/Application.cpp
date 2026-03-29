#include "Application.h"
#include "MeshUploader.h"
#include "ObjectNode.h"
#include "Logger.h"
#include <imgui.h>
#include <iostream>
#include <any>

namespace {

// Snap the current ImGui window to screen edges when within snapDist pixels.
// Must be called between Begin() and End().
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
    // Setup ImGui plugin with a scene tree panel
    viewer_.plugins.push_back(&imguiPlugin_);
    imguiPlugin_.widgets.push_back(&imguiMenu_);
    
    // Use trackball rotation instead of the default two-axis valuator
    viewer_.core().rotation_type = igl::opengl::ViewerCore::ROTATION_TYPE_TRACKBALL;
    
    scenePanel_ = std::make_unique<UI::SceneTreePanel>(scene_);

    // Sync ImGui tree click → 3D scene selection (selects node + all descendants)
    // Highlight meshes without firing the selection callback back to the tree
    scenePanel_->SetNodeSelectionCallback([this](const std::vector<uint16_t>& nodeIds) {
        if (!selector_) return;
        std::vector<int> dataIds;
        for (uint16_t nid : nodeIds) {
            auto it = nodeToDataId_.find(nid);
            if (it != nodeToDataId_.end())
                dataIds.push_back(it->second);
        }
        selector_->ClearSelection();
        if (!dataIds.empty())
            selector_->SelectMeshes(dataIds, /*fireCallback=*/false);
    });

    // Zoom-to-fit button callback (zooms to node + all descendants)
    scenePanel_->SetZoomCallback([this](const std::vector<uint16_t>& nodeIds) {
        if (camera_) camera_->ZoomToNodes(nodeIds, nodeToDataId_);
    });

    // Replace the default two-window layout with a single tabbed window
    imguiMenu_.callback_draw_viewer_window = [this]() {
        ImGui::SetNextWindowSize(ImVec2(320.0f, 600.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f),     ImGuiCond_FirstUseEver);

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
            if (ImGui::BeginTabItem("Viewer")) {
                ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
                imguiMenu_.draw_viewer_menu();
                ImGui::PopItemWidth();

                // Sync draw options from the active mesh to all other meshes
                auto& active = viewer_.data();
                for (int i = 0; i < viewer_.data_list.size(); ++i) {
                    auto& d = viewer_.data_list[i];
                    if (d.id == active.id) continue;
                    d.show_lines         = active.show_lines;
                    d.show_faces         = active.show_faces;
                    d.show_overlay       = active.show_overlay;
                    d.show_overlay_depth = active.show_overlay_depth;
                    d.show_vertex_labels = active.show_vertex_labels;
                    d.show_face_labels   = active.show_face_labels;
                    d.show_custom_labels = active.show_custom_labels;
                    d.show_texture       = active.show_texture;
                    d.line_color         = active.line_color;
                    d.shininess          = active.shininess;
                    if (d.face_based != active.face_based) {
                        d.set_face_based(active.face_based);
                    }
                    if (d.invert_normals != active.invert_normals) {
                        d.invert_normals = active.invert_normals;
                        d.dirty |= igl::opengl::MeshGL::DIRTY_NORMAL;
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    };
    
    // Upload meshes and setup selector
    selector_ = std::make_unique<MeshSelector>(viewer_);
    MeshUploader::UploadMeshes(viewer_, scene_, *selector_, nodeToDataId_,
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
    
    // Animate camera zoom transitions each frame
    camera_ = std::make_unique<CameraController>(viewer_);
    viewer_.callback_pre_draw = [this](igl::opengl::glfw::Viewer&) -> bool {
        return camera_->Update();
    };
    
    MeshUploader::DrawCoordinateAxes(viewer_, 10.0);
    
    std::cout << "\nControls:\n";
    std::cout << "  N - Next mesh\n";
    std::cout << "  P - Previous mesh\n";
    std::cout << "  C - Clear selection\n";
}

void Application::Run() {
    viewer_.launch();
}

} // namespace Debugger3DS
