#include "Application.h"
#include "3DS/SceneObjects/ObjectNode.h"
#include "Logger.h"
#include <imgui.h>
#include <iostream>
#include <ranges>
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

    // Sync ImGui tree click → 3D scene selection
    scenePanel_->SetNodeSelectionCallback([this](uint16_t nodeId) {
        auto it = nodeToDataId_.find(nodeId);
        if (it != nodeToDataId_.end() && selector_)
            selector_->SelectMesh(it->second);
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
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    };
    
    // Upload meshes and setup selector
    UploadMeshes();
    
    DrawCoordinateAxes(10.0);
    
    std::cout << "\nControls:\n";
    std::cout << "  N - Next mesh\n";
    std::cout << "  P - Previous mesh\n";
    std::cout << "  C - Clear selection\n";
}

void Application::Run() {
    viewer_.launch();
}

std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>> Application::GetMeshesToRender() const {
    std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>> meshData;
    
    bool hasValidAssociations = std::ranges::any_of(scene_.objectNodes, 
        [](const auto& node) { return node->associatedMesh != nullptr; });
    
    if (scene_.objectNodes.empty() || !hasValidAssociations) {
        for (const auto& mesh : scene_.meshes) {
            Eigen::MatrixXd V;
            Eigen::MatrixXi F;
            mesh->ToEigenMatrices(V, F);
            meshData.emplace_back(V, F);
        }
    } else {
        for (const auto& node : scene_.objectNodes) {
            if (node->associatedMesh) {
                Eigen::MatrixXd V;
                Eigen::MatrixXi F;
                Eigen::Matrix4f nodeTransform = scene_.GetNodeGlobalTransform(node);
                node->associatedMesh->ToEigenMatrices(V, F, nodeTransform);
                meshData.emplace_back(V, F);
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
    
    bool hasValidAssociations = std::ranges::any_of(scene_.objectNodes, 
        [](const auto& node) { return node->associatedMesh != nullptr; });
    bool usingObjectNodes = !scene_.objectNodes.empty() && hasValidAssociations;
    
    for (size_t i = 0; i < meshData.size(); ++i) {
        std::string meshName = usingObjectNodes ? scene_.objectNodes[i]->associatedMeshName : scene_.meshes[i]->name;
        if (meshName == "$$$DUMMY")
            continue;
        
        const auto& [V, F] = meshData[i];
        
        int data_id = (i == 0) ? viewer_.data().id : viewer_.append_mesh();
        viewer_.data(data_id).set_mesh(V, F);
        viewer_.data(data_id).set_face_based(true);
        
        if (i > 0) {
            Eigen::RowVector3d color;
            color << (i * 0.3) - floor(i * 0.3), (i * 0.7) - floor(i * 0.7), (i * 0.5) - floor(i * 0.5);
            viewer_.data(data_id).set_colors(color);
        }
        
        if (usingObjectNodes) {
            auto objectNodePtr = scene_.objectNodes[i];
            auto nodeTransform = scene_.GetNodeGlobalTransform(objectNodePtr);
            selector_->AddMeshWithTransform(data_id, std::any(objectNodePtr), meshName,
                                          objectNodePtr->boundingBox.min, objectNodePtr->boundingBox.max,
                                          nodeTransform);
            nodeToDataId_[objectNodePtr->nodeId] = data_id;
        } else {
            selector_->AddMesh(data_id, std::any{}, meshName);
        }
    }
    
    selector_->EnableSelection();
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
