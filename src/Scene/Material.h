#pragma once

#include <string>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Material information
    struct Material {
        std::string name;
        Eigen::Vector3f ambient = Eigen::Vector3f(0.2f, 0.2f, 0.2f);
        Eigen::Vector3f diffuse = Eigen::Vector3f(0.8f, 0.8f, 0.8f);
        Eigen::Vector3f specular = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
        float shininess = 0.0f;
        float shininessPercent = 0.0f;
        float transparency = 0.0f;
        float transparencyFalloff = 0.0f;
        float reflectionBlur = 0.0f;
        float selfIllumination = 0.0f;
        float wireSize = 1.0f;
        uint16_t shadingType = 0;  // 0=Flat, 1=Gouraud, 2=Phong, 3=Metal
        std::string textureMap;
        
        Material() = default;
        Material(const std::string& materialName) : name(materialName) {}
    };
    
} // namespace Debugger3DS