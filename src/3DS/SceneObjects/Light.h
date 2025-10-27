#pragma once

#include "NamedObject.h"
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Light object
    struct Light : public NamedObject {
        using NamedObject::NamedObject; // Inherit constructors

        Eigen::Vector3f position;
        Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
        bool isSpotlight = false;
        Eigen::Vector3f target; // For spotlights
        float hotspotAngle = 45.0f;
        float falloffAngle = 60.0f;
        bool isOn = true;
    };
    
} // namespace Debugger3DS