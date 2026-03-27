#include "Application.h"
#include "3DS/SceneObjects/ObjectNode.h"
#include "Logger.h"
#include <imgui.h>
#include <iostream>
#include <ranges>
#include <any>
#include <algorithm>

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

    // Sync ImGui tree click → 3D scene selection
    scenePanel_->SetNodeSelectionCallback([this](uint16_t nodeId) {
        auto it = nodeToDataId_.find(nodeId);
        if (it != nodeToDataId_.end() && selector_)
            selector_->SelectMesh(it->second);
    });

    // Zoom-to-fit button callback
    scenePanel_->SetZoomCallback([this](uint16_t nodeId) {
        ZoomToNode(nodeId);
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
    UploadMeshes();
    
    // Animate camera zoom transitions each frame
    viewer_.callback_pre_draw = [this](igl::opengl::glfw::Viewer&) -> bool {
        if (!cameraAnim_.active) return false;
        
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - cameraAnim_.startTime).count();
        float t = std::clamp(elapsed / cameraAnim_.duration, 0.0f, 1.0f);
        
        // Smooth-step interpolation
        float s = t * t * (3.0f - 2.0f * t);
        
        viewer_.core().camera_base_zoom = 
            cameraAnim_.startZoom + s * (cameraAnim_.targetZoom - cameraAnim_.startZoom);
        viewer_.core().camera_base_translation = 
            cameraAnim_.startTranslation + s * (cameraAnim_.targetTranslation - cameraAnim_.startTranslation);
        
        if (t >= 1.0f)
            cameraAnim_.active = false;
        
        return false;
    };
    
    DrawCoordinateAxes(10.0);
    
    std::cout << "\nControls:\n";
    std::cout << "  N - Next mesh\n";
    std::cout << "  P - Previous mesh\n";
    std::cout << "  C - Clear selection\n";
}

void Application::Run() {
    viewer_.launch();
}

std::vector<Application::MeshEntry> Application::GetMeshesToRender() const {
    std::vector<MeshEntry> meshData;
    
    bool hasValidAssociations = std::ranges::any_of(scene_.objectNodes, 
        [](const auto& node) { return node->associatedMesh != nullptr; });
    
    if (scene_.objectNodes.empty() || !hasValidAssociations) {
        for (const auto& mesh : scene_.meshes) {
            MeshEntry entry;
            mesh->ToEigenMatrices(entry.V, entry.F);
            entry.meshName = mesh->name;
            meshData.push_back(std::move(entry));
        }
    } else {
        for (const auto& node : scene_.objectNodes) {
            if (node->associatedMesh) {
                MeshEntry entry;
                Eigen::Matrix4f nodeTransform = scene_.GetNodeGlobalTransform(node);
                node->associatedMesh->ToEigenMatrices(entry.V, entry.F, nodeTransform);
                entry.node = node;
                entry.meshName = node->associatedMeshName;
                meshData.push_back(std::move(entry));
            }
        }
    }
    
    return meshData;
}

void Application::UploadMeshes() {
    auto meshData = GetMeshesToRender();
    
    selector_ = std::make_unique<MeshSelector>(viewer_);
    
    // Set domain-specific selection callback — sync 3D selection → ImGui tree
    selector_->SetSelectionCallback([this](const std::any& userData) {
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
    
    size_t uploadedCount = 0;
    for (auto& entry : meshData) {
        if (entry.meshName == "$$$DUMMY")
            continue;
        
        int data_id = (uploadedCount == 0) ? viewer_.data().id : viewer_.append_mesh();
        viewer_.data(data_id).set_mesh(entry.V, entry.F);
        viewer_.data(data_id).set_face_based(true);
        
        if (uploadedCount > 0) {
            Eigen::RowVector3d color;
            color << (uploadedCount * 0.3) - floor(uploadedCount * 0.3),
                     (uploadedCount * 0.7) - floor(uploadedCount * 0.7),
                     (uploadedCount * 0.5) - floor(uploadedCount * 0.5);
            viewer_.data(data_id).set_colors(color);
        }
        
        if (entry.node) {
            auto nodeTransform = scene_.GetNodeGlobalTransform(entry.node);
            selector_->AddMeshWithTransform(data_id, std::any(entry.node), entry.meshName,
                                          entry.node->boundingBox.min, entry.node->boundingBox.max,
                                          nodeTransform);
            nodeToDataId_[entry.node->nodeId] = data_id;
        } else {
            selector_->AddMesh(data_id, std::any{}, entry.meshName);
        }
        ++uploadedCount;
    }
    
    selector_->EnableSelection();
}

void Application::ZoomToNode(uint16_t nodeId) {
    auto it = nodeToDataId_.find(nodeId);
    if (it == nodeToDataId_.end()) return;
    
    int dataId = it->second;
    const auto& V = viewer_.data(dataId).V;
    const auto& F = viewer_.data(dataId).F;
    if (V.rows() == 0) return;
    
    float targetZoom;
    Eigen::Vector3f targetShift;
    viewer_.core().get_scale_and_shift_to_fit_mesh(V, F, targetZoom, targetShift);
    
    // Reset user pan/zoom so only base values drive the camera
    cameraAnim_.startZoom = viewer_.core().camera_base_zoom * viewer_.core().camera_zoom;
    cameraAnim_.startTranslation = viewer_.core().camera_base_translation + viewer_.core().camera_translation;
    viewer_.core().camera_zoom = 1.0f;
    viewer_.core().camera_translation.setZero();
    
    cameraAnim_.targetZoom = targetZoom;
    cameraAnim_.targetTranslation = targetShift;
    cameraAnim_.active = true;
    cameraAnim_.startTime = std::chrono::steady_clock::now();
}

void Application::DrawCoordinateAxes(double axisLength) {
    Eigen::MatrixXd start(1, 3), end(1, 3);
    start << 0, 0, 0;
    
    end << axisLength, 0, 0;
    viewer_.data().add_edges(start, end, Eigen::RowVector3d(1, 0, 0));
    
    end << 0, axisLength, 0;
    viewer_.data().add_edges(start, end, Eigen::RowVector3d(0, 1, 0));
    
    end << 0, 0, axisLength;
    viewer_.data().add_edges(start, end, Eigen::RowVector3d(0, 0, 1));
}

} // namespace Debugger3DS
