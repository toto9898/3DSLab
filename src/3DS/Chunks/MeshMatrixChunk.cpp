#include "MeshMatrixChunk.h"
#include "../Importer.h"
#include <iostream>

namespace Debugger3DS {

    bool MeshMatrixChunk::ReadData(Importer& importer) {
        // Get reference to current mesh
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Read 4x3 matrix (12 floats) - 3DS stores as column vectors: [X-axis Y-axis Z-axis Position]
        Eigen::Matrix4f meshMatrix = Eigen::Matrix4f::Identity();
        for (int col = 0; col < 4; ++col) {  // Read columns (X, Y, Z axes + position)
            for (int row = 0; row < 3; ++row) {  // Each column has 3 components
                float value;
                if (!ReadFloat(value)) {
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
        
        if (targetMesh_->name == "Arch27_012" || targetMesh_->name == "Arch27_014" || targetMesh_->name == "Arch27_015"){
            std::cout << "Mesh Matrix for " << targetMesh_->name << ":\n" << meshMatrix << std::endl;
        }

        // Check if matrix is invertible before applying inverse
        float determinant = meshMatrix.determinant();
        
        if (std::abs(determinant) > 0.0001f) {
            // Apply inverse to transform from local mesh space to world space
            targetMesh_->ApplyTransform(meshMatrix.inverse());
        }
        return true;
    }

} // namespace Debugger3DS