#include "MeshSelector.h"
#include "Renderer.h"
#include "Mesh.h"
#include "CameraController.h"
#include "Window.h"
#include <imgui.h>
#include <iostream>
#include <cmath>
#include <limits>

namespace Debugger3DS {

void MeshSelector::Init(Renderer& renderer, CameraController& camera) {
    renderer_ = &renderer;
    camera_ = &camera;
}

void MeshSelector::AddMesh(int meshId, std::any userData, uint32_t color, const std::string& name, const AABB& bbox) {
    meshIds_.push_back(meshId);
    userData_.push_back(std::move(userData));
    meshNames_.push_back(name.empty() ? "Mesh " + std::to_string(meshId) : name);
    meshBBoxes_.push_back(bbox);
    originalColors_.push_back(color);
}

void MeshSelector::AddMeshWithTransform(int meshId, std::any userData, uint32_t color, const std::string& name,
                                         const Eigen::Vector3f& bboxMin, const Eigen::Vector3f& bboxMax,
                                         const Eigen::Matrix4f& transform) {
    Eigen::Vector3f corners[8] = {
        bboxMin,
        {bboxMax.x(), bboxMin.y(), bboxMin.z()},
        {bboxMin.x(), bboxMax.y(), bboxMin.z()},
        {bboxMax.x(), bboxMax.y(), bboxMin.z()},
        {bboxMin.x(), bboxMin.y(), bboxMax.z()},
        {bboxMax.x(), bboxMin.y(), bboxMax.z()},
        {bboxMin.x(), bboxMax.y(), bboxMax.z()},
        bboxMax
    };

    Eigen::Vector3d tMin(std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());
    Eigen::Vector3d tMax(std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::lowest());

    for (int c = 0; c < 8; ++c) {
        Eigen::Vector4f h(corners[c].x(), corners[c].y(), corners[c].z(), 1.0f);
        Eigen::Vector4f t = transform * h;
        Eigen::Vector3d tc = t.head<3>().cast<double>();
        tMin = tMin.cwiseMin(tc);
        tMax = tMax.cwiseMax(tc);
    }

    AABB bbox(tMin, tMax);
    AddMesh(meshId, std::move(userData), color, name, bbox);
}

std::string MeshSelector::GetMeshName(int index) const {
    if (index >= 0 && index < static_cast<int>(meshNames_.size()))
        return meshNames_[index];
    return "";
}

void MeshSelector::SetSelectionCallback(std::function<void(const std::any&)> callback) {
    selectionCallback_ = std::move(callback);
}

void MeshSelector::ProcessInput(const InputState& input, uint16_t viewportW, uint16_t viewportH) {
    // Keyboard: N/P/C
    if (input.lastKey == GLFW_KEY_N && !input.keyConsumed && !ImGui::GetIO().WantCaptureKeyboard) {
        if (!meshIds_.empty()) {
            currentIndex_ = (currentIndex_ + 1) % static_cast<int>(meshIds_.size());
            selectedMeshId_ = meshIds_[currentIndex_];
            selectedUserData_ = userData_[currentIndex_];
            additionalSelectedIds_.clear();
            std::cout << "Selected: " << meshNames_[currentIndex_] << " (ID: " << selectedMeshId_ << ")" << std::endl;
            HighlightSelected();
            if (selectionCallback_) selectionCallback_(selectedUserData_);
        }
    }
    if (input.lastKey == GLFW_KEY_P && !input.keyConsumed && !ImGui::GetIO().WantCaptureKeyboard) {
        if (!meshIds_.empty()) {
            currentIndex_ = (currentIndex_ - 1 + static_cast<int>(meshIds_.size())) % static_cast<int>(meshIds_.size());
            selectedMeshId_ = meshIds_[currentIndex_];
            selectedUserData_ = userData_[currentIndex_];
            additionalSelectedIds_.clear();
            std::cout << "Selected: " << meshNames_[currentIndex_] << " (ID: " << selectedMeshId_ << ")" << std::endl;
            HighlightSelected();
            if (selectionCallback_) selectionCallback_(selectedUserData_);
        }
    }
    if (input.lastKey == GLFW_KEY_C && !input.keyConsumed && !ImGui::GetIO().WantCaptureKeyboard) {
        ClearSelection();
        std::cout << "Selection cleared" << std::endl;
    }

    // Mouse click detection
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (input.mouseJustPressed[0]) {
        isDragging_ = false;
        mouseDownX_ = input.mouseX;
        mouseDownY_ = input.mouseY;
    }

    // Detect drag
    if (input.mouseButtons[0]) {
        double dx = input.mouseX - mouseDownX_;
        double dy = input.mouseY - mouseDownY_;
        if (std::sqrt(dx * dx + dy * dy) > 5.0)
            isDragging_ = true;
    }

    if (input.mouseJustReleased[0] && !isDragging_) {
        int hitMeshId = FindMeshUnderCursor(input.mouseX, input.mouseY, viewportW, viewportH);
        if (hitMeshId >= 0) {
            SelectMesh(hitMeshId);
        } else if (selectedMeshId_ >= 0) {
            ClearSelection();
            std::cout << "Selection cleared (clicked empty space)" << std::endl;
        }
    }
}

void MeshSelector::SelectMesh(int meshId) {
    auto it = std::find(meshIds_.begin(), meshIds_.end(), meshId);
    if (it == meshIds_.end()) return;

    int index = static_cast<int>(std::distance(meshIds_.begin(), it));

    if (selectedMeshId_ == meshId && additionalSelectedIds_.empty()) {
        std::cout << "Deselected: " << meshNames_[index] << std::endl;
        ClearSelection();
    } else {
        additionalSelectedIds_.clear();
        currentIndex_ = index;
        selectedMeshId_ = meshId;
        selectedUserData_ = userData_[index];
        std::cout << "Selected: " << meshNames_[currentIndex_] << " (ID: " << selectedMeshId_ << ")" << std::endl;
        HighlightSelected();
        if (selectionCallback_) selectionCallback_(selectedUserData_);
    }
}

void MeshSelector::SelectMeshes(const std::vector<int>& meshIds, bool fireCallback) {
    // Reset all meshes first
    for (size_t i = 0; i < meshIds_.size(); ++i) {
        renderer_->RestoreMesh(meshIds_[i]);
    }
    additionalSelectedIds_.clear();

    if (meshIds.empty()) {
        ClearSelection();
        return;
    }

    selectedMeshId_ = meshIds[0];
    auto it = std::find(meshIds_.begin(), meshIds_.end(), meshIds[0]);
    if (it != meshIds_.end()) {
        currentIndex_ = static_cast<int>(std::distance(meshIds_.begin(), it));
        selectedUserData_ = userData_[currentIndex_];
    }

    for (size_t i = 1; i < meshIds.size(); ++i)
        additionalSelectedIds_.push_back(meshIds[i]);

    HighlightSelected();
    if (fireCallback && selectionCallback_) selectionCallback_(selectedUserData_);
}

void MeshSelector::HighlightSelected() {
    if (!renderer_ || selectedMeshId_ < 0 || meshIds_.empty()) return;

    // Make all non-selected meshes semi-transparent with their original color
    for (size_t i = 0; i < meshIds_.size(); ++i) {
        renderer_->RestoreMesh(meshIds_[i]);
        uint32_t color = originalColors_[i];
        // Set alpha to ~50% (replace high byte with 0x80)
        uint32_t halfAlpha = (color & 0x00FFFFFFu) | (0x80u << 24);
        renderer_->SetMeshColor(meshIds_[i], halfAlpha);
        renderer_->SetMeshTransparent(meshIds_[i], true);
    }

    // Highlight selected with yellow (fully opaque)
    uint32_t yellow = Renderer::PackColor(1.0f, 1.0f, 0.0f);
    renderer_->SetMeshColor(selectedMeshId_, yellow);
    renderer_->SetMeshTransparent(selectedMeshId_, false);
    for (int id : additionalSelectedIds_) {
        renderer_->SetMeshColor(id, yellow);
        renderer_->SetMeshTransparent(id, false);
    }
}

void MeshSelector::Reset() {
    ClearSelection();
    meshIds_.clear();
    userData_.clear();
    meshNames_.clear();
    meshBBoxes_.clear();
    originalColors_.clear();
}

void MeshSelector::ClearSelection() {
    if (renderer_) {
        for (size_t i = 0; i < meshIds_.size(); ++i) {
            renderer_->RestoreMesh(meshIds_[i]);
            renderer_->SetMeshTransparent(meshIds_[i], false);
        }
    }
    additionalSelectedIds_.clear();
    selectedUserData_.reset();
    selectedMeshId_ = -1;
    currentIndex_ = -1;
    if (selectionCallback_) selectionCallback_(std::any{});
}

Eigen::Vector3f MeshSelector::GetSelectionCenter() const {
    bool any = false;
    Eigen::Vector3d minPt(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    Eigen::Vector3d maxPt(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest());

    auto addBBox = [&](int meshId) {
        auto it = std::find(meshIds_.begin(), meshIds_.end(), meshId);
        if (it == meshIds_.end()) return;
        int idx = static_cast<int>(std::distance(meshIds_.begin(), it));
        const AABB& b = meshBBoxes_[idx];
        minPt = minPt.cwiseMin(b.min);
        maxPt = maxPt.cwiseMax(b.max);
        any = true;
    };

    if (selectedMeshId_ >= 0) addBBox(selectedMeshId_);
    for (int id : additionalSelectedIds_) addBBox(id);

    if (!any) return Eigen::Vector3f::Zero();
    Eigen::Vector3d center = (minPt + maxPt) * 0.5;
    return center.cast<float>();
}

bool MeshSelector::HasSelection() const {
    return selectedMeshId_ >= 0 || !additionalSelectedIds_.empty();
}

int MeshSelector::FindMeshUnderCursor(double mouseX, double mouseY, uint16_t viewportW, uint16_t viewportH) {
    if (!renderer_ || !camera_) return -1;

    float aspect = static_cast<float>(viewportW) / static_cast<float>(viewportH);
    bool hmgDepth = renderer_->IsHomogeneousDepth();
    Eigen::Matrix4f view = camera_->GetViewMatrix();
    Eigen::Matrix4f proj = camera_->GetProjectionMatrix(aspect, hmgDepth);

    // Compute ray in world space
    Eigen::Vector3f pos1, pos2;
    // Flip Y for screen coords (GLFW y=0 at top, NDC y=-1 at bottom)
    double flippedY = viewportH - mouseY;
    Unproject(mouseX, flippedY, 0.0, view, proj, viewportW, viewportH, hmgDepth, pos1);
    Unproject(mouseX, flippedY, 1.0, view, proj, viewportW, viewportH, hmgDepth, pos2);

    Eigen::Vector3d rayOrigin = pos1.cast<double>();
    Eigen::Vector3d rayDir = (pos2 - pos1).normalized().cast<double>();

    int closestMeshId = -1;
    double minDepth = std::numeric_limits<double>::max();

    for (size_t i = 0; i < meshIds_.size(); ++i) {
        int meshId = meshIds_[i];
        const GpuMesh* mesh = renderer_->GetMesh(meshId);
        if (!mesh || !mesh->sourceMesh || mesh->sourceMesh->vertices.empty()) continue;

        // Transform ray into local (model) space
        Eigen::Matrix4d invModel = mesh->modelMatrix.cast<double>().inverse();
        Eigen::Vector4d localOrigin4 = invModel * Eigen::Vector4d(rayOrigin.x(), rayOrigin.y(), rayOrigin.z(), 1.0);
        Eigen::Vector3d localOrigin = localOrigin4.head<3>();
        Eigen::Vector3d localDir = (invModel.block<3, 3>(0, 0) * rayDir).normalized();

        const auto& verts = mesh->sourceMesh->vertices;
        const auto& indices = mesh->invertedWinding
            ? mesh->sourceMesh->GetInvertedWindingIndices()
            : mesh->sourceMesh->faceIndices;
        int nFaces = static_cast<int>(indices.size()) / 3;
        for (int f = 0; f < nFaces; ++f) {
            int i0 = indices[3 * f];
            int i1 = indices[3 * f + 1];
            int i2 = indices[3 * f + 2];
            Eigen::Vector3d v0 = verts[i0].cast<double>();
            Eigen::Vector3d v1 = verts[i1].cast<double>();
            Eigen::Vector3d v2 = verts[i2].cast<double>();

            double t, u, v;
            if (RayTriangleIntersect(localOrigin, localDir, v0, v1, v2, t, u, v)) {
                // Transform hit distance to world space for correct depth comparison
                Eigen::Vector3d localHit = localOrigin + t * localDir;
                Eigen::Vector4d worldHit4 = mesh->modelMatrix.cast<double>() * Eigen::Vector4d(localHit.x(), localHit.y(), localHit.z(), 1.0);
                double worldT = (worldHit4.head<3>() - rayOrigin).dot(rayDir);
                if (worldT > 0 && worldT < minDepth) {
                    minDepth = worldT;
                    closestMeshId = meshId;
                }
            }
        }
    }

    return closestMeshId;
}

void MeshSelector::Unproject(double screenX, double screenY, double depth,
                              const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj,
                              uint16_t viewportW, uint16_t viewportH,
                              bool homogeneousDepth,
                              Eigen::Vector3f& worldPos) {
    // NDC coordinates
    float ndcX = static_cast<float>((2.0 * screenX / viewportW) - 1.0);
    float ndcY = static_cast<float>((2.0 * screenY / viewportH) - 1.0);
    // Homogeneous depth: NDC z in [-1,1], otherwise [0,1]
    float ndcZ = homogeneousDepth
        ? static_cast<float>(2.0 * depth - 1.0)
        : static_cast<float>(depth);

    Eigen::Matrix4f invVP = (proj * view).inverse();
    Eigen::Vector4f ndc(ndcX, ndcY, ndcZ, 1.0f);
    Eigen::Vector4f world = invVP * ndc;

    worldPos = world.head<3>() / world.w();
}

bool MeshSelector::RayTriangleIntersect(const Eigen::Vector3d& origin, const Eigen::Vector3d& dir,
                                         const Eigen::Vector3d& v0, const Eigen::Vector3d& v1,
                                         const Eigen::Vector3d& v2,
                                         double& t, double& u, double& v) {
    constexpr double EPSILON = 1e-8;
    Eigen::Vector3d e1 = v1 - v0;
    Eigen::Vector3d e2 = v2 - v0;
    Eigen::Vector3d h = dir.cross(e2);
    double a = e1.dot(h);

    if (a > -EPSILON && a < EPSILON) return false;

    double f = 1.0 / a;
    Eigen::Vector3d s = origin - v0;
    u = f * s.dot(h);
    if (u < 0.0 || u > 1.0) return false;

    Eigen::Vector3d q = s.cross(e1);
    v = f * dir.dot(q);
    if (v < 0.0 || u + v > 1.0) return false;

    t = f * e2.dot(q);
    return t > EPSILON;
}

} // namespace Debugger3DS
