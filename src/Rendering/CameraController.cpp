#include "CameraController.h"
#include "Renderer.h"
#include "Mesh.h"
#include <algorithm>
#include <cmath>

namespace Debugger3DS::Rendering {

void CameraController::Init(float fovDeg, float near, float far) {
    fovDeg_ = fovDeg;
    nearPlane_ = near;
    farPlane_ = far;
}

void CameraController::OnMouseDrag(float prevX, float prevY, float currX, float currY,
                                    bool leftDrag, bool rightDrag,
                                    float viewportW, float viewportH,
                                    float pivotScreenX, float pivotScreenY) {
    // Determine pivot in pixels; default to viewport center
    float centerX = (pivotScreenX >= 0.0f) ? pivotScreenX : (viewportW * 0.5f);
    float centerY = (pivotScreenY >= 0.0f) ? pivotScreenY : (viewportH * 0.5f);

    if (leftDrag) {
        // orientation sign controls horizontal direction: default flipped so left/right match user
        float orientationSign = invertRotation_ ? 1.0f : -1.0f;

        // Incremental world-space rotation (computed below)
        Eigen::Quaternionf deltaQ = Eigen::Quaternionf::Identity();

        if (rotationMode_ == RotationMode::Arcball) {
            // Arcball rotation: map previous and current mouse positions to the unit sphere
            Eigen::Vector3f v0 = ArcballVector(prevX, prevY, viewportW, viewportH, centerX, centerY);
            Eigen::Vector3f v1 = ArcballVector(currX, currY, viewportW, viewportH, centerX, centerY);

            Eigen::Vector3f axis = v0.cross(v1);
            float axisLen = axis.norm();
            if (axisLen > 1e-6f) {
                axis /= axisLen;
                float dot = std::clamp(v0.dot(v1), -1.0f, 1.0f);
                // Direction controlled by invertRotation_; angle scaled by rotateSpeed_
                float angle = orientationSign * std::acos(dot) * rotateSpeed_;
                deltaQ = Eigen::Quaternionf(Eigen::AngleAxisf(angle, axis));
                rotation_ = deltaQ * rotation_;
                rotation_.normalize();
            }
        } else {
            // Free turntable-like rotation (not locked): horizontal drags => yaw around world-up (Y),
            // vertical drags => pitch around camera-right (X). Vertical sign inverted to match user expectation.
            float deltaX = currX - prevX;
            float deltaY = currY - prevY;

            float fracX = deltaX / viewportW;
            float fracY = deltaY / viewportH;

            // Map full-width drag -> 360deg yaw; full-height drag -> 180deg pitch
            // Horizontal sign respects the invert flag; vertical mapping is fixed so only
            // left/right are inverted when the setting is toggled.
            float angleYaw = orientationSign * (2.0f * static_cast<float>(M_PI)) * fracX * rotateSpeed_;
            float anglePitch = -static_cast<float>(M_PI) * fracY * rotateSpeed_;

            // Yaw about world Z (blue axis)
            Eigen::Quaternionf qYaw(Eigen::AngleAxisf(angleYaw, Eigen::Vector3f::UnitZ()));
            Eigen::Quaternionf rotYawed = qYaw * rotation_;

            // Pitch about camera's right axis after yaw
            Eigen::Vector3f rightAfterYaw = rotYawed * Eigen::Vector3f::UnitX();
            Eigen::Quaternionf qPitch(Eigen::AngleAxisf(anglePitch, rightAfterYaw));

            deltaQ = qPitch * qYaw;
            rotation_ = deltaQ * rotation_;
            rotation_.normalize();
        }

        // When orbiting around a custom pivot (e.g. selected object center),
        // also rotate target_ around that pivot so the camera orbits it.
        if (hasCustomPivot_) {
            target_ = orbitPivot_ + deltaQ * (target_ - orbitPivot_);
            // Update distance since target moved
            Eigen::Vector3f eye = target_ + rotation_ * Eigen::Vector3f(0, 0, distance_);
            distance_ = (eye - target_).norm();
        }
    }
    if (rightDrag) {
        // Pan: move target in camera's XY plane using pixel delta
        float deltaX = currX - prevX;
        float deltaY = currY - prevY;
        Eigen::Vector3f right = rotation_ * Eigen::Vector3f::UnitX();
        Eigen::Vector3f up    = rotation_ * Eigen::Vector3f::UnitY();
        float scale = distance_ * panSpeed_;
        target_ -= right * deltaX * scale;
        target_ += up    * deltaY * scale;
    }
}

void CameraController::OnScroll(float delta) {
    float factor = 1.0f - delta * zoomSpeed_;
    factor = std::clamp(factor, 0.1f, 10.0f);
    distance_ *= factor;
    distance_ = std::max(distance_, 0.001f);
}

bool CameraController::Update() {
    if (!anim_.active) return false;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - anim_.startTime).count();
    float t = std::clamp(elapsed / anim_.duration, 0.0f, 1.0f);

    // Smooth-step
    float s = t * t * (3.0f - 2.0f * t);

    distance_ = anim_.startDistance + s * (anim_.targetDistance - anim_.startDistance);
    target_   = anim_.startTarget   + s * (anim_.endTarget - anim_.startTarget);

    if (t >= 1.0f)
        anim_.active = false;

    return true;
}

void CameraController::ZoomToNodes(const std::vector<uint16_t>& nodeIds,
                                    const std::unordered_map<uint16_t, int>& nodeToDataId,
                                    const Renderer& renderer) {
    Eigen::Vector3d bmin( std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max());
    Eigen::Vector3d bmax(-std::numeric_limits<double>::max(),
                         -std::numeric_limits<double>::max(),
                         -std::numeric_limits<double>::max());
    bool found = false;

    for (uint16_t nid : nodeIds) {
        auto it = nodeToDataId.find(nid);
        if (it == nodeToDataId.end()) continue;
        const GpuMesh* mesh = renderer.GetMesh(it->second);
        if (!mesh || !mesh->sourceMesh || mesh->sourceMesh->vertices.empty()) continue;
        found = true;
        // Use precomputed world-space bbox
        bmin = bmin.cwiseMin(mesh->bboxMin.cast<double>());
        bmax = bmax.cwiseMax(mesh->bboxMax.cast<double>());
    }
    if (!found) return;

    ZoomToFit(bmin, bmax);
}

void CameraController::ZoomToFit(const Eigen::Vector3d& bboxMin, const Eigen::Vector3d& bboxMax) {
    Eigen::Vector3d center = (bboxMin + bboxMax) * 0.5;
    double radius = (bboxMax - bboxMin).norm() * 0.5;
    if (radius < 1e-6) radius = 1.0;

    float fovRad = fovDeg_ * static_cast<float>(M_PI) / 180.0f;
    float targetDist = static_cast<float>(radius / std::sin(fovRad * 0.5));

    anim_.startDistance = distance_;
    anim_.targetDistance = targetDist;
    anim_.startTarget = target_;
    anim_.endTarget = center.cast<float>();
    anim_.active = true;
    anim_.startTime = std::chrono::steady_clock::now();
}

void CameraController::SetRotationMode(RotationMode mode)
{
    rotationMode_ = mode;
}

CameraController::RotationMode CameraController::GetRotationMode() const
{
    return rotationMode_;
}

void CameraController::SetInvertRotation(bool invert)
{
    invertRotation_ = invert;
}

bool CameraController::GetInvertRotation() const
{
    return invertRotation_;
}

Eigen::Matrix4f CameraController::GetViewMatrix() const {
    // Camera position = target + rotation * (0, 0, distance)
    Eigen::Vector3f forward = rotation_ * Eigen::Vector3f(0, 0, distance_);
    Eigen::Vector3f eye = target_ + forward;
    Eigen::Vector3f up  = rotation_ * Eigen::Vector3f::UnitY();

    // lookAt
    Eigen::Vector3f f = (target_ - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view(0, 0) =  s.x(); view(0, 1) =  s.y(); view(0, 2) =  s.z();
    view(1, 0) =  u.x(); view(1, 1) =  u.y(); view(1, 2) =  u.z();
    view(2, 0) = -f.x(); view(2, 1) = -f.y(); view(2, 2) = -f.z();
    view(0, 3) = -s.dot(eye);
    view(1, 3) = -u.dot(eye);
    view(2, 3) =  f.dot(eye);

    return view;
}

Eigen::Vector3f CameraController::GetEyePosition() const {
    return target_ + rotation_ * Eigen::Vector3f(0, 0, distance_);
}

Eigen::Matrix4f CameraController::GetProjectionMatrix(float aspectRatio, bool homogeneousDepth) const {
    float fovRad = fovDeg_ * static_cast<float>(M_PI) / 180.0f;
    float tanHalf = std::tan(fovRad * 0.5f);

    // Adapt near/far planes to current camera distance so geometry is never clipped
    float near = std::max(distance_ * 0.001f, 0.01f);
    float far  = std::max(distance_ * 100.0f, farPlane_);

    Eigen::Matrix4f proj = Eigen::Matrix4f::Zero();
    proj(0, 0) = 1.0f / (aspectRatio * tanHalf);
    proj(1, 1) = 1.0f / tanHalf;
    proj(3, 2) = -1.0f;

    if (homogeneousDepth) {
        // OpenGL / Vulkan: NDC z in [-1, 1]
        proj(2, 2) = -(far + near) / (far - near);
        proj(2, 3) = -(2.0f * far * near) / (far - near);
    } else {
        // Direct3D: NDC z in [0, 1]
        proj(2, 2) = -far / (far - near);
        proj(2, 3) = -(far * near) / (far - near);
    }

    return proj;
}

Eigen::Vector3f CameraController::ArcballVector(float x, float y, float w, float h) const {
    // Default mapping around viewport center
    float centerX = w * 0.5f;
    float centerY = h * 0.5f;
    // Map pixel coordinates to [-1, 1] relative to center
    Eigen::Vector3f v(
        (2.0f * (x - centerX)) / w,
        (2.0f * (centerY - y)) / h,
        0.0f
    );
    float d = v.x() * v.x() + v.y() * v.y();
    if (d <= 1.0f)
        v.z() = std::sqrt(1.0f - d);
    else
        v.normalize();
    return v;
}

Eigen::Vector3f CameraController::ArcballVector(float x, float y, float w, float h, float centerX, float centerY) const {
    // Map pixel coordinates to [-1, 1] relative to provided center
    Eigen::Vector3f v(
        (2.0f * (x - centerX)) / w,
        (2.0f * (centerY - y)) / h,
        0.0f
    );
    float d = v.x() * v.x() + v.y() * v.y();
    if (d <= 1.0f)
        v.z() = std::sqrt(1.0f - d);
    else
        v.normalize();
    return v;
}

void CameraController::SetRotateSpeed(float speed)
{
    rotateSpeed_ = speed;
}

void CameraController::SetTarget(const Eigen::Vector3f& target)
{
    target_ = target;
}

void CameraController::SetTargetKeepEye(const Eigen::Vector3f& target)
{
    // Preserve the current eye position while changing the orbit center.
    Eigen::Vector3f eye = GetEyePosition();

    Eigen::Vector3f offset = eye - target;
    float d = offset.norm();
    if (d < 1e-7f) {
        target_ = target;
        return;
    }

    distance_ = d;
    target_ = target;

    // Reconstruct rotation from the offset direction using pure yaw/pitch
    // (matching the turntable convention) so no roll is introduced.
    // The turntable builds: rotation = qPitch(around right) * qYaw(around Z).
    // With identity rotation the backward dir is (0,0,1).
    // After yaw θ around Z and pitch φ around the resulting right axis:
    //   backward = (sin(θ)*sin(φ), -cos(θ)*sin(φ), cos(φ))
    // Solving: φ = atan2(‖xy‖, z),  θ = atan2(x, -y)
    Eigen::Vector3f dir = offset / d;
    float xyLen = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
    float pitch = std::atan2(xyLen, dir.z());
    float yaw = std::atan2(dir.x(), -dir.y());

    Eigen::Quaternionf qYaw(Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitZ()));
    Eigen::Vector3f rightAxis = qYaw * Eigen::Vector3f::UnitX();
    Eigen::Quaternionf qPitch(Eigen::AngleAxisf(pitch, rightAxis));
    rotation_ = (qPitch * qYaw).normalized();
}

void CameraController::SetOrbitPivot(const Eigen::Vector3f& pivot)
{
    orbitPivot_ = pivot;
    hasCustomPivot_ = true;
}

} // namespace Debugger3DS::Rendering
