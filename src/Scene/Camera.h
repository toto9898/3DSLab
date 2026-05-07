#pragma once

#include "NamedObject.h"
#include <Eigen/Dense>

namespace Debugger3DS::Scene {

/// @brief Parsed camera from an N_CAMERA chunk.
struct Camera : public NamedObject {
    ///@cond INHERIT
    using NamedObject::NamedObject;
    ///@endcond

    Eigen::Vector3f position;          ///< World-space camera position.
    Eigen::Vector3f target;            ///< World-space look-at target.
    float bankAngle   = 0.0f;         ///< Roll/bank angle in degrees.
    float focalLength = 50.0f;        ///< Focal length in millimetres.

    float nearRange = 0.0f;           ///< Near clip distance (CAM_RANGES chunk).
    float farRange  = 0.0f;           ///< Far clip distance (CAM_RANGES chunk).
};

} // namespace Debugger3DS::Scene