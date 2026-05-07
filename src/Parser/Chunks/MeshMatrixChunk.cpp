#include "MeshMatrixChunk.h"
#include "Importer.h"
#include <iostream>

namespace Debugger3DS::Parser::Chunks {

using namespace Debugger3DS::Scene;
using namespace Debugger3DS::Parser;

    bool MeshMatrixChunk::ReadData(Importer& importer) {
        // Get reference to current mesh
        targetMesh_ = importer.GetCurrentMesh();
        if (!targetMesh_) {
            return false;
        }
        
        // Bulk read 4x3 matrix (12 floats) - 3DS stores as column vectors: [X-axis Y-axis Z-axis Position]
        float raw[12];
        stream_.read(reinterpret_cast<char*>(raw), sizeof(raw));
        if (!stream_.good() || stream_.tellg() > dataEndPos_) {
            return false;
        }
        
        // Fill Eigen Matrix4f (column-major, 4 rows per column, but 3DS only stores 3 rows)
        Eigen::Matrix4f& meshMatrix = targetMesh_->meshMatrix;
        for (int col = 0; col < 4; ++col) {
            meshMatrix(0, col) = raw[col * 3];
            meshMatrix(1, col) = raw[col * 3 + 1];
            meshMatrix(2, col) = raw[col * 3 + 2];
        }
        // Fourth row is always [0 0 0 1] for affine transformations
        meshMatrix(3, 0) = 0.0f;
        meshMatrix(3, 1) = 0.0f;
        meshMatrix(3, 2) = 0.0f;
        meshMatrix(3, 3) = 1.0f;
        
        return true;
    }

} // namespace Debugger3DS::Parser::Chunks