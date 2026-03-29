#include "CameraController.h"
#include <algorithm>

namespace Debugger3DS {

void CameraController::ZoomToNodes(const std::vector<uint16_t>& nodeIds,
                                    const std::unordered_map<uint16_t, int>& nodeToDataId) {
    // Combine vertices and faces from all matching nodes
    int totalRows = 0, totalFaces = 0;
    for (uint16_t nid : nodeIds) {
        auto it = nodeToDataId.find(nid);
        if (it == nodeToDataId.end()) continue;
        const auto& V = viewer_.data(it->second).V;
        const auto& F = viewer_.data(it->second).F;
        if (V.rows() == 0) continue;
        totalRows += V.rows();
        totalFaces += F.rows();
    }
    if (totalRows == 0) return;

    Eigen::MatrixXd allV(totalRows, 3);
    Eigen::MatrixXi allF(totalFaces, 3);
    int vOffset = 0, fOffset = 0;
    for (uint16_t nid : nodeIds) {
        auto it = nodeToDataId.find(nid);
        if (it == nodeToDataId.end()) continue;
        const auto& V = viewer_.data(it->second).V;
        const auto& F = viewer_.data(it->second).F;
        if (V.rows() == 0) continue;
        allV.middleRows(vOffset, V.rows()) = V;
        allF.middleRows(fOffset, F.rows()) = F.array() + vOffset;
        vOffset += V.rows();
        fOffset += F.rows();
    }

    float targetZoom;
    Eigen::Vector3f targetShift;
    viewer_.core().get_scale_and_shift_to_fit_mesh(allV, allF, targetZoom, targetShift);

    // Reset user pan/zoom so only base values drive the camera
    anim_.startZoom = viewer_.core().camera_base_zoom * viewer_.core().camera_zoom;
    anim_.startTranslation = viewer_.core().camera_base_translation + viewer_.core().camera_translation;
    viewer_.core().camera_zoom = 1.0f;
    viewer_.core().camera_translation.setZero();

    anim_.targetZoom = targetZoom;
    anim_.targetTranslation = targetShift;
    anim_.active = true;
    anim_.startTime = std::chrono::steady_clock::now();
}

bool CameraController::Update() {
    if (!anim_.active) return false;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - anim_.startTime).count();
    float t = std::clamp(elapsed / anim_.duration, 0.0f, 1.0f);

    // Smooth-step interpolation
    float s = t * t * (3.0f - 2.0f * t);

    viewer_.core().camera_base_zoom =
        anim_.startZoom + s * (anim_.targetZoom - anim_.startZoom);
    viewer_.core().camera_base_translation =
        anim_.startTranslation + s * (anim_.targetTranslation - anim_.startTranslation);

    if (t >= 1.0f)
        anim_.active = false;

    return false;
}

} // namespace Debugger3DS
