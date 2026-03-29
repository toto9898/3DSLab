#pragma once

#include "NamedObject.h"
#include <vector>
#include <map>
#include <memory>
#include <Eigen/Dense>

namespace Debugger3DS {
    
    // Forward declarations
    struct Material;
    
    // Face with material information
    struct Face {
        uint16_t a = 0, b = 0, c = 0;   // Vertex indices
        uint16_t flags = 0;             // Edge visibility flags
        std::shared_ptr<Material> material = nullptr;  // Material reference
    };

    // Mesh object
    class Mesh : public NamedObject {
    public:
        std::vector<Eigen::Vector3f> vertices;
        std::vector<Face> faces;
        std::vector<Eigen::Vector2f> texCoords;
        std::vector<uint32_t> smoothingGroups;
        Eigen::Matrix4f meshMatrix = Eigen::Matrix4f::Identity();


        // Material assignments with shared pointers
        std::map<std::shared_ptr<Material>, std::vector<uint16_t>> materialGroups;

        Mesh() : NamedObject("Unnamed Mesh") {}
        Mesh(const std::string& meshName) : NamedObject(meshName) {}
        
        // Convert to Eigen matrices for libigl
        void ToEigenMatrices(Eigen::MatrixXd& V, Eigen::MatrixXi& F) const;
        
        // Convert to Eigen matrices with a transform applied (non-destructive)
        void ToEigenMatrices(Eigen::MatrixXd& V, Eigen::MatrixXi& F, const Eigen::Matrix4f& transform) const;
        
        // Get bounding box
        std::pair<Eigen::Vector3f, Eigen::Vector3f> GetBoundingBox() const;
        
        // Smoothing group methods
        void SetSmoothingGroups(const std::vector<uint32_t>& groups) { smoothingGroups = groups; }
        const std::vector<uint32_t>& GetSmoothingGroups() const { return smoothingGroups; }
        size_t GetFaceCount() const { return faces.size(); }
    };
    
} // namespace Debugger3DS