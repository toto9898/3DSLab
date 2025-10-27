#include "Mesh.h"

namespace Debugger3DS {
    
    void Mesh::ToEigenMatrices(Eigen::MatrixXd& V, Eigen::MatrixXi& F) const {
        // Convert vertices
        V.resize(vertices.size(), 3);
        for (size_t i = 0; i < vertices.size(); ++i) {
            V.row(i) = vertices[i].cast<double>();
        }
        
        // Convert faces
        F.resize(faces.size(), 3);
        for (size_t i = 0; i < faces.size(); ++i) {
            F.row(i) << faces[i].a, faces[i].b, faces[i].c;
        }
    }
    
    void Mesh::ApplyTransform(const Eigen::Matrix4f& transform) {
        for (auto& vertex : vertices) {
            Eigen::Vector4f v4(vertex.x(), vertex.y(), vertex.z(), 1.0f);
            v4 = transform * v4;
            vertex = v4.head<3>();
        }
    }
    
    std::pair<Eigen::Vector3f, Eigen::Vector3f> Mesh::GetBoundingBox() const {
        if (vertices.empty()) {
            return {Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()};
        }
        
        Eigen::Vector3f minBounds = vertices[0];
        Eigen::Vector3f maxBounds = vertices[0];
        
        for (const auto& vertex : vertices) {
            minBounds = minBounds.cwiseMin(vertex);
            maxBounds = maxBounds.cwiseMax(vertex);
        }
        
        return {minBounds, maxBounds};
    }
    
} // namespace Debugger3DS