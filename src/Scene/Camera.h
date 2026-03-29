#pragma once

#include "NamedObject.h"
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Camera object
    struct Camera : public NamedObject {
        using NamedObject::NamedObject; // Inherit constructors

        Eigen::Vector3f position;
        Eigen::Vector3f target;
        float bankAngle = 0.0f;
        float focalLength = 50.0f;
    };
    
} // namespace Debugger3DS