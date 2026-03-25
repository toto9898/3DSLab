#include "Application.h"
#include "3DS/SceneObjects/ObjectNode.h"
#include "Logger.h"
#include <iostream>
#include <ranges>
#include <any>

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
    
    scenePanel_ = std::make_unique<UI::SceneTreePanel>(scene_);
    imguiMenu_.callback_draw_custom_window = [this]() {
        scenePanel_->Draw();
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
    
    // Set domain-specific selection callback
    selector_->SetSelectionCallback([](const std::any& userData) {
        if (!userData.has_value()) {
            std::cout << "\nNo object selected" << std::endl;
            return;
        }
        auto objectNode = std::any_cast<ObjectNodePtr>(userData);
        if (objectNode) {
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
