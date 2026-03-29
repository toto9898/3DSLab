#pragma once

#include <Eigen/Dense>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace Debugger3DS {

class Renderer;

class CameraController {
public:
    CameraController() = default;

    // Initialize with default camera state
    void Init(float fovDeg = 45.0f, float near = 0.1f, float far = 10000.0f);

    // Process input each frame. Returns true if the camera changed.
    // deltaX/Y: mouse delta this frame (0 when not dragging)
    // leftDrag: left button held, rightDrag: right button held
    // scrollDelta: scroll wheel delta
    void OnMouseDrag(float deltaX, float deltaY, bool leftDrag, bool rightDrag, float viewportW, float viewportH);
    void OnScroll(float delta);

    // Update zoom animation. Call every frame.
    bool Update();

    // Zoom to fit a set of nodes (given their V/F data via the renderer)
    void ZoomToNodes(const std::vector<uint16_t>& nodeIds,
                     const std::unordered_map<uint16_t, int>& nodeToDataId,
                     const Renderer& renderer);

    // Zoom to fit a bounding box
    void ZoomToFit(const Eigen::Vector3d& bboxMin, const Eigen::Vector3d& bboxMax);

    // Get current view/projection matrices
    Eigen::Matrix4f GetViewMatrix() const;
    Eigen::Matrix4f GetProjectionMatrix(float aspectRatio, bool homogeneousDepth = true) const;

    // Get eye position in world space (for lighting)
    Eigen::Vector3f GetEyePosition() const;

    float GetFov() const { return fovDeg_; }

private:
    // Trackball: rotation is stored as a quaternion
    Eigen::Quaternionf rotation_ = Eigen::Quaternionf::Identity();
    Eigen::Vector3f target_ = Eigen::Vector3f::Zero();       // look-at target
    float distance_ = 5.0f;                                    // distance from target
    float fovDeg_ = 45.0f;
    float nearPlane_ = 0.1f;
    float farPlane_ = 10000.0f;
    float panSpeed_ = 0.005f;
    float rotateSpeed_ = 0.005f;
    float zoomSpeed_ = 0.1f;

    // Smooth zoom animation
    struct Animation {
        bool active = false;
        float startDistance = 1.0f;
        float targetDistance = 1.0f;
        Eigen::Vector3f startTarget = Eigen::Vector3f::Zero();
        Eigen::Vector3f endTarget = Eigen::Vector3f::Zero();
        float duration = 0.4f;
        std::chrono::steady_clock::time_point startTime;
    } anim_;

    // Arcball helper: map screen coords to unit sphere
    Eigen::Vector3f ArcballVector(float x, float y, float w, float h) const;
};

} // namespace Debugger3DS
