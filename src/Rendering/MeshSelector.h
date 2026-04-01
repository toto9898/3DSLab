#pragma once

#include "AABB.h"
#include <Eigen/Dense>
#include <vector>
#include <functional>
#include <any>
#include <string>
#include <map>
#include <cstdint>

namespace Debugger3DS {

class Renderer;
class CameraController;
struct InputState;

class MeshSelector {
public:
    MeshSelector() = default;

    void Init(Renderer& renderer, CameraController& camera);

    // Add a mesh with its renderer id, user data, original color, and optional bounding box
    void AddMesh(int meshId, std::any userData, uint32_t color, const std::string& name = "", const AABB& bbox = AABB());

    // Add a mesh with bounding box computed from transformation
    void AddMeshWithTransform(int meshId, std::any userData, uint32_t color, const std::string& name,
                               const Eigen::Vector3f& bboxMin, const Eigen::Vector3f& bboxMax,
                               const Eigen::Matrix4f& transform);

    // Process input for this frame (call each frame)
    void ProcessInput(const InputState& input, uint16_t viewportW, uint16_t viewportH);

    // Set callback for when selection changes
    void SetSelectionCallback(std::function<void(const std::any&)> callback);

    // Get user data for currently selected mesh
    const std::any& GetSelectedUserData() const { return selectedUserData_; }

    // Whether any mesh is currently selected
    bool HasSelection() const;

    // Get mesh name by index
    std::string GetMeshName(int index) const;

    // Return center of bounding box covering currently selected meshes.
    // If no selection, returns a zero vector.
    Eigen::Vector3f GetSelectionCenter() const;

    // Highlight the selected mesh
    void HighlightSelected();

    // Clear selection
    void ClearSelection();

    // Select mesh by renderer id (toggles if already selected)
    void SelectMesh(int meshId);

    // Select multiple meshes by renderer id
    void SelectMeshes(const std::vector<int>& meshIds, bool fireCallback = true);

private:
    Renderer* renderer_ = nullptr;
    CameraController* camera_ = nullptr;

    std::vector<int> meshIds_;
    std::vector<std::any> userData_;
    std::vector<std::string> meshNames_;
    std::vector<AABB> meshBBoxes_;
    std::vector<uint32_t> originalColors_;

    std::any selectedUserData_;
    int selectedMeshId_ = -1;
    int currentIndex_ = -1;
    std::vector<int> additionalSelectedIds_;

    std::function<void(const std::any&)> selectionCallback_;

    // Mouse drag detection
    bool isDragging_ = false;
    double mouseDownX_ = 0;
    double mouseDownY_ = 0;

    // Find mesh under cursor using ray casting
    int FindMeshUnderCursor(double mouseX, double mouseY, uint16_t viewportW, uint16_t viewportH);

    // Unproject screen coords to world ray
    static void Unproject(double screenX, double screenY, double depth,
                          const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj,
                          uint16_t viewportW, uint16_t viewportH,
                          bool homogeneousDepth,
                          Eigen::Vector3f& worldPos);

    // Möller–Trumbore ray-triangle intersection
    static bool RayTriangleIntersect(const Eigen::Vector3d& origin, const Eigen::Vector3d& dir,
                                     const Eigen::Vector3d& v0, const Eigen::Vector3d& v1,
                                     const Eigen::Vector3d& v2,
                                     double& t, double& u, double& v);
};

} // namespace Debugger3DS
