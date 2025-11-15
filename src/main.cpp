#include <igl/opengl/glfw/Viewer.h>
#include "3DS/Importer.h"
#include "MeshSelector.h"
#include <iostream>
#include <ranges>
#include "Logger.h"

//constexpr auto kMeshFilePath = "D:\\Programming\\3DSLab\\src\\3DS\\Samples\\LC_ChipmunkStudio_011922a2.3ds";
constexpr auto kMeshFilePath = "D:\\Programming\\3DSLab\\src\\3DS\\Samples\\55.3ds";

static void GetMeshesToRender(const Debugger3DS::Scene& scene, std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>>& meshData);
static void DrawCoordinateAxes(igl::opengl::glfw::Viewer& viewer, double axisLength = 10.0);

int main(int argc, char *argv[])
{
    logging::Logger::enabled = false;

    // Import 3DS file
    Debugger3DS::Importer importer;
    if (!importer.Import3DS(kMeshFilePath)) {
        logging::log << "Failed to load 3DS file" << std::endl;
        return -1;
    }

    auto scene = importer.GetScene();

    // Create viewer
    igl::opengl::glfw::Viewer viewer;

    logging::Logger::enabled = true;

    // Get meshes to render (with ObjectNode transforms applied if available)
    std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>> meshData;
    GetMeshesToRender(scene, meshData);

    // Setup mesh selector
    Debugger3DS::MeshSelector selector(viewer);

    // Determine if we're rendering via object nodes or direct meshes
    bool hasValidAssociations = std::ranges::any_of(scene.objectNodes, 
        [](const auto& node) { return node->associatedMesh != nullptr; });
    bool usingObjectNodes = !scene.objectNodes.empty() && hasValidAssociations;

    // Add each mesh as a separate object
    for (size_t i = 0; i < meshData.size(); ++i) {
        std::string meshName = usingObjectNodes ? scene.objectNodes[i]->associatedMeshName : scene.meshes[i]->name;
        if (meshName == "$$$DUMMY")
            continue;

        const auto& [V, F] = meshData[i];
        
        int data_id = (i == 0) ? viewer.data().id : viewer.append_mesh();
        viewer.data(data_id).set_mesh(V, F);
        viewer.data(data_id).set_face_based(true);
        
        // Add mesh to selector
        
        if (usingObjectNodes) {
            auto objectNodePtr = scene.objectNodes[i];
            auto nodeTransform = scene.GetNodeGlobalTransform(objectNodePtr);
            
            selector.AddMeshWithTransform(data_id, objectNodePtr, meshName, 
                                        objectNodePtr->boundingBox.min, objectNodePtr->boundingBox.max,
                                        nodeTransform);
        } else {
            selector.AddMesh(data_id, nullptr, meshName);
        }
        
        // Give each mesh a different color for easy identification
        if (i > 0) {
            Eigen::RowVector3d color;
            color << (i * 0.3) - floor(i * 0.3), (i * 0.7) - floor(i * 0.7), (i * 0.5) - floor(i * 0.5);
            viewer.data(data_id).set_colors(color);
        }
    }

    // Enable mesh selection (use N/P keys to cycle, C to clear)
    selector.EnableSelection();
    selector.SetSelectionCallback([&scene](std::shared_ptr<Debugger3DS::ObjectNode> objectNode) {
        if (objectNode) {
            std::cout << objectNode->PrintInfo(scene.currentFrame);
            
            // Print mesh matrix if available
            if (objectNode->associatedMesh) {
                std::cout << "\nMesh Matrix:" << std::endl;
                std::cout << objectNode->associatedMesh->meshMatrix << std::endl;
            }
        } else {
            std::cout << "\nNo object selected" << std::endl;
        }
    });

    // Draw coordinate axes
    DrawCoordinateAxes(viewer, 10.0);

    std::cout << "\nControls:\n";
    std::cout << "  N - Next mesh\n";
    std::cout << "  P - Previous mesh\n";
    std::cout << "  C - Clear selection\n";

    viewer.launch();
}

static void DrawCoordinateAxes(igl::opengl::glfw::Viewer& viewer, double axisLength) {
    // X axis (red)
    Eigen::MatrixXd x_start(1, 3), x_end(1, 3);
    x_start << 0, 0, 0;
    x_end << axisLength, 0, 0;
    viewer.data().add_edges(x_start, x_end, Eigen::RowVector3d(1, 0, 0));

    // Y axis (green)
    Eigen::MatrixXd y_start(1, 3), y_end(1, 3);
    y_start << 0, 0, 0;
    y_end << 0, axisLength, 0;
    viewer.data().add_edges(y_start, y_end, Eigen::RowVector3d(0, 1, 0));

    // Z axis (blue)
    Eigen::MatrixXd z_start(1, 3), z_end(1, 3);
    z_start << 0, 0, 0;
    z_end << 0, 0, axisLength;
    viewer.data().add_edges(z_start, z_end, Eigen::RowVector3d(0, 0, 1));
}

static void GetMeshesToRender(const Debugger3DS::Scene& scene, std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>>& meshData) {
    meshData.clear();

    bool hasValidAssociations = std::ranges::any_of(scene.objectNodes, 
    [](const auto& node) { return node->associatedMesh != nullptr; });

    if (scene.objectNodes.empty() || !hasValidAssociations) {
        // No object nodes, render meshes directly
        for (const auto& mesh : scene.meshes) {
            Eigen::MatrixXd V;
            Eigen::MatrixXi F;
            
            mesh->ToEigenMatrices(V, F);
            meshData.emplace_back(V, F);
        }
    }
    else {
        // Render meshes through object nodes
        bool anyNodeRendered = false;
        int nodeIndex = 0;
        for (const auto& node : scene.objectNodes) {
            //if (nodeIndex > 1) break; // Temporary: only process first two nodes for debugging
            if (node->associatedMesh) {                
                Eigen::MatrixXd V;
                Eigen::MatrixXi F;
                Eigen::Matrix4f nodeTransform = scene.GetNodeGlobalTransform(node);

                node->associatedMesh->ApplyTransform(nodeTransform);
                node->associatedMesh->ToEigenMatrices(V, F);                
                meshData.emplace_back(V, F);
                
                anyNodeRendered = true;
                nodeIndex++;
            }
        }
    }
}
