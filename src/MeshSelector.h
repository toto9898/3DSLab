#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <vector>
#include <functional>
#include <memory>
#include "3DS/SceneObjects/ObjectNode.h"

namespace Debugger3DS {

// Axis-aligned bounding box
struct AABB {
    Eigen::Vector3d min;
    Eigen::Vector3d max;
    
    AABB() : min(Eigen::Vector3d::Zero()), max(Eigen::Vector3d::Zero()) {}
    AABB(const Eigen::Vector3d& minPt, const Eigen::Vector3d& maxPt) : min(minPt), max(maxPt) {}
    
    // Check if ray intersects this bounding box
    bool IntersectsRay(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction, double& tMin, double& tMax) const;
};

class MeshSelector {
public:
    MeshSelector(igl::opengl::glfw::Viewer& viewer);
    
    // Add a mesh with its data_id, object node pointer, and optional bounding box
    void AddMesh(int dataId, std::shared_ptr<ObjectNode> objectNode, const std::string& name = "", const AABB& bbox = AABB());
    
    // Add a mesh with automatic bounding box computation from transformation
    void AddMeshWithTransform(int dataId, std::shared_ptr<ObjectNode> objectNode, const std::string& name, 
                               const Eigen::Vector3f& bboxMin, const Eigen::Vector3f& bboxMax,
                               const Eigen::Matrix4f& transform);
    
    // Setup mouse/keyboard callbacks for selection
    void EnableSelection();
    
    // Disable selection
    void DisableSelection();
    
    // Get currently selected object node (nullptr if none)
    std::shared_ptr<ObjectNode> GetSelectedObjectNode() const { return selectedObjectNode_; }
    
    // Get mesh name by index
    std::string GetMeshName(int index) const;
    
    // Set callback for when selection changes
    void SetSelectionCallback(std::function<void(std::shared_ptr<ObjectNode>)> callback);
    
    // Highlight the selected mesh
    void HighlightSelected();
    
    // Clear selection
    void ClearSelection();
    
private:
    igl::opengl::glfw::Viewer& viewer_;
    std::vector<int> meshIds_;
    std::vector<std::shared_ptr<ObjectNode>> objectNodes_;
    std::vector<std::string> meshNames_;
    std::vector<AABB> meshBBoxes_;
    std::shared_ptr<ObjectNode> selectedObjectNode_;
    int selectedMeshId_;
    int currentIndex_;
    
    std::function<void(std::shared_ptr<ObjectNode>)> selectionCallback_;
    
    // Original colors for restoring
    std::map<int, Eigen::MatrixXd> originalColors_;
    
    // Mouse drag detection
    bool isDragging_;
    double mouseDownX_;
    double mouseDownY_;
    
    // Keyboard callback
    bool OnKeyPressed(unsigned int key, int modifier);
    
    // Mouse callbacks
    bool OnMouseDown(int button, int modifier);
    bool OnMouseUp(int button, int modifier);
    bool OnMouseMove(int mouse_x, int mouse_y);
    
    // Find mesh under mouse cursor using ray casting
    int FindMeshUnderCursor();
    
    // Select mesh by data_id
    void SelectMesh(int dataId);
};

} // namespace Debugger3DS
