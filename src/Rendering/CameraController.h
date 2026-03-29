#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include <Eigen/Dense>
#include <chrono>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace Debugger3DS {

class CameraController {
public:
    explicit CameraController(igl::opengl::glfw::Viewer& viewer)
        : viewer_(viewer) {}

    // Start a smooth camera animation to fit the given node IDs.
    // nodeToDataId maps nodeId → viewer data_id.
    void ZoomToNodes(const std::vector<uint16_t>& nodeIds,
                     const std::unordered_map<uint16_t, int>& nodeToDataId);

    // Should be called every frame (e.g. in callback_pre_draw).
    // Returns false (to avoid consuming the event).
    bool Update();

private:
    struct Animation {
        bool active = false;
        float startZoom = 1.0f;
        float targetZoom = 1.0f;
        Eigen::Vector3f startTranslation = Eigen::Vector3f::Zero();
        Eigen::Vector3f targetTranslation = Eigen::Vector3f::Zero();
        float duration = 0.4f; // seconds
        std::chrono::steady_clock::time_point startTime;
    };

    igl::opengl::glfw::Viewer& viewer_;
    Animation anim_;
};

} // namespace Debugger3DS
