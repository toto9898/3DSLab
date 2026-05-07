#pragma once

#include "NamedObject.h"
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Light object
    struct Light : public NamedObject {
        using NamedObject::NamedObject; // Inherit constructors

        Eigen::Vector3f position;
        Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
        bool isSpotlight  = false;
        Eigen::Vector3f target; // For spotlights
        float hotspotAngle = 45.0f;
        float falloffAngle = 60.0f;
        bool isOn = true;

        // Attenuation (from DL_INNER_RANGE / DL_OUTER_RANGE)
        float innerRange  = 0.0f;  // distance at which light starts to attenuate
        float outerRange  = 0.0f;  // distance at which light reaches zero
        // Intensity multiplier (from DL_MULTIPLIER)
        float multiplier  = 1.0f;
    };
    
} // namespace Debugger3DS