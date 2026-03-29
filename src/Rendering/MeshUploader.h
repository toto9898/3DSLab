#pragma once

#include <igl/opengl/glfw/Viewer.h>
#include "Scene.h"
#include "MeshSelector.h"
#include "ObjectNode.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace Debugger3DS {

class MeshUploader {
public:
    struct MeshEntry {
        Eigen::MatrixXd V;
        Eigen::MatrixXi F;
        ObjectNodePtr node;       // null when falling back to raw meshes
        std::string meshName;
    };

    // Prepare mesh data from scene (with transforms applied non-destructively)
    static std::vector<MeshEntry> GetMeshesToRender(const Scene& scene);

    // Upload meshes to the viewer, register them with the selector, and populate nodeToDataId.
    static void UploadMeshes(igl::opengl::glfw::Viewer& viewer,
                             const Scene& scene,
                             MeshSelector& selector,
                             std::unordered_map<uint16_t, int>& nodeToDataId,
                             std::function<void(const std::any&)> selectionCallback);

    // Draw coordinate axes overlay on the first data slot.
    static void DrawCoordinateAxes(igl::opengl::glfw::Viewer& viewer, double axisLength = 10.0);
};

} // namespace Debugger3DS
