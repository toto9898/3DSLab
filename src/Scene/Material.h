#pragma once

#include <string>
#include <Eigen/Dense>

namespace Debugger3DS::Scene {

/// @brief PBR-style surface material parsed from the 3DS MATERIAL chunk.
struct Material {
    std::string name;                                                      ///< Material name.
    Eigen::Vector3f ambient  = Eigen::Vector3f(0.2f, 0.2f, 0.2f);        ///< Ambient RGB colour.
    Eigen::Vector3f diffuse  = Eigen::Vector3f(0.8f, 0.8f, 0.8f);        ///< Diffuse RGB colour.
    Eigen::Vector3f specular = Eigen::Vector3f(0.0f, 0.0f, 0.0f);        ///< Specular RGB colour.
    float shininess          = 0.0f;   ///< Phong shininess exponent.
    float shininessPercent   = 0.0f;   ///< Raw shininess as a 0–100 percentage (from INT/FLOAT_PERCENTAGE).
    float transparency       = 0.0f;   ///< Opacity: 0 = fully opaque, 1 = fully transparent.
    float transparencyFalloff = 0.0f;  ///< Transparency falloff amount.
    float reflectionBlur     = 0.0f;   ///< Reflection blur amount.
    float selfIllumination   = 0.0f;   ///< Self-illumination factor.
    float wireSize           = 1.0f;   ///< Wire-frame line width.
    uint16_t shadingType     = 0;      ///< Shading model: 0=Flat, 1=Gouraud, 2=Phong, 3=Metal.
    std::string textureMap;            ///< Filename of the diffuse texture map (empty if none).

    Material() = default;
    /// @param materialName Initial material name.
    Material(const std::string& materialName) : name(materialName) {}
};

} // namespace Debugger3DS::Scene