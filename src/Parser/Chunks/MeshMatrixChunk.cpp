#include "MeshMatrixChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS {

    bool MeshMatrixChunk::ReadData(Importer& importer) {
        // Get reference to current mesh
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Read 4x3 matrix (12 floats) - 3DS stores as column vectors: [X-axis Y-axis Z-axis Position]
        Eigen::Matrix4f& meshMatrix = targetMesh_->meshMatrix;
        for (int col = 0; col < 4; ++col) {  // Read columns (X, Y, Z axes + position)
            for (int row = 0; row < 3; ++row) {  // Each column has 3 components
                float value;
                if (!Read(value)) {
                    return false;
                }
                meshMatrix(row, col) = value;
            }
        }
        // Fourth row is always [0 0 0 1] for affine transformations
        meshMatrix(3, 0) = 0.0f;
        meshMatrix(3, 1) = 0.0f;
        meshMatrix(3, 2) = 0.0f;
        meshMatrix(3, 3) = 1.0f;
        
        return true;
    }

} // namespace Debugger3DS