#pragma once

#include "NamedObject.h"
#include <Eigen/Dense>

namespace Debugger3DS::Scene {

/// @brief Parsed light source from an N_DIRECT_LIGHT chunk.
struct Light : public NamedObject {
    ///@cond INHERIT
    using NamedObject::NamedObject;
    ///@endcond

    Eigen::Vector3f position;                                  ///< World-space position.
    Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f); ///< Light colour (RGB, 0–1).
    bool isSpotlight  = false;                                 ///< @c true if this is a spotlight.
    Eigen::Vector3f target;                                    ///< Spotlight target point (spotlights only).
    float hotspotAngle = 45.0f;                                ///< Inner (full-intensity) cone half-angle in degrees.
    float falloffAngle = 60.0f;                                ///< Outer (zero-intensity) cone half-angle in degrees.
    bool isOn = true;                                          ///< @c false if the light has been turned off (DL_OFF).

    float innerRange  = 0.0f;  ///< Distance at which attenuation begins (DL_INNER_RANGE).
    float outerRange  = 0.0f;  ///< Distance at which the light reaches zero (DL_OUTER_RANGE).
    float multiplier  = 1.0f;  ///< Intensity multiplier (DL_MULTIPLIER).
};

} // namespace Debugger3DS::Scene