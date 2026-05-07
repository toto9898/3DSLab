#include "Mesh.h"

namespace Debugger3DS::Scene {
    
    const std::vector<uint16_t>& Mesh::GetInvertedWindingIndices() const {
        if (invertedWindingIndices_.empty() && !faceIndices.empty()) {
            invertedWindingIndices_ = faceIndices;
            for (size_t i = 0; i < invertedWindingIndices_.size(); i += 3)
                std::swap(invertedWindingIndices_[i + 1], invertedWindingIndices_[i + 2]);
        }
        return invertedWindingIndices_;
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
    
} // namespace Debugger3DS::Scene