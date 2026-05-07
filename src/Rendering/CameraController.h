#pragma once

#include <Eigen/Dense>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace Debugger3DS::Rendering {

class Renderer;

/// @brief Arcball / turntable orbit camera with smooth zoom animation.
///
/// Translates raw mouse drag and scroll-wheel events into view and projection
/// matrices suitable for bgfx.  Call Update() once per frame to advance the
/// animated zoom.
class CameraController {
public:
    CameraController() = default;

    /// @brief Set up the camera with the given projection parameters.
    /// @param fovDeg Vertical field of view in degrees.
    /// @param near   Near clip distance.
    /// @param far    Far clip distance.
    void Init(float fovDeg = 45.0f, float near = 0.1f, float far = 10000.0f);

    /// @brief Handle a mouse drag event.
    /// @param prevX       Mouse X at the start of this drag delta (pixels).
    /// @param prevY       Mouse Y at the start of this drag delta (pixels).
    /// @param currX       Mouse X at the end of this drag delta (pixels).
    /// @param currY       Mouse Y at the end of this drag delta (pixels).
    /// @param leftDrag    @c true while the left mouse button is held.
    /// @param rightDrag   @c true while the right mouse button is held (pan).
    /// @param viewportW   Viewport width in pixels.
    /// @param viewportH   Viewport height in pixels.
    /// @param pivotScreenX  Rotation pivot X in window-space pixels; pass negative to use viewport centre.
    /// @param pivotScreenY  Rotation pivot Y in window-space pixels; pass negative to use viewport centre.
    void OnMouseDrag(float prevX, float prevY, float currX, float currY, bool leftDrag, bool rightDrag, float viewportW, float viewportH, float pivotScreenX = -1.0f, float pivotScreenY = -1.0f);

    /// @brief Handle a scroll-wheel event.
    /// @param delta Scroll delta (positive = zoom in).
    void OnScroll(float delta);

    /// @brief Advance the zoom animation.  Call once per frame.
    /// @return @c true if the view changed this frame (triggers a re-render).
    bool Update();

    /// @brief Animate the camera to fit the given node IDs.
    /// @param nodeIds      Node IDs to frame.
    /// @param nodeToDataId Map from node ID to renderer data ID.
    /// @param renderer     Renderer to query geometry bounds from.
    void ZoomToNodes(const std::vector<uint16_t>& nodeIds,
                     const std::unordered_map<uint16_t, int>& nodeToDataId,
                     const Renderer& renderer);

    /// @brief Animate the camera to fit the given world-space bounding box.
    void ZoomToFit(const Eigen::Vector3d& bboxMin, const Eigen::Vector3d& bboxMax);

    Eigen::Matrix4f GetViewMatrix() const;                                            ///< Current view matrix.
    Eigen::Matrix4f GetProjectionMatrix(float aspectRatio, bool homogeneousDepth = true) const; ///< Current projection matrix.
    Eigen::Vector3f GetEyePosition() const;                                           ///< Eye position in world space.

    float GetFov() const { return fovDeg_; } ///< Vertical FOV in degrees.

    /// @brief Set the arcball rotation speed multiplier.
    void SetRotateSpeed(float speed);

    /// @brief Rotation mode selection.
    enum class RotationMode { Arcball = 0, Turntable = 1 };
    void SetRotationMode(RotationMode mode); ///< Switch between arcball and turntable modes.
    RotationMode GetRotationMode() const;    ///< Current rotation mode.

    /// @brief Mirror horizontal and vertical rotation directions.
    void SetInvertRotation(bool invert);
    bool GetInvertRotation() const; ///< @c true if rotation directions are inverted.

    /// @brief Move the look-at target (also shifts the camera to maintain the offset).
    void SetTarget(const Eigen::Vector3f& target);

    /// @brief Change the look-at target while keeping the eye position fixed.
    void SetTargetKeepEye(const Eigen::Vector3f& target);

    /// @brief Set the orbit pivot independently of the look-at target.
    /// Rotations will orbit around this point without immediately moving the camera.
    void SetOrbitPivot(const Eigen::Vector3f& pivot);

private:
    // Trackball: rotation is stored as a quaternion
    Eigen::Quaternionf rotation_ = Eigen::Quaternionf::Identity();
    Eigen::Vector3f target_ = Eigen::Vector3f::Zero();       // look-at target
    Eigen::Vector3f orbitPivot_ = Eigen::Vector3f::Zero();    // rotation orbit center
    bool hasCustomPivot_ = false;                              // true when orbitPivot_ != target_
    float distance_ = 5.0f;                                    // distance from target
    float fovDeg_ = 45.0f;
    float nearPlane_ = 0.1f;
    float farPlane_ = 10000.0f;
    float panSpeed_ = 0.005f;
    float rotateSpeed_ = 1.0f;
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
    // Overload that maps relative to a specific center (in pixels)
    Eigen::Vector3f ArcballVector(float x, float y, float w, float h, float centerX, float centerY) const;

    // Rotation mode (defaults to Turntable for conventional viewer feel)
    RotationMode rotationMode_ = RotationMode::Turntable;
    bool invertRotation_ = false;
};

} // namespace Debugger3DS::Rendering::Rendering::Rendering
