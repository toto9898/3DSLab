#pragma once

#include "AABB.h"
#include <Eigen/Dense>
#include <vector>
#include <functional>
#include <any>
#include <string>
#include <map>
#include <cstdint>

namespace Debugger3DS::Rendering {

class Renderer;
class CameraController;
struct InputState;

/// @brief Ray-cast-based mesh picker that tracks selection state and fires callbacks.
///
/// Register meshes with AddMesh() or AddMeshWithTransform(), then call
/// ProcessInput() every frame.  When the selection changes, the registered
/// callback is invoked with the selected mesh's user-data payload.
class MeshSelector {
public:
    MeshSelector() = default;

    /// @brief Bind to a renderer and camera for ray-casting.
    void Init(Renderer& renderer, CameraController& camera);

    /// @brief Register a mesh for picking.
    /// @param meshId   Renderer-side data ID.
    /// @param userData Opaque payload returned to the selection callback.
    /// @param color    RGBA tint colour (used for highlight state).
    /// @param name     Display name (shown in output log).
    /// @param bbox     Optional world-space AABB for fast ray rejection.
    void AddMesh(int meshId, std::any userData, uint32_t color, const std::string& name = "", const AABB& bbox = AABB());

    /// @brief Register a mesh whose AABB is derived from a local bounding box + transform.
    void AddMeshWithTransform(int meshId, std::any userData, uint32_t color, const std::string& name,
                               const Eigen::Vector3f& bboxMin, const Eigen::Vector3f& bboxMax,
                               const Eigen::Matrix4f& transform);

    /// @brief Sample input and update selection state.  Call once per frame.
    void ProcessInput(const InputState& input, uint16_t viewportW, uint16_t viewportH);

    /// @brief Set the callback invoked whenever the selection changes.
    /// @param callback Receives the @c std::any user-data of the newly selected mesh.
    void SetSelectionCallback(std::function<void(const std::any&)> callback);

    /// @return User-data of the currently selected mesh (empty @c std::any if none).
    const std::any& GetSelectedUserData() const { return selectedUserData_; }

    /// @return @c true if any mesh is currently selected.
    bool HasSelection() const;

    /// @return Display name of the mesh at @p index.
    std::string GetMeshName(int index) const;

    /// @brief Compute the centre of the world-space AABB covering all selected meshes.
    Eigen::Vector3f GetSelectionCenter() const;

    /// @brief Apply the highlight colour to the selected mesh in the renderer.
    void HighlightSelected();

    /// @brief Deselect all meshes.
    void ClearSelection();

    /// @brief Remove all registered meshes and reset selection state.
    void Reset();

    /// @brief Toggle selection of the mesh with the given renderer ID.
    void SelectMesh(int meshId);

    /// @brief Select a list of meshes by renderer ID.
    /// @param meshIds      Renderer mesh IDs to select.
    /// @param fireCallback @c true to invoke the selection callback after selecting.
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

} // namespace Debugger3DS::Rendering
