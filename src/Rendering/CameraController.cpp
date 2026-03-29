#include "CameraController.h"
#include "Renderer.h"
#include <algorithm>
#include <cmath>

namespace Debugger3DS {

void CameraController::Init(float fovDeg, float near, float far) {
    fovDeg_ = fovDeg;
    nearPlane_ = near;
    farPlane_ = far;
}

void CameraController::OnMouseDrag(float deltaX, float deltaY,
                                    bool leftDrag, bool rightDrag,
                                    float viewportW, float viewportH) {
    if (leftDrag) {
        // Trackball rotation
        float angle = std::sqrt(deltaX * deltaX + deltaY * deltaY) * rotateSpeed_;
        if (angle > 1e-6f) {
            // Rotation axis is perpendicular to mouse motion in camera space
            // Camera right = rotation * X, camera up = rotation * Y
            Eigen::Vector3f right = rotation_ * Eigen::Vector3f::UnitX();
            Eigen::Vector3f up    = rotation_ * Eigen::Vector3f::UnitY();
            Eigen::Vector3f axis  = (-deltaX * up - deltaY * right).normalized();
            Eigen::Quaternionf rot(Eigen::AngleAxisf(angle, axis));
            rotation_ = rot * rotation_;
            rotation_.normalize();
        }
    }
    if (rightDrag) {
        // Pan: move target in camera's XY plane
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
        if (!mesh || mesh->V.rows() == 0) continue;
        found = true;
        for (int r = 0; r < mesh->V.rows(); ++r) {
            bmin = bmin.cwiseMin(mesh->V.row(r).transpose());
            bmax = bmax.cwiseMax(mesh->V.row(r).transpose());
        }
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

    Eigen::Matrix4f proj = Eigen::Matrix4f::Zero();
    proj(0, 0) = 1.0f / (aspectRatio * tanHalf);
    proj(1, 1) = 1.0f / tanHalf;
    proj(3, 2) = -1.0f;

    if (homogeneousDepth) {
        // OpenGL / Vulkan: NDC z in [-1, 1]
        proj(2, 2) = -(farPlane_ + nearPlane_) / (farPlane_ - nearPlane_);
        proj(2, 3) = -(2.0f * farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
    } else {
        // Direct3D: NDC z in [0, 1]
        proj(2, 2) = -farPlane_ / (farPlane_ - nearPlane_);
        proj(2, 3) = -(farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
    }

    return proj;
}

Eigen::Vector3f CameraController::ArcballVector(float x, float y, float w, float h) const {
    // Map pixel coordinates to [-1, 1]
    Eigen::Vector3f v(
        (2.0f * x - w) / w,
        (h - 2.0f * y) / h,
        0.0f
    );
    float d = v.x() * v.x() + v.y() * v.y();
    if (d <= 1.0f)
        v.z() = std::sqrt(1.0f - d);
    else
        v.normalize();
    return v;
}

} // namespace Debugger3DS
