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
    
    void Mesh::ToEigenMatrices(Eigen::MatrixXd& V, Eigen::MatrixXi& F, const Eigen::Matrix4f& transform) const {
        Eigen::Matrix4d transformD = transform.cast<double>();
        
        V.resize(vertices.size(), 3);
        for (size_t i = 0; i < vertices.size(); ++i) {
            Eigen::Vector4d v4(vertices[i].x(), vertices[i].y(), vertices[i].z(), 1.0);
            v4 = transformD * v4;
            V.row(i) = v4.head<3>().transpose();
        }
        
        F.resize(faces.size(), 3);
        for (size_t i = 0; i < faces.size(); ++i) {
            F.row(i) << faces[i].a, faces[i].b, faces[i].c;
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