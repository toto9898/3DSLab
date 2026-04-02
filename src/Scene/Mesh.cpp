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
    
    void Mesh::ToEigenMatrices(Eigen::MatrixXd& V, Eigen::MatrixXi& F, const Eigen::Matrix4f& transform, bool invertWinding) const {
        Eigen::Matrix4d transformD = transform.cast<double>();
        
        V.resize(vertices.size(), 3);
        for (size_t i = 0; i < vertices.size(); ++i) {
            Eigen::Vector4d v4(vertices[i].x(), vertices[i].y(), vertices[i].z(), 1.0);
            v4 = transformD * v4;
            V.row(i) = v4.head<3>().transpose();
        }
        
        const auto& src = invertWinding ? GetInvertedWindingFaces() : faces;
        F.resize(src.size(), 3);
        for (size_t i = 0; i < src.size(); ++i) {
            F.row(i) << src[i].a, src[i].b, src[i].c;
        }
    }
    
    const std::vector<Face>& Mesh::GetInvertedWindingFaces() const {
        if (invertedWindingFaces_.empty() && !faces.empty()) {
            invertedWindingFaces_.reserve(faces.size());
            for (const auto& f : faces) {
                Face inv;
                inv.a = f.a;
                inv.b = f.c;
                inv.c = f.b;
                inv.flags = f.flags;
                inv.material = f.material;
                invertedWindingFaces_.push_back(inv);
            }
        }
        return invertedWindingFaces_;
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